import subprocess
import time
import os
import signal
import sys
import urllib2

def internet_on():
    try:
        urllib2.urlopen('http://216.58.192.142', timeout=1)
        return True
    except urllib2.URLError as err: 
        return False

linenumb = str(1)
if len(sys.argv) > 1:
    linenumb = sys.argv[1]

minterms = 1000000;
iteration = 0;
trigraphfile = open('trigraph.txt','r')
foundline = False
for line in trigraphfile:
    if foundline == False:
        if linenumb in line:
            foundline = True
        else:
            continue
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
            if(time.time() - init > 10):
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

    if not internet_on():
        sys.exit()
        





