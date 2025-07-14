import sys
import optuna
import logging
import numpy as np
import time
import json
from archSpec import *
from paraDefine import *
from visual import *

start_time = time.time()
global buildFromExistModel
buildFromExistModel = False
maxIt = 1

if buildFromExistModel :
    with open('history.json', 'r') as f:
        json_str = f.read()
    evaluCGRA = json.loads(json_str)
else:
    evaluCGRA = []

def objective(trial: optuna.Trial) -> float:

    sample = paramspace(trial)
    spec = formatSpec(sample)
    specGenerate(spec)
    run_fgra_flow()
    eva = usrWeighted()
    evaluCGRA.append(eva[1])
    return eva[0]

if __name__ == "__main__":

    optuna.logging.get_logger("optuna").addHandler(logging.StreamHandler(sys.stdout))
    # study_name = "homogenerous16"
    # storage_name = "sqlite:///usrModelStore/{}.db".format(study_name)
    sampler = optuna.samplers.TPESampler(multivariate=True)
    #sampler = optuna.samplers.QMCSampler() 
    #sampler = optuna.samplers.RandomSampler()
    # study = optuna.create_study(sampler=sampler, directions=["minimize"],study_name=study_name,storage=storage_name,load_if_exists=True)
    study = optuna.create_study(sampler=sampler, directions=["minimize"])
    study.optimize(objective, n_trials=maxIt, show_progress_bar=True)
    # visual(study, evaluCGRA, buildFromExistModel)
    
    print('\n <<<<< design space exploration finished >>>>>')
    end_time = time.time()
    print('\n Total Time = '+str(end_time-start_time)+'\n')
