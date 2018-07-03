#include <iostream>
#include <iterator>
#include <algorithm>
#include <ctime>
#include <map>
#include <set>
#include <utility>
#include <stack> 
#include <stdexcept>
#include <vector>
#include <fstream>
#include <giac/config.h>
#include <giac/gen.h>
#include <giac/unary.h>

#include <giac/giac.h>

typedef std::vector<std::pair<int,int>> _graph;
std::map<int,std::string> var_edge_map;
std::set<int> emptyset;

using namespace giac;
//using p_type = polynomial<integer,monomial<int>>;
//    p_type x{"x"},y{"y"},z{"z"};
//    std::cout<< x*y*z + (x-z)*(x+y) << '\n';
//

bool vector_contains (std::vector<int> vect,int k){
    for(int thing : vect){
        if(k == thing){
            return true;
        }
    }

    return false;
    
}

void initial_processing (std::vector<std::string>& all_lines, std::vector<std::string>& periods, std::vector<std::string>& unprocessed_edges){

    for(auto graph_info : all_lines){
        std::string delim = ":=";

        auto pos = graph_info.find(delim);
        std::string period = graph_info.substr(0,pos);
        graph_info.erase(0,delim.length() + period.length() + 2);
        //std::cout << period << '\n';

        auto second_pos = graph_info.find(']');
        std::string edges = graph_info.substr(0,second_pos);
        //std::cout << edges << '\n';

        periods.push_back(period);
        unprocessed_edges.push_back(edges);
    }

}

void reduced_mod_p (gen& poly, int p){
    if(poly.type != _POLY){
        std::cerr << "reduced mod p not type poly \n";
        exit(0);
    }

    auto polyptr = poly._POLYptr;
    for (auto& monomial : polyptr -> coord){
       monomial.value = monomial.value % p; 
    }
}

gen find_coeff_p (gen& poly, int p, vecteur exps){
    if(poly.type != _POLY){
        std::cerr << "need to be sparse poly type \n";
        exit(0);
    }

    index_t ind;
    for(auto var : exps){
        ind.push_back(p-1);
    }

    index_m i(ind);

    auto polyptr = poly._POLYptr;
    for (auto& monomial : polyptr -> coord){
        if(monomial.index == i){
            std::cout << monomial.index << '\n';
            return monomial.value;
        }
    }

    return 0;
}

//We construct the graph data structures for all graphs in the graphs file.
void populate_graphs(std::vector<std::string>& unprocessed_edges, std::vector<_graph>& graphs,
        std::map<_graph,std::set<int>>& vertex_set, bool decomplete = false){
    for(auto edges : unprocessed_edges){

        _graph  edges_for_graph;
        std::set<int> vertices;
        int first_vertex;
        int second_vertex;
        bool is_first = true;

        for(int i = 0; i < edges.length(); i++){
            std::string number;
            bool digit_found = false;
            while(isdigit(edges[i])){
                number.append(std::string(1,edges[i]));
                i++;
                digit_found = true;
            }
            if(digit_found){
                if(is_first){
                    first_vertex= stoi(number);
                    is_first = false;
                    vertices.insert(first_vertex);
                }
                else{
                    is_first = true;
                    second_vertex = stoi(number);

                    vertices.insert(second_vertex);
                    auto edge_pair = std::make_pair(first_vertex,second_vertex);
                    edges_for_graph.push_back(edge_pair);

                  //  std::cout << first_edge << std::endl;
                }

                digit_found = false;
            }
        }
        //Decomplete the graph by removing a vertex and all associated edges.
        graphs.push_back(edges_for_graph);
        vertex_set.insert(std::pair<_graph, std::set<int>> (edges_for_graph, vertices));

    }
}

std::set<std::set<int>> get_triangles(const _graph& graph){
    std::set<std::set<int>> toret;
    std::map<int,std::set<int>> neighbourmap;

    for(auto edges : graph){
        if(neighbourmap.find(edges.first) == neighbourmap.end() && 
                neighbourmap.find(edges.second) == neighbourmap.end()){
            neighbourmap.insert({edges.first,{edges.second}});
            neighbourmap.insert({edges.second,{edges.first}});
        }

        else{
            neighbourmap[edges.first].insert(edges.second);
            neighbourmap[edges.second].insert(edges.first);
        }
    }

    for(auto pair : neighbourmap){
        for(int neighbour : pair.second){
            for(int neighbour2 : pair.second){

                if(neighbour == neighbour2){
                    continue;
                }

                if (neighbourmap[neighbour].find(neighbour2) != neighbourmap[neighbour].end()){
                    toret.insert(std::set<int>({neighbour2,neighbour,pair.first}));
                }

            }
        }
    }


    return toret;
}

void scan_triangles_and_decomplete(const std::vector<_graph>& graphs,
        std::vector<bool>& has_triangles, std::vector<int>& vertices_to_decomplete){
    int notricount = 0;
    int tricount = 0;
    for(auto& graph : graphs){
        std::set<std::set<int>> triangles;
        triangles = get_triangles(graph);
//        std::cout << triangles.size() << periods[tricount + notricount];

        has_triangles.push_back(!triangles.empty());

        //All decompleted vertices just end up begin "2" anyways...
        if(!triangles.empty()){
            auto it = (triangles.begin()->begin());
            //Take the second vertex, seems to be more effective for some reason.
            //std::advance(it,1);
            int vertex = *(++it);
            vertices_to_decomplete.push_back(vertex);
//            std::cout << vertex << '\n';
            tricount++;
        }
        else{
            //We use a random vertex to decomplete, choose 2.
            vertices_to_decomplete.push_back(2);
            notricount++;
        }
    }

//    std::cout << "NUM TRIANGLE GRAPHS " << tricount << '\n';
//    std::cout << "NUM NO_TRIANGLE GRAPHS " << notricount << '\n';

}


std::vector<_graph> get_incidence_matrices(std::vector<std::vector<std::vector<int>>>& incidence_matrices,
        std::vector<_graph>& graphs, std::map<_graph,std::set<int>>& vertex_set, std::vector<int>& vertices_to_decomplete){

    std::vector<_graph> toret;
    int i = 0;
    for(auto graph : graphs){
        _graph toins;
        int decomplete_vertex = vertices_to_decomplete[i];
        std::vector<std::vector<int>> incidence_matrix;
        int column_length = vertex_set[graph].size();
        for(auto edges : graph){
            std::vector<int> edge_column;
            //Decomplete here
            if(edges.first == decomplete_vertex || 
                    edges.second == decomplete_vertex){
                continue;
            }

            toins.push_back(edges);
           
            if(column_length == decomplete_vertex){
                //std::cout << "choose different vertex to delete\n";
//                exit(1);
            }

            for (int i = 0; i < column_length-1; i++){

                int to_pushback = 0;
                //std::cout << edges.first << ' ' << edges.second << '\n';
                //First column correponds to vertex 1, etc
                if(i == decomplete_vertex - 1){
                   continue;
                } 

                //0 row corresponds to edge 1, etc
                if(i == (edges.first - 1)){
                    to_pushback = -1;
                }

                else if (i == (edges.second - 1)){
                    to_pushback = 1;
                }

                edge_column.push_back(to_pushback);
            }
            incidence_matrix.push_back(edge_column);
        }
        toret.push_back(toins);
        incidence_matrices.push_back(incidence_matrix);
        i++;
    }

    return toret;
}

//expressions : empty vecteur to be populated with the correct edge variables.
//inc_matrix : the incidence matrices. the rows (first index) are the edges, the columns are the vertices
//I : set I with integer edges. 
//J : set J with integer edges. 
//K : set K with integer edges.
vecteur
compute_kirchoff_matrix(vecteur& expressions, const std::vector<std::vector<int>>& inc_matrix, const std::set<int>& I, 
        const std::set<int>& J, const std::set<int>& K,bool laplace = false){

    if(expressions.size() != 0){
        std::cout << "Have an empty expression before computing kirchoff matrix " << '\n';
        exit(0);
    }
    if(I.size() != J.size()){
        std::cout << " I != J size, exiting." << '\n';
        std::cout << I.size() << J.size() << '\n';
        exit(0);
    }

    int num_edge = inc_matrix.size();
    int num_vertex = inc_matrix[0].size();
    vecteur kirchoff_matrix;
    vecteur exps;
    vecteur exps_to_return;
    
    //populate the expressions vector for determining the poly-variables.
    for(int i = 0; i < num_edge; i++){

        gen edgesym;
        if(I.find(i) != I.end() || J.find(i) != J.end() || K.find(i) != K.end()){
            //Put a dummy vector so we can discern its type.
            exps.push_back(makevecteur(1));
        }

        else{
            std::string edge_var = "a_" + std::to_string(i);
            edgesym = gen(std::string(edge_var),giac::context0);
            exps.push_back(edgesym);
            exps_to_return.push_back(edgesym);

            //Bernard said to do this after parsing a variable.
            eval(edgesym,1,giac::context0);
            //std::cout << edgesym.type << "TYPE \n";
        }
    }

    expressions = exps_to_return;

    for(int i = 0; i < num_edge; i++){

        vecteur kirchoff_row;

        if(I.find(i) != I.end()){

//            for(int z = 0; z < num_edge + num_vertex; z++){
//                gen thisgen = 0;
//                kirchoff_row.push_back(0);
//            }
//
//            kirchoff_matrix.push_back(kirchoff_row);
            
            //std::cout << kirchoff_matrix << "KIRCH KAT \n";
            //std::cout << "I found " << i+1 << '\n';
            continue;
        }

        gen edgesym = exps[i];
        gen poly_edgesym;

        if(edgesym.type != _VECT){
            poly_edgesym = sym2r(edgesym,exps_to_return,giac::context0);
        }

        else{
            poly_edgesym = 0;
        }
            
        for(int j = 0; j < num_edge;j++){

            if(J.find(j) != J.end()){
//                kirchoff_row.push_back(0);
                //std::cout << "J found " << j+1 << '\n';
                continue;
            }

            if(j == i){
                if(K.find(j) != K.end()){
                    //std::cout << "K found " << j+1 << '\n';
                    kirchoff_row.push_back(0);
                }

                else{
                    //kirchoff_row.push_back(poly_edgesym);
                    if(laplace){
                        kirchoff_row.push_back(1);
                    }
                    else{
                        kirchoff_row.push_back(edgesym);
                    }
                }
            }

            else{
               //kirchoff_row[j] = sym2r(0,exps,giac::context0);
               //kirchoff_row.push_back(sym2r(0,exps,giac::context0));
               kirchoff_row.push_back(0);
            }
        }

        for(int k = num_edge; k < num_edge + num_vertex; k++){
            //kirchoff_row[k] = sym2r(inc_matrix[i][k - num_edge],exps,giac::context0);
            kirchoff_row.push_back(inc_matrix[i][k - num_edge]);
        }

//        for(auto thing : kirchoff_row){
//            std::cout << thing << ",";
//        }
//        std::cout << std::endl;

        //kirchoff_matrix[i] = kirchoff_row;
        kirchoff_matrix.push_back(kirchoff_row);

    }

    for(int i = 0; i < num_vertex; i++){
        giac::vecteur kirchoff_row;

        for(int j = 0; j < num_edge; j++){
            if(J.find(j) != J.end()){
                //kirchoff_row.push_back(0);
                //std::cout << "J found " << j+1 << '\n';
                continue;
            }
            //kirchoff_row[j] = sym2r(inc_matrix[j][i],exps,giac::context0);
            //kirchoff_row[j] = -inc_matrix[j][i]; //negative because the lower left incidence matrix is negative.
//            kirchoff_row.push_back(sym2r(-inc_matrix[j][i],exps,giac::context0));
            kirchoff_row.push_back(-inc_matrix[j][i]);
        }

        for(int k = num_edge; k < num_vertex + num_edge; k++){
            //kirchoff_row[k] = sym2r(0,exps,giac::context0);
            //kirchoff_row[k] = 0;
//            kirchoff_row.push_back(sym2r(0,exps,giac::context0));
            kirchoff_row.push_back(0);
        }

//        for(auto thing : kirchoff_row){
//            std::cout << thing << ",";
//        }
//        std::cout << std::endl;

        kirchoff_matrix.push_back(kirchoff_row);
    }
    
    return kirchoff_matrix;
}

std::vector<int> detect_edge_sequence(std::vector<std::vector<int>>& inc_matrix,
        std::set<int>& del_cont_edges, const vecteur& to_reduce,const std::set<std::set<int>>& triangles,
        _graph decompleted_graph, std::set<int>& edges_other_3valent){
    int num_edge= inc_matrix.size();
    int num_vertex = inc_matrix[0].size();

    std::vector<std::pair<int,std::vector<int>>> edge_sequences;
    std::vector<int> edge_sequence_good;

    for(int twice = 0; twice < 1; twice++){
        for(int i  = 0 ; i < num_vertex; i++){
            std::vector<int> edge_subseq;
            int connectivity = 0;
            for(int j = 0; j < num_edge ; j++){

//                if(twice == 0){
//                    if(I.find(j) != I.end()){
//                        continue;
//                    }
//                }
//
//                else{
//                    if(J.find(j) != J.end()){
//                        continue;
//                    }
//                }
                    

//                std::cout <<inc_matrix[j][i] << ' ';
                if(inc_matrix[j][i] != 0){

                    if(del_cont_edges.find(j) == del_cont_edges.end()){

                        int end1 = decompleted_graph[j].first;
                        int end2 = decompleted_graph[j].second;
                        bool is_triangle_edge = false;

//                        if(edges_other_3valent.find(j) != edges_other_3valent.end()){
//                            if(!vector_contains(edge_sequence_good,j)){
//                                edge_sequence_good.push_back(j);
//                            }
//                            std::cout << j << "other 3 valent\n" << '\n';
//                        }

                        for(auto& st : triangles){
                            if(st.find(end1) != st.end() &&
                                    st.find(end2) != st.end()){
                                is_triangle_edge = true;
                                std::cout << j << "edge triangle\n" << end1 << " " << end2 << '\n';

                                if(!vector_contains(edge_sequence_good,j)){
                                    edge_sequence_good.push_back(j);
                                }

                            }
                        }

                        if(!is_triangle_edge){
                            connectivity++;
                            edge_subseq.push_back(j);
                        }
                    }

                    else{
                        edge_subseq.push_back(-1);
                        connectivity= connectivity + 1;
                    }
                }
            }
            auto subseq_pair = std::make_pair(connectivity,edge_subseq);
            edge_sequences.push_back(subseq_pair);
//            std:: cout << '\n';
        }
    }

    std::sort(edge_sequences.begin(), edge_sequences.end());

    for(auto& pair : edge_sequences){
        std::cout << pair.second << "," << pair.first << '\n';
    }


    while(!edge_sequences.empty()){
        auto pair = edge_sequences[0];
        auto vect = pair.second;
        bool empty = true;
        
        for(int edge : vect){
            if(edge != -1){
                empty = false; 
                continue;
            }
        }

        if(empty){
            edge_sequences.erase(edge_sequences.begin());
            continue;
        }

        int good_edge = -1;
        for(int edge : vect){

            if(edge == -1){
                continue;
            }

            edge_sequence_good.push_back(edge);
            good_edge = edge;
            break;
        }

        std::cout << pair.first << " " << pair.second << '\n';
        for(auto& vectpair : edge_sequences){
            for(auto it = vectpair.second.begin(); it != vectpair.second.end(); ++it){
                if(*it == good_edge){
                    vectpair.first -= 1;
                    vectpair.second.erase(it);
                    break;
               }
            }
        }

        std::sort(edge_sequences.begin(), edge_sequences.end());
    }

    std::cout << "EDGE SEEQ GOOD " << edge_sequence_good << '\n';
    std::cout << "var seq good";

    std::ofstream elim_seq_file("elim_seq");
    vecteur elim_seq;

    for (auto& indet : to_reduce){
        elim_seq.push_back(indet);
    }

    for (auto edge : edge_sequence_good){
        bool dont_print = false;
        for(auto& indet : to_reduce){
            if("a_" + std::to_string(edge) == std::string(indet._IDNTptr->id_name)){
                dont_print =true;
           }
        }

        if(!dont_print){
            std::string name = "a_" + std::to_string(edge);
            elim_seq.push_back(giac::gen(name,giac::context0));
        }
    }

    std::cout << elim_seq << "\n";
    elim_seq_file << "elim_seq := " << elim_seq << ";\n";
    elim_seq_file.close();
    return edge_sequence_good;
}

int main(int argc, char** argv)
{
    std::cout << argv[0] << '\n';

    if(argc == 1){
        printf("needs an argument\n");
        return 0;
    }

    std::vector<_graph> graphs;
    std::vector<std::vector<std::vector<int>>> incidence_matrices;
    std::vector<std::string> periods;
    std::vector<std::string> unprocessed_edges;
    std::map<_graph, std::set<int>> vertex_set;
    std::vector<std::string> all_lines;
    std::string filename = argv[1];
    std::ifstream graph_file(filename);

    // Get each line of the periods file
    if(graph_file.is_open())
    {
        std::string line;
        while(std::getline(graph_file,line)){
            all_lines.push_back(line);
        }
        graph_file.close();
    }

    initial_processing(all_lines,periods,unprocessed_edges); 
    populate_graphs(unprocessed_edges, graphs, vertex_set); 

    std::vector<_graph> decompleted_graphs;
    std::vector<int> vertices_to_decomplete;
    std::vector<bool> has_triangles;
    
    scan_triangles_and_decomplete(graphs,has_triangles,vertices_to_decomplete);
    decompleted_graphs = get_incidence_matrices(incidence_matrices,graphs,vertex_set,vertices_to_decomplete);

    /*Testing things. This is where we operate only on one specific graph.
     * Change this to iterating over all graphs.
     */

    context ct;
    gen x(std::string("x"),&ct);

    //CONSTRUCTION OF D^6

    //First we find the edges to construct the 6 invariant with.
    //We want 2 sets of edges that are disjoint.

    int edgeindex = 0;

    std::ofstream graphcsv_file("gr.csv");
    for (auto edge : decompleted_graphs[0]){
        std::cout << edgeindex << " | " << edge.first << "," << edge.second << '\n';
        graphcsv_file << edge.first << "," << edge.second << '\n';
        edgeindex++;
    }
    graphcsv_file.close();

    std::set<std::set<int>> triangles_for_test;
    triangles_for_test = get_triangles(decompleted_graphs[0]);
   //TODO
   triangles_for_test = std::set<std::set<int>>();

    for(auto st : triangles_for_test){
        for(int vertex : st){
            std::cout << vertex << ',';
        }
        std::cout << '\n';
    }

    bool has_triangle = has_triangles[0];

    if(!has_triangle){
        std::cout << "doesn't have a triangle. need to make changes to algorithm before proceeding" << '\n';
        exit(1);
    }


    auto inc_matrix = incidence_matrices[0];
    int num_edge = inc_matrix.size();
    int num_vertex = inc_matrix[0].size();

    for(auto column : inc_matrix){
        for( auto element : column){
//            std::cout << element << ' ';
        }

//        std::cout << '\n';
    }

    //Get the 3- valent vertices and the corresponding edges.
    std::set<int> edges_other_3valent;
    std::set<int> del_cont_edges;
    std::vector<std::set<int>> all_edges_3valent;
    std::vector<int> edges_3_valent;
    int edge_joined_between = -1;
    bool joined_3valent = false;
    _graph decompleted_graph = decompleted_graphs[0];

    std::map<int,std::set<int>> vertex_edges;
    for(int i = 0; i < decompleted_graph.size(); i++){

        if(vertex_edges.find(decompleted_graph[i].first) == vertex_edges.end()){
            vertex_edges[decompleted_graph[i].first] = {i};
        }

        else{
            vertex_edges[decompleted_graph[i].first].insert(i);
        }

        if(vertex_edges.find(decompleted_graph[i].second) == vertex_edges.end()){
            vertex_edges[decompleted_graph[i].second] = {i};
        }

        else{
            vertex_edges[decompleted_graph[i].second].insert(i);
        }

    }

    for(auto pair : vertex_edges){
        if(pair.second.size() == 3){
           all_edges_3valent.push_back(pair.second); 
        }
    }

    std::cout << all_edges_3valent.size() << "# 3 valent vertices \n";
    if (has_triangle){

        int index1 = -1;
        int index2 = -1;
        int common_edge = -1;

        for(auto& set1 : all_edges_3valent){
            for(auto& set2 : all_edges_3valent){
                if(set1 == set2){
                    continue;
                }
                for(int edge1 : set1){
                    for(int edge2 : set2){
                        if(edge1 == edge2){
                            common_edge = edge1;
                            joined_3valent = true;
                            for(int i = 0; i < all_edges_3valent.size(); i++){

                                if(all_edges_3valent[i] == set1){
                                    index1 = i;
                                }

                                if(all_edges_3valent[i] == set2){
                                    index2 = i;
                                }
                            }
                        }
                    }
                }
            }
        }

        if(index1 == -1 || index2 == -1){
            std::cerr << "wrong indices";
        }

        for(int edge : all_edges_3valent[index1]){
            if(edge != common_edge){
                edges_3_valent.push_back(edge);
                del_cont_edges.insert(edge);
                std::cout<< edge << "set 1 6 inv\n";
            }
        }

        for(int edge : all_edges_3valent[index2]){
            if(edge != common_edge){
                edges_3_valent.push_back(edge);
                del_cont_edges.insert(edge);
                std::cout<< edge << "set 2 6 inv\n";
            }
        }

        for(int i = 0; i < all_edges_3valent.size(); i++){
            if(i != index1 && i != index2){

                if(edges_3_valent.size() == 6){
                    edges_other_3valent = all_edges_3valent[i];
                    std::cout << i << " other 3 valent \n";
                }

                for(int edge : all_edges_3valent[i]){
                    if(!vector_contains(edges_3_valent,edge)){

                        if(edges_3_valent.size() == 6){
                            continue;
                        }

                        edges_3_valent.push_back(edge);
                        del_cont_edges.insert(edge);
                        std::cout<< edge << "other set 6 inv\n";
                        
                    }
                }
            }
        }
    }

    for(auto thing : edges_other_3valent){
        std::cout << "other 3 valent ";
        std::cout << thing << '\n';
    }

    if(edges_3_valent.size() != 6){
        std::cout << "6 inv doesn't have 6 edges, error \n " << '\n';
    }

    vecteur dodgson1_mat;
    vecteur dodgson2_mat;
    gen dodgson1_poly;
    gen dodgson2_poly;
    gen fiveinv;

   std::set<int> I_1,I_2,J_1,J_2,K_1,K_2,I_1_sec,J_1_sec,K_1_sec;

   if(has_triangle && !joined_3valent){
       std::cerr << "ERROR, triangle found but 3 valent vertices are not joined\n" << '\n';
   }

   if(!has_triangle && joined_3valent){
       std::cerr << "ERROR, triangle not found but 3 valent vertices are joined\n" << '\n';
   }

   if(!joined_3valent){ 
        //Doing the math with the 2 3-valent vertices gets this as the 6 invariant... we label 
        //edges 1,2,6 for the first 3-valent vertex and 3,4,5 for the other.
        I_1 = {edges_3_valent[0],edges_3_valent[1]};
        J_1 = {edges_3_valent[3],edges_3_valent[4]};
        K_1 = {edges_3_valent[2],edges_3_valent[5]};

        std::cout << edges_3_valent[0] << edges_3_valent[1]  << edges_3_valent[2] <<   edges_3_valent[3] << edges_3_valent[4]<< edges_3_valent[5];

        I_2 = {edges_3_valent[0],edges_3_valent[3],edges_3_valent[5],edges_3_valent[2]};
        J_2 = {edges_3_valent[1],edges_3_valent[4],edges_3_valent[5],edges_3_valent[2]};

        vecteur exps_all;
        vecteur exps_topass;

        auto dodgson1 = compute_kirchoff_matrix(exps_topass,inc_matrix,I_1,J_1,K_1);
        auto dodgson2 = compute_kirchoff_matrix(exps_all,inc_matrix,I_2,J_2,K_2);

        dodgson1_poly = _det(dodgson1,giac::context0);
        dodgson2_poly = _det(dodgson2,giac::context0);
        
    }

   else{

        I_1 = {edges_3_valent[0],edges_3_valent[1],edges_3_valent[2]};
        J_1 = {edges_3_valent[2],edges_3_valent[5],edges_3_valent[4]};
        K_1 = {edge_joined_between,edges_3_valent[3]};

        std::cout << edges_3_valent[0] << edges_3_valent[1] << edges_3_valent[2] << edges_3_valent[3] << edges_3_valent[4]<< edges_3_valent[5];

        I_1_sec = {edges_3_valent[0],edges_3_valent[1],edges_3_valent[3]};
        J_1_sec = {edges_3_valent[3],edges_3_valent[5],edges_3_valent[4]};
        K_1_sec = {edge_joined_between,edges_3_valent[2]};

        I_2 = {edge_joined_between,edges_3_valent[0],edges_3_valent[3],edges_3_valent[5]};
        J_2 = {edge_joined_between,edges_3_valent[1],edges_3_valent[3],edges_3_valent[4]};
        K_2 = {edges_3_valent[2]};

        vecteur exps_all;
        vecteur exps_1;
        vecteur exps_topass;

        auto dodgson1 = compute_kirchoff_matrix(exps_topass,inc_matrix,I_1,J_1,K_1);
        auto dodgson2 = compute_kirchoff_matrix(exps_all,inc_matrix,I_1_sec,J_1_sec,K_1_sec);
        auto dodgson3 = compute_kirchoff_matrix(exps_1,inc_matrix,I_2,J_2,K_2);

        dodgson1_poly = _det(dodgson1,giac::context0) - _det(dodgson2,giac::context0);
        dodgson2_poly = _det(dodgson3,giac::context0);
   }

    vecteur exps_test;
    
    fiveinv = dodgson1_poly * dodgson2_poly;
    std::ofstream fiveinv_file("fiveinv");
    fiveinv_file << "fiveinv := " << fiveinv << ":" << '\n';
    fiveinv_file.close();


    std::cout << "factoring\n";

    auto factors1 = giac::_factors(dodgson1_poly,giac::context0);
    auto factors2 = giac::_factors(dodgson2_poly,giac::context0);
    vecteur to_reduce;
//    std::cout << factors1 << "done factoring\n" ;
//    std::cout << factors2 << "done factoring\n" ;
    std::set<std::string> all_indets1;
    std::set<std::string> all_indets2;
    int size_thresh = 3;


    for(int k = 1; k < size_thresh; k++){
        for(int i = 0; i < factors1._VECTptr->size(); i = i + 2){
            auto indets = giac::_lname((*(factors1._VECTptr))[i],giac::context0);
            for(auto& indet : *(indets._VECTptr)){
                all_indets1.insert(indet._IDNTptr->id_name);
            }
            std::cout << indets << '\n';
            if(indets._VECTptr->size() == k){
                for(auto& indet : *(indets._VECTptr)){
                    if (k == 1){
                        del_cont_edges.insert(std::stoi(std::string(indet._IDNTptr->id_name).substr(2)));
                        std::cout <<"DELCONT" << '\n';
                    }
                    to_reduce.push_back(indet);
                }
            }
            
        }
    }

    for(int i = 0; i < factors2._VECTptr->size(); i = i + 2){
        auto indets = giac::_lname((*(factors2._VECTptr))[i],giac::context0);
        std::cout << indets << '\n';

        for(auto& indet : *(indets._VECTptr)){
            all_indets2.insert(indet._IDNTptr->id_name);
        }

        if(indets._VECTptr->size() < size_thresh){
            for(auto& indet : *(indets._VECTptr)){
                bool contains_elt = false;

                for(auto& elt : to_reduce){
                    if(elt == indet){
                        contains_elt =true;
                    }
                }

                if(!contains_elt){
                    if(indets._VECTptr->size() == 1){
                        to_reduce.insert(to_reduce.begin(),indet);
                    }
                    else{
                        //a_xxxxxx
                        del_cont_edges.insert(std::stoi(std::string(indet._IDNTptr->id_name).substr(2)));
                        std::cout <<"DELCONT" << '\n';
                        to_reduce.push_back(indet);
                    }
                }
            }
        }
    }

    //Automatically reduce if one polynomial has no variable but the other does.
    for(auto id : all_indets1){
        if(all_indets2.find(id) == all_indets2.end()){
            std::cout << id << "DIFF \n";
            to_reduce.insert(to_reduce.begin(),giac::gen(id,giac::context0));
            std::cout <<"DELCONT" << '\n';
        }
    }

    for(auto id : all_indets2){
        if(all_indets1.find(id) == all_indets1.end()){
            std::cout << id << "DIFF \n";
            to_reduce.insert(to_reduce.begin(),giac::gen(id,giac::context0));
            del_cont_edges.insert(std::stoi(std::string(id).substr(2)));
            std::cout <<"DELCONT" << '\n';
        }
    }

    std::cout << to_reduce << " TO REDUCE \n";

    std::vector<int> edge_sequence = detect_edge_sequence(inc_matrix,del_cont_edges,to_reduce,
            triangles_for_test,decompleted_graphs[0],edges_other_3valent); 

    std::ofstream edges3valent_file("edges3valent");
    std::ofstream edgeseq_file("edgeseq");
    std::ofstream dodgson1_file("dodgson1");
    std::ofstream dodgson2_file("dodgson2");

    for(auto elt : edges_3_valent){
        edges3valent_file << elt << '\n';
    }

    for(auto elt : edge_sequence){
        edgeseq_file << elt << '\n';
    }

    for(int i = 0; i < dodgson1_mat.size(); i++){
        for (int j = 0; j < dodgson1_mat.size(); j++){
            if(j != dodgson1_mat.size()-1){
                dodgson1_file<< dodgson1_mat[i][j] << ',';
            }

            else{
                dodgson1_file<< dodgson1_mat[i][j];
            }
        }
        if(i != dodgson1_mat.size()){
            dodgson1_file<< '\n';
        }
    }
    
    for(int i = 0; i < dodgson2_mat.size(); i++){
        for (int j = 0; j < dodgson2_mat.size(); j++){
            if(j != dodgson2_mat.size()-1){
                dodgson2_file << dodgson2_mat[i][j] << ',';
            }

            else{
                dodgson2_file << dodgson2_mat[i][j];
            }
        }
        if(i != dodgson2_mat.size()){
            dodgson2_file << '\n';
        }
    }

    edges3valent_file.close();
    edgeseq_file.close();
    dodgson1_file.close();
    dodgson2_file.close();
       

    //I_1.insert(bridge);
    //J_1.insert(bridge);
    //K_2.insert(bridge);
    //
    vecteur exp5;
    vecteur exp6;
    vecteur exp7;
    vecteur exp8;

//    std::vector<int> edgesrand = {0,1,2,6,10,11};
////    std::vector<int> edgesrand = {2,8,9,15,16};
//    std::set<int> I_9 = {edgesrand[0],edgesrand[1]};
//    std::set<int> J_9 = {edgesrand[3],edgesrand[4]};
//    std::set<int> K_9 = {edgesrand[2],edgesrand[5]};
//
//    std::set<int> I_19 = {edgesrand[0],edgesrand[3]};
//    std::set<int> J_19 = {edgesrand[1],edgesrand[4]};
//    std::set<int> K_19 = {edgesrand[2]};
//
//    //std::cout << edgesrand[0] << edgesrand[1]  << edgesrand[3] << edgesrand[4];
//
//    std::set<int> I_8 = {edgesrand[0],edgesrand[3],edgesrand[2],edgesrand[5]};
//    std::set<int> J_8 = {edgesrand[1],edgesrand[4],edgesrand[2],edgesrand[5]};
//    std::set<int> K_8;
//
//    std::set<int> I_18 = {edgesrand[0],edgesrand[1],edgesrand[2]};
//    std::set<int> J_18 = {edgesrand[3],edgesrand[4],edgesrand[2]};
//    std::set<int> K_18;
//
//    vecteur poly_1 = compute_kirchoff_matrix(exp5,inc_matrix,I_1,J_1,K_1);
//    auto poly_2 = compute_kirchoff_matrix(exp6,inc_matrix,I_2,J_2,K_2);
//
//    auto dodgpoly1 = _det(poly_1,giac::context0);
//    auto dodgpoly2 = _det(poly_2,giac::context0);
//
//    auto fiveinv = dodgpoly1*dodgpoly2;
//


    exit(0);

    //std::cout << r2e(nth_inv, exps_all, giac::context0)<< '\n';

    std::set<int> primes = {2};

    for(int p : primes){

        clock_t begin = clock();

        vecteur old_symbols = makevecteur(gen(std::string("b0_0"),&ct),gen(std::string("b0_1"),&ct));
        vecteur new_symbols;
        std::map<std::string, gen> substitute_map;
        std::map<std::string, std::vector<std::set<int>>> IJK;

        std::vector<std::set<int>> to_insert1 = {I_1,J_1,K_1};
        std::vector<std::set<int>> to_insert2 = {I_2,J_2,K_2};

        IJK.insert(std::make_pair(std::string(old_symbols[0]._IDNTptr->id_name),to_insert1));
        IJK.insert(std::make_pair(std::string(old_symbols[1]._IDNTptr->id_name),to_insert2));

        //std::cout << IJK.size() << '\n';

        gen sixinv(1);
        for(int i = 0; i < p-1; i++){
            sixinv = sixinv * old_symbols[0] * old_symbols[1];
        }

        for(int i = 0; i < edge_sequence.size(); i++){
            new_symbols.clear();
            bool last = false;

            if(i == edge_sequence.size() -1){
                last = true;
            }
            for(int j = 0; j < old_symbols.size(); j++){
                std::string key = old_symbols[j]._IDNTptr -> id_name;
                std::string id = key.substr(key.find('_') + 1);
                int term = std::stoi(id);

                std::set<int> i_1 = IJK[key][0];
                std::set<int> j_1 = IJK[key][1];
                std::set<int> k_1 = IJK[key][2];
                std::set<int> i_2 = IJK[key][0];
                std::set<int> j_2 = IJK[key][1];
                std::set<int> k_2 = IJK[key][2];

                int edge = edge_sequence[i];
                i_2.insert(edge);
                j_2.insert(edge);
                k_1.insert(edge);

                bool term1_zero = true;
                bool term2_zero = true;

                vecteur exps1;
                vecteur exps2;

                auto dodgson1 = _det(compute_kirchoff_matrix(exps1,inc_matrix,i_1,j_1,k_1),&ct);

                std::string var1 = "b" + std::to_string(i+1) + "_" + std::to_string(term * 2+1);
                gen a_var = gen(var1,&ct);
                if(!is_zero(dodgson1,&ct)){
                    term1_zero = false;
                    new_symbols.push_back(a_var);

                    IJK.insert(std::make_pair(std::string(a_var._IDNTptr-> id_name), std::vector<std::set<int>>{i_1,j_1,k_1}));
                }


                auto dodgson2 = _det(compute_kirchoff_matrix(exps2,inc_matrix,i_2,j_2,k_2),&ct);
                std::string var2 = "b" + std::to_string(i+1) + "_" + std::to_string(term * 2);
                gen b_var = gen(var2,&ct);
                if(!is_zero(dodgson2,&ct)){
                    term2_zero = false;
                    new_symbols.push_back(b_var);

                    IJK.insert(std::make_pair(std::string(b_var._IDNTptr->id_name), std::vector<std::set<int>>{i_2,j_2,k_2}));
                }
                
                if(!term1_zero){
                    std::cerr << var1 << ',' << dodgson1 << '\n';
                }

                if(!term2_zero){
                    std::cerr << var2 << ',' << dodgson2 << '\n';
                }

                gen subvalue;
                
                if(!last){
                    if(term2_zero){
                        subvalue = a_var; 
                    }

                    else if (term1_zero){
                        subvalue = x * b_var;
                    }
                    
                    else{
                       subvalue = x*b_var + a_var; 
                    }
                }

                else{
                    if(term2_zero){
                        subvalue = dodgson1;
                    }

                    else{
                        subvalue = dodgson2;
                    }
                }


                substitute_map.insert(std::make_pair(key, subvalue));

                if(term1_zero && term2_zero){
                    std::cerr << "both are zero\n";
                    exit(0);
                }
            }

            vecteur tosub;
            for(int k = 0; k < old_symbols.size(); k++){
                std::string identifier = std::string(old_symbols[k]._IDNTptr -> id_name);
                tosub.push_back(substitute_map[identifier]);

            }

            gen subcmd = makesequence(sixinv,old_symbols,tosub);
            gen poly_in_x = _subst(subcmd,&ct);

            gen coeffcmd = makesequence(poly_in_x,x,p-1);

            if(!last){
                poly_in_x = _coeff(coeffcmd,&ct);
            }

            
            sixinv = poly_in_x;
            std::cerr << "<" << sixinv << "ITERATION  " << i << '\n';
            old_symbols = new_symbols;
        }

        clock_t end = clock();

        std::cout << (end-begin)/(double)CLOCKS_PER_SEC << " seconds taken. coeff for p = " << p << ": " << sixinv << ", c_2 is "
            << sixinv % p << '\n';
    }
}


