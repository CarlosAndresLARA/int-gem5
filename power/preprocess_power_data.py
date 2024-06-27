import os
import sys
import csv

path = "../gem5/m5out/" #Specify the absolute path to where your stats are stored
name = "stats" #Specify the name of your stats file, omit the extension 


cwd = os.getcwd()
out_dir = path + name
if not os.path.exists(out_dir):
    os.mkdir(out_dir)
print(out_dir)

tout_dir = path + name + '_t'
if not os.path.exists(tout_dir):
    os.mkdir(tout_dir)

comp_dir = path + name + '_plots'
if not os.path.exists(comp_dir):
    os.mkdir(comp_dir)

f = open(path + name + ".txt","r")

stats = {}
times = {}
it = 0
lix = 0 
t = 0

cnt = 0
while True:
    line = f.readline()
    if not line:
        break
    if line != "\n":
        sline = line.split()
        if (sline[0].find('simSeconds') != -1) or \
        (sline[0].find('armCluster.clk_domain.clock') != -1) or \
        (sline[0].find('voltage_domain.voltage') != -1) or \
        (sline[0].find('cpus0.ipc') != -1) or \
        (sline[0].find('cpus0.dcache.overallHits::total') != -1) or \
        (sline[0].find('cpus0.dcache.overallMisses::total') != -1) or \
        (sline[0].find('cpus0.committedInstType_0::IntAlu') != -1) or \
        (sline[0].find('cpus0.committedInstType_0::MemRead') != -1) or \
        (sline[0].find('cpus0.committedInstType_0::MemWrite') != -1) \
        :
            if(sline[0].find('simSeconds') != -1):
                t += 1
            if sline[0] not in stats:
                stats[sline[0]] = []
                times[sline[0]] = []
            stats[sline[0]].append(sline[1])
            times[sline[0]].append(t)
    if lix == 1000000000: #some arbitrary threshold that allows to limit the RAM usage by dividing the work into multiple "parts"
        print("Iteration " + str(it))
        if it < 10:
            pstr = '_part0'
        else:
            pstr = '_part'
        for keys,value in stats.items():
            g = open(out_dir + '/' + keys + pstr + str(it) + '.csv', 'w')
            with g:
                write = csv.writer(g)
                write.writerow(value)
            g.close()
        for keys,value in times.items():
            g = open(tout_dir + '/' + keys + pstr + str(it) + '.csv', 'w')
            with g:
                write = csv.writer(g)
                write.writerow(value)
            g.close()
        it += 1
        stats = {}
        times = {}
        lix = 0
    lix += 1
f.close()


print("Final iteration")
if it < 10:
    pstr = '_part0' + str(it)
else:
    pstr = '_part' + str(it)
if it == 0:
    pstr = ''
for keys,value in stats.items():
    f = open(out_dir + '/' + keys + pstr + '.csv', 'w')
    with f:
        write = csv.writer(f)
        write.writerow(value)
    f.close()
for keys,value in times.items():
    f = open(tout_dir + '/' + keys + pstr + '.csv', 'w')
    with f:
        write = csv.writer(f)
        write.writerow(value)
    f.close()
