import json
import subprocess

def copy_json_file():
    with open('cgra_spec_bestHomo.json', 'r') as f:
        data = json.load(f)
    with open('cgra_spec.json', 'w') as f:
        json.dump(data, f)

copy_json_file()
subprocess.run('cp cgra_spec.json ../cgra-mg/src/main/resources/', shell=True)
subprocess.run('cd ../cgra-mg && ./run.sh', shell=True).returncode
subprocess.run('cp ../cgra-mg/area.txt ./sampleEva',shell=True).returncode
subprocess.run('cd ../cgra-compiler && ./run_hete.sh', shell=True).returncode
subprocess.run('cp ../cgra-compiler/result/time.txt ./sampleEva',shell=True).returncode
subprocess.run('cp ../cgra-compiler/result/ii.txt ./sampleEva',shell=True).returncode
subprocess.run('cp ../cgra-compiler/result/mappingFailureRate.txt ./sampleEva',shell=True).returncode

with open('./sampleEva/area.txt','r') as f:
    area = json.load(f)
with open('./sampleEva/time.txt','r') as f:
    time = json.load(f)
with open('./sampleEva/ii.txt','r') as f:
    ii = json.load(f)
with open('./sampleEva/mappingFailureRate.txt','r') as f:
    mappingFailureRate = json.load(f)

print("\nArea is : %d", area)
print("Mapping time is : %d", time)
print("II is : %d", ii)
print("Mapping Failture rate is : %d", mappingFailureRate)


