import subprocess
import time
import os
import signal

minterms = 1000000;
iteration = 0;
trigraphfile = open('trigraph.txt','r')
for line in trigraphfile:
    iteration = 0;
    minterms = 1000000;
    modes = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15];
    for i in modes:
        graphsfile = open('tempgraphs.txt','w')
        graphsfile.write(line);
        graphsfile.close()
        args = "/home/jim/co_ura/coding/c2code/gen tempgraphs.txt " + str(i) + " > log.txt"
        print(args)
        subprocess.call(args,shell=True)
        mapleargs = "maple protogen.m > maplelog.txt"
        init = time.time()
        p = subprocess.Popen(mapleargs,shell=True, preexec_fn = os.setsid)

        wasbroken = False;
        while p.poll() is None:
            if(time.time() - init > 5):
                os.killpg(os.getpgid(p.pid), signal.SIGTERM)
                print("broken");
                wasbroken = True;
                break

        file = open("maxterm_out.txt","r")
        terms = float(file.read())
        if wasbroken:
            terms = 10000000
        print(terms,iteration,i)
        if minterms > terms:
            minterms = terms
            iteration = i

    print(line)
    print(minterms,iteration)

    graphsfile = open('tempgraphs.txt','w')
    toapp = str(minterms) + "-" + line
    graphsfile.write(toapp);
    graphsfile.close()


    args = "/home/jim/co_ura/coding/c2code/gen tempgraphs.txt " + str(iteration) + " > log.txt"
    print(args)
    subprocess.call(args,shell=True)
    args = "maple protogen_seq.m > maplelog.txt" 
    print(args)
    subprocess.call(args,shell=True)






