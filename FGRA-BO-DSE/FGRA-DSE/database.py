import numpy  as np
import json
import subprocess
import time
from numpy import *
from hebo.design_space.design_space import DesignSpace
from hebo.optimizers.hebo import HEBO

start_time = time.time()

allSample = {}

params = [
    {'name' : 'num_row',   'type' : 'int', 'lb' : 4,  'ub' : 7 },
    {'name' : 'num_colum', 'type' : 'int', 'lb' : 4,  'ub' : 7 },
    {'name' : 'num_track', 'type' : 'int', 'lb' : 1,  'ub' : 4 },
    {'name' : 'max_delay', 'type' : 'int', 'lb' : 2,  'ub' : 7 },
    {'name' : 'num_ipin_side1', 'type' : 'int', 'lb' : 0,  'ub' : 2  },
    {'name' : 'num_ipin_side2', 'type' : 'int', 'lb' : 0,  'ub' : 2  },
    {'name' : 'num_ipin_side3', 'type' : 'int', 'lb' : 0,  'ub' : 2  },
    {'name' : 'num_otrack_side1', 'type' : 'int', 'lb' : 0,  'ub' : 4 },
    {'name' : 'num_otrack_side2', 'type' : 'int', 'lb' : 0,  'ub' : 4 },
    {'name' : 'num_otrack_side3', 'type' : 'int', 'lb' : 0,  'ub' : 4 },
    {'name' : 'num_otrack_side4', 'type' : 'int', 'lb' : 0,  'ub' : 4 },
    {'name' : 'num_itrack_side1', 'type' : 'int', 'lb' : 0,  'ub' : 4 },
    {'name' : 'num_itrack_side2', 'type' : 'int', 'lb' : 0,  'ub' : 4 },
    {'name' : 'num_itrack_side3', 'type' : 'int', 'lb' : 0,  'ub' : 4 },
    {'name' : 'num_itrack_side4', 'type' : 'int', 'lb' : 0,  'ub' : 4 },
]

op = {  
        "track_reged_mode" : 1,
        "num_rf_reg"       : 1,
        "data_width"       : 32,
        "memory_depth"     : 64,
        "num_mem_perin"    : 1,
        "num_mem_out"      : 1,
        "max_II"           : 8,
        "cfg_data_width"   : 32,
        "cfg_addr_width"   : 18,
        "cfg_blk_offset"   : 2,
        "operations"       : ["PASS", "ADD", "SUB", "MUL", "AND", "OR", "XOR"]
    }

def addGIB(row,col,opin_2_ipin_connect_flexibility,
           opin_2_otrack_connect_flexibility,itrack_2_ipin_connect_flexibility):
    gib = []
    for i in range(row+1):
        gib.append([])
        for j in range(col+1):
            gib[i].append({"opin_2_ipin_connect_flexibility" : opin_2_ipin_connect_flexibility,
                           "opin_2_otrack_connect_flexibility" : opin_2_otrack_connect_flexibility,
                           "itrack_2_ipin_connect_flexibility" : itrack_2_ipin_connect_flexibility})
    return gib


def addGPE(row,col,num_rf_reg,operations,max_delay):
    gpe = []
    for i in range(row):
        gpe.append([])
        for j in range(col):
            gpe[i].append({"num_rf_reg"  : num_rf_reg,
                           "operations"  : operations,
                           "max_delay"   : max_delay,
                           "from_dir"    : [4,5,7,6],
                           "to_dir"      : [4,5,7,6]})
    return gpe

def addLSU(row,max_delay,memory_depth,num_mem_perin,num_mem_out):
    lsu = []
    for i in range(row):
        lsu.append([])
        for j in range(2):
            lsu[i].append({"max_delay" : max_delay,
                         "memory_depth" : memory_depth,
                         "num_mem_perin" : num_mem_perin,
                         "num_mem_out" : num_mem_out})
    return lsu

def formatSpec(sample : dict) -> dict:
    opin_2_ipin_connect_flexibility = {
        'num_ipin_side1' : sample['num_ipin_side1'],
        'num_ipin_side2' : sample['num_ipin_side2'],
        'num_ipin_side3' : sample['num_ipin_side3']
    }
    del sample['num_ipin_side1']
    del sample['num_ipin_side2']
    del sample['num_ipin_side3']
    sample['opin_2_ipin_connect_flexibility'] = opin_2_ipin_connect_flexibility

    opin_2_otrack_connect_flexibility = {
        'num_otrack_side1' : sample['num_otrack_side1'],
        'num_otrack_side2' : sample['num_otrack_side2'],
        'num_otrack_side3' : sample['num_otrack_side3'],
        'num_otrack_side4' : sample['num_otrack_side4']
    }
    del sample['num_otrack_side1']
    del sample['num_otrack_side2']
    del sample['num_otrack_side3']
    del sample['num_otrack_side4']
    sample['opin_2_otrack_connect_flexibility'] = opin_2_otrack_connect_flexibility

    itrack_2_ipin_connect_flexibility = {
        'num_itrack_side1' : sample['num_itrack_side1'],
        'num_itrack_side2' : sample['num_itrack_side2'],
        'num_itrack_side3' : sample['num_itrack_side3'],
        'num_itrack_side4' : sample['num_itrack_side4']
    }
    del sample['num_itrack_side1']
    del sample['num_itrack_side2']
    del sample['num_itrack_side3']
    del sample['num_itrack_side4']
    sample['itrack_2_ipin_connect_flexibility'] = itrack_2_ipin_connect_flexibility

    row = sample["num_row"]
    col = sample["num_colum"]
    opin_2_ipin_connect_flexibility = sample['opin_2_ipin_connect_flexibility']
    opin_2_otrack_connect_flexibility = sample['opin_2_otrack_connect_flexibility']
    itrack_2_ipin_connect_flexibility = sample['itrack_2_ipin_connect_flexibility']
    num_rf_reg = sample['num_rf_reg']
    operations = sample['operations']
    max_delay = sample['max_delay']
    memory_depth = sample['memory_depth']
    num_mem_perin = sample['num_mem_perin']
    num_mem_out = sample['num_mem_out']
    GIB = {"gibs":addGIB(row,col,opin_2_ipin_connect_flexibility,
                         opin_2_otrack_connect_flexibility,
                         itrack_2_ipin_connect_flexibility)}
    sample.update(**GIB)
    GPE = {"gpes":addGPE(row,col,num_rf_reg,operations,max_delay)}
    sample.update(**GPE)
    LSU = {"lsus":addLSU(row,max_delay,memory_depth,num_mem_perin,num_mem_out)}
    sample.update(**LSU)

    return sample

def run_cgra():
    detect1 = subprocess.run('cd ../cgra-mg && ./run.sh', shell=True).returncode
    detect2 = subprocess.run('cp ../cgra-mg/area.txt .',shell=True).returncode
    detect3 = subprocess.run('rm ../cgra-mg/area.txt',shell=True).returncode
    detect4 = subprocess.run('cd ../cgra-compiler && ./run.sh', shell=True).returncode
    detect5 = subprocess.run('cp ../cgra-compiler/result/throughput.txt .',shell=True).returncode
    detect6 = subprocess.run('rm ../cgra-compiler/result/throughput.txt',shell=True).returncode
    detect7 = subprocess.run('cp ../cgra-compiler/result/ii.txt .',shell=True).returncode
    detect8 = subprocess.run('rm ../cgra-compiler/result/ii.txt',shell=True).returncode
    detect9 = subprocess.run('cp ../cgra-compiler/result/mappingFailureRate.txt .',shell=True).returncode
    detect10 = subprocess.run('rm ../cgra-compiler/result/mappingFailureRate.txt',shell=True).returncode
    if (detect1==1 or detect2==1 or detect3==1 or detect5==1 or detect6==1 or 
    detect7==1 or detect8==1 or detect9==1 or detect10==1): 
        raise TypeError

space = DesignSpace().parse(params)

opt   = HEBO(space)

print('  <<<<< DataBase Constructing >>>>>')


dataSet = space.sample(2000)
print(dataSet)
areaArray = np.array([])
throughputArray = np.array([])
iiArray = np.array([])
failureArray = np.array([])

for index,row in dataSet.iterrows():

    dataset = row.to_frame().T
    rec = dataset
    sample = rec.to_dict('records')[0]
    sample.update(**op)
    spec = formatSpec(sample)

    try:
        #re1 = subprocess.run('rm ../cgra-mg/src/main/resources/cgra_spec.json',shell=True).returncode
        with open('cgra_spec.json', 'w') as f:
            json.dump(spec, f, indent=4)
        re2 = subprocess.run('cp cgra_spec.json ../cgra-mg/src/main/resources/', shell=True)
        if(re2==1): raise TypeError
    except TypeError:
        print("cgra_spec file generation error!")
        
    try:
        run_cgra()
    except TypeError:
        print("--------RUNNING ERROR--------")
        with open('area.txt', 'w') as a:
            a.write("1000000")
        with open('throughput.txt', 'w') as p:   
            p.write("100")
        with open('ii.txt', 'w') as l:   
            l.write("100")
        with open('mappingFailureRate.txt', 'w') as m:   
            m.write("1")
    
    with open('area.txt', 'r') as a:
        area = float(a.read().strip())
    with open('throughput.txt', 'r') as p:   
        throughput = float(p.read().strip())
    with open('ii.txt', 'r') as l:   
        ii = float(l.read().strip())
    with open('mappingFailureRate.txt', 'r') as m:   
        mappingFailureRate = float(m.read().strip())
    
    areaArray = np.append(areaArray,area)
    throughputArray = np.append(throughputArray,throughput)
    iiArray = np.append(iiArray,ii)
    failureArray = np.append(failureArray,mappingFailureRate)

dataSet["area"] = areaArray
dataSet["throughput"] = throughputArray
dataSet["ii"] = iiArray
dataSet["failtureRate"] = failureArray
dataSet = dataSet.drop_duplicates()
# choose all colum
df_subset = dataSet.loc[:, :]
# save as csv
df_subset.to_csv('dataBase.csv', index=False)

end_time = time.time()
print('\n Total Time = '+str(end_time-start_time)+'\n')


