#include <iostream>
#include <ctime>
#include <map>
#include <set>
#include <utility>
#include <stack>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <string>

#include <ginac/ginac.h>
#include <piranha/piranha.hpp>

using namespace GiNaC;

typedef std::vector<std::pair<int,int>> _graph;
//using p_type = polynomial<integer,monomial<int>>;
//    p_type x{"x"},y{"y"},z{"z"};
//    std::cout<< x*y*z + (x-z)*(x+y) << '\n';
//
void initial_processing (std::vector<std::string>& all_lines, std::vector<std::string>& periods, std::vector<std::string>& unprocessed_edges){

    for(auto graph_info : all_lines){
        std::string delim = ":=";

        auto pos = graph_info.find(delim);
        std::string period = graph_info.substr(0,pos);
        graph_info.erase(0,delim.length() + period.length() + 2);
        std::cout << period << '\n';

        auto second_pos = graph_info.find(']');
        std::string edges = graph_info.substr(0,second_pos);
        std::cout << edges << '\n';

        periods.push_back(period);
        unprocessed_edges.push_back(edges);
    }

}

void populate_graphs(std::vector<std::string>& unprocessed_edges, std::vector<_graph>& graphs,
        std::map<_graph,std::set<int>>& vertex_set){
    for(auto edges : unprocessed_edges){

        std::vector<std::pair<int,int>> edges_for_graph;
        std::set<int> vertices;
        int first_edge;
        int second_edge;
        bool is_first = true;

        for(int i = 0; i < edges.length(); i++){
            if (isdigit(edges[i])){
                if(is_first){
                    first_edge = int(edges[i]) - '0';
                    is_first = false;
                    vertices.insert(first_edge);
                }
                else{
                    is_first = true;
                    second_edge = int(edges[i]) - '0';
                    vertices.insert(second_edge);
                    auto edge_pair = std::make_pair(first_edge,second_edge);
                    edges_for_graph.push_back(edge_pair);

                  //  std::cout << first_edge << std::endl;
                }
            }
        }
        graphs.push_back(edges_for_graph);
        vertex_set.insert(std::pair<_graph, std::set<int>> (edges_for_graph, vertices));
    }
}

void get_incidence_matrices(std::vector<std::vector<std::vector<ex>>>& incidence_matrices,
        std::vector<_graph>& graphs, std::map<_graph,std::set<int>>& vertex_set){
    for(auto graph : graphs){
        int column_length = vertex_set[graph].size();
        std::vector<std::vector<ex>> incidence_matrix;
        for(auto edges : graph){
            std::vector<ex> edge_column;
            for (int i = 0; i < column_length - 1; i++){

                int to_pushback = 0;
                //std::cout << edges.first << ' ' << edges.second << '\n';
                
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
        incidence_matrices.push_back(incidence_matrix);

//        for(auto col : incidence_matrix){
//            for(auto ent : col){
//                std::cout<< ent << ' ';
//            }
//            std::cout<< ' ' << std::endl;
//        }
    }
}

matrix
compute_kirchoff_matrix(std::vector<std::vector<ex>>& inc_matrix){

    int num_edge = inc_matrix.size();
    int num_vertex = inc_matrix[0].size();

    std::vector<std::vector<ex>>  kirchoff_matrix;

    for(int i = 0; i < num_edge ; i++){
        std::vector<ex> kirchoff_row;

        std::string edge_var = "a_" + std::to_string(i);
        symbol edgesym(edge_var);

        for(int j = 0; j < num_edge;j++){
            if(j == i){
                kirchoff_row.push_back(edgesym);
            }

            else{
                kirchoff_row.push_back(0);
            }
        }
        kirchoff_row.insert(kirchoff_row.end(),inc_matrix[i].begin(),inc_matrix[i].end());

        for(auto thing : kirchoff_row){
            std::cout << thing << "  ";
        }
        std::cout << std::endl;

        kirchoff_matrix.push_back(kirchoff_row);
    }

    for(int i = 0; i < num_vertex; i++){
        std::vector<ex> kirchoff_row;
        for(int j = 0; j < num_edge; j++){
            kirchoff_row.push_back(inc_matrix[j][i]);
        }

        for(int k = 0; k < num_vertex; k++){
            kirchoff_row.push_back(0);
        }

        for(auto thing : kirchoff_row){
            std::cout << thing << "  ";
        }
        std::cout << std::endl;

        kirchoff_matrix.push_back(kirchoff_row);
    }

    lst list_mat;
    for(auto vect : kirchoff_matrix){
        for(auto item : vect){
            list_mat.append(item);
        }
    }

    return matrix(num_edge + num_vertex, num_edge + num_vertex, list_mat);
}

int main(int argc, char** argv)
{
    piranha::init();
    std::cout << argv[0] << '\n';

    if(argc == 1){
        printf("needs an argument\n");
        return 0;
    }

    std::vector<_graph> graphs;
    std::vector<std::vector<std::vector<ex>>> incidence_matrices;
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
            std::cout << line << '\n';
        }
        graph_file.close();
    }

    initial_processing(all_lines,periods,unprocessed_edges); 
    populate_graphs(unprocessed_edges, graphs, vertex_set); 
    get_incidence_matrices(incidence_matrices,graphs,vertex_set);

    auto kirchoff_mat = compute_kirchoff_matrix(incidence_matrices[0]);
    std::cout<< kirchoff_mat << "\n";
    clock_t begin = clock();
    determinant(kirchoff_mat, determinant_algo::laplace);
    clock_t end = clock();
    std::cout << "LAPLACE " << (end-begin)/ CLOCKS_PER_SEC << '\n';

    //begin = clock();
    //determinant(kirchoff_mat, determinant_algo::divfree);
    //end = clock();
    //std::cout << "DIVFREE " << (end-begin)/ CLOCKS_PER_SEC << '\n';

    begin = clock();
    determinant(kirchoff_mat, determinant_algo::bareiss);
    end = clock();
    std::cout << "BAREISS  " << (end-begin)/ CLOCKS_PER_SEC << '\n';

}


