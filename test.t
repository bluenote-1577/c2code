macro mysubst[P,var, serie]
{
   private _ALL;
   coef_tabexp(P, tc, tm, var)$
   s =  sum j=1 to size(tc) { return tc[j]*serie**tm[1][j]; }$
   return s;
};

_mode= POLYV;

_affc =7$
/*vnumR a$
r/ead(kirchoffmat,a)$
n = size(a)$
vnumR kirchoff_mat[1:n]$
read(kirchoffmat,kirchoff_mat)$
*/
vnumR edge_sequence$
read(edgeseq,edge_sequence)$

maple;
with (LinearAlgebra):
d1 := ImportMatrix(dodgson1,source=csv):
d2 := ImportMatrix(dodgson2,source=csv):
for i from 1 to Dimension(d1)[1] do for j from 1 to Dimension(d1)[1] do if type(d1[i][j],string) then d1(i,j):=convert(d1[i][j],symbol) end if end do end do:
d1poly := Determinant(d1,method=multivar):
for i from 1 to Dimension(d2)[1] do for j from 1 to Dimension(d2)[1] do if type(d2[i][j],string) then d2(i,j):=convert(d2[i][j],symbol) end if end do end do:
d2poly := Determinant(d2,method=multivar):

end;

maple_get(d1poly)$
maple_get(d2poly)$
p =13$

sixinv = b_1^(p-1)*b_2^(p-1)$
dimvar old_var[1:2]$
dim old_poly[1:2]$
old_var[1] := b_1$
old_var[2] := b_2$
old_poly[1] = d1poly$
old_poly[2] = d2poly$

for j=1 to size(edge_sequence) {
//for j=1 to 3 {
    var_to_extract := crevar("a_" + str(edge_sequence[j]-1))$
    dimvar new_var[1:2^(j+1)]$
    dim new_poly[1:2^(j+1)]$

    for k=1 to size(old_var) {
        
        new_poly[2*k] = coef_ext(old_poly[k],(var_to_extract,1))$
        new_poly[2*k - 1] =coef_num(old_poly[k],(var_to_extract,0))$
        new_var[2*k - 1] :=  crevar("b_" + str(j) + str(2*k - 1))$
        new_var[2*k] := crevar("b_" + str(j) + str(2*k))$

        multiplier1 = 1$
        multiplier2 = 1$

        if(new_poly[2*k-1] == 0) then {
                multiplier1 = 0$
            };

            if(new_poly[2*k] == 0) then {
                multiplier2 = 0$
            };


        if(j == size(edge_sequence)) then {
            
//            if(multiplier1 != 0) then {
//                old_var[k];
//                new_poly[2*k-1];
//            };
            
//            if(multiplier2 != 0) then {
//                old_var[k];
//                new_poly[2*k];
//            };

//            old_var[k];
            sixinv = coef_num(sixinv,(old_var[k],new_poly[2*k-1] + new_poly[2*k]))$
        }

        else {
            sixinv = %mysubst[sixinv,[old_var[k]],multiplier2*x*new_var[2*k] + multiplier1*new_var[2*k-1]]$
        };
    };

    if(j != size(edge_sequence)) then{
        sixinv = coef_ext(sixinv,(x,p-1))$
        old_var = new_var$
        old_poly = new_poly$
    };
};

sixinv;

     



    
