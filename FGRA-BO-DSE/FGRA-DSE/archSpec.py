import subprocess
import json

def formatSpec(sample : dict) -> dict:
    diag_iopin_connect_cg = True if sample['diag_iopin_connect_cg']==1 else False
    fclist_cg = [sample['num_itrack_per_ipin_cg'],
                 sample['num_otrack_per_opin_cg'],
                 sample['num_ipin_per_opin_cg']]
    fgra_cg_gibs = {
        "diag_iopin_connect" : diag_iopin_connect_cg,
        "fclist" : fclist_cg
    }
    del sample['num_itrack_per_ipin_cg']
    del sample['num_otrack_per_opin_cg']
    del sample['num_ipin_per_opin_cg']
    del sample['diag_iopin_connect_cg']

    diag_iopin_connect_fg = True if sample['diag_iopin_connect_fg']==1 else False
    fclist_fg = [sample['num_itrack_per_ipin_fg'],
                 sample['num_otrack_per_opin_fg'],
                 sample['num_ipin_per_opin_fg']]
    fgra_fg_gibs = {
        "diag_iopin_connect" : diag_iopin_connect_fg,
        "fclist" : fclist_fg
    }
    del sample['num_itrack_per_ipin_fg']
    del sample['num_otrack_per_opin_fg']
    del sample['num_ipin_per_opin_fg']
    del sample['diag_iopin_connect_fg']

    row = sample["fgra_num_row"]
    col = sample["fgra_num_colum"]
    max_delay_cg_gpe = sample["max_delay_cg_gpe"]
    max_delay_fg_gpe = sample["max_delay_fg_gpe"]
    del sample["max_delay_cg_gpe"]
    del sample["max_delay_fg_gpe"]
    num_input_lut = sample["num_input_lut"]
    del sample["num_input_lut"]
    operations = sample["fgra_gpe_operations"]
    max_delay_cg_iob = sample["max_delay_cg_iob"]
    max_delay_fg_iob = sample["max_delay_fg_iob"]
    iob_mode = sample["iob_mode"]
    del sample["max_delay_cg_iob"]
    del sample["max_delay_fg_iob"]
    del sample["iob_mode"]

    gib_cg = {"fgra_cg_gibs":addGIB(row,col,fgra_cg_gibs)}
    sample.update(**gib_cg)
    gib_fg = {"fgra_fg_gibs":addGIB(row,col,fgra_fg_gibs)}
    sample.update(**gib_fg)

    gpes = {"fgra_gpes":addGPE(row, col, max_delay_cg_gpe, max_delay_fg_gpe, 
                                num_input_lut, operations)}
    sample.update(**gpes)

    iobs = {"fgra_iobs":addIOB(row, iob_mode, max_delay_cg_iob, max_delay_fg_iob)}
    sample.update(**iobs)

    return sample

def addGIB(row, col, fgra_gib):
    gib = []
    for i in range(row+1):
        gib.append([])
        for j in range(col+1):
            gib[i].append(fgra_gib)
    return gib


def addGPE(row, col, max_delay_cg, max_delay_fg, num_input_lut, operations):
    gpe = []
    for i in range(row):
        gpe.append([])
        for j in range(col):
            gpe[i].append({"max_delay_cg" : max_delay_cg,
                           "max_delay_fg" : max_delay_fg,
                           "num_input_lut" : num_input_lut,
                           "operations"  : operations })
    return gpe

def addIOB(row, iob_mode, max_delay_cg, max_delay_fg):
    iob = []
    for i in range(2):
        iob.append([])
        for j in range(row):
            iob[i].append({"iob_mode" : iob_mode,
                            "has_io_fg" : True,
                            "max_delay_cg" : max_delay_cg,
                            "max_delay_fg" : max_delay_fg })
    return iob

def formatSpec_hete(sample : dict) -> dict:
    diag_iopin_connect_cg = True if sample['diag_iopin_connect_cg']==1 else False
    fclist_cg = [sample['num_itrack_per_ipin_cg'],
                 sample['num_otrack_per_opin_cg'],
                 sample['num_ipin_per_opin_cg']]
    fgra_cg_gibs = {
        "diag_iopin_connect" : diag_iopin_connect_cg,
        "fclist" : fclist_cg
    }
    del sample['num_itrack_per_ipin_cg']
    del sample['num_otrack_per_opin_cg']
    del sample['num_ipin_per_opin_cg']
    del sample['diag_iopin_connect_cg']
    diag_iopin_connect_cg_l = True if sample['diag_iopin_connect_cg_l']==1 else False
    fclist_cg_l = [sample['num_itrack_per_ipin_cg_l'],
                 sample['num_otrack_per_opin_cg_l'],
                 sample['num_ipin_per_opin_cg_l']]
    fgra_cg_gibs_l = {
        "diag_iopin_connect" : diag_iopin_connect_cg_l,
        "fclist" : fclist_cg_l
    }
    del sample['num_itrack_per_ipin_cg_l']
    del sample['num_otrack_per_opin_cg_l']
    del sample['num_ipin_per_opin_cg_l']
    del sample['diag_iopin_connect_cg_l']

    diag_iopin_connect_fg = True if sample['diag_iopin_connect_fg']==1 else False
    fclist_fg = [sample['num_itrack_per_ipin_fg'],
                 sample['num_otrack_per_opin_fg'],
                 sample['num_ipin_per_opin_fg']]
    fgra_fg_gibs = {
        "diag_iopin_connect" : diag_iopin_connect_fg,
        "fclist" : fclist_fg
    }
    del sample['num_itrack_per_ipin_fg']
    del sample['num_otrack_per_opin_fg']
    del sample['num_ipin_per_opin_fg']
    del sample['diag_iopin_connect_fg']
    diag_iopin_connect_fg_l = True if sample['diag_iopin_connect_fg_l']==1 else False
    fclist_fg_l = [sample['num_itrack_per_ipin_fg_l'],
                 sample['num_otrack_per_opin_fg_l'],
                 sample['num_ipin_per_opin_fg_l']]
    fgra_fg_gibs_l = {
        "diag_iopin_connect" : diag_iopin_connect_fg_l,
        "fclist" : fclist_fg_l
    }
    del sample['num_itrack_per_ipin_fg_l']
    del sample['num_otrack_per_opin_fg_l']
    del sample['num_ipin_per_opin_fg_l']
    del sample['diag_iopin_connect_fg_l']

    row = sample["fgra_num_row"]
    col = sample["fgra_num_colum"]
    max_delay_cg_gpe = sample["max_delay_cg_gpe"]
    max_delay_fg_gpe = sample["max_delay_fg_gpe"]
    del sample["max_delay_cg_gpe"]
    del sample["max_delay_fg_gpe"]
    max_delay_cg_gpe_l = sample["max_delay_cg_gpe_l"]
    max_delay_fg_gpe_l = sample["max_delay_fg_gpe_l"]
    del sample["max_delay_cg_gpe_l"]
    del sample["max_delay_fg_gpe_l"]
    num_input_lut = sample["num_input_lut"]
    del sample["num_input_lut"]
    num_input_lut_l = sample["num_input_lut_l"]
    del sample["num_input_lut_l"]
    max_delay_cg_iob = sample["max_delay_cg_iob"]
    max_delay_fg_iob = sample["max_delay_fg_iob"]
    iob_mode = sample["iob_mode"]
    del sample["max_delay_cg_iob"]
    del sample["max_delay_fg_iob"]
    del sample["iob_mode"]

    gib_cg = {"fgra_cg_gibs":addGIBHete(row,col,fgra_cg_gibs, fgra_cg_gibs_l)}
    sample.update(**gib_cg)
    gib_fg = {"fgra_fg_gibs":addGIBHete(row,col,fgra_fg_gibs, fgra_fg_gibs_l)}
    sample.update(**gib_fg)

    gpes = {"fgra_gpes":addGPEHete(row, col, max_delay_cg_gpe, max_delay_fg_gpe, 
                                max_delay_cg_gpe_l, max_delay_fg_gpe_l, 
                                num_input_lut, num_input_lut_l)}
    sample.update(**gpes)

    iobs = {"fgra_iobs":addIOB(row, iob_mode, max_delay_cg_iob, max_delay_fg_iob)}
    sample.update(**iobs)

    return sample

def addGIBHete(row, col, fgra_gib, fgra_gib_l):
    gib = []
    for i in range(row+1):
        gib.append([])
        if(i==0 or i==row):
            for j in range(col+1):
                gib[i].append(fgra_gib_l)
        else:
            for j in range(col+1):
                if(j==0 or j==col):
                    gib[i].append(fgra_gib_l)
                else:
                    gib[i].append(fgra_gib)
    return gib

def addGPEHete(row, col, max_delay_cg, max_delay_fg, 
               max_delay_cg_l, max_delay_fg_l, 
               num_input_lut, num_input_lut_l):
    assert row%2 == 0 and col%2 == 0 and row >= 6 and col >= 6
    gpe = []
    opD1 = ["PASS", "ADD", "SUB", "MUL", 
            "AND", "OR", "XOR", "SHL", 
            "ASHR", "LSHR", "EQ", "NE", "ULT", "SEL", "NOT", 
            "SLT", "ACC", "CACC",  "CIACC", 
            "CDIACC", "ISEL", "CISEL", "SEXT", "ZEXT"]
    opD2 = ["PASS", "ADD", "SUB", "MUL", 
            "LSHR", "SHL","NE", "AND", "OR", "XOR", 
            "EQ", "ULT", "SEL", "NOT", 
            "SLT", "ACC", "CACC",  "CIACC", 
            "CDIACC", "ISEL", "CISEL", "SEXT", "ZEXT"]
    opD3 = ["PASS", "ADD", "SUB", "MUL", 
            "EQ", "ULT", "SEL", "AND", "OR", "XOR",
            "SLT"]

    for i in range(row):
        gpe.append([])
        for j in range(col):
            if(i==0 or i==row-1 or j==0 or j==col-1):
                gpe[i].append({"max_delay_cg" : max_delay_cg_l,
                            "max_delay_fg" : max_delay_fg_l,
                            "num_input_lut" : num_input_lut_l,
                            "operations"  : opD3 })
            elif(i==1 or i==row-2 or j==1 or j==col-2):
                gpe[i].append({"max_delay_cg" : max_delay_cg,
                            "max_delay_fg" : max_delay_fg,
                            "num_input_lut" : num_input_lut,
                            "operations"  : opD2 })
            else:
                gpe[i].append({"max_delay_cg" : max_delay_cg,
                            "max_delay_fg" : max_delay_fg,
                            "num_input_lut" : num_input_lut,
                            "operations"  : opD1 })
    return gpe

def is_file_empty(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
        return not content.strip()

def run_fgra():
    detect1 = subprocess.run('cd ../fgra-mg && ./run.sh', shell=True).returncode
    if(detect1==1):raise TypeError

    detect2 = subprocess.run('cp ../fgra-mg/area.txt ./sampleEva',shell=True).returncode
    if(detect2==1):raise TypeError
    if is_file_empty('../fgra-mg/area.txt') : raise TypeError
    detect3 = subprocess.run('rm ../fgra-mg/area.txt',shell=True).returncode
    if(detect3==1):raise TypeError

    detect4 = subprocess.run('cd ../fgra-compiler/fgra-compiler && ./run.sh', shell=True).returncode
    
    detect5 = subprocess.run('cp ../fgra-compiler/fgra-compiler/result/ii.txt ./sampleEva',shell=True).returncode
    if(detect5==1):raise TypeError
    if is_file_empty('../fgra-compiler/fgra-compiler/result/ii.txt') : raise TypeError
    detect6 = subprocess.run('rm ../fgra-compiler/fgra-compiler/result/ii.txt',shell=True).returncode
    if(detect6==1):raise TypeError

    detect7 = subprocess.run('cp ../fgra-compiler/fgra-compiler/result/mappingFailureRate.txt ./sampleEva',shell=True).returncode
    if(detect7==1):raise TypeError
    if is_file_empty('../fgra-compiler/fgra-compiler/result/mappingFailureRate.txt') : raise TypeError
    detect8 = subprocess.run('rm ../fgra-compiler/fgra-compiler/result/mappingFailureRate.txt',shell=True).returncode
    if(detect8==1):raise TypeError

    detect9 = subprocess.run('cp ../fgra-compiler/fgra-compiler/result/latency.txt ./sampleEva',shell=True).returncode
    if(detect9==1):raise TypeError
    if is_file_empty('../fgra-compiler/fgra-compiler/result/latency.txt') : raise TypeError
    detect10 = subprocess.run('rm ../fgra-compiler/fgra-compiler/result/latency.txt',shell=True).returncode
    if(detect10==1):raise TypeError

    detect11 = subprocess.run('cp ../fgra-compiler/fgra-compiler/result/usage.txt ./sampleEva',shell=True).returncode
    if(detect11==1):raise TypeError
    if is_file_empty('../fgra-compiler/fgra-compiler/result/usage.txt') : raise TypeError
    detect12 = subprocess.run('rm ../fgra-compiler/fgra-compiler/result/usage.txt',shell=True).returncode
    if(detect12==1):raise TypeError

# def run_hete():
#     detect1 = subprocess.run('cd ../fgra-mg && ./run.sh', shell=True).returncode
#     if(detect1==1):raise TypeError
#     detect2 = subprocess.run('cp ../fgra-mg/area.txt ./sampleEva',shell=True).returncode
#     if(detect2==1):raise TypeError
#     detect3 = subprocess.run('rm ../fgra-mg/area.txt',shell=True).returncode
#     if(detect3==1):raise TypeError
#     detect4 = subprocess.run('cd ../fgra-compiler && ./run_hete.sh', shell=True).returncode
    
#     detect5 = subprocess.run('cp ../fgra-compiler/result/ii.txt ./sampleEva',shell=True).returncode
#     if(detect5==1):raise TypeError
#     detect6 = subprocess.run('rm ../fgra-compiler/result/ii.txt',shell=True).returncode
#     if(detect6==1):raise TypeError
#     detect7 = subprocess.run('cp ../fgra-compiler/result/mappingFailureRate.txt ./sampleEva',shell=True).returncode
#     if(detect7==1):raise TypeError
#     detect8 = subprocess.run('rm ../fgra-compiler/result/mappingFailureRate.txt',shell=True).returncode
#     if(detect8==1):raise TypeError


def run_fgra_flow():
    print('---------------------------Design Space Exploring---------------------------')
    try:
        run_fgra()
    except TypeError:
        print("--------RUNNING ERROR--------")
        with open('./sampleEva/area.txt', 'w') as a:
            a.write("2000000")
        with open('./sampleEva/ii.txt', 'w') as l:   
            l.write("100")
        with open('./sampleEva/mappingFailureRate.txt', 'w') as m:   
            m.write("1")
        with open('./sampleEva/latency.txt', 'w') as p:   
            p.write("100")
        with open('./sampleEva/usage.txt', 'w') as p:   
            p.write("0")

# def run_hete_fgra():
#     print('---------------------------Design Space Exploring---------------------------')
#     try:
#         run_hete()
#     except TypeError:
#         print("--------RUNNING ERROR--------")
#         with open('./sampleEva/area.txt', 'w') as a:
#             a.write("5000000")
#         with open('./sampleEva/ii.txt', 'w') as l:   
#             l.write("100")
#         with open('./sampleEva/mappingFailureRate.txt', 'w') as m:   
#             m.write("1")
#         with open('./sampleEva/latency.txt', 'w') as p:   
#             p.write("100")

def specGenerate(spec:dict):
    try: 
        #re1 = subprocess.run('rm ../fgra-mg/src/main/resources/fgra_spec.json',shell=True).returncode
        with open('fgra_spec.json', 'w') as f:
            json.dump(spec, f, indent=4)
        re2 = subprocess.run('cp fgra_spec.json ../fgra-mg/src/main/resources/', shell=True)
        if(re2==1): raise TypeError
    except TypeError:
        print("fgra_spec file generation error!")