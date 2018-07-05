import subprocess

minterms = 1000000;
iteration = 0;
for i in range(0,16):
    args = "/home/jim/co_ura/coding/c2code/gen graphs.txt " + str(i) + " > log.txt"
    print(args)
    subprocess.call(args,shell=True)
    mapleargs = "maple protogen.m > maplelog.txt"
    subprocess.call(mapleargs,shell=True)
    file = open("maxterm_out.txt","r")
    terms = float(file.read())
    print(terms,iteration,i)
    if minterms > terms:
        minterms = terms
        iteration = i


print(minterms,iteration)





