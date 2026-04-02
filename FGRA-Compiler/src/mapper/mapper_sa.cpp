
#include "mapper/mapper_sa.h"

MapperSA::MapperSA(ADG* adg, int timeout, int maxIter, bool objOpt) : Mapper(adg){
    setTimeOut(timeout);
    setMaxIters(maxIter);
    setObjOpt(objOpt);
}

// MapperSA::MapperSA(ADG* adg, DFG* dfg) : Mapper(adg, dfg){}

MapperSA::MapperSA(ADG* adg, DFG* dfg, int timeout, int maxIter, bool objOpt) : Mapper(adg, dfg){
    setTimeOut(timeout);
    setMaxIters(maxIter);
    setObjOpt(objOpt);
}

MapperSA::~MapperSA(){}

// map the DFG to the ADG, mapper API
bool MapperSA::mapper(){
    bool succeed;
    if(_objOpt){ // objective optimization
        succeed = pnrSyncOpt();
    }else{
        succeed = pnrSync(MAX_TEMP, _maxIters, 3*_maxIters, true, true) == 1;
    }
    return succeed;
}


// PnR, Data Synchronization, and objective optimization
bool MapperSA::pnrSyncOpt(){
    float temp = MAX_TEMP; // temperature
    int maxIterPerTemp = _maxIters;
    int maxItersNoImprv = 3*_maxIters;  // if not improved for maxItersNoImprv, end
    int maxItersPnrSync = _maxIters; // pnrSync iteration number
    int maxItersNoImprvPnrSync = 3*maxItersPnrSync;
    int lastImprvIter = 0;
    int totalIter = 0;
    float newObj;
    float oldObj = 10.0;
    float minObj = 10.0;
    bool succeed = false;
    bool exit = false;
    ADG* adg = _mapping->getADG();
    DFG* dfg = _mapping->getDFG();
    Mapping* bestMapping = new Mapping(adg, dfg);
    Mapping* lastAcceptMapping = new Mapping(adg, dfg);
    bool initObj = true;
    spdlog::info("Start mapping optimization");
    while(temp > MIN_TEMP){
        std::cout << "-" << std::flush;
        for(int iter = 0; iter < maxIterPerTemp; iter++){     
            totalIter++;       
            // PnR and Data Synchronization 
            int res = pnrSync(temp, maxItersPnrSync, maxItersNoImprvPnrSync, (!succeed), (!succeed));
            if(res == 0){ // PnR and Data Synchronization failed
                continue;
            }else if(res == -1){ // preMapCheck fail
                exit = true;
                break;
            }
            succeed = true;            
            // Objective function
            // newObj = objFunc(_mapping);//TODO: Update when io scheduler is involed
            newObj = objFunc(_mapping, initObj);
            // std::cout << "newObj: " << newObj << " latency: " << _mapping->maxLat()<< std::endl;
            spdlog::debug("Object: {:f}", newObj);
            float difObj = newObj - oldObj;
            if(metropolis(difObj, temp)){ // accept new solution according to the Metropolis rule
                if(newObj < minObj){ // get better result
                    minObj = newObj;
                    *bestMapping = *_mapping; // cache better mapping status, ##### DEFAULT "=" IS OK #####
                    lastImprvIter = totalIter;       
                    spdlog::warn("###### Better object: {:f} ######", newObj);
                }
                *lastAcceptMapping = *_mapping; // can keep trying based on current status          
                oldObj = newObj;
            }else{
                *_mapping = *lastAcceptMapping; // restart from the cached status 
            }
            initObj = false;
        }        
        *_mapping = *bestMapping;
        if(exit || totalIter - lastImprvIter > maxItersNoImprv) break; // if not improved for long time, STOP   
        temp = annealFunc(temp); //  annealling
        maxIterPerTemp = (int)(0.95*maxIterPerTemp);
    }
    delete bestMapping; 
    delete lastAcceptMapping;
    if(succeed){
        spdlog::warn("######## Best object: {:f} ########", minObj);
        spdlog::warn("######## Best max latency: {} ########", _mapping->maxLat());
        // std::cout << "\nBest max latency: " << _mapping->maxLat() << std::endl;
    }   
    std::cout << "\n"; 
    return succeed;
}


// PnR and Data Synchronization
// return -1 : preMapCheck failed; 0 : fail; 1 : success
int MapperSA::pnrSync(float T0, int maxItersPerTemp, int maxItersNoImprv, bool modifyDfg, bool update){     
    int res = 1;
    ADG* adg = _mapping->getADG();
    int maxItersUpdate = maxItersNoImprv;
    
    // 1、初始化python接口  
    Py_Initialize();
    if(!Py_IsInitialized()){
        std::cout << "python init fail\n";
    }
    // 2、初始化python系统文件路径，保证可以访问到 .py文件
    PyRun_SimpleString("import sys");
    std::string str1 = "sys.path.append('";
    std::string str2 = PROJECT_PATH;
    std::string str3 = "/src/Py_Tools')";
    std::string pyCMD = str1 + str2 + str3;
    std::cout << pyCMD << "\n";
    PyRun_SimpleString(pyCMD.c_str());

    while(true){
        res = 1;
        int validPartition = 0;
        int iterCount = 0;
        //@yuan: every time we begin to map, we need to reset the partion scheme for non-multiport IOB Node
        _mapping->updateNonMultiPortBank(true);
        while(!pnrSyncSameDfg(T0, maxItersPerTemp, maxItersNoImprv)){
            iterCount ++;
            int II = _mapping->II();
            if(runningTimeMS() > getTimeOut()){
                std::cout << "Time Out~~~~~" << std::endl;
                spdlog::info("Time out!!!!!!"); 
                res = -1;
                break;
            }
            if(iterCount == maxItersUpdate && update){//@yuan: mapping fails for long time, maybe we can consider updating the partition scheme
                iterCount = 0;
                DFG* dfg = _mapping->getDFG();
                auto multiportIOs = dfg->multiportIOs(); 
                std::string maxArray;//@yuan: the array with maximum N
                int maxN = 0;//used maximum N
                int maxII = 0;//maximum AII
                for(auto& elem : multiportIOs){
                    BankingSolution curSolution = dfg->getCurrBankingSolution(elem.first);
                    int curN = curSolution.N;
                    if(curN > maxN){
                        maxArray = elem.first;
                        maxN = curN;
                    }
                }       
                dfg->updateBankingSettings(maxArray, false);
                spdlog::warn("Mapping fails for a long time, update the partition scheme for array {0}", maxArray); 
            }
            // std::cout << "evaluate II: " << _mapping->evaluateII() << " II: " << II << std::endl;
            if(_mapping->evaluateII() > II){
                spdlog::warn("Increase II from {0} to {1}", II, II+1); 
                _mapping->setII(II+1);
                // _mapping->setII(std::min(II+1, 7));
                continue;
            }
            spdlog::info("Current total latency violation: {}", _mapping->totalViolation()); 
            spdlog::info("Current max latency violation: {}", _mapping->maxViolation());      
            if(!modifyDfg){ // cannot modify DFG, stop iteration
                res = 0;
                break;
            }          
            spdlog::warn("Insert pass-through nodes into DFG");     
            // std::cout << "Insert pass-through nodes into DFG~~~~~~~~~~~~~~~~~~~~~~" << std::endl;                       
            DFG* newDfg = new DFG();
            _mapping->insertPassDfgNodes(newDfg, _CGOnly); // insert pass-through nodes into DFG         
            // newDfg->print();                
            setDFG(newDfg, true);  //  update the _dfg and initialize      
            int numNodesNew = newDfg->nodes().size();
            spdlog::warn("DFG node number: {}", numNodesNew);                                                     
            if(!(preMapCheckPhase1(adg, newDfg) && preMapCheckPhase2(adg, newDfg))){
                res = -1;
                break;
            }
    
        }
        if(res > 0 && update){//@yuan: PnR and schedule success, check the conflicts 
            //@yuan: if no conflict, break
            //@yuan_ddp: we check the RAW dependency is satisfied first
            // if(_mapping->updateDynamicDist()){
            //     DFG* newDfg = new DFG();
            //     *newDfg = *(_mapping->getDFG());
            //     setDFG(newDfg, true);
            //     continue;
            // }
            DFG* dfg = _mapping->getDFG();
            auto multiportIOs = dfg->multiportIOs();
            auto getABC = ([] (std::vector<std::pair<int, int>> pattern){
                    std::vector<int> result;
                    int size = pattern.size();
                    if(size == 0){
                        assert(false && "no pattern?");
                    }else{
                        result.push_back(pattern[0].first);
                        for(int i = 1; i < size; i++){
                            result.push_back(pattern[i].first - pattern[i-1].first + 
                                                result.back() * pattern[i-1].second);
                        }
                    }
                    return result;
                }
            );
            bool allnoConf = true;
            for(auto &elem : multiportIOs){
                auto arrayName = elem.first;
                // auto LSNodeIDs = elem.second.first;
                // LSNodeIDs.insert(LSNodeIDs.end(), elem.second.second.begin(), elem.second.second.end());
                auto temSoluition = dfg->getCurrBankingSolution(arrayName);
                auto ctrlsteps = temSoluition.scheduledSteps;
                bool noConflict = true;
                // std::cout << "\nhandling array " << arrayName << " ------- N: " << temSoluition.N << " B: " << temSoluition.B << "\n";
                for(auto &elem : ctrlsteps){
                    int currStep = elem.first;
                    auto LSNodeIDs = elem.second;
                    // std::cout << "currStep: " << currStep << " node size: " << LSNodeIDs.size() << "\n";
                    for(int i = 0; i < LSNodeIDs.size() && noConflict; i++){
                        auto Node0 = dynamic_cast<DFGIONode*>(dfg->node(LSNodeIDs[i]));
                        auto Pattern0 =  Node0->pattern();
                        int offset0 = Node0->memOffset() + Node0->reducedMemOffset();
                        auto ABC_0 = getABC(Pattern0);
                        auto srcNodeOpLat = _mapping->getDFG()->getOutNodes().count(LSNodeIDs[i])? 0 :  _mapping->getDFG()->node(LSNodeIDs[i])->opLatency();
                        auto lat0 = _mapping->dfgNodeAttr(LSNodeIDs[i]).lat - srcNodeOpLat;
                        for(int j = i + 1; j < LSNodeIDs.size() && noConflict; j++){
                            auto Node1 = dynamic_cast<DFGIONode*>(dfg->node(LSNodeIDs[j]));
                            auto Pattern1 = Node1->pattern();
                            int offset1 = Node1->memOffset() + Node1->reducedMemOffset();
                            auto ABC_1 = getABC(Pattern1);
                            auto dstNodeOpLat = _mapping->getDFG()->getOutNodes().count(LSNodeIDs[j])? 0 : _mapping->getDFG()->node(LSNodeIDs[j])->opLatency();
                            auto lat1 = _mapping->dfgNodeAttr(LSNodeIDs[j]).lat - dstNodeOpLat;
                            int coffs[11] = {0};
                            int counts[3] = {0};
                            for(int l = 0; l < Pattern0.size(); l++){
                                coffs[l] = ABC_0[l] / 4;
                                coffs[l+4] = ABC_1[l] / 4;
                                counts[l] = Pattern0[l].second;
                            }
                            coffs[3] = offset0 / 4;
                            coffs[7] = offset1 / 4;
                            coffs[8] = temSoluition.N;
                            coffs[9] = temSoluition.B;
                            int distance = (lat1 - lat0) / _mapping->II();
                            coffs[10] = distance;
                            // std::cout << Node1->name() << " latency: " << lat1 <<" to " <<  Node0->name() << " latency: " << lat0 << " the distance after PnR: " << distance << std::endl;
                            // for(auto test : coffs){
                            //     std::cout << test << " " ;
                            // }
                            // std::cout << "\n";
                            if(conflictpolytope_same_step(coffs, counts)){
                                noConflict = false;
                            }
                        }
                    }
                }
                if(!noConflict){
                    allnoConf = false;
                    dfg->updateBankingSettings(arrayName, false);
                    spdlog::info("[Fusion] Has memory conflict after mapping!"); 
                    std::cout << "~";
                }
            }
            if(allnoConf){
                break;
            }
        }else{
            break;            
        }        
    }
    
    //8、结束python接口初始化
    Py_Finalize();
    //@yuan: TODO: check the data size is overflow after partition
    if(res > 0){
        //@yuan: after PnR we need to check whether the available banks can accommodate the non-multiport memory access and allocate the free banks to non-multiport memory access
        //@yuan: after PnR and synchronization succeed, try to use more memory banks for non-multiport 
        _mapping->updateNonMultiPortBank(false);
        spdlog::info("PnR and data synchronization succeed!");
        spdlog::info("Max latency: {}", _mapping->maxLat());     
        spdlog::info("Final II: {}", _mapping->II());      
    }else{
        spdlog::info("PnR and Data Synchronization failed!");
    }
    return res;
}

// PnR and Data Synchronization on the same DFG, i.e. without modifying DFG
bool MapperSA::pnrSyncSameDfg(float T0, int maxIterPerTemp, int maxItersNoImprv){
    float temp = T0;
    ADG* adg = _mapping->getADG();
    DFG* dfg = _mapping->getDFG();
    Mapping* curMapping = new Mapping(adg, dfg);
    Mapping* lastAcceptMapping = new Mapping(adg, dfg);
    *curMapping = *_mapping;
    *lastAcceptMapping = *_mapping;
    // int maxItersNoImprv = 50;  // if not improved for maxItersNoImprv, end
    int lastImprvIter = 0;
    int totalIter = 0;
    bool succeed = false;
    bool exit = false;
    int newVio;
    int oldVio = 0x7fffffff;
    int minVio = 0x7fffffff;
    while(temp > MIN_TEMP){
        std::cout << "." << std::flush;
        for(int iter = 0; iter < maxIterPerTemp; iter++){
            totalIter++;
            if(runningTimeMS() > getTimeOut()){
                exit = true;
                break;
            }
            unmapSA(curMapping, temp);
            // placement and routing
            bool pnrSucceed = incrPnR(curMapping);
            if(!pnrSucceed){ // current PnR failed 
                spdlog::debug("PnR failed once!");
                // std::cout << "PnR failed once!\n";
                continue; // retry based on current status
            }
            spdlog::info("PnR succeed, start data synchronization");
            // Data synchronization : schedule the latency of DFG nodes
            curMapping->latencySchedule();
            spdlog::info("Complete data synchronization, check II and latency violation");
            newVio = (curMapping->evaluateII() - curMapping->II()) * 1000;
            // std::cout << "evalute II: " << curMapping->evaluateII() << " current II: " << curMapping->II() <<" newVio: "<< newVio<< std::endl;
            newVio += curMapping->totalViolation() * curMapping->numVioEdges(); // latency violations
            // std::cout << "totalViolation: " << curMapping->totalViolation() << " numVioEdges: " << curMapping->numVioEdges() << std::endl;
            // if(newVio == 0){
            //     int jjj = 0;
            // }
            newVio += curMapping->hasVioMemDenpendency(newVio);
            // std::cout << "totalViolation: " << curMapping->totalViolation() << " numVioEdges: " << curMapping->numVioEdges() << std::endl;
            // std::cout << "newVio: " << newVio << std::endl;
            if(newVio == 0){
                succeed = true;
                exit = true;
                *_mapping = *curMapping; // keep better mapping status, ##### DEFAULT "=" IS OK #####
                break;
            }
            spdlog::info("Violation cost: {}", newVio);
            int difVio = newVio - oldVio;
            if(metropolis(difVio, temp)){ // accept new solution according to the Metropolis rule
                if(newVio < minVio){ // get better result
                    minVio = newVio;
                    lastImprvIter = totalIter;
                    *_mapping = *curMapping; // cache better mapping status, ##### DEFAULT "=" IS OK #####
                    spdlog::info("#### Smaller violation: {} ####", minVio);
                }
                *lastAcceptMapping = *curMapping; // can keep trying based on current status            
                oldVio = newVio;
            }else{
                *curMapping = *lastAcceptMapping; // keep trying based on last accept status  
            }
        }
        if(exit || totalIter - lastImprvIter >= maxItersNoImprv) break;
        // keep trying based on last loacl optimal status 
        // _mapping->clearADGNodeUsage();
        *curMapping = *_mapping; 
        *lastAcceptMapping = *_mapping;
        temp = annealFunc(temp); //  annealling
        maxIterPerTemp = (int)(0.95*maxIterPerTemp);
    }
    delete curMapping; 
    delete lastAcceptMapping;      
    return succeed;
}

// PnR with SA temperature(max = 100)
int MapperSA::pnr(Mapping* mapping, int temp){
    unmapSome(mapping, temp);
    return incrPnR(mapping);
}


// unmap some DFG nodes
/*void MapperSA::unmapSome(Mapping* mapping, int temp){
    for(auto& elem : mapping->getDFG()->nodes()){
        auto node = elem.second;
        if((rand()%MAX_TEMP < temp) && mapping->isMapped(node)){
            mapping->unmapDfgNode(node);
        }
    }
}*/

// unmap some DFG nodes with SA temperature(max = 100)
void MapperSA::unmapSA(Mapping* mapping, float temp){
    for(auto& elem : mapping->getDFG()->nodes()){
        auto node = elem.second;
        if((randfloat() < temp/MAX_TEMP) && mapping->isMapped(node)){
            mapping->unmapDfgNode(node);
        }
    }
    // auto dfg = mapping->getDFG();
    // int N = mapping->numNodeMapped();
    // int cnt = 0;
    // for(int i = 0; i < N; i++){
    //     if(randfloat() < temp/MAX_TEMP){
    //         cnt++;
    //     }
    // }
    // int resvNum = N - cnt; // nodes keeping previous mapped location
    // int numNodes = dfgNodeIdPlaceOrder.size();
    // for(int i = resvNum; i < numNodes; i++){     
    //     auto dfgNode = dfg->node(dfgNodeIdPlaceOrder[i]);
    //     if(mapping->isMapped(dfgNode)){
    //         mapping->unmapDfgNode(dfgNode);
    //     }
    // }
}

// incremental PnR, try to map all the left DFG nodes based on current mapping status
int MapperSA::incrPnR(Mapping* mapping){
    auto dfg = mapping->getDFG();
    // // cache DFG node IDs
    // std::vector<int> dfgNodeIds;
    // for(auto node : dfg->topoNodes()){ // topological order
    //     dfgNodeIds.push_back(node->id());
    //     // std::cout << node->id() << ", ";
    // }
    // // std::cout << std::endl;
    // // sort DFG nodes according to their candidate numbers
    // // std::random_shuffle(dfgNodeIds.begin(), dfgNodeIds.end()); // randomly sort will cause long routing paths
    // std::stable_sort(dfgNodeIds.begin(), dfgNodeIds.end(), [&](int a, int b){
    //     return candidatesCnt[a] <  candidatesCnt[b];
    // });
    // for(auto id : dfgNodeIds){ // topological order
    //     std::cout << id << "(" << candidatesCnt[id] << ")" << ", ";
    // }
    // std::cout << std::endl;
    // start mapping
    for(int id : dfgNodeIdPlaceOrder){     
        /*if(dfg->isIONode(id)){ // IO node is mapped during mapping computing nodes
            continue;
        }  */ 
        auto dfgNode = dfg->node(id);
        spdlog::debug("Mapping DFG node {0}, id: {1}", dfgNode->name(), id);    
        // std::cout << "Mapping DFG node: " << dfgNode->name() << std::endl; 
        if(!mapping->isMapped(dfgNode)){
            // find candidate ADG nodes for this DFG node
            auto nodeCandidates = findCandidates(mapping, dfgNode, 30, 10);
            //@yuan: for the test, delete when the code finishing
            // if(dfgNode->operation() == "LUT"){
            //     std::cout << " LUT Node candidate size: " << nodeCandidates.size() << std::endl;
            //     //exit(0);
            // }
            // if(dfgNode->operation() == "OUTPUT"){
            //     std::cout << " OUTPUT Node: " <<dfgNode->name() << " candidate size: " << nodeCandidates.size() << std::endl;
            //     //exit(0);
            // }
            // std::cout << "Mapping DFG node: " << dfgNode->name() <<" with " << nodeCandidates.size()<< " candidates" << std::endl; 
            if(nodeCandidates.empty() || tryCandidates(mapping, dfgNode, nodeCandidates) == -1){
                spdlog::debug("Cannot map DFG node {0} : {1} with {2} candidates", dfgNode->id(), dfgNode->name(), nodeCandidates.size());
                // Graphviz viz(mapping, "results");
                // viz.printDFGEdgePath();
                return false;
            }
        }
        spdlog::info("Mapping DFG node {0} : {1} to ADG node {2}", dfgNode->name(), id, mapping->mappedNode(dfgNode)->name());
    }
    return true;    
}



// try to map one DFG node to one of its candidates
// return selected candidate index
int MapperSA::tryCandidates(Mapping* mapping, DFGNode* dfgNode, const std::vector<ADGNode*>& candidates){
    // // sort candidates according to their distances with the mapped src and dst ADG nodes of this DFG node 
    // std::vector<int> sortedIdx = sortCandidates(mapping, dfgNode, candidates);
    int idx = 0;
    for(auto& candidate : candidates){
        if(mapping->mapDfgNode(dfgNode, candidate)){      
            spdlog::debug("Map DFG node {0} to ADG node {1}", dfgNode->name(), candidate->name());      
            return idx;
        }
        idx++;
        spdlog::debug("Cannot map DFG node {0} to ADG node {1}", dfgNode->name(), candidate->name());
    }
    return -1;
}

// find candidates for one DFG node based on current mapping status
std::vector<ADGNode*> MapperSA::findCandidates(Mapping* mapping, DFGNode* dfgNode, int range, int maxCandidates){
    std::vector<ADGNode*> candidates;
    int type = mapping->mappedFUType(dfgNode);
    auto dfgOp = dfgNode->operation();
    // if(dfgOp == "TCLOAD"){
    //     ;
    // }
    // std::cout << "find candidate for : " << dfgNode->name() << std::endl;
    for(auto& elem : mapping->getADG()->nodes()){
        auto adgNode = elem.second;
        //select GPE node
        if(adgNode->type() == "GIB"){  
            continue;
        }
        // check if the DFG node operationis supported
        if((dfgOp == "INPUT" || dfgOp == "OUTPUT" || dfgOp == "LOAD" || dfgOp == "STORE" || dfgOp == "CINPUT" || dfgOp == "COUTPUT" || dfgOp == "CLOAD" || dfgOp == "CSTORE"
        || dfgOp == "TLOAD" || dfgOp == "TSTORE" || dfgOp == "TCLOAD" || dfgOp == "TCSTORE") && adgNode->type() == "IOB"){
            // std::cout << "DFGNode op: " << dfgOp << std::endl;
            IOBNode* iobNode = dynamic_cast<IOBNode*>(adgNode);
            if(!iobNode->opCapable(dfgOp)){
                continue;
            }
            if(!mapping->isMapped(iobNode, type)){
                // std::cout << "add 1 candidate: " << iobNode->id() << std::endl;
                auto dfg = mapping->getDFG();
                auto adg = mapping->getADG();
                int dfgNodeId = dfgNode->id();
                bool isMultiportIo = dfg->isMultiportIoNode(dfgNodeId);
                int iobIdx = dynamic_cast<IOBNode*>(adgNode)->index();
                const std::vector<int>& iobGrp = adg->getIobGrp(iobIdx); // @yuan: get the IOB that is connected to the same bank as the current adgNode
                std::string memRefName = dynamic_cast<DFGIONode*>(dfgNode)->memRefName();
                auto& multportIobs = mapping->multportIobs(); // get the multiport IO assignment
                if(isMultiportIo && !multportIobs[memRefName].empty()){// this multiport IO has been assigned to an IOB Group
                    int assignedIobId = *multportIobs[memRefName].begin();
                    int assignedIobIdx = dynamic_cast<IOBNode*>(adg->node(assignedIobId))->index(); //@yuan: IOB index starts from 0
                    if(std::find(iobGrp.begin(), iobGrp.end(), assignedIobIdx) == iobGrp.end()){ // this IOB not connected to the same group, i.e. the same multiport IO is assigned to other bank before
                        continue;
                    } 
                }else{
                    int preserveNum = 1;
                    if(isMultiportIo){
                        preserveNum = dfg->getMultiportNum(dfgNode, memRefName); //@yuan: get the number of multiport of this IOB (i.e. the number of load/store operations to the same SPM)
                    }
                    // std::cout << "memRefName: " << memRefName << " preserveNum: " << preserveNum << std::endl;
                    int used = 0; // @yuan: the number of used IOB on current bank
                    std::set<std::string> mapped_multiport;
                    for(int elem : iobGrp){ // @yuan: for each index of the same bank IOB
                        int curId = adg->getIobIdFromIdx(elem);
                        IOBNode* curNode = dynamic_cast<IOBNode*>(adg->node(curId));              
                        if(mapping->isMapped(curNode, type)){ // current ADGNode is mapped
                            auto curDfgNode = mapping->mappedNode(curNode, type);
                            auto curDfgIoNode = dynamic_cast<DFGIONode*>(curDfgNode);
                            std::string curName = curDfgIoNode->memRefName();
                            // std::cout << "curName: " << curName << std::endl;
                            if(dfg->isMultiportIoNode(curDfgIoNode)){ // is multiport
                                if(!mapped_multiport.count(curName)){
                                    used += dfg->getMultiportNum(curDfgNode, curName);
                                    mapped_multiport.insert(curName);
                                }
                            }else{
                                used += 1;
                            }
                        }   
                    }
                    if(preserveNum > iobGrp.size() - used){ // not have enough available IOBs 
                        continue;
                    } 
                    // std::cout << "memRefName: " << memRefName << " preserveNum: " << preserveNum << " used: " << used << std::endl;
                }
                candidates.push_back(iobNode);
            }
        }else if(adgNode->type() == "GPE"){
            GPENode* gpeNode = dynamic_cast<GPENode*>(adgNode);
            if(dfgOp == "LUT" ){
                if(!gpeNode->hasLUT()){
                    continue;
                }
                //@yuan: should consider the input of lut
                int dfgLutsize = dfgNode->LUTsize();
                int gpeFginput = gpeNode->numInputLUT();
                if(dfgLutsize > gpeFginput){
                    continue;
                }
            }else if(!gpeNode->opCapable(dfgOp)){
                continue;
            }else if(dfgNode->isPacked()){
                if(!gpeNode->isMIMD()) continue;
                bool isSupported = true;
                for(auto op: dfgNode->multiOps()){
                    if(!gpeNode->opCapable(op))
                        isSupported = false;
                }
                if(!isSupported) continue;
            }
            if(!mapping->isMapped(gpeNode, type)){
                candidates.push_back(gpeNode);
            }
        }
        
    }
    // randomly select candidates
    std::random_shuffle(candidates.begin(), candidates.end());
    int num = std::min((int)candidates.size(), range);
    candidates.erase(candidates.begin()+num, candidates.end());
    // sort candidates according to their distances with the mapped src and dst ADG nodes of this DFG node 
    std::vector<int> sortedIdx = sortCandidates(mapping, dfgNode, candidates); //TODO: Should keep the distance between candidate and IO in the cost function?
    int cdtNum = std::min(num, maxCandidates);
    std::vector<ADGNode*> sortedCandidates;
    for(int i = 0; i < cdtNum; i++){
        sortedCandidates.push_back(candidates[sortedIdx[i]]);
    }
    return sortedCandidates;
}


// get the shortest distance between ADG node and the available IOB
int MapperSA::getAdgNode2IODist(Mapping* mapping, int id){
    // shortest distance between ADG node (GPE node) and the ADG IO
    int minDist = 0x7fffffff;
    for(auto& jnode : getADG()->nodes()){            
        if(jnode.second->type() == "IOB" && mapping->isIOBFree(jnode.second)){                
            minDist = std::min(minDist, getAdgNodeDist(jnode.first, id));
        }                   
    }
    return minDist;
}

// // get the shortest distance between ADG node and the available ADG input
// int MapperSA::getAdgNode2InputDist(Mapping* mapping, int id){
//     // shortest distance between ADG node (GPE node) and the ADG IO
//     int minDist = 0x7fffffff;
//     for(auto& jnode : getADG()->nodes()){            
//         if(jnode.second->type() == "IB" && mapping->isIBAvail(jnode.second)){                
//             minDist = std::min(minDist, getAdgNodeDist(jnode.first, id));
//         }                   
//     }
//     return minDist;
// }

// // get the shortest distance between ADG node and the available ADG output
// int MapperSA::getAdgNode2OutputDist(Mapping* mapping, int id){
//     int minDist = 0x7fffffff;
//     for(auto& jnode : getADG()->nodes()){            
//         if(jnode.second->type() == "OB" && mapping->isOBAvail(jnode.second)){                
//             minDist = std::min(minDist, getAdgNodeDist(id, jnode.first));
//         }                   
//     }
//     return minDist;
// }

// sort candidates according to their distances with the mapped src and dst ADG nodes of this DFG node 
// return sorted index of candidates
std::vector<int> MapperSA::sortCandidates(Mapping* mapping, DFGNode* dfgNode, const std::vector<ADGNode*>& candidates){
    // mapped ADG node IDs of the source and destination node of this DFG node
    std::vector<int> srcAdgNodeId, dstAdgNodeId; 
    //int num2in = 0;  // connected to DFG input port
    //int num2out = 0; // connected to DFG output port
    DFG* dfg = mapping->getDFG();
    for(auto& elem : dfgNode->inputs()){
        for(auto& ins : elem.second){
            int inNodeId = ins.second.first;
            auto inNode = dfg->node(inNodeId);
            auto adgNode = mapping->mappedNode(inNode);
            if(adgNode){
                srcAdgNodeId.push_back(adgNode->id());
            //}else if(dfg->isIONode(inNodeId)){ // connected to DFG input node
            //    num2in++;
            }
        }
    }
    for(auto& elem : dfgNode->outputs()){
        for(auto& outs : elem.second){
            for(auto outNode : outs.second){
                auto adgNode = mapping->mappedNode(dfg->node(outNode.first));
                if(adgNode){
                    dstAdgNodeId.push_back(adgNode->id());
                //}else if(dfg->isIONode(outNode.first)){ // connected to DFG output node
                //    num2out++;
                }
            }   
        }     
    }
    // sum distance between candidate and the srcAdgNode & dstAdgNode & IO
    std::vector<int> sortedIdx, sumDist; // <candidate-index, sum-distance>
    for(int i = 0; i < candidates.size(); i++){
        int sum = 0;
        int cdtId = candidates[i]->id();
        ADG* adg = mapping->getADG();
        ADGNode* cdtNode = adg->node(cdtId);
        for(auto id : srcAdgNodeId){
            //std::cout << " srcNode: " << id << " to candidate node: " << cdtId << " dst: " << getAdgNodeDist(id, cdtId) << std::endl;
            sum += getAdgNodeDist(id, cdtId);
        }
        for(auto id : dstAdgNodeId){
            //std::cout << " candidate node: " << cdtId << " to dstnode: " << id << " dst: " << getAdgNodeDist(cdtId, id) << std::endl;
            sum += getAdgNodeDist(cdtId, id);
        }
        //@yuan: we encourage to use the GPE's LUT that has the same size as DFG's LUT node
        if(dfgNode->operation() == "LUT"){
            int DFGNodeSize = dfgNode->LUTsize();
            GPENode* gpe = dynamic_cast<GPENode*>(cdtNode);
            sum += (gpe->numInputLUT() - DFGNodeSize);
        }else if(dfgNode->isPacked()){ //@hw_yuan: we want the packed node is mapped to the GPE with minimum size
            GPENode* gpe = dynamic_cast<GPENode*>(cdtNode);
            sum += gpe->CGEusId().size();
        }
        //@yuan: for IO nodes, we encourage to make more multiport access into one group
        if(dfg->isIONode(dfgNode->id())){
            DFGIONode* iobDfg = dynamic_cast<DFGIONode*>(dfgNode);
            if(dfg->isMultiportIoNode(iobDfg)){
                IOBNode* iob = dynamic_cast<IOBNode*>(cdtNode);
                const std::vector<int>& iobGrp = adg->getIobGrp(iob->index()); // @yuan: get the IOB that is connected to the same bank as the current adgNode
                for(int iobIndex : iobGrp){
                    int iobId = adg->getIobIdFromIdx(iobIndex);
                    // std::cout << "IOBGrp size: "<<iobGrp.size()<<" near IOB Index: " << iobIndex <<" near IOB Id: " << iobId<< " cdtId: " << cdtId << " cdtId name: " << iob->name()<< std::endl;
                    ADGNode* iobNode = adg->node(iobId);
                    // std::cout << "near IOB Name: " << iobNode->name() << std::endl;
                    if(!mapping->isIOBFree(iobNode) && iobId != cdtId){
                        auto iobAttr = mapping->adgNodeAttr(iobId);
                        auto mappedDfgNode = iobAttr.dfgNodes;
                        for(auto elem : mappedDfgNode){
                            DFGIONode* mappedIobDfg = dynamic_cast<DFGIONode*>(elem.second);
                            if(dfg->isMultiportIoNode(mappedIobDfg) && iobDfg->memRefName() != mappedIobDfg->memRefName()){
                                sum -= 1;
                            }
                        }
                    }
                }
            }
        }
        //@yuan: resource-aware strategy, the more operations supported, the priority is lower
        int supportOp = dynamic_cast<FUNode*>(cdtNode)->operations().size();
        // sum = sum + cdtNode->getUsage(dfgNode->id());
        //int node2ioDist = getAdgNode2IODist(mapping, cdtId);
        //sum += (num2in + num2out) * node2ioDist;
        // sum += num2in * getAdgNode2InputDist(mapping, cdtId);
        // sum += num2out * getAdgNode2OutputDist(mapping, cdtId);
        sumDist.push_back(sum - supportOp);
        sortedIdx.push_back(i);
    }
    std::sort(sortedIdx.begin(), sortedIdx.end(), [&sumDist](int a, int b){
        return sumDist[a] < sumDist[b];
    });
    return sortedIdx;
}


// // try to map one DFG node to one candidates
// bool MapperSA::tryCandidate(Mapping* mapping, DFGNode* dfgNode, ADGNode* candidate){
//     DFG* dfg = mapping->getDFG();
//     bool succeed = true;
//     std::vector<DFGEdge*> routedEdges;
//     std::vector<DFGEdge*> inEdges;  // edges connected to DFG input node
//     std::vector<DFGEdge*> outEdges; // edges connected to DFG output node
//     // route the src edges whose src nodes have been mapped or connected to DFG input node
//     for(auto& elem : dfgNode->inputEdges()){
//         for(auto& edgePair : elem.second){
//             DFGEdge* edge = dfg->edge(edgePair.second);
//             DFGNode* inNode = dfg->node(edge->srcId());
//             ADGNode* adgNode = mapping->mappedNode(inNode);
//             bool routed = false;
//             if(adgNode){
//                 succeed = mapping->routeDfgEdge(edge, adgNode, candidate); // route edge between candidate and adgNode       
//                 routed = true;          
//             }else if(dfg->isIONode(edge->srcId())){ // connected to DFG input node which has not been placed
//                 succeed = mapping->routeDfgEdge(edge, candidate, true); // route edge between candidate and ADG IOB
//                 inEdges.push_back(edge);
//                 routed = true; 
//             }
//             if(!succeed){
//                 break;
//             }else if(routed){
//                 routedEdges.push_back(edge); // cache the routed edge
//             }
//         }
//         if(!succeed) break;
//     }
//     if(succeed){
//         // route the dst edge whose dst nodes have been mapped or connected to DFG output port
//         for(auto& elem : dfgNode->outputEdges()){
//             for(auto& bitEdges : elem.second){
//                 for(int outEdgeId : bitEdges.second){
//                     DFGEdge* edge = dfg->edge(outEdgeId);
//                     DFGNode* outNode = dfg->node(edge->dstId());
//                     ADGNode* adgNode = mapping->mappedNode(outNode);
//                     bool routed = false;
//                     if(adgNode){
//                         succeed = mapping->routeDfgEdge(edge, candidate, adgNode); // route edge between candidate and adgNode  
//                         routed = true;                      
//                     }else if(dfg->isIONode(edge->dstId())){ // connected to DFG output node which has not been placed
//                         succeed = mapping->routeDfgEdge(edge, candidate, false); // route edge between candidate and ADG IOB
//                         outEdges.push_back(edge);
//                         routed = true; 
//                     }
//                     if(!succeed){
//                         break;
//                     }else if(routed){
//                         routedEdges.push_back(edge); // cache the routed edge
//                     }
//                 } 
//                 if(!succeed) break;  
//             }  
//             if(!succeed) break;   
//         }
//     }
//     if(!succeed){
//         for(auto re : routedEdges){ // unroute all the routed edges
//             mapping->unrouteDfgEdge(re);
//         }
//         return false;
//     }
//     // map the DFG node to this candidate
//     bool res = mapping->mapDfgNode(dfgNode, candidate);
//     spdlog::debug("Map DFG node {0} to ADG node {1}", dfgNode->name(), candidate->name()); 
//     // map DFG input and output nodes
//     for(auto& edge : inEdges){
//         DFGNode* inDfgNode = dfg->node(edge->srcId());
//         auto inLinkAttr = mapping->dfgEdgeAttr(edge->id()).edgeLinks.begin(); // the link across the IOB
//         ADGNode* inAdgNode = inLinkAttr->adgNode;
//         res &= mapping->mapDfgNode(inDfgNode, inAdgNode);
//         spdlog::debug("Map DFG node {0} to ADG node {1}", inDfgNode->name(), inAdgNode->name()); 
//     }
//     for(auto& edge : outEdges){
//         DFGNode* outDfgNode = dfg->node(edge->dstId());
//         auto outLinkAttr = mapping->dfgEdgeAttr(edge->id()).edgeLinks.rbegin(); // the link across the IOB
//         ADGNode* outAdgNode = outLinkAttr->adgNode;
//         res &= mapping->mapDfgNode(outDfgNode, outAdgNode);
//         spdlog::debug("Map DFG node {0} to ADG node {1}", outDfgNode->name(), outAdgNode->name()); 
//     }
//     assert(res);
//     return true;
// }




// // objective funtion
// int MapperSA::objFunc(Mapping* mapping){
//     int maxEdgeLen = 0;
//     int totalEdgeLen = 0;
//     mapping->getEdgeLen(totalEdgeLen, maxEdgeLen);
//     int II = mapping->getII();
//     int obj = mapping->maxLat() * 100 +
//               maxEdgeLen * 10 + II * 20 + totalEdgeLen;
//     // std::cout << "new obj: " << obj << std::endl;            
//     return obj;
// }

float MapperSA::objFunc(Mapping* mapping, bool init){
    float w_lat = 0.05;  // weight of _dfgLat
    float w_node = 0.15; // weight of _mappedAdgNodeNum
    float w_ii = 0.60;  // weight of _II
    float w_dep = 0.20;  // weight of _ioDeps
    float obj = 1.0;
    _sched->ioSchedule(mapping);
    if(init){
        _dfgLat = mapping->maxLat();
        _mappedAdgNodeNum = mapping->getMappedAdgNodeNum();
        _II = mapping->II();
        _ioDeps = _sched->getDepCost();
        if(_ioDeps == 0){
            obj = 1.0 - w_dep;
        }
    }else{
        obj = w_lat * mapping->maxLat() / _dfgLat +
              w_node * mapping->getMappedAdgNodeNum() / _mappedAdgNodeNum +
              w_ii * mapping->II() / _II;              
        if(_ioDeps > 0){
            obj += w_dep * _sched->getDepCost() / _ioDeps;
        }else{
            obj += _sched->getDepCost();
        }
    }
    return obj;
}


// SA the probablity of accepting new solution
bool MapperSA::metropolis(float diff, float temp){
    if(diff < 0){
        return true;
    }else{
        return randfloat() < exp(-diff/temp);
    }
}
/*bool MapperSA::metropolis(double diff, double temp){
    if(diff < 0){
        return true;
    }else{
        double val = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
        return val < exp(-diff/temp);
    }
}*/


// annealing funtion
int MapperSA::annealFunc(int temp){
    float k = 0.85;
    return int(k*temp);
}