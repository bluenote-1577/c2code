#include <stack>
#include <string>
#include <cstring>
#include <sstream>
#include <omp.h>
#include <math.h>
#include <stdlib.h>
#include <unordered_map>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <map>
#include <vector>
#include <iostream>
#include <ostream>
#include <unordered_set>
#include <set>
#include <memory>

//typedef std::unordered_map<std::string,int> variables;
int NUM_THREADS = 3;
unsigned char  p =7; 
std::vector<std::vector<std::pair<std::vector<int>,std::vector<int>>>> sub_maps;
std::map<int,int> size_map;
std::vector<int>svect =  std::vector<int>({0});
std::vector<int>pvect = std::vector<int>({-2}); 

inline bool isInteger(const std::string & s)
{
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false ;

   char * k ;
   strtol(s.c_str(), &k, 10) ;

   return (*k == 0) ;
}

struct monomial{
    std::vector<unsigned char> vars;

    bool operator==(const monomial &other) const{
        return (vars == other.vars);
    }

    monomial(std::vector<unsigned char> a_vars) : vars(a_vars){}
    monomial(const monomial &m){
        vars = m.vars;
    } 

    monomial(){};
};

typedef std::unordered_map<monomial,int> polynomial;

struct vect_mono{

    bool operator==(const vect_mono &other) const{
        return (vars == other.vars);
    }

    std::vector<std::pair<unsigned char,unsigned char>> vars;

    vect_mono(const monomial& mon)
    {
        vars.reserve(mon.vars.size() + 1);
        for(int i = 0,end = mon.vars.size(); i < end; i++){
            if(mon.vars[i] != 0){
                vars.emplace_back(i,mon.vars[i]);
            }
        }
    }

    
    vect_mono(){};

};

//typedef ska::flat_hash_map<vect_mono_key,long long int> polynomial_vect;
//typedef google::dense_hash_map<vect_mono_key,long long int> polynomial_vect;
//typedef tsl::hopscotch_map<vect_mono_key,long long int> polynomial_vect;

struct monomial_x{
    std::vector<int>* xvars;
    std::vector<int>* vars;
    unsigned char expx;
    vect_mono mono;
    //For debugging
    long int cf;
    unsigned char exponent;
    unsigned char exp;

    monomial_x( std::vector<int>* xvars,  std::vector<int>* vars, unsigned char expx,
            unsigned char exp, int a_cf, unsigned char x_exp): xvars(xvars), vars(vars), expx(expx),cf(a_cf), exponent(x_exp),
   exp(exp) {

       if(!xvars->empty()){
           if((*xvars)[0] < 0){
               if ((*xvars)[0] != -2){
                   if(expx %2 == 1){
                       cf *= -1;
                   }
               }
           }
       }

       if(!vars->empty()){
            if((*vars)[0] < 0){
               if ((*vars)[0] != -2){
                   if(exp %2 == 1){
                       cf *= -1;
                   }
               }
           }
       }
   }
       //For debugging might be useful
//        if(a_cf == 0){
//            return;
//        }
//
//       for(int i = 0,end = xvars.size(); i < end ; i++){
//           if(xvars[i] < 0 ){
//               if(xvars[i] != -2){
//                   if(expx % 2 == 1){
//                       cf *= -1;
//                   }
//               }
//           }
//           else{
//               mono.vars.emplace_back(xvars[i],expx);
//           }
//       } 
//
//       for(int i = 0,end = vars.size(); i < end; i++){
//           if(vars[i] < 0 ){
//              if(vars[i] != -2){
//                  if(exp % 2 == 1){
//                      cf *= -1;
//                  }
//               }
//           }
//           else{
//               mono.vars.emplace_back(vars[i],exp);
////               std::cout << (int)vars[i] << (int)exp << "emplaced\n";
//           }
//       }
//    }
};

struct mono_x_node{
    monomial_x* mon_ptr;
    std::vector<mono_x_node*> children;
    mono_x_node(monomial_x* monx) : mon_ptr(monx){
        children.reserve(p);
    }

    ~mono_x_node(){
        for(unsigned int i = 0; i < children.size(); i++){
            delete children[i];
        }
    }

    mono_x_node(){
    }
};

void process_recipe(std::vector<std::vector<std::pair<std::vector<int>,std::vector<int>>>>& sub_maps,
        std::map<int,int>& size_map, std::string recipe_name){

    std::ifstream dodgson_file(recipe_name);
    int num_iter = 0;

    if(dodgson_file.is_open())
    {
        std::set<std::string> var_set_new;
        std::set<std::string> var_set_old;
        std::map<std::string,int> old_to_index;
        std::map<std::string,int> new_to_index;
        std::vector<std::pair<std::vector<int>,std::vector<int>>> sub_map;
        std::string line;
        bool first = true;
        while(std::getline(dodgson_file,line)){

            int numvar = 0;
            if(line[0] == '>'){

               numvar = std::stoi(line.substr(1));
               size_map.insert({num_iter,numvar});
                if(!first){
                   sub_maps.push_back(sub_map);
                }
               sub_map.clear();
               sub_map.resize(size_map[num_iter]);
               var_set_new.clear();
               var_set_old.clear();
               old_to_index.clear();
               new_to_index.clear();
               num_iter++;
               first = true;
            }

            else if(line[0] == '-'){
                var_set_old.insert(line.substr(1));
            }

            else if(line[0] == '+'){
                var_set_new.insert(line.substr(1));
            }

            else if(line[0] != '>'){
                if(first){
                    int itcount = 0;
                    for(auto it = var_set_old.begin(),end = var_set_old.end(); it != end; ++it){
                        old_to_index.insert(std::make_pair(*it,itcount)); 
//                        std::cout << itcount << " " << *it << ' ';
                        itcount++;
                    }
//                    std::cout << var_set_old.size() << "\n";

                    itcount = 0;
                    for(auto it = var_set_new.begin(),end = var_set_new.end(); it != end; ++it){
                        new_to_index.insert(std::make_pair(*it,itcount)); 
//                        std::cout << itcount << " " << *it << ' ';
                        itcount++;
                    }
//                    std::cout<< var_set_new.size()  << "\n";
                    first = false;
                }

                int pos1 = line.find(',');
                std::string var = line.substr(0,pos1);
                line.erase(0,pos1+1);

                std::vector<int> x_vars;
                std::vector<int> nox_vars;

                int pos2 = line.find(',');
                std::string xvars_line = line.substr(0,pos2);
                line.erase(0,pos2+1);

                size_t inter_pos;

                if(isInteger(xvars_line)){
                    int x = std::stoi(xvars_line);
                    if(x == 1){
                        x_vars.push_back(-2);
                    }
                    if(x == -1){
                        x_vars.push_back(-3);
                    }
                }
                    
                else{
//                    std::cout << var << ' ' << xvars_line << " line " << '\n';
                    if(xvars_line[0] == '-'){
                        x_vars.push_back(-1);
                        xvars_line = xvars_line.substr(1);
                    }

                    while((inter_pos = xvars_line.find('*')) != std::string::npos){
                        std::string token = xvars_line.substr(0,inter_pos);
                        x_vars.push_back(new_to_index[token]);
                        xvars_line.erase(0,inter_pos+1);
                    }
                    x_vars.push_back(new_to_index[xvars_line]);
                }

                if(isInteger(line)){
                    int x = std::stoi(line);
                    if(x == 1){
                        nox_vars.push_back(-2);
                    }
                    if(x == -1){
                        nox_vars.push_back(-3);
                    }
                }

                else{
//                std::cout << var << ' ' <<  line << " line " << '\n';
                    if(line[0] == '-'){
                        nox_vars.push_back(-1);
                        line = line.substr(1);
                    }

                    while((inter_pos = line.find('*')) != std::string::npos){
                        std::string token = line.substr(0,inter_pos);
                        nox_vars.push_back(new_to_index[token]);
                        line.erase(0,inter_pos+1);
                    }

                    nox_vars.push_back(new_to_index[line]);
                }

//                std::cout << sub_map.size()  << old_to_index[var] << '\n';
                sub_map[old_to_index[var]] = std::make_pair(x_vars,nox_vars);
            }

        }
        dodgson_file.close();
    }
    
}

typedef std::vector<std::pair<vect_mono,int>> iter_polyn; 
namespace std{

    template <>
    struct hash<monomial> {
        std::size_t operator()(const monomial& k) const
        {
            std::size_t toreturn = 17;

            using std::hash;

            for(int i = 0, end = k.vars.size(); i <end ; i++){
                if(k.vars[i] != 0){
                    toreturn ^= k.vars[i] + 311*i + 0x9e3779b9 + (toreturn << 6) + (toreturn >> 2);
                }
            }

            return toreturn;
        }
    };
}

std::ostream& operator<<(std::ostream& os, const monomial_x& m){
        os << m.cf << "*";

        for (unsigned int i = 0; i < m.xvars->size(); i++){
            os << (int)(*m.xvars)[i]<< "^" << (int)m.expx <<  ' ';
        }

        for (unsigned int i = 0; i < m.vars->size(); i++){
            os << (int)(*m.vars)[i] << "^" << (int)m.exp <<  ' ';
        }

        return os;
}

std::ostream& operator<<(std::ostream& os, const vect_mono& m){

        for (unsigned int i = 0; i < m.vars.size(); i++){
            os << (int)m.vars[i].first << "^" <<  (int)m.vars[i].second << '*';
        }

        return os;
}

bool sort_function ( const std::vector<monomial_x>& vec1, const std::vector<monomial_x>& vec2){
    return (vec1.size() < vec2.size());
}

std::vector<monomial_x>
tabled_lookup (unsigned char var, unsigned char exponent, unsigned int iteration){ 
    std::vector<int>* xvars = &sub_maps[iteration][var].first;
    std::vector<int>* vars = &sub_maps[iteration][var].second;    

    if(iteration+1 == size_map.size()){
        xvars = &svect;
        vars = &pvect;  
    }

    std::vector<int> emptyvect;
    
    bool b_zero = false;
    bool a_zero = false;

    if(xvars->empty()){
        a_zero = true;
    }
    
    if(vars->empty()){
        b_zero = true;
    }

    if(a_zero){
        return std::vector<monomial_x>({monomial_x(&emptyvect,vars,0,exponent,1,0)});
    }

    if(b_zero){
       return std::vector<monomial_x>({monomial_x(xvars,&emptyvect,exponent,0,1,exponent)});
    }

    if(a_zero && b_zero){
        std::cout << "both zero" << '\n';
        exit(0);
    }

    int NONE = 0;

    switch(exponent){
        case 1: {
                   return std::vector<monomial_x>({
                   monomial_x(&emptyvect,vars,NONE,1,1,0),
                   monomial_x(xvars,&emptyvect,1,NONE,1,1)
                           });
                }

        case 2: {

                   return std::vector<monomial_x>({
                   monomial_x(&emptyvect,vars,NONE,2,1,0),
                   monomial_x(xvars,vars,1,1,2%p,1),
                   monomial_x(xvars,&emptyvect,2,NONE,1,2)
                           });
                }

        case 3: {

                   return std::vector<monomial_x>({
                   monomial_x(&emptyvect,vars,NONE,3,1,0),
                   monomial_x(xvars,vars,1,2,3%p,1),
                   monomial_x(xvars,vars,2,1,3%p,2),
                   monomial_x(xvars,&emptyvect,3,NONE,1,3)
                   });
                }
        case 4: {
                   
                   return std::vector<monomial_x>({
                   monomial_x(&emptyvect,vars,NONE,4,1,0),
                   monomial_x(xvars,vars,1,3,4%p,1),
                   monomial_x(xvars,vars,2,2,6%p,2),
                   monomial_x(xvars,vars,3,1,4%p,3),
                   monomial_x(xvars,&emptyvect,4,NONE,1,4)
                   });
                }

        case 5: {
                   
                   return std::vector<monomial_x>({
                   monomial_x(&emptyvect,vars,NONE,5,1,0),
                   monomial_x(xvars,vars,1,4,5%p,1),
                   monomial_x(xvars,vars,2,3,10%p,2),
                   monomial_x(xvars,vars,3,2,10%p,3),
                   monomial_x(xvars,vars,4,1,5%p,4),
                   monomial_x(xvars,&emptyvect,5,NONE,1,5)
                   });
                }
       case 6: {
                   
                   return std::vector<monomial_x>({
                   monomial_x(&emptyvect,vars,NONE,6,1,0),
                   monomial_x(xvars,vars,1,5,6%p,1),
                   monomial_x(xvars,vars,2,4,15%p,2),
                   monomial_x(xvars,vars,3,3,20%p,3),
                   monomial_x(xvars,vars,4,2,15%p,4),
                   monomial_x(xvars,vars,5,1,6%p,5),
                   monomial_x(xvars,&emptyvect,6,NONE,1,6)
                   });
                }

      case 7: {
                   return std::vector<monomial_x>({
                   monomial_x(&emptyvect,vars,NONE,7,1,0),
                   monomial_x(xvars,vars,1,6,7%p,1),
                   monomial_x(xvars,vars,2,5,21%p,2),
                   monomial_x(xvars,vars,3,4,35%p,3),
                   monomial_x(xvars,vars,4,3,35%p,4),
                   monomial_x(xvars,vars,5,2,21%p,5),
                   monomial_x(xvars,vars,6,1,7%p,6),
                   monomial_x(xvars,&emptyvect,7,NONE,1,7)
                   });

              }

      case 8: {
                   return std::vector<monomial_x>({
                    monomial_x(&emptyvect,vars,NONE,8,1,0),
                   monomial_x(xvars,vars,1,7,8%p,1),
                   monomial_x(xvars,vars,2,6,28%p,2),
                   monomial_x(xvars,vars,3,5,56%p,3),
                   monomial_x(xvars,vars,4,4,70%p,4),
                   monomial_x(xvars,vars,5,3,56%p,5),
                   monomial_x(xvars,vars,6,2,28%p,6),
                   monomial_x(xvars,vars,7,1,8%p,7),
                   monomial_x(xvars,&emptyvect,8,NONE,1,8)
                  });
              }

      case 9: {
                  return std::vector<monomial_x>({
                   monomial_x(&emptyvect,vars,NONE,9,1,0),
                   monomial_x(xvars,vars,1,8,9%p,1),
                   monomial_x(xvars,vars,2,7,36%p,2),
                   monomial_x(xvars,vars,3,6,84%p,3),
                   monomial_x(xvars,vars,4,5,126%p,4),
                   monomial_x(xvars,vars,5,4,126%p,5),
                   monomial_x(xvars,vars,6,3,84%p,6),
                   monomial_x(xvars,vars,7,2,36%p,7),
                   monomial_x(xvars,vars,8,1,9%p,8),
                   monomial_x(xvars,&emptyvect,9,NONE,1,9)
                  });

              }
      case 10:{
                    return std::vector<monomial_x>({
                     monomial_x(&emptyvect,vars,NONE,10,1,0),
                   monomial_x(xvars,vars,1,9,10%p,1),
                   monomial_x(xvars,vars,2,8,45%p,2),
                   monomial_x(xvars,vars,3,7,120%p,3),
                   monomial_x(xvars,vars,4,6,210%p,4),
                   monomial_x(xvars,vars,5,5,252%p,5),
                   monomial_x(xvars,vars,6,4,210%p,6),
                   monomial_x(xvars,vars,7,3,120%p,7),
                   monomial_x(xvars,vars,8,2,45%p,8),
                   monomial_x(xvars,vars,9,1,10%p,9),
                   monomial_x(xvars,&emptyvect,10,NONE,1,10)
                  });

              }

      case 11:{
                      return std::vector<monomial_x>({
                     monomial_x(&emptyvect,vars,NONE,11,1,0),
                   monomial_x(xvars,vars,1,10,11%p,1),
                   monomial_x(xvars,vars,2,9,55%p,2),
                   monomial_x(xvars,vars,3,8,165%p,3),
                   monomial_x(xvars,vars,4,7,330%p,4),
                   monomial_x(xvars,vars,5,6,462%p,5),
                   monomial_x(xvars,vars,6,5,462%p,6),
                   monomial_x(xvars,vars,7,4,330%p,7),
                   monomial_x(xvars,vars,8,3,165%p,8),
                   monomial_x(xvars,vars,9,2,55%p,9),
                   monomial_x(xvars,vars,10,1,11%p,10),
                   monomial_x(xvars,&emptyvect,11,NONE,1,11)
                  });
              }
        case 12:{
                      return std::vector<monomial_x>({
                     monomial_x(&emptyvect,vars,NONE,12,1,0),
                   monomial_x(xvars,vars,1,11,12%p,1),
                   monomial_x(xvars,vars,2,10,66%p,2),
                   monomial_x(xvars,vars,3,9,220%p,3),
                   monomial_x(xvars,vars,4,8,495%p,4),
                   monomial_x(xvars,vars,5,7,792%p,5),
                   monomial_x(xvars,vars,6,6,924%p,6),
                   monomial_x(xvars,vars,7,5,792%p,7),
                   monomial_x(xvars,vars,8,4,495%p,8),
                   monomial_x(xvars,vars,9,3,220%p,9),
                   monomial_x(xvars,vars,10,2,66%p,10),
                   monomial_x(xvars,vars,11,1,12%p,11),
                   monomial_x(xvars,&emptyvect,12,NONE,1,12)
                  });
              }

        case 13:{
                      return std::vector<monomial_x>({
                     monomial_x(&emptyvect,vars,NONE,12,1,0),
                   monomial_x(xvars,vars,1,12,13%p,1),
                   monomial_x(xvars,vars,2,11,78%p,2),
                   monomial_x(xvars,vars,3,10,286%p,3),
                   monomial_x(xvars,vars,4,9,715%p,4),
                   monomial_x(xvars,vars,5,8,1287%p,5),
                   monomial_x(xvars,vars,6,7,1716%p,6),
                   monomial_x(xvars,vars,7,6,1716%p,7),
                   monomial_x(xvars,vars,8,5,1287%p,8),
                   monomial_x(xvars,vars,9,4,715%p,9),
                   monomial_x(xvars,vars,10,3,286%p,10),
                   monomial_x(xvars,vars,11,2,78%p,11),
                   monomial_x(xvars,vars,12,1,13%p,12),
                   monomial_x(xvars,&emptyvect,13,NONE,1,13)
                  });
              }

        case 14:{
                      return std::vector<monomial_x>({
                     monomial_x(&emptyvect,vars,NONE,12,1,0),
                   monomial_x(xvars,vars,1,13,14%p,1),
                   monomial_x(xvars,vars,2,12,91%p,2),
                   monomial_x(xvars,vars,3,11,364%p,3),
                   monomial_x(xvars,vars,4,10,1001%p,4),
                   monomial_x(xvars,vars,5,9,2002%p,5),
                   monomial_x(xvars,vars,6,8,3003%p,6),
                   monomial_x(xvars,vars,7,7,3432%p,7),
                   monomial_x(xvars,vars,8,6,3003%p,8),
                   monomial_x(xvars,vars,9,5,2002%p,9),
                   monomial_x(xvars,vars,10,4,1001%p,10),
                   monomial_x(xvars,vars,11,3,364%p,11),
                   monomial_x(xvars,vars,12,2,91%p,12),
                   monomial_x(xvars,vars,13,1,14%p,13),
                   monomial_x(xvars,&emptyvect,14,NONE,1,14)
                  });
              }

        default:{
                    //TODO
                    std::cout << int(exponent) << " shouldn't reach here, exponent > 5\n ";
                    exit(0);
                }

    }
};

bool
obtain_multiplicands ( 
        mono_x_node* parent, 
        int current_weight,
        std::vector<std::vector<monomial_x>>& to_mult,
        unsigned int iteration){

    
    if(iteration == to_mult.size()){
        return true;
    }
    
    int max_weight_add = 0;

    for(unsigned int k = iteration + 1,end = to_mult.size(); k < end; k++){
        if(to_mult[k].size() != 1){
//            max_weight_add += to_mult[k].begin()->exponent; 
            max_weight_add += to_mult[k].size() - 1; 
        }

        else{
            max_weight_add += to_mult[k][0].exponent;
        }
    }

    bool bad = true;

    for(unsigned int i = 0,end = to_mult[iteration].size(); i < end; i++){

        if(to_mult[iteration][i].cf == 0){
            continue;
        }

        else if(to_mult[iteration][i].exponent + current_weight + max_weight_add < p-1){
            continue;
        }

        else if (to_mult[iteration][i].exponent + current_weight > p-1){
            continue;
        }

        mono_x_node* possible_child = new mono_x_node(&to_mult[iteration][i]);

        if(obtain_multiplicands(possible_child, current_weight + to_mult[iteration][i].exponent, to_mult, iteration + 1)){
            parent->children.push_back(possible_child);
            bad = false;
        }
    }

    if(bad){
        return false;
    }
};

void multiply_on_tree(polynomial& toadd, const mono_x_node& node, int cf, const monomial& mono_build){

    monomial new_monomial(mono_build);

    for(auto mono = node.mon_ptr->xvars->begin(),end = node.mon_ptr->xvars->end(); mono != end; ++mono){
        int s = *mono;
       if(s >= 0){
         new_monomial.vars[s] += node.mon_ptr->expx; 
       }

    }

   for(auto mono = node.mon_ptr->vars->begin(), end = node.mon_ptr->vars->end(); mono != end; ++mono){
        int s = *mono;
       if(s >= 0){
          new_monomial.vars[s] += node.mon_ptr->exp; 
       }
   }

   cf = cf * node.mon_ptr->cf;
    
    if(node.children.empty()){
        {
            auto monval = toadd.find(new_monomial); 
            if(monval != toadd.end()){
                monval->second += cf;
                monval->second %= p;
            }
            else{
                cf %= p;
                toadd.insert(std::make_pair(new_monomial,cf));
            }
        }
    }
    else{
        for(auto it = node.children.begin(), end = node.children.end(); it!=end; ++it){
            multiply_on_tree(toadd, **it, cf, new_monomial); 
        }
    }
}

void print_tree(mono_x_node node){
    if(!node.children.empty()){
        std::cout << " call ";
    }
    for(auto child : node.children){
        std::cout << *child->mon_ptr << ' ';
    }

    for(auto child : node.children){
        print_tree(*child);
    }
}

int main (int argc, char** argv){

    omp_set_num_threads(NUM_THREADS);
    //Read the recipe file for substi. rules.

    if(argc == 1){
        std::cerr << "needs arg\n";
        exit(1);
    }
    
    

    std::string recipe_name = std::string(argv[1]);

    std::ofstream c2_outfile("c2.txt", std::ofstream::out | std::ofstream::app);
    c2_outfile << recipe_name << '\n';
    c2_outfile.close();

    process_recipe(sub_maps,size_map,recipe_name);

//    std::cout << sub_maps[1].size();
//    for(auto& thing : sub_maps){
//        for(int i = 0; i < thing.size(); i++){
//            std::cout << "SUBS " << i << '\n';
//            for(auto& thing : thing[i].first){
//                std::cout << thing << ' ';
//            }
//            for(auto& thing : thing[i].second){
//                std::cout << thing << ' ';
//            }
//            std::cout << '\n';
//        }
//    }

    std::set<unsigned short> primes = {11,13};
    for(auto it = primes.begin(); it!= primes.end(); ++it){
        p = *it;
        vect_mono start;

        for(int i = 0; i < size_map[0]; i++){
            start.vars.push_back(std::make_pair(i,p-1));
        }

        iter_polyn actual_poly = {std::make_pair(start,1)};
        double accum = 0;
        for(unsigned int iter = 0; iter < size_map.size(); iter++){
            polynomial toadd[NUM_THREADS];

            std::cout << "size " << actual_poly.size() << '\n';
            double begin = omp_get_wtime();
           
            #pragma omp parallel for 
            for(unsigned int k = 0; k < actual_poly.size(); k++){
                auto* mono_it = &actual_poly[k];

                if(mono_it->second != 0){
                    if(k%10000 == 0 && k != 0){
                        std::cout << k << '\n';
                    }
                    std::vector<std::vector<monomial_x>> to_mult;
                    to_mult.reserve(20);

                    for(auto elt = mono_it->first.vars.begin(), 
                            end = mono_it->first.vars.end(); 
                            elt !=  end; 
                            ++elt){
                        to_mult.push_back(tabled_lookup(elt->first,elt->second,iter));
                    }

                    std::sort (to_mult.begin(), to_mult.end(), sort_function);
                    mono_x_node parent;
                    obtain_multiplicands(&parent,0,to_mult,0);

                    if(!parent.children.empty()){
//                        print_tree(parent);
//                        std::cout << '\n';
                    }
                    
                    for(auto it = parent.children.begin(), end = parent.children.end(); it!= end; ++it){
                        auto ptr_node = *it;
                        monomial toins;
                        if(iter+1 == size_map.size()){
                            toins.vars = std::vector<unsigned char>(1,0);
                        }
                        else{
                            toins.vars = std::vector<unsigned char>(size_map[iter+1],0);
                        }

                        int index = omp_get_thread_num();
                        multiply_on_tree(toadd[index], *ptr_node, mono_it->second, toins);
                        
                    }
                }  
            }

            polynomial temp;
            actual_poly.clear();
            for(int i = 0; i < NUM_THREADS; i++){
                for(auto monomial = toadd[i].begin(), end = toadd[i].end(); monomial != end; ++monomial){
                    if(monomial->second != 0){
                        temp[monomial->first] += monomial -> second;
                        temp[monomial->first] %= p;
                    }
                }
            }

            actual_poly.reserve(temp.size());
            for(auto monomial = temp.begin(), end = temp.end(); monomial != end; ++monomial){
                if(monomial -> second != 0){
                    actual_poly.emplace_back(monomial->first,monomial->second);
                }
            }

            double end = omp_get_wtime();
            accum += end-begin;
    //        std::cout << i << '\n';
        }


        std::cout << "time taken " << accum << '\n';
        std::cout << actual_poly.size() << "final size\n";
        if(actual_poly.size() > 0){
            std::cout << "c2 is " << actual_poly[0].second % p << '\n';
        }

        std::ofstream c2_outfile("c2.txt", std::ofstream::out | std::ofstream::app);
        c2_outfile << "prime " << p <<  " " << actual_poly[0].second % p << '\n';
        c2_outfile.close();

        for(auto thing : actual_poly){
            if(thing.second != 1){
                std::cout << thing.second << '*' << thing.first  << '+';
            }
            else {
                std::cout << thing.first  << '+';
            }
        }

    }
}

