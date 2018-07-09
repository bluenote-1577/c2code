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
int NUM_THREADS = 1;
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
                        std::cout << itcount << " " << *it << ' ';
                        itcount++;
                    }
                    std::cout << var_set_old.size() << "\n";

                    itcount = 0;
                    for(auto it = var_set_new.begin(),end = var_set_new.end(); it != end; ++it){
                        new_to_index.insert(std::make_pair(*it,itcount)); 
                        std::cout << itcount << " " << *it << ' ';
                        itcount++;
                    }
                    std::cout<< var_set_new.size()  << "\n";
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
                     monomial_x(&emptyvect,vars,NONE,13,1,0),
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
                     monomial_x(&emptyvect,vars,NONE,14,1,0),
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

        case 15:{
                      return std::vector<monomial_x>({
                     monomial_x(&emptyvect,vars,NONE,15,1,0),
                   monomial_x(xvars,vars,1,14,15%p,1),
                   monomial_x(xvars,vars,2,13,105%p,2),
                   monomial_x(xvars,vars,3,12,455%p,3),
                   monomial_x(xvars,vars,4,11,1365%p,4),
                   monomial_x(xvars,vars,5,10,3003%p,5),
                   monomial_x(xvars,vars,6,9,5005%p,6),
                   monomial_x(xvars,vars,7,8,6435%p,7),
                   monomial_x(xvars,vars,8,7,6435%p,8),
                   monomial_x(xvars,vars,9,6,5005%p,9),
                   monomial_x(xvars,vars,10,5,3003%p,10),
                   monomial_x(xvars,vars,11,4,1365%p,11),
                   monomial_x(xvars,vars,12,3,455%p,12),
                   monomial_x(xvars,vars,13,2,105%p,13),
                   monomial_x(xvars,vars,14,1,15%p,14),
                   monomial_x(xvars,&emptyvect,15,NONE,1,15)
                  });
              }

        case 16:{
                      return std::vector<monomial_x>({
                     monomial_x(&emptyvect,vars,NONE,16,1,0),
                   monomial_x(xvars,vars,1,15,16%p,1),
                   monomial_x(xvars,vars,2,14,120%p,2),
                   monomial_x(xvars,vars,3,13,560%p,3),
                   monomial_x(xvars,vars,4,12,1820%p,4),
                   monomial_x(xvars,vars,5,11,4368%p,5),
                   monomial_x(xvars,vars,6,10,8008%p,6),
                   monomial_x(xvars,vars,7,9,11440%p,7),
                   monomial_x(xvars,vars,8,8,12870%p,8),
                   monomial_x(xvars,vars,9,7,11440%p,9),
                   monomial_x(xvars,vars,10,6,8008%p,10),
                   monomial_x(xvars,vars,11,5,4368%p,11),
                   monomial_x(xvars,vars,12,4,1820%p,12),
                   monomial_x(xvars,vars,13,3,560%p,13),
                   monomial_x(xvars,vars,14,2,120%p,14),
                   monomial_x(xvars,vars,15,1,16%p,15),
                   monomial_x(xvars,&emptyvect,16,NONE,1,16)
                  });
              }

        case 17:{
                      return std::vector<monomial_x>({
                     monomial_x(&emptyvect,vars,NONE,17,1,0),
                   monomial_x(xvars,vars,1,16,17%p,1),
                   monomial_x(xvars,vars,2,15,136%p,2),
                   monomial_x(xvars,vars,3,14,680%p,3),
                   monomial_x(xvars,vars,4,13,2380%p,4),
                   monomial_x(xvars,vars,5,12,6188%p,5),
                   monomial_x(xvars,vars,6,11,12376%p,6),
                   monomial_x(xvars,vars,7,10,19448%p,7),
                   monomial_x(xvars,vars,8,9,24310%p,8),
                   monomial_x(xvars,vars,9,8,24310%p,9),
                   monomial_x(xvars,vars,10,7,19448%p,10),
                   monomial_x(xvars,vars,11,6,12376%p,11),
                   monomial_x(xvars,vars,12,5,6188%p,12),
                   monomial_x(xvars,vars,13,4,2380%p,13),
                   monomial_x(xvars,vars,14,3,680%p,14),
                   monomial_x(xvars,vars,15,2,136%p,15),
                   monomial_x(xvars,vars,16,1,17%p,16),
                   monomial_x(xvars,&emptyvect,17,NONE,1,17)
                  });
              }

        case 18:{
                      return std::vector<monomial_x>({
                     monomial_x(&emptyvect,vars,NONE,18,1,0),
                   monomial_x(xvars,vars,1,17,18%p,1),
                   monomial_x(xvars,vars,2,16,153%p,2),
                   monomial_x(xvars,vars,3,15,816%p,3),
                   monomial_x(xvars,vars,4,14,3060%p,4),
                   monomial_x(xvars,vars,5,13,8568%p,5),
                   monomial_x(xvars,vars,6,12,18564%p,6),
                   monomial_x(xvars,vars,7,11,31824%p,7),
                   monomial_x(xvars,vars,8,10,43758%p,8),
                   monomial_x(xvars,vars,9,9,48620%p,9),
                   monomial_x(xvars,vars,10,8,43758%p,10),
                   monomial_x(xvars,vars,11,7,31824%p,11),
                   monomial_x(xvars,vars,12,6,18564%p,12),
                   monomial_x(xvars,vars,13,5,8568%p,13),
                   monomial_x(xvars,vars,14,4,3060%p,14),
                   monomial_x(xvars,vars,15,3,816%p,15),
                   monomial_x(xvars,vars,16,2,153%p,16),
                   monomial_x(xvars,vars,17,1,18%p,17),
                   monomial_x(xvars,&emptyvect,18,NONE,1,18)
                  });
              }

        case 19:{
                    return std::vector<monomial_x>({ 
monomial_x(&emptyvect,vars,NONE,19,1,0),
    monomial_x(xvars,vars,1,18,19%p,1),
    monomial_x(xvars,vars,2,17,171%p,2),
    monomial_x(xvars,vars,3,16,969%p,3),
    monomial_x(xvars,vars,4,15,3876%p,4),
    monomial_x(xvars,vars,5,14,11628%p,5),
    monomial_x(xvars,vars,6,13,27132%p,6),
    monomial_x(xvars,vars,7,12,50388%p,7),
    monomial_x(xvars,vars,8,11,75582%p,8),
    monomial_x(xvars,vars,9,10,92378%p,9),
    monomial_x(xvars,vars,10,9,92378%p,10),
    monomial_x(xvars,vars,11,8,75582%p,11),
    monomial_x(xvars,vars,12,7,50388%p,12),
    monomial_x(xvars,vars,13,6,27132%p,13),
    monomial_x(xvars,vars,14,5,11628%p,14),
    monomial_x(xvars,vars,15,4,3876%p,15),
    monomial_x(xvars,vars,16,3,969%p,16),
    monomial_x(xvars,vars,17,2,171%p,17),
    monomial_x(xvars,vars,18,1,19%p,18),
    monomial_x(xvars,&emptyvect,19,NONE,1,19)

                            });
                }

         case 20:{
                    return std::vector<monomial_x>({ 
    monomial_x(&emptyvect,vars,NONE,20,1,0),
    monomial_x(xvars,vars,1,19,20%p,1),
    monomial_x(xvars,vars,2,18,190%p,2),
    monomial_x(xvars,vars,3,17,1140%p,3),
    monomial_x(xvars,vars,4,16,4845%p,4),
    monomial_x(xvars,vars,5,15,15504%p,5),
    monomial_x(xvars,vars,6,14,38760%p,6),
    monomial_x(xvars,vars,7,13,77520%p,7),
    monomial_x(xvars,vars,8,12,125970%p,8),
    monomial_x(xvars,vars,9,11,167960%p,9),
    monomial_x(xvars,vars,10,10,184756%p,10),
    monomial_x(xvars,vars,11,9,167960%p,11),
    monomial_x(xvars,vars,12,8,125970%p,12),
    monomial_x(xvars,vars,13,7,77520%p,13),
    monomial_x(xvars,vars,14,6,38760%p,14),
    monomial_x(xvars,vars,15,5,15504%p,15),
    monomial_x(xvars,vars,16,4,4845%p,16),
    monomial_x(xvars,vars,17,3,1140%p,17),
    monomial_x(xvars,vars,18,2,190%p,18),
    monomial_x(xvars,vars,19,1,20%p,19),
    monomial_x(xvars,&emptyvect,20,NONE,1,20)
                                       });
                 }

                     case 21:{
                    return std::vector<monomial_x>({ 
monomial_x(&emptyvect,vars,NONE,21,1,0),
    monomial_x(xvars,vars,1,20,21%p,1),
    monomial_x(xvars,vars,2,19,210%p,2),
    monomial_x(xvars,vars,3,18,1330%p,3),
    monomial_x(xvars,vars,4,17,5985%p,4),
    monomial_x(xvars,vars,5,16,20349%p,5),
    monomial_x(xvars,vars,6,15,54264%p,6),
    monomial_x(xvars,vars,7,14,116280%p,7),
    monomial_x(xvars,vars,8,13,203490%p,8),
    monomial_x(xvars,vars,9,12,293930%p,9),
    monomial_x(xvars,vars,10,11,352716%p,10),
    monomial_x(xvars,vars,11,10,352716%p,11),
    monomial_x(xvars,vars,12,9,293930%p,12),
    monomial_x(xvars,vars,13,8,203490%p,13),
    monomial_x(xvars,vars,14,7,116280%p,14),
    monomial_x(xvars,vars,15,6,54264%p,15),
    monomial_x(xvars,vars,16,5,20349%p,16),
    monomial_x(xvars,vars,17,4,5985%p,17),
    monomial_x(xvars,vars,18,3,1330%p,18),
    monomial_x(xvars,vars,19,2,210%p,19),
    monomial_x(xvars,vars,20,1,21%p,20),
    monomial_x(xvars,&emptyvect,21,NONE,1,21)

                             });
                     }
                     case 22:{
                    return std::vector<monomial_x>({ 
monomial_x(&emptyvect,vars,NONE,22,1,0),
    monomial_x(xvars,vars,1,21,22%p,1),
    monomial_x(xvars,vars,2,20,231%p,2),
    monomial_x(xvars,vars,3,19,1540%p,3),
    monomial_x(xvars,vars,4,18,7315%p,4),
    monomial_x(xvars,vars,5,17,26334%p,5),
    monomial_x(xvars,vars,6,16,74613%p,6),
    monomial_x(xvars,vars,7,15,170544%p,7),
    monomial_x(xvars,vars,8,14,319770%p,8),
    monomial_x(xvars,vars,9,13,497420%p,9),
    monomial_x(xvars,vars,10,12,646646%p,10),
    monomial_x(xvars,vars,11,11,705432%p,11),
    monomial_x(xvars,vars,12,10,646646%p,12),
    monomial_x(xvars,vars,13,9,497420%p,13),
    monomial_x(xvars,vars,14,8,319770%p,14),
    monomial_x(xvars,vars,15,7,170544%p,15),
    monomial_x(xvars,vars,16,6,74613%p,16),
    monomial_x(xvars,vars,17,5,26334%p,17),
    monomial_x(xvars,vars,18,4,7315%p,18),
    monomial_x(xvars,vars,19,3,1540%p,19),
    monomial_x(xvars,vars,20,2,231%p,20),
    monomial_x(xvars,vars,21,1,22%p,21),
    monomial_x(xvars,&emptyvect,22,NONE,1,22)

                             });
                             }

                     case 23:{
                    return std::vector<monomial_x>({ 
monomial_x(&emptyvect,vars,NONE,23,1,0),
    monomial_x(xvars,vars,1,22,23%p,1),
    monomial_x(xvars,vars,2,21,253%p,2),
    monomial_x(xvars,vars,3,20,1771%p,3),
    monomial_x(xvars,vars,4,19,8855%p,4),
    monomial_x(xvars,vars,5,18,33649%p,5),
    monomial_x(xvars,vars,6,17,100947%p,6),
    monomial_x(xvars,vars,7,16,245157%p,7),
    monomial_x(xvars,vars,8,15,490314%p,8),
    monomial_x(xvars,vars,9,14,817190%p,9),
    monomial_x(xvars,vars,10,13,1144066%p,10),
    monomial_x(xvars,vars,11,12,1352078%p,11),
    monomial_x(xvars,vars,12,11,1352078%p,12),
    monomial_x(xvars,vars,13,10,1144066%p,13),
    monomial_x(xvars,vars,14,9,817190%p,14),
    monomial_x(xvars,vars,15,8,490314%p,15),
    monomial_x(xvars,vars,16,7,245157%p,16),
    monomial_x(xvars,vars,17,6,100947%p,17),
    monomial_x(xvars,vars,18,5,33649%p,18),
    monomial_x(xvars,vars,19,4,8855%p,19),
    monomial_x(xvars,vars,20,3,1771%p,20),
    monomial_x(xvars,vars,21,2,253%p,21),
    monomial_x(xvars,vars,22,1,23%p,22),
    monomial_x(xvars,&emptyvect,23,NONE,1,23)

                             });
             }

                     case 24:{
                    return std::vector<monomial_x>({ 
monomial_x(&emptyvect,vars,NONE,24,1,0),
    monomial_x(xvars,vars,1,23,24%p,1),
    monomial_x(xvars,vars,2,22,276%p,2),
    monomial_x(xvars,vars,3,21,2024%p,3),
    monomial_x(xvars,vars,4,20,10626%p,4),
    monomial_x(xvars,vars,5,19,42504%p,5),
    monomial_x(xvars,vars,6,18,134596%p,6),
    monomial_x(xvars,vars,7,17,346104%p,7),
    monomial_x(xvars,vars,8,16,735471%p,8),
    monomial_x(xvars,vars,9,15,1307504%p,9),
    monomial_x(xvars,vars,10,14,1961256%p,10),
    monomial_x(xvars,vars,11,13,2496144%p,11),
    monomial_x(xvars,vars,12,12,2704156%p,12),
    monomial_x(xvars,vars,13,11,2496144%p,13),
    monomial_x(xvars,vars,14,10,1961256%p,14),
    monomial_x(xvars,vars,15,9,1307504%p,15),
    monomial_x(xvars,vars,16,8,735471%p,16),
    monomial_x(xvars,vars,17,7,346104%p,17),
    monomial_x(xvars,vars,18,6,134596%p,18),
    monomial_x(xvars,vars,19,5,42504%p,19),
    monomial_x(xvars,vars,20,4,10626%p,20),
    monomial_x(xvars,vars,21,3,2024%p,21),
    monomial_x(xvars,vars,22,2,276%p,22),
    monomial_x(xvars,vars,23,1,24%p,23),
    monomial_x(xvars,&emptyvect,24,NONE,1,24)

                             });
             }

                     case 25:{
                    return std::vector<monomial_x>({ 
monomial_x(&emptyvect,vars,NONE,25,1,0),
    monomial_x(xvars,vars,1,24,25%p,1),
    monomial_x(xvars,vars,2,23,300%p,2),
    monomial_x(xvars,vars,3,22,2300%p,3),
    monomial_x(xvars,vars,4,21,12650%p,4),
    monomial_x(xvars,vars,5,20,53130%p,5),
    monomial_x(xvars,vars,6,19,177100%p,6),
    monomial_x(xvars,vars,7,18,480700%p,7),
    monomial_x(xvars,vars,8,17,1081575%p,8),
    monomial_x(xvars,vars,9,16,2042975%p,9),
    monomial_x(xvars,vars,10,15,3268760%p,10),
    monomial_x(xvars,vars,11,14,4457400%p,11),
    monomial_x(xvars,vars,12,13,5200300%p,12),
    monomial_x(xvars,vars,13,12,5200300%p,13),
    monomial_x(xvars,vars,14,11,4457400%p,14),
    monomial_x(xvars,vars,15,10,3268760%p,15),
    monomial_x(xvars,vars,16,9,2042975%p,16),
    monomial_x(xvars,vars,17,8,1081575%p,17),
    monomial_x(xvars,vars,18,7,480700%p,18),
    monomial_x(xvars,vars,19,6,177100%p,19),
    monomial_x(xvars,vars,20,5,53130%p,20),
    monomial_x(xvars,vars,21,4,12650%p,21),
    monomial_x(xvars,vars,22,3,2300%p,22),
    monomial_x(xvars,vars,23,2,300%p,23),
    monomial_x(xvars,vars,24,1,25%p,24),
    monomial_x(xvars,&emptyvect,25,NONE,1,25)

                             });
             }

                     case 26:{
                    return std::vector<monomial_x>({ 
monomial_x(&emptyvect,vars,NONE,26,1,0),
    monomial_x(xvars,vars,1,25,26%p,1),
    monomial_x(xvars,vars,2,24,325%p,2),
    monomial_x(xvars,vars,3,23,2600%p,3),
    monomial_x(xvars,vars,4,22,14950%p,4),
    monomial_x(xvars,vars,5,21,65780%p,5),
    monomial_x(xvars,vars,6,20,230230%p,6),
    monomial_x(xvars,vars,7,19,657800%p,7),
    monomial_x(xvars,vars,8,18,1562275%p,8),
    monomial_x(xvars,vars,9,17,3124550%p,9),
    monomial_x(xvars,vars,10,16,5311735%p,10),
    monomial_x(xvars,vars,11,15,7726160%p,11),
    monomial_x(xvars,vars,12,14,9657700%p,12),
    monomial_x(xvars,vars,13,13,10400600%p,13),
    monomial_x(xvars,vars,14,12,9657700%p,14),
    monomial_x(xvars,vars,15,11,7726160%p,15),
    monomial_x(xvars,vars,16,10,5311735%p,16),
    monomial_x(xvars,vars,17,9,3124550%p,17),
    monomial_x(xvars,vars,18,8,1562275%p,18),
    monomial_x(xvars,vars,19,7,657800%p,19),
    monomial_x(xvars,vars,20,6,230230%p,20),
    monomial_x(xvars,vars,21,5,65780%p,21),
    monomial_x(xvars,vars,22,4,14950%p,22),
    monomial_x(xvars,vars,23,3,2600%p,23),
    monomial_x(xvars,vars,24,2,325%p,24),
    monomial_x(xvars,vars,25,1,26%p,25),
    monomial_x(xvars,&emptyvect,26,NONE,1,26)

                             });
             }

                     case 27:{
                    return std::vector<monomial_x>({ 
monomial_x(&emptyvect,vars,NONE,27,1,0),
    monomial_x(xvars,vars,1,26,27%p,1),
    monomial_x(xvars,vars,2,25,351%p,2),
    monomial_x(xvars,vars,3,24,2925%p,3),
    monomial_x(xvars,vars,4,23,17550%p,4),
    monomial_x(xvars,vars,5,22,80730%p,5),
    monomial_x(xvars,vars,6,21,296010%p,6),
    monomial_x(xvars,vars,7,20,888030%p,7),
    monomial_x(xvars,vars,8,19,2220075%p,8),
    monomial_x(xvars,vars,9,18,4686825%p,9),
    monomial_x(xvars,vars,10,17,8436285%p,10),
    monomial_x(xvars,vars,11,16,13037895%p,11),
    monomial_x(xvars,vars,12,15,17383860%p,12),
    monomial_x(xvars,vars,13,14,20058300%p,13),
    monomial_x(xvars,vars,14,13,20058300%p,14),
    monomial_x(xvars,vars,15,12,17383860%p,15),
    monomial_x(xvars,vars,16,11,13037895%p,16),
    monomial_x(xvars,vars,17,10,8436285%p,17),
    monomial_x(xvars,vars,18,9,4686825%p,18),
    monomial_x(xvars,vars,19,8,2220075%p,19),
    monomial_x(xvars,vars,20,7,888030%p,20),
    monomial_x(xvars,vars,21,6,296010%p,21),
    monomial_x(xvars,vars,22,5,80730%p,22),
    monomial_x(xvars,vars,23,4,17550%p,23),
    monomial_x(xvars,vars,24,3,2925%p,24),
    monomial_x(xvars,vars,25,2,351%p,25),
    monomial_x(xvars,vars,26,1,27%p,26),
    monomial_x(xvars,&emptyvect,27,NONE,1,27)

                             });
             }
                     case 28:{
                    return std::vector<monomial_x>({ 
monomial_x(&emptyvect,vars,NONE,28,1,0),
    monomial_x(xvars,vars,1,27,28%p,1),
    monomial_x(xvars,vars,2,26,378%p,2),
    monomial_x(xvars,vars,3,25,3276%p,3),
    monomial_x(xvars,vars,4,24,20475%p,4),
    monomial_x(xvars,vars,5,23,98280%p,5),
    monomial_x(xvars,vars,6,22,376740%p,6),
    monomial_x(xvars,vars,7,21,1184040%p,7),
    monomial_x(xvars,vars,8,20,3108105%p,8),
    monomial_x(xvars,vars,9,19,6906900%p,9),
    monomial_x(xvars,vars,10,18,13123110%p,10),
    monomial_x(xvars,vars,11,17,21474180%p,11),
    monomial_x(xvars,vars,12,16,30421755%p,12),
    monomial_x(xvars,vars,13,15,37442160%p,13),
    monomial_x(xvars,vars,14,14,40116600%p,14),
    monomial_x(xvars,vars,15,13,37442160%p,15),
    monomial_x(xvars,vars,16,12,30421755%p,16),
    monomial_x(xvars,vars,17,11,21474180%p,17),
    monomial_x(xvars,vars,18,10,13123110%p,18),
    monomial_x(xvars,vars,19,9,6906900%p,19),
    monomial_x(xvars,vars,20,8,3108105%p,20),
    monomial_x(xvars,vars,21,7,1184040%p,21),
    monomial_x(xvars,vars,22,6,376740%p,22),
    monomial_x(xvars,vars,23,5,98280%p,23),
    monomial_x(xvars,vars,24,4,20475%p,24),
    monomial_x(xvars,vars,25,3,3276%p,25),
    monomial_x(xvars,vars,26,2,378%p,26),
    monomial_x(xvars,vars,27,1,28%p,27),
    monomial_x(xvars,&emptyvect,28,NONE,1,28),

                             });
             }

                     case 29:{
                    return std::vector<monomial_x>({ 
monomial_x(&emptyvect,vars,NONE,29,1,0),
    monomial_x(xvars,vars,1,28,29%p,1),
    monomial_x(xvars,vars,2,27,406%p,2),
    monomial_x(xvars,vars,3,26,3654%p,3),
    monomial_x(xvars,vars,4,25,23751%p,4),
    monomial_x(xvars,vars,5,24,118755%p,5),
    monomial_x(xvars,vars,6,23,475020%p,6),
    monomial_x(xvars,vars,7,22,1560780%p,7),
    monomial_x(xvars,vars,8,21,4292145%p,8),
    monomial_x(xvars,vars,9,20,10015005%p,9),
    monomial_x(xvars,vars,10,19,20030010%p,10),
    monomial_x(xvars,vars,11,18,34597290%p,11),
    monomial_x(xvars,vars,12,17,51895935%p,12),
    monomial_x(xvars,vars,13,16,67863915%p,13),
    monomial_x(xvars,vars,14,15,77558760%p,14),
    monomial_x(xvars,vars,15,14,77558760%p,15),
    monomial_x(xvars,vars,16,13,67863915%p,16),
    monomial_x(xvars,vars,17,12,51895935%p,17),
    monomial_x(xvars,vars,18,11,34597290%p,18),
    monomial_x(xvars,vars,19,10,20030010%p,19),
    monomial_x(xvars,vars,20,9,10015005%p,20),
    monomial_x(xvars,vars,21,8,4292145%p,21),
    monomial_x(xvars,vars,22,7,1560780%p,22),
    monomial_x(xvars,vars,23,6,475020%p,23),
    monomial_x(xvars,vars,24,5,118755%p,24),
    monomial_x(xvars,vars,25,4,23751%p,25),
    monomial_x(xvars,vars,26,3,3654%p,26),
    monomial_x(xvars,vars,27,2,406%p,27),
    monomial_x(xvars,vars,28,1,29%p,28),
    monomial_x(xvars,&emptyvect,29,NONE,1,29),

                             });
             }
                     case 30:{
                    return std::vector<monomial_x>({ 
monomial_x(&emptyvect,vars,NONE,30,1,0),
    monomial_x(xvars,vars,1,29,30%p,1),
    monomial_x(xvars,vars,2,28,435%p,2),
    monomial_x(xvars,vars,3,27,4060%p,3),
    monomial_x(xvars,vars,4,26,27405%p,4),
    monomial_x(xvars,vars,5,25,142506%p,5),
    monomial_x(xvars,vars,6,24,593775%p,6),
    monomial_x(xvars,vars,7,23,2035800%p,7),
    monomial_x(xvars,vars,8,22,5852925%p,8),
    monomial_x(xvars,vars,9,21,14307150%p,9),
    monomial_x(xvars,vars,10,20,30045015%p,10),
    monomial_x(xvars,vars,11,19,54627300%p,11),
    monomial_x(xvars,vars,12,18,86493225%p,12),
    monomial_x(xvars,vars,13,17,119759850%p,13),
    monomial_x(xvars,vars,14,16,145422675%p,14),
    monomial_x(xvars,vars,15,15,155117520%p,15),
    monomial_x(xvars,vars,16,14,145422675%p,16),
    monomial_x(xvars,vars,17,13,119759850%p,17),
    monomial_x(xvars,vars,18,12,86493225%p,18),
    monomial_x(xvars,vars,19,11,54627300%p,19),
    monomial_x(xvars,vars,20,10,30045015%p,20),
    monomial_x(xvars,vars,21,9,14307150%p,21),
    monomial_x(xvars,vars,22,8,5852925%p,22),
    monomial_x(xvars,vars,23,7,2035800%p,23),
    monomial_x(xvars,vars,24,6,593775%p,24),
    monomial_x(xvars,vars,25,5,142506%p,25),
    monomial_x(xvars,vars,26,4,27405%p,26),
    monomial_x(xvars,vars,27,3,4060%p,27),
    monomial_x(xvars,vars,28,2,435%p,28),
    monomial_x(xvars,vars,29,1,30%p,29),
    monomial_x(xvars,&emptyvect,30,NONE,1,30),

                             });
             }

        default:{
                    //TODO
                    std::cout << int(exponent) << " shouldn't reach here, exponent\n ";
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
            max_weight_add += to_mult[k].back().exponent; 
//            max_weight_add += to_mult[k].size() - 1; 
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

        std::cerr << *possible_child->mon_ptr << '\n';
        if(obtain_multiplicands(possible_child, current_weight + to_mult[iteration][i].exponent, to_mult, iteration + 1)){
            std::cerr << "success " << *possible_child->mon_ptr << '\n';
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
    c2_outfile << '\n' << recipe_name << '\n' << "c2seq = ";
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

    std::set<unsigned short> primes = {2,3,5,7,11,13,17};
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

                    std::cerr << to_mult.size() << '\n';
                    std::cerr << to_mult.begin()->size() << '\n';
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
                toadd[i].clear();
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
        if(actual_poly.size() == 0){
//            c2_outfile << "prime " << (int)p <<  ", " << 0 << '\n';
            c2_outfile << 0 << ",";;
        }
        else{
            int c2;
            if(actual_poly[0].second < 0){
                c2 = actual_poly[0].second += p;
            }
            else{
                c2 = actual_poly[0].second;
            }
//            c2_outfile << "prime " << (int)p <<  "," << c2 << '\n';
                c2_outfile << c2 << ",";
        }
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

