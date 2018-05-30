#include <iostream>
#include <fstream>
#include <ctime>
#include <unordered_map>
#include <set>
#include <utility>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <string>

#include <ginac/ginac.h>
#include <piranha/piranha.hpp>
using namespace piranha;

using p_type = polynomial<integer,k_monomial>;

typedef std::pair<std::string,std::pair<int,int>> edge;

//    p_type x{"x"},y{"y"},z{"z"};
//    std::cout<< x*y*z + (x-z)*(x+y) << '\n';

struct _graph
{
    int V,E;
    std::vector<edge> edges;

};

int numspan = 0;
// To represent Disjoint Sets
struct DisjointSets
{
    std::vector<int> parent;
    std::vector<int> rnk;
    int n;
 
    // Constructor.
    DisjointSets(int n)
    {
        // Allocate memory
        this->n = n;
        parent = std::vector<int>(n+1);
        rnk = std::vector<int>(n+1);
 
        // Initially, all vertices are in
        // different sets and have rank 0.
        for (int i = 0; i <= n; i++)
        {
            rnk[i] = 0;
 
            //every element is parent of itself
            parent[i] = i;
        }
    }
 
    // Find the parent of a node 'u'
    // Path Compression
    int find(int u)
    {
        /* Make the parent of the nodes in the path
           from u--> parent[u] point to parent[u] */
        if (u != parent[u])
            parent[u] = find(parent[u]);
        return parent[u];
    }
 
    // Union by rank
    void merge(int x, int y)
    {
        x = find(x), y = find(y);
 
        /* Make tree with smaller height
           a subtree of the other tree  */
        if (rnk[x] > rnk[y])
            parent[y] = x;
        else // If rnk[x] <= rnk[y]
            parent[x] = y;
 
        if (rnk[x] == rnk[y])
            rnk[y]++;
    }
};

void kruskal(const _graph& parent, DisjointSets& ds, std::vector<edge>& spanning_tree, std::set<edge>& spanning_set, 
        const std::set<edge>& forced, const std::set<edge>& restricted){

    for(auto param_edges : forced){

//        if(restricted.find(param_edges) != restricted.end()){
//            std::cout << param_edges.second.first << param_edges.second.second << "BAD rest = for \n";
//            exit(0);
//        }

        int edge1 = param_edges.second.first;
        int edge2 = param_edges.second.second;

        //edges are labeled 1,...,n but indices start from 0.
        int set_edge1 = ds.find(edge1); 
        int set_edge2 = ds.find(edge2);

        if(set_edge1 != set_edge2){
            //std::cerr << edge1 << '-' << edge2 << "FORCED \n";
            ds.merge(set_edge1,set_edge2);

            //spanning_tree.push_back(param_edges);
            spanning_set.insert(param_edges);
        }

        else{ //This forced set is no good, induces a cycle or something. This shouldn't happen, just a sanity check.
            //std::cerr << "BAD" << '\n';
            return;
        }
    }

   
    for( auto param_edges : parent.edges){

        if(restricted.find(param_edges) != restricted.end() ||
                forced.find(param_edges) != forced.end() ){
            //std::cerr << "restricted " << param_edges.second.first << " " << param_edges.second.second << '\n';
            continue;
        }

        int edge1 = param_edges.second.first;
        int edge2 = param_edges.second.second;

        //edges are labeled 1,...,n but indices start from 0.
        int set_edge1 = ds.find(edge1); 
        int set_edge2 = ds.find(edge2);

        if(set_edge1 != set_edge2){
            //std::cerr << edge1 << '-' << edge2 << '\n';
            ds.merge(set_edge1,set_edge2);

            spanning_tree.push_back(param_edges);
            spanning_set.insert(param_edges);
        }
    }
}


void initial_processing (std::vector<std::string>& all_lines, std::vector<std::string>& periods, std::vector<std::string>& unprocessed_edges){

    for(auto graph_info : all_lines){
        std::string delim = ":=";

        auto pos = graph_info.find(delim);
        std::string period = graph_info.substr(0,pos);
        graph_info.erase(0,delim.length() + period.length() + 2);
        std::cerr << period << '\n';

        auto second_pos = graph_info.find(']');
        std::string edges = graph_info.substr(0,second_pos);
        std::cout << edges << '\n';

        periods.push_back(period);
        unprocessed_edges.push_back(edges);
    }

}

//Returns number of vertices in the graph
_graph populate_graph(std::string edge_for_graph){

    _graph graph;
    std::set<int> vertices;
    int first_edge;
    int second_edge;
    bool is_first = true;
    int param_id = 0;

    for(int i = 0; i < edge_for_graph.length(); i++){
        std::string number;
        bool digit_found = false;

        while(isdigit(edge_for_graph[i])){
                number.append(std::string(1,edge_for_graph[i]));
                i++;
                digit_found = true;
        }

        if(digit_found){

            if(is_first){
                first_edge = stoi(number);
                is_first = false;
                vertices.insert(first_edge);
            }
            else{
                param_id++;
                is_first = true;
                second_edge = stoi(number);
                vertices.insert(second_edge);

                auto edge_pair = std::make_pair(first_edge,second_edge);
                std::string param = "x_" + std::to_string(param_id);
                auto schwinger_edge = std::make_pair(param,edge_pair);

                graph.edges.push_back(schwinger_edge);

              //  std::cout << first_edge << std::endl;
            }

            digit_found = false;
        }
    }

    graph.V = vertices.size();
    return graph;
}

GiNaC::ex calculate_kirchoff (_graph parent, std::set<edge> forced, std::set<edge> restricted)
{
    DisjointSets ds(parent.V);
    std::vector<edge> spanning_tree;
    std::set<edge> spanning_set;
    GiNaC::ex toret(0);

    kruskal(parent, ds, spanning_tree, spanning_set, forced, restricted);

    if(spanning_tree.size() + forced.size() == (parent.V - 1)){

        for(auto edge : spanning_tree){
            //std::cerr << edge.second.first << "," << edge.second.second << " | ";
        }
        //std::cerr<< "spans!"  << "\n\n";
        numspan++;

        if(numspan % 10000 == 0){
            std::cout << numspan << '\n';
        }
    }

    else{
        //std::cerr <<"no span!" << "\n\n";
        return toret;
    }

    GiNaC::ex init(1);
    for(auto edge : parent.edges){
        if(spanning_set.find(edge) != spanning_set.end()){
            continue;
        }

        GiNaC::symbol var(edge.first);
        init *= var;
    }
    
    //std::cout << init << '\n';

    for(int i = 0; i < spanning_tree.size(); i++){
        std::set<edge> rest = restricted;
        rest.insert(spanning_tree[i]); 

        std::set<edge> forc = forced;
        for(int j = 0; j < i; j++){
            forc.insert(spanning_tree[j]);
        }

        init += calculate_kirchoff(parent,forc,rest);
    }

    return init;
}

GiNaC::ex calculate_dodgson (_graph parent, std::set<int> I, std::set<int> J, std::set<int> K, 
        std::set<edge> forced, std::set<edge> restricted)
{
    DisjointSets ds_I(parent.V);
    DisjointSets ds_J(parent.V);

    std::set<edge> Iset;
    std::set<edge> Jset;
    std::set<edge> Kset;
    std::set<edge> IcupJset;

    for(int edge : I){
        Iset.insert(parent.edges[edge]);
        IcupJset.insert(parent.edges[edge]);
    }

    for(int edge : J){
        Jset.insert(parent.edges[edge]);
        IcupJset.insert(parent.edges[edge]);
    }
    
    for(int edge : K){
        Kset.insert(parent.edges[edge]);
    }

    for(auto edge : restricted){
        IcupJset.insert(edge);
    }

    for(auto edge : forced){
        Iset.insert(edge);
        Jset.insert(edge);
    }


    std::vector<edge> spanning_tree_I;
    std::vector<edge> spanning_tree_J;
    std::set<edge> spanning_set_I;
    std::set<edge> spanning_set_J;

    GiNaC::ex toret(0);

    kruskal(parent, ds_I, spanning_tree_I,spanning_set_I, Iset, IcupJset);
    kruskal(parent, ds_J, spanning_tree_J,spanning_set_J, Jset, IcupJset);
    
    if(spanning_tree_I.size() == (parent.V - 1) &&
            spanning_tree_J.size() == (parent.V-1)){

        //for(auto edge : spanning_tree){
        //    std::cout << edge.second.first << "," << edge.second.second << " | ";
        //}
        //std::cout<< "spans!"  << "\n\n";
        numspan++;

        if(numspan % 10000 == 0){
            std::cout << numspan << '\n';
        }
    }

    else{
        //std::cout <<"no span!" << "\n\n";
        return toret;
    }

    GiNaC::ex init(1);
    for(auto edge : parent.edges){
        if(spanning_set_I.find(edge) != spanning_set_I.end() ||
                spanning_set_J.find(edge) != spanning_set_J.end()){
            continue;
        }
        
        if(Kset.find(edge) != Kset.end()){
            GiNaC::symbol var(0);
            init *= var;
        }

        else{
            GiNaC::symbol var(edge.first);
            init *= var;
        }
    }
    
    //std::cout << init << '\n';
    //
    if(spanning_tree_J != spanning_tree_I){
        std::cout<<"error spanninig tree not equal" << '\n';
        exit(0);
    }

    for(int i = 0; i < spanning_tree_I.size(); i++){
        std::set<edge> rest = restricted;
        rest.insert(spanning_tree_I[i]); 

        std::set<edge> forc = forced;
        for(int j = 0; j < i; j++){
            forc.insert(spanning_tree_I[j]);
        }

        init += calculate_dodgson (parent,I,J,K, forc,rest);
    }

    return init;
}



int main(int argc, char** argv)
{
    init();
    std::cout << argv[0] << '\n';

    if(argc == 1){
        printf("needs an argument\n");
        return 0;
    }

    std::vector<std::string> periods;
    std::vector<std::string> unprocessed_edges;
    std::unordered_map<std::string, std::set<int>> vertex_set;
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
    _graph feynman_graph = populate_graph(unprocessed_edges[0]);
    std::set<edge> forced;
    std::set<edge> restricted;
    
    std::ofstream myfile;
    myfile.open("out.txt");

    auto kirc = calculate_kirchoff(feynman_graph,forced,restricted);
    //for(int i = 0 ; i < 1; i++){
    //    kirc *= kirc;
    //}
    
    //myfile << "kirchoff = " << GiNaC::expand(kirc) << '\n';
    std::cout << "num span = " << numspan << '\n';

    myfile.close();

}



