
#include "mapper/mapper.h"


Mapper::Mapper(ADG* adg): _adg(adg) {
    initializeAdg();
    _sched = new IOScheduler(adg);
}

Mapper::Mapper(ADG* adg, DFG* dfg): _adg(adg), _dfg(dfg) {
    initializeAdg();
    initializeDfg(false);
    _mapping = new Mapping(adg, dfg);
    // initializeCandidates();
    _isDfgModified = false;
    sortDfgNodeInPlaceOrder();
    _sched = new IOScheduler(adg);
}

Mapper::~Mapper(){
    if(_mapping != nullptr){
        delete _mapping;
    }
    if(_dfgModified != nullptr){
        delete _dfgModified;
    }
}

// set DFG and initialize DFG
// modify: if the DFG is a modified one
void Mapper::setDFG(DFG* dfg, bool modify){ 
    _dfg = dfg; 
    initializeDfg(modify);
    if(_mapping != nullptr){
        delete _mapping;
    }
    _mapping = new Mapping(_adg, dfg);
    // initializeCandidates();
    if(modify){
        setDfgModified(dfg);
    } else{
        _isDfgModified = false;
    }
    sortDfgNodeInPlaceOrder();

    // detect loops on the DFG
    // getLoops(_dfg);
}


// set modified DFG and delete the old one
void Mapper::setDfgModified(DFG* dfg){
    if(_dfgModified != nullptr){
        delete _dfgModified;
    }
    _dfgModified = dfg;
    _isDfgModified = true;
}

// // set ADG and initialize ADG
// void Mapper::setADG(ADG* adg){ 
//     _adg = adg; 
//     initializeAdg();
// }


// initialize mapping status of ADG
void Mapper::initializeAdg(){
    // std::cout << "Initialize ADG\n";
    calAdgNodeDist();
    calAdgOpCnt();
}


// initialize mapping status of DFG
void Mapper::initializeDfg(bool modify){
    // topoSortDfgNodes();
    //@yuan: get the Nmax and Bmax for memory partition
    int DateinByte = _dfg->CGWidth() / 8;
    int Bmax = _adg->iobSpadBankSize() / DateinByte;
    int Nmax = _adg->iobToSpadBanks().begin()->second.size();
    int maxBankNum = _adg->numIobNodes();
    //@yuan: we need to check whether the sum of N exceeds the architecture limitation
    // if(false){//@yuan: for test packing, pass the memory partition
    if(!modify) setStartTime();
    _dfg->setOverflag(Bmax, DateinByte);
    _dfg->genBankingSolution(modify, Nmax, Bmax, maxBankNum);
    if(!modify) std::cout << "Partition Scheme generation time(s): " << runningTimeMS()/1000 << std::endl;
    // }
    // //@hw_yuan: initial the _MII
    // _dfg->setMII(std::max(_dfg->MPII(), 1));
    _dfg->topoSortNodes();  
    // _dfg->detectMultiportIOs();
    _dfg->detectBackEdgeLoops();
}


// // initialize candidates of DFG nodes
// void Mapper::initializeCandidates(){
//     Candidate cdt(_mapping, 50);
//     cdt.findCandidates();
//     // candidates = cdt.candidates(); // RETURN &
//     candidatesCnt = cdt.cnt();
// }


// // sort dfg nodes in reversed topological order
// // depth-first search
// void Mapper::dfs(DFGNode* node, std::map<int, bool>& visited){
//     int nodeId = node->id();
//     if(visited.count(nodeId) && visited[nodeId]){
//         return; // already visited
//     }
//     visited[nodeId] = true;
//     for(auto& in : node->inputs()){
//         int inNodeId = in.second.first;
//         if(inNodeId == _dfg->id()){ // node connected to DFG input port
//             continue;
//         }
//         dfs(_dfg->node(inNodeId), visited); // visit input node
//     }
//     dfgNodeTopo.push_back(_dfg->node(nodeId));
// }

// // sort dfg nodes in reversed topological order
// void Mapper::topoSortDfgNodes(){
//     std::map<int, bool> visited; // node visited status
//     for(auto& in : _dfg->outputs()){
//         int inNodeId = in.second.first;
//         if(inNodeId == _dfg->id()){ // node connected to DFG input port
//             continue;
//         }
//         dfs(_dfg->node(inNodeId), visited); // visit input node
//     }
// }


// calculate the shortest path among ADG nodes
void Mapper::calAdgNodeDist(){
    // map ADG node id to continuous index starting from 0
    std::map<int, int> _adgNodeId2Idx;
    // distances among ADG nodes
    std::vector<std::vector<int>> _adgNodeDist; // [node-idx][node-idx]
    int i = 0;
    // if the ADG node with the index is GIB
    std::map<int, bool> adgNodeIdx2GIB;
    for(auto& node : _adg->nodes()){
        adgNodeIdx2GIB[i] = (node.second->type() == "GIB");
        _adgNodeId2Idx[node.first] = i++;
    }
    int n = i; // total node number
    int inf = 0x7fffffff;
    _adgNodeDist.assign(n, std::vector<int>(n, inf));
    for(auto& node : _adg->nodes()){
        int idx = _adgNodeId2Idx[node.first];
        _adgNodeDist[idx][idx] = 0;
        for(auto& srcs : node.second->inputs()){
            for(auto& src : srcs.second){
                int srcId = src.second.first;
                int srcIdx = _adgNodeId2Idx[srcId];
                if(_adgNodeDist[srcIdx][idx] == inf){
                    int srcPort = src.second.second;
                    ADGNode* srcNode = _adg->node(srcId);
                    int dist = 1;
                    if(srcNode->type() == "GIB" && node.second->type() == "GIB"){
                        if(dynamic_cast<GIBNode*>(srcNode)->outReged(srcPort)){ // output port reged
                            dist = 2;
                        }
                    }
                    _adgNodeDist[srcIdx][idx] = dist;
                }
            }
        }
    }
    // Floyd algorithm
    for (int k = 0; k < n; ++k) {
        if(adgNodeIdx2GIB[k]){
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    if (_adgNodeDist[i][k] < inf && _adgNodeDist[k][j] < inf &&
                        _adgNodeDist[i][j] > _adgNodeDist[i][k] + _adgNodeDist[k][j]) {
                        _adgNodeDist[i][j] = _adgNodeDist[i][k] + _adgNodeDist[k][j];
                    }
                }
            }
        }        
    }

    // shortest distance between two ADG nodes (GPE/IOB nodes)
    for(auto& inode : _adg->nodes()){
        if(inode.second->type() == "GIB"){
            continue;
        }
        int i = _adgNodeId2Idx[inode.first];
        for(auto& jnode : _adg->nodes()){
            if(jnode.second->type() == "GIB" || (inode.second->type() == "IOB" && jnode.second->type() == "IOB")){
                continue;
            }
            int j = _adgNodeId2Idx[jnode.first];
            _adgNode2NodeDist[std::make_pair(inode.first, jnode.first)] = _adgNodeDist[i][j];
            // std::cout << inode.first << "," << jnode.first << ": " << _adgNodeDist[i][j] << "  ";
        }
        // std::cout << std::endl;
    }

    // // shortest distance between ADG node (GPE node) and the ADG IO
    // for(auto& inode : _adg->nodes()){
    //     if(inode.second->type() != "GPE"){
    //         continue;
    //     }
    //     int i = _adgNodeId2Idx[inode.first];
    //     int minDist2IB = inf;
    //     int minDist2OB = inf;
    //     for(auto& jnode : _adg->nodes()){            
    //         auto jtype = jnode.second->type();
    //         int j = _adgNodeId2Idx[jnode.first];
    //         if(jtype == "IB"){                
    //             minDist2IB = std::min(minDist2IB, _adgNodeDist[j][i]);
    //         }else if(jtype == "OB"){
    //             minDist2OB = std::min(minDist2OB, _adgNodeDist[i][j]);
    //         }                       
    //     }
    //     _adgNode2IODist[inode.first] = std::make_pair(minDist2IB, minDist2OB);
    //     // std::cout << inode.first << ": " << minDist2IB << "," << minDist2OB << std::endl;
    // }
}


// get the shortest distance between two ADG nodes
int Mapper::getAdgNodeDist(int srcId, int dstId){
    // return _adgNodeDist[_adgNodeId2Idx[srcId]][_adgNodeId2Idx[dstId]];
    return _adgNode2NodeDist[std::make_pair(srcId, dstId)];
}

// // get the shortest distance between ADG node and ADG input
// int Mapper::getAdgNode2InputDist(int id){
//     return _adgNode2IODist[id].first;
// }

// // get the shortest distance between ADG node and ADG input
// int Mapper::getAdgNode2OutputDist(int id){
//     return _adgNode2IODist[id].second;
// }

// calculate supported operation count of ADG
void Mapper::calAdgOpCnt(){
    for(auto& elem : _adg->nodes()){       
        if(elem.second->type() == "GPE"){
            auto node = dynamic_cast<GPENode*>(elem.second);
            for(auto& op : node->operations()){
                if(adgOpCnt.count(op)){
                    adgOpCnt[op] += 1;
                } else {
                    adgOpCnt[op] = 1;
                }                 
            }
        }
    }
}


// calculate the number of the candidates for one DFG node
int Mapper::calCandidatesCnt(DFGNode* dfgNode, int maxCandidates){
    int candidatesCnt = 0;
    if(_dfg->isIONode(dfgNode->id())){
        candidatesCnt = _adg->numIobNodes();
    }else{
        for(auto& elem : _adg->nodes()){
            auto adgNode = elem.second;
            //select GPE node
            if(adgNode->type() != "GPE"){  
                continue;
            }
            GPENode* gpeNode = dynamic_cast<GPENode*>(adgNode);
            // check if the DFG node operationis supported
            // if(dfgNode->operation() == "LUT"){
            //     if(gpeNode->hasLUT()){
            //        candidatesCnt++; 
            //     }
            // }else if(gpeNode->opCapable(dfgNode->operation())){
            //     candidatesCnt++;
            // }
            if(dfgNode->operation() == "LUT"){
                if(gpeNode->hasLUT()){
                   candidatesCnt++; 
                }
            }else if(dfgNode->isPacked()){
                if(!gpeNode->isMIMD()) continue;
                bool isSupported = true;
                for(auto op : dfgNode->multiOps()){
                    if(!gpeNode->opCapable(op)){
                        isSupported = false;
                        break;
                    }
                }
                if(isSupported) candidatesCnt++;
            }else if(gpeNode->opCapable(dfgNode->operation())){
                candidatesCnt++;
            }
            if(candidatesCnt >= maxCandidates){
                break;
            }
        }
    }
    /*if(dfgNode->operation() == "LUT"){
        std::cout << "node name: " << dfgNode->name() << " candidatecnt: " << candidatesCnt << std::endl;
    }*/
    return std::min(candidatesCnt, maxCandidates);
}

// sort the DFG node IDs in placing order
void Mapper::sortDfgNodeInPlaceOrder(){
    std::map<int, int> candidatesCnt; // <dfgnode-id, count>
    dfgNodeIdPlaceOrder.clear();
    // topological order
    // std::cout << "topo order: " << std::endl;
    for(auto nodeId : _dfg->topoNodes()){ 
        DFGNode *node = _dfg->node(nodeId);
        dfgNodeIdPlaceOrder.push_back(nodeId);
        // std::cout << nodeId << ", ";
        int cnt = calCandidatesCnt(node, 50);
        // std::cout << " name: " << node->name() << " op: "<<node->operation() << " op_cnt: "<<cnt<<std::endl;
        candidatesCnt[nodeId] = cnt;
    }
    // std::cout << std::endl;
    // sort DFG nodes according to their candidate numbers
    // std::random_shuffle(dfgNodeIds.begin(), dfgNodeIds.end()); // randomly sort will cause long routing paths
    std::stable_sort(dfgNodeIdPlaceOrder.begin(), dfgNodeIdPlaceOrder.end(), [&](int a, int b){
        return candidatesCnt[a] <  candidatesCnt[b];
    });
    // std::cout << "placer order: " << std::endl;
    // int i = 1;
    // for(int nodeidx : dfgNodeIdPlaceOrder){
    //     std::cout << i <<" name: " << _dfg->node(nodeidx)->name() << " op: "<<_dfg->node(nodeidx)->operation() << std::endl;
    //     i++;
    // }
}


// ===== timestamp functions >>>>>>>>>
void Mapper::setStartTime(){
    _start = std::chrono::steady_clock::now();
}


void Mapper::setTimeOut(double timeout){
    _timeout = timeout;
}


//get the running time in millisecond
double Mapper::runningTimeMS(){
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end-_start).count();
}



// ==== map functions below >>>>>>>>
// check if the DFG can be mapped to the ADG according to the resources
bool Mapper::preMapCheckPhase1(ADG* adg, DFG* dfg){
    std::map<int, int> IOwidthCnt;
    std::map<std::string, int> dfgOpCnt; 
    std::map<int, int> lutDfgOpCnt; 
    std::map<std::string, int> partitionedMem;
    int NSum = 0;
    for(auto& elem : dfg->nodes()){
        if(dfg->isIONode(elem.first)){
            //@yuan: for I/O node, they have only 1 grain 
            assert(!elem.second->bitWidths().empty());
            auto IONode = dynamic_cast<DFGIONode*>(elem.second);
            if(IONode->MultiportType() > 1){
                std::string MemName = IONode->memRefName();
                if(!partitionedMem.count(MemName)){
                    partitionedMem[MemName]=IONode->NumMultiportBank();
                }
                continue;//@yuan: if current node is multiport memory acessment, just record its N
            }
            int bitwidth = *IONode->bitWidths().begin();
            if(IOwidthCnt.count(bitwidth)){
                IOwidthCnt[bitwidth] += 1;
            } else {
                IOwidthCnt[bitwidth] = 1;
            }
        }else{
            auto op = elem.second->operation();
            /*auto nodename = elem.second->name();
            std::cout <<" node name: " << nodename << " op: " << op <<std::endl;*/
            if(op == "LUT"){
                // int lutSize = elem.second->LUTsize();
                // if(lutDfgOpCnt.count(lutSize)){
                //     lutDfgOpCnt[lutSize] += 1;
                // } else {
                //     lutDfgOpCnt[lutSize] = 1;
                // }
                continue;
            }
            assert(!op.empty());
            if(dfgOpCnt.count(op)){
                dfgOpCnt[op] += 1;
            } else {
                dfgOpCnt[op] = 1;
            }
        }
    }
    //@yuan:: accumulate all the N
    for(auto &elem : partitionedMem){
        NSum += elem.second;
    }
    // std::cout << "Nsum: " << NSum << std::endl;
    // std::cout << "gpe nodes num: " << adg->numGpeNodes() << " dfg node: " << dfg->nodes().size() << " ionodes: " << dfg->ioNodes().size() << " lutnodes: " << dfg->lutNodes().size() << std::endl;
    // first, check the I/O node number
    for(auto& elem : IOwidthCnt){
        // std::cout << "IO Num: " << elem.second << std::endl;
        if(adg->numIobNodes() < elem.second){ 
            std::cout << "This DFG has too many I/O nodes!\n";
            return false;
        }
    }
    // if all the IOB are multiport node, check here
    if(adg->numIobNodes() < NSum){ 
        std::cout << "This DFG has too many Multi-port I/O nodes!\n";
        return false;
    }

    // second, check the computing node number
    //@hw_yuan: doesn't check here
    // if(adg->numGpeNodes() < (dfg->nodes().size() - dfg->ioNodes().size() - dfg->lutNodes().size())){
    //     std::cout << "This DFG has too many computing nodes!\n";
    //     return false;
    // }
    // third, check if there are enough ADG nodes that can map the LUT nodes
    int adgLUTCnt = 0;
    for(auto& elem : adg->nodes()){
        if(elem.second->type() != "GPE"){
            continue;
        }
        //std::cout << "a GPE" << std::endl;
        GPENode *gpe_node = dynamic_cast<GPENode*>(elem.second);
        if(gpe_node->hasLUT()){
            adgLUTCnt ++;
        }
    }
    // std::cout << " adgLUTCnt: " << adgLUTCnt << " num lut: " << dfg->lutNodes().size() << std::endl;
    if(adgLUTCnt < dfg->lutNodes().size()){
        std::cout << "This DFG has too many lut nodes!\n";
        return false;
    }
    // fourth, check if there are enough ADG IOspad to map the multport
    auto iobToSpadBanks = adg->iobToSpadBanks().begin()->second.size();
    if(iobToSpadBanks < dfg->MPII()){
        std::cout << "This DFG has too many multport I/O nodes!\n";
        return false;
    }
    
    // supported operation count of ADG
    // std::map<std::string, int> adgOpCnt; 
    // for(auto& elem : adg->nodes()){       
    //     if(elem.second->type() == "GPE"){
    //         auto node = dynamic_cast<GPENode*>(elem.second);
    //         for(auto& op : node->operations()){
    //             if(adgOpCnt.count(op)){
    //                 adgOpCnt[op] += 1;
    //             } else {
    //                 adgOpCnt[op] = 1;
    //             }                 
    //         }
    //     }
    // }
    // operation count of DFG
    /*for(auto& elem : dfg->nodes()){
        if(dfg->isIONode(elem.first)){
            continue;
        }
        auto op = elem.second->operation();
        auto nodename = elem.second->name();
        std::cout <<" node name: " << nodename << " op: " << op <<std::endl;
        if(op == "LUT"){
            continue;
        }
        assert(!op.empty());
        if(dfgOpCnt.count(op)){
            dfgOpCnt[op] += 1;
        } else {
            dfgOpCnt[op] = 1;
        }
    }*/
    for(auto& elem : dfgOpCnt){
        if(adgOpCnt[elem.first] < elem.second){ 
            std::cout << "No enough ADG nodes to support " << elem.first << " adg_num: " << adgOpCnt[elem.first] << " op_num: " << elem.second<< std::endl;
            return false; // there should be enough ADG nodes that support this operation
        }
    }
    return true;
}

bool Mapper::preMapCheckPhase2(ADG* adg, DFG* dfg){
    // phase2, check the computing node number
    //@hw_yuan: doesn't check here
    // std::cout << "node size: " << dfg->nodes().size() << std::endl;
    if(adg->numGpeNodes() < (dfg->nodes().size() - dfg->ioNodes().size() - dfg->lutNodes().size())){
        std::cout << "This DFG has too many computing nodes!\n";
        return false;
    }
    return true;
}
// // map the DFG to the ADG
// bool Mapper::mapping(){

// }


// mapper with running time
bool Mapper::mapperTimed(std::string resultDir){
    setStartTime();
    // check if the DFG can be mapped to the ADG according to the resources
    //@yuan: fortest: if testing packing ,it will be commented 
    if(!preMapCheckPhase1(getADG(), getDFG())){
        return false;
    }
    std::cout << "fine-grain mapping, before mapper!!" << std::endl;
    // exit(0);
    std::cout << "Pre-map checking phase 1 passed!\n";
    //@hw_yuan TODO: some flag should be added to determine whether the opration-packing is enabled?
    int numMIMDGpeNodes = getADG()->numMIMDGpeNodes();
    bool enablePack = numMIMDGpeNodes > 0;
    // setStartTime();
    if(enablePack){
        auto packStart = std::chrono::steady_clock::now();
        // std::cout << "Architecture has " << numMIMDGpeNodes << " MIMD-GPEs, begin to pack!" << std::endl;
        DFG* newDfg = new DFG();
        // opsPacking(newDfg, resultDir);
        opsAAPacking(newDfg, resultDir);
        auto packEnd = std::chrono::steady_clock::now();
        double packingTime = std::chrono::duration_cast<std::chrono::milliseconds>(packEnd - packStart).count();
        std::cout << std::fixed <<  "Packing time(s): " << packingTime/1000 << std::endl;
        std::cout << "after packing node num: " << newDfg->nodes().size() << " num edge: " << newDfg->edges().size() << " num io node: " << newDfg->ioNodes().size() << std::endl;
        
    }
    // std::cout << "Packing time(s): " << runningTimeMS()/1000 << std::endl;
    // exit(0);
    if(!preMapCheckPhase2(getADG(), getDFG())){
        return false;
    }
    std::cout << "Pre-map checking phase 2 passed!\n";
    bool succeed = mapper();
    std::cout << "\nRunning time(s): " << runningTimeMS()/1000 << std::endl;
    return succeed;
}


// execute mapping, timing sceduling, visualizing, config getting
// dumpConfig : dump configuration file
// dumpMappedViz : dump mapped visual graph
// resultDir: mapped result directory
// CGOnly; coarse-only
bool Mapper::execute(bool dumpConfig, bool dumpMappedViz, std::string resultDir, bool CGOnly){
    _CGOnly = CGOnly;
    std::cout << "Start mapping >>>>>>\n";
    bool res = mapperTimed(resultDir);
    if(res){
        // std::cout << "II: " << getII() << std::endl;
        std::string dir;
        if(!resultDir.empty()){
            dir = resultDir;
        }else{
            dir = "results"; // default directory
        }
        if(dumpMappedViz){
            Graphviz viz(_mapping, dir);
            viz.drawDFG();
            viz.drawADG();
            viz.dumpDFGIO(); 
            // viz.printDFGEdgePath();
        }
        _mapping->getDFG()->deleteMemEdge();
        if(dumpConfig){
            //@yuan: generate the SoC call function
            std::ofstream func_ofs(dir + "/cgra_execute.c");
            std::ofstream call_ofs(dir + "/cgra_call.txt");
            std::ofstream bits_ofs(dir + "/config.bit");
            _sched->execute(_mapping, func_ofs, call_ofs, bits_ofs);
            // Configuration cfg(_mapping);
            // cfg.dumpCfgData(ofs);
            // _sched->execute(_mapping, ofs);
            func_ofs.close();
            call_ofs.close();
            bits_ofs.close();

        }   
        // if(dumpMappedViz){
        //     Graphviz viz(_mapping, dir);
        //     viz.drawDFG();
        //     viz.drawADG();
        //     viz.dumpDFGIO(); 
        //     // viz.printDFGEdgePath();
        // }
        // _mapping->getDFG()->deleteMemEdge();    
        std::cout << "Succeed to map DFG to ADG!<<<<<<\n";
        std::cout << "Final II = " << _mapping->II() <<" !<<<<<<\n";
        std::cout << "**************** Memory partition and shecdule results ****************\n";
        auto multiPortIO = _mapping->getDFG()->multiportIOs();
        for(auto& elem : multiPortIO){
            auto curBnakingSolution = _mapping->getDFG()->getCurrBankingSolution(elem.first);
            std::cout << "Array Name: " << elem.first << "; N: " << curBnakingSolution.N << "; B: " << curBnakingSolution.B<<"\n";
            for(auto& step : curBnakingSolution.scheduledSteps){
                std::cout << " >> Step " << step.first << "<< \n";
                for(auto& id : step.second){
                    std::cout << _mapping->getDFG()->node(id)->name() << " ";
                }
                std::cout << "\n" ;
            }
            std::cout << "\n" ;
        }
    } else{
        std::cout << "Fail to map DFG to ADG!<<<<<<\n";
    }
    return res;
}


// get max latency of the mapped DFG
int Mapper::getMaxLat()
{
    return _mapping->maxLat();
}


// @yuan: get the loops on DFG
void Mapper::getLoops(DFG* dfg){
    dfg->getLoops();
} 

// @yuan: get the II of the DFG
int Mapper::getII(){
    return _mapping->II();
}

//@hw_yuan: packing the DFG Nodes into a bigger node
void Mapper::opsPacking(DFG* newDfg, std::string resultDir){
    *newDfg = *_dfg;
    int maxPackedNode = getADG()->numMIMDGpeNodes();
    int maxNodeId = newDfg->nodes().rbegin()->first; // std::map auto sort the key
    int maxEdgeId = newDfg->edges().rbegin()->first;  
    bool hasPacked_temp = true;
    int cgWidth = _dfg->CGWidth();
    int packStage = 1;
    int packedNodeAtFirstStage = 0;
    while(packStage < 4){// since GPE has 4 ports, the maximum stage of packing is 3 (up to 4 nodes can be packed)
        int traverseIdx = 0;
        bool hasPacked = false;
        // std::cout << "topo node size: "<< newDfg->topoNodes().size() << std::endl;
        for(auto it = newDfg->topoNodes().rbegin(); it != newDfg->topoNodes().rend(); ++it){// based-on reverse topological order
            int id = *it;
            DFGNode *node = newDfg->node(id);
            traverseIdx ++;
            if(newDfg->isIONode(id) || node->accumulative() || node->initSelection() || node->operation() == "LUT" || (packStage > 1 && !node->isPacked())){ // IO/accumulative LUT node can not be packed
                continue;
            }
            //from stage 2 on, only packed node can be packed again
            if(node->isPacked()){
                //for now, the maximum inputs (coarse-grained) is 4; TODO: maybe we need to consider about the fine-grained input when adding some operation has it (excepted SEL)
                if((node->multiOps().size() != packStage) || node->numInputs(cgWidth) >= 4) continue;
            }
            std::cout << " name: " << node->name() << " op: "<<node->operation() << " id: "<<id << " traverseIdx: " << traverseIdx <<std::endl;
            std::cout << "packing stage: " << packStage << " node name: " << node->name() << std::endl;
            //select the candidate node for packing
            bool hasCandidate = false;
            DFGNode *candidateNode = nullptr;
            std::set<DFGEdge*> internalEdges;// the internal edges will be merged after packing
            int maxAddInterEdgeLat = 0;
            for(auto inEdges : node->inputEdges()){
                int width = inEdges.first;
                for(auto edgeWithSameWidth: inEdges.second){
                    int edgeId = edgeWithSameWidth.second;
                    DFGEdge *edge = newDfg->edge(edgeId); 
                    DFGNode *srcNode = newDfg->node(edge->srcId());
                    maxAddInterEdgeLat = std::max(maxAddInterEdgeLat, edge->addDelay());
                    if(edge->isBackEdge() || newDfg->isIONode(edge->srcId()) || srcNode->accumulative() || srcNode->initSelection()) continue; // can not pack two nodes have back-edge connection; can not pack the IOB/ACC node
                    std::cout << "Src Node: " << srcNode->name() << std::endl;
                    auto srcOutEdgesPerPort = srcNode->outputEdges(width);// for now, one node has only 1 output port, therefore, the width is determined
                    if(srcOutEdgesPerPort.size() > 1 || srcOutEdgesPerPort.begin()->second.size() > 2) continue;
                    for(auto outEdges : srcOutEdgesPerPort){
                        if(outEdges.second.size() == 1){//the src node can only connect to current node
                            if(isCompatible(node, srcNode)){
                                // std::cout << "Compatible 1~" << std::endl;
                                candidateNode = srcNode;
                                internalEdges.insert(edge);
                                hasCandidate = true;
                            }
                        }else{
                            for(auto e : outEdges.second){
                                if(e == edgeId) continue;
                                DFGEdge *extraEdge = newDfg->edge(e); 
                                std::cout << "extraEdge->dstId(): " << extraEdge->dstId() << " id: " << id << std::endl;
                                if(extraEdge->dstId() == id && isCompatible(node, srcNode)){
                                    // std::cout << "Compatible 2~" << std::endl;
                                    internalEdges.insert(edge);
                                    internalEdges.insert(extraEdge);
                                    candidateNode = srcNode;
                                    hasCandidate = true;
                                }
                            }
                        }
                        if(hasCandidate) goto pack;
                    }
                }
            }
            pack: if(hasCandidate){//if we find the candidate node, then just pack them here; packing 2 packed node is unsupport
                // std::cout << "begin to pack, candidate node name: " << candidateNode->name() << " id: " << candidateNode->id() << " current node: " << node->name() << " id: " << node->id()<< std::endl;
                DFGNode* newNode = new DFGNode();
                newNode->setId(++maxNodeId);
                newNode->setOperation(node->operation());//just save the last operation, for the packed node
                newNode->setOpLatency(candidateNode->opLatency() + node->opLatency() - maxAddInterEdgeLat);
                std::vector<std::string> multiOps;
                multiOps.push_back(candidateNode->operation());
                std::string name = candidateNode->operation();
                if(node->isPacked()){
                    for(auto elem : node->multiOps()){
                        // std::cout << "op: " << elem << std::endl;
                        name += " " + elem;
                        multiOps.push_back(elem);
                    }
                }else{
                    multiOps.push_back(node->operation());
                    name += " " + node->operation();
                }
                newNode->setPacked(true);
                newNode->setMultiOps(multiOps);
                newNode->setName(name + "_" +std::to_string(maxNodeId));
                std::map<int, std::set<DFGEdge*>> externalSrcInEdges;
                for(auto inEdges : candidateNode->inputEdges()){
                    int width = inEdges.first;
                    for(auto edgeWithSameWidth: inEdges.second){
                        DFGEdge* e = new DFGEdge(++maxEdgeId);// the new edge connected to new packed node
                        DFGEdge* oldEdge = newDfg->edge(edgeWithSameWidth.second);
                        // std::cout << "candidate src edge id: " << edgeWithSameWidth.second << std::endl;
                        int dstId = newNode->id();
                        int dstPort = width > 1 ? oldEdge->dstPortIdx() : 4; // for the fine-grained input, there 4 coarse-grained inputs before it
                        int srcId = oldEdge->srcId();
                        int srcPort = oldEdge->srcPortIdx();
                        newNode->addInternalAttribute(newDfg->getEuType(candidateNode->operation()), "GPE", dstPort, dstPort, width);
                        // std::cout <<"src name: " << newDfg->node(srcId)->name()<<" srcId: " << srcId << " srcPort: " << srcPort << " dstId: " << dstId << " dstPort: " << dstPort << " edge id: " << e->id()<< std::endl;
                        e->setEdge(width, srcId, srcPort, dstId, dstPort);
                        e->setBackEdge(oldEdge->isBackEdge());
                        e->setType(oldEdge->type());
                        e->setIterDist(oldEdge->iterDist());
                        e->setAddDelay(oldEdge->addDelay());
                        externalSrcInEdges[width].insert(e);
                    }
                }
                std::map<int, std::set<DFGEdge*>> externalDstInEdges;
                for(auto inEdges : node->inputEdges()){
                    int width = inEdges.first;
                    int numEdgeWithSameWidth = 0;
                    for(auto edgeWithSameWidth: inEdges.second){
                        // std::cout << "edge id: " << edgeWithSameWidth.second << std::endl;
                        DFGEdge* oldEdge = newDfg->edge(edgeWithSameWidth.second);
                        if(internalEdges.count(oldEdge)) continue;
                        // int oldAddDelay = oldEdge->addDelay(
                        DFGEdge* e = new DFGEdge(++maxEdgeId);// the new edge connected to new packed node
                        int dstId = newNode->id();
                        int srcId = oldEdge->srcId();
                        int srcPort = oldEdge->srcPortIdx();
                        // std::cout << "src: " << newDfg->node(srcId)->name() << " edge id: " << edgeWithSameWidth.second << std::endl;
                        int oldDstPort = oldEdge->dstPortIdx();//TODO: we need to record the original dstport for PnR
                        int newDstPort = 0;
                        if(width > 1){
                            newDstPort = candidateNode->numInputs(width) + numEdgeWithSameWidth + int(node->hasImm(width)) + int(node->has2ndImm());
                        }else{
                            newDstPort = 4;
                        }
                        if(node->isPacked()){
                            // bool hasAddedInternal = false;
                            for(auto& EuInputs : node->internalAttrMap()){
                                std::string opName = EuInputs.first;
                                for(auto& eachOperand : EuInputs.second){
                                    // std::cout << "opName: " << opName << " edge id: " << eachOperand.second.edgeId << " old edge id: " << oldEdge->id() << std::endl;
                                    if(eachOperand.second.srcPort == oldDstPort && eachOperand.second.width == width){
                                        int operand = eachOperand.first;
                                        int width = eachOperand.second.width;
                                        newNode->addInternalAttribute(opName, "GPE", newDstPort, operand, width);
                                        // hasAddedInternal = true;
                                        // if(hasAddedInternal) goto addNew;
                                    }
                                }
                            }
                            // addNew: if(!hasAddedInternal){
                            //     std::string latestOp = node->multiOps().back();
                            //     newNode->addInternalAttribute(newDfg->getEuType(latestOp), "GPE", newDstPort, operand, e->id(), width);
                            // }
                        }else{
                            newNode->addInternalAttribute(newDfg->getEuType(node->operation()), "GPE", newDstPort, oldDstPort, width);
                            // std::cout << "nopacked opName: " << newDfg->getEuType(node->operation()) << " edge id: " << e->id() << std::endl;
                        }
                        e->setEdge(width, srcId, srcPort, dstId, newDstPort);
                        e->setBackEdge(oldEdge->isBackEdge());
                        e->setType(oldEdge->type());
                        e->setIterDist(oldEdge->iterDist());
                        e->setAddDelay(oldEdge->addDelay() + candidateNode->opLatency() - maxAddInterEdgeLat);//additional delay equals to older additional delay + candidate node's operation's delay - maxAddInterEdgeLat
                        // std::cout <<"externalDstInEdges: src name: " << newDfg->node(srcId)->name()<<" srcId: " << srcId << " srcPort: " << srcPort << " dstId: " << dstId << " dstPort: " << newDstPort << " add delay: " << e->addDelay() << std::endl;
                        externalDstInEdges[width].insert(e);
                        numEdgeWithSameWidth++;
                    }
                }
                //we need to handle the output edges here
                //it's noting that the candidate node's output are completely connected to current node
                //hence, there is no need to record these outputs explicitly
                std::map<int, std::set<DFGEdge*>> externalDstOutEdges;
                for(auto outEdges : node->outputEdges()){
                    int width = outEdges.first;
                    for(auto edgeWithSameWidthPort: outEdges.second){
                        int srcPort = edgeWithSameWidthPort.first;
                        for(auto edgeId : edgeWithSameWidthPort.second){
                            DFGEdge* oldEdge = newDfg->edge(edgeId);
                            DFGEdge* e = new DFGEdge(++maxEdgeId);// the new output edge
                            int dstId = oldEdge->dstId();
                            int srcId = newNode->id();
                            int dstPort = oldEdge->dstPortIdx();
                            e->setEdge(width, srcId, srcPort, dstId, dstPort);
                            e->setBackEdge(oldEdge->isBackEdge());
                            e->setType(oldEdge->type());
                            e->setIterDist(oldEdge->iterDist());
                            e->setAddDelay(oldEdge->addDelay());
                            externalDstOutEdges[width].insert(e);
                        }
                    }
                }           
                //we can assign the old inter-Eu connections within this loop     
                for(auto& EuInputs : node->internalAttrMap()){
                    std::string opName = EuInputs.first;
                    for(auto& eachOperand : EuInputs.second){
                        if(eachOperand.second.srcName != "GPE" && eachOperand.second.srcName != "CONST"){
                            int operand = eachOperand.first;
                            int width = eachOperand.second.width;
                            newNode->addInternalAttribute(opName, eachOperand.second.srcName, 0, operand, width);
                        }
                    }
                }
                //finally, we handle the new inter-Eu connections 
                // std::cout << "new internal edge size: " << internalEdges.size() << std::endl;
                for(auto edge : internalEdges){
                    std::string latestOp;
                    if(node->isPacked()){
                        latestOp = node->multiOps().front();
                        int tempLat = maxAddInterEdgeLat;
                        for(auto elem : node->multiOps()){
                            tempLat -= Operations::latency(elem);
                            if(tempLat < 0){
                                latestOp = elem;
                                break;
                            }
                        }
                    }else{
                        latestOp = node->operation();
                    }
                    int dstPort = edge->dstPortIdx();
                    int width = edge->bitWidth();
                    if(maxAddInterEdgeLat == 0){
                        newNode->addInternalAttribute(newDfg->getEuType(latestOp), newDfg->getEuType(candidateNode->operation()), 0, dstPort, width);
                    }else{//for internal edge has additional delay, we need to handle it carefully
                        // std::cout << "latestOp: " << latestOp << " type: " << newDfg->getEuType(latestOp) << " node internal size: " << node->internalAttrMap().size()<< std::endl;
                        auto internalAttr = node->internalAttrMap(newDfg->getEuType(latestOp));
                        for(auto elem : internalAttr){
                            if((elem.second.srcPort == dstPort) && (elem.second.width == width)){
                                newNode->addInternalAttribute(newDfg->getEuType(latestOp), newDfg->getEuType(candidateNode->operation()), 0, elem.first, width);
                            }
                        }
                    }
                    // std::cout << "candidate node: " << candidateNode->name() << " to packed node: " << node->name() << " lastop: " << latestOp <<" type: "<< newDfg->getEuType(latestOp)<<std::endl; 
                }
                newDfg->addNode(newNode);
                //set the constant for packed node
                //for now, there are no fine-grained constant input for non-LUT node
                if(node->hasImm(cgWidth)){
                    // std::set<int> idxTemp = node->immIdxs();
                    // newNode->clearImmMultiIdxs();
                    newNode->setImm(cgWidth, std::make_pair(candidateNode->numInputs(cgWidth), node->immValue(cgWidth)));// the record imm Idx shift to right
                    if(node->has2ndImm()){
                        newNode->set2ndImm(node->get2ndImm());
                        newNode->set2ndImmIdx(newNode->immIdx(cgWidth) + 1);
                    }
                    //for old-node has constant, we connect all the Eus to this constant 
                    if(node->isPacked()){// the node has 2 constant, it must be packed node
                        for(auto& EuInputs : node->internalAttrMap()){
                            std::string opName = EuInputs.first;
                            for(auto& eachOperand : EuInputs.second){
                                if(eachOperand.second.srcName == "CONST"){
                                    int operand = eachOperand.first;
                                    // int width = eachOperand.second.width;
                                    newNode->addInternalAttribute(opName, "CONST", candidateNode->numInputs(cgWidth), operand, cgWidth);
                                }else if(eachOperand.second.srcName == "Second_Const"){// if one node has second constant, it must have 2 constants
                                    int operand = eachOperand.first;
                                    newNode->addInternalAttribute(opName, "Second_Const", candidateNode->numInputs(cgWidth) + 1, operand, cgWidth);
                                }
                            }
                        }
                    }else{
                        newNode->addInternalAttribute(newDfg->getEuType(node->operation()), "CONST", candidateNode->numInputs(cgWidth), node->immIdx(cgWidth), cgWidth);
                    }
                    if(candidateNode->hasImm(cgWidth)){
                        if(candidateNode->immValue(cgWidth) == node->immValue(cgWidth)){
                            newNode->addInternalAttribute(newDfg->getEuType(candidateNode->operation()), "CONST", candidateNode->numInputs(cgWidth), candidateNode->immIdx(cgWidth), cgWidth);
                        }else{
                            if(node->has2ndImm()){// old node has seconde constant, using it
                                newNode->addInternalAttribute(newDfg->getEuType(candidateNode->operation()), "Second_Const", candidateNode->numInputs(cgWidth) + 1, candidateNode->immIdx(cgWidth), cgWidth);
                            }else{
                                newNode->addInternalAttribute(newDfg->getEuType(candidateNode->operation()), "Second_Const", candidateNode->immIdx(cgWidth), candidateNode->immIdx(cgWidth), cgWidth);
                                newNode->set2ndImm(candidateNode->immValue(cgWidth));
                                newNode->set2ndImmIdx(candidateNode->immIdx(cgWidth));
                            }
                        }
                    }
                }else if(candidateNode->hasImm(cgWidth)){
                    newNode->setImm(cgWidth, std::make_pair(candidateNode->immIdx(cgWidth), candidateNode->immValue(cgWidth)));
                    newNode->addInternalAttribute(newDfg->getEuType(candidateNode->operation()), "CONST", candidateNode->immIdx(cgWidth), candidateNode->immIdx(cgWidth), cgWidth);
                    // newNode->setImmIdxs(candidateNode->immIdx(cgWidth));
                }
                //delete the old nodes
                newDfg->delNode(candidateNode->id());
                newDfg->delNode(node->id());
                //add all the new edges
                for(auto elem : externalSrcInEdges){
                    // std::cout << "externalSrcInEdges width: " << elem.first << " size: " << elem.second.size() <<std::endl;
                    for(auto edge: elem.second){
                        newDfg->addEdge(edge);
                    }
                }
                for(auto elem : externalDstInEdges){
                    // std::cout << "externalDstInEdges width: " << elem.first << " size: " << elem.second.size() <<std::endl;
                    for(auto edge: elem.second){
                        newDfg->addEdge(edge);
                    }
                }
                for(auto elem : externalDstOutEdges){
                    // std::cout << "externalDstOutEdges width: " << elem.first << " size: " << elem.second.size() <<std::endl;
                    for(auto edge: elem.second){
                        newDfg->addEdge(edge);
                    }
                }
                hasPacked = true; 
                // std::cout << "internalEdges size: " << internalEdges.size() << std::endl;
                break;               
            }else{
                continue;
            }
        }
        // std::cout << std::endl;
        // std::cout << "traverseIdx: " << traverseIdx << " node size: " << newDfg->nodes().size() << std::endl;
        if(hasPacked){// after each packing, we need to re-fresh the topological order
            // std::cout << "Packing once!" << std::endl;
            // newDfg->setMII(std::max(newDfg->MPII(), 1));
            newDfg->topoSortNodes();
            // newDfg->dumpDFG(resultDir);
            if(packStage == 1) packedNodeAtFirstStage ++;
            if(packedNodeAtFirstStage == maxPackedNode && packStage == 1) packStage ++;
            // newDfg->detectBackEdgeLoops();
        }else if(traverseIdx == newDfg->nodes().size()){//if no packing after traversing all the nodes, forward to next stage
            packStage ++;
        }
    }
    // std::cout << "packedNodeAtFirstStage: " << packedNodeAtFirstStage << std::endl;
    // packing finished, set the new DFG
    setDFG(newDfg, true);
    newDfg->dumpDFG(resultDir, true);
}



//@hw_yuan: check whether these two node can be packed, based on the supported operations of GPE
//besides, if these two nodes can be packed and have constant operands, the value of constant should be the same
//packing 2 packed nodes is unsupport
bool Mapper::isCompatible(DFGNode* curNode, DFGNode* canNode){
    if(canNode->isPacked()) return false;// TODO FIX: should consider comprehensively
    std::string opTypeCanNode = _dfg->getEuType(canNode->operation());
    std::set<std::string> allOperations;
    allOperations.insert(canNode->operation());
    if(curNode->isPacked()){
        for(auto& elem : curNode->multiOps()){
            allOperations.insert(elem);
            if(opTypeCanNode == _dfg->getEuType(elem))
                return false;
        }
    }else{
        allOperations.insert(curNode->operation());
        std::string opTypeCurNode = _dfg->getEuType(curNode->operation());
        if(opTypeCanNode == opTypeCurNode) return false;
    }
    int cgWidth = _dfg->CGWidth();
    bool constCompatible = true;
    bool hasOpCompatible = false;
    for(auto & elem : _adg->nodes()){
        auto adgNode = elem.second;
        bool opCompatible = true;
        //select GPE node
        if(adgNode->type() == "GIB" || adgNode->type() == "IOB"){  
            continue;
        }
        GPENode* gpeNode = dynamic_cast<GPENode*>(adgNode);
        if(!gpeNode->isMIMD()) continue;
        for(auto op : allOperations){
            if(!gpeNode->opCapable(op)){
                opCompatible = false;
                break;
            }
        }
        if(opCompatible) {
            hasOpCompatible = true;
            break; // if we have the GPE that can accomodate all the operations of packed node, just quit
        }
    }
    if(curNode->hasImm(cgWidth) && canNode->hasImm(cgWidth)){// for coarse-grained nodes to be packed, there is no one have 1-bit constant
        //@hw_yuan: for now, we can support the packed node has 2 diffenrent constants
        if(curNode->immValue(cgWidth) != canNode->immValue(cgWidth)){//
            if(curNode->has2ndImm()){
                if(curNode->get2ndImm() != canNode->immValue(cgWidth))
                    constCompatible = false;
            }
        }        
    }
    return hasOpCompatible & constCompatible;
}


//@hw_yuan: packing the DFG Nodes into a bigger node based on AA_Pack in VPR
void Mapper::opsAAPacking(DFG* newDfg, std::string resultDir){
    *newDfg = *_dfg;
    int cgWidth = _dfg->CGWidth();
    int maxPackedNode = getADG()->numMIMDGpeNodes();
    // int maxPackedNode = 100;
    int maxNodeId = newDfg->nodes().rbegin()->first; // std::map auto sort the key
    int maxEdgeId = newDfg->edges().rbegin()->first;  
    int numPackedNode = 0;
    /*****
        first, we sort the nodes based on their degree, which is also the attracted factor
    *****/
    std::map<int, int> degreeCnt; // <dfgnode-id, count>
    std::deque<int> dfgNodeIdDegreeOrder;// only record the node can be packed
    for(auto nodeId : newDfg->topoNodes()){ 
        DFGNode *node = newDfg->node(nodeId);
        if(newDfg->isIONode(nodeId) || node->accumulative() || node->initSelection() || node->operation() == "LUT" || node->operation() == "XCORE" || node->operation() == "PASS" || node->isConstArgIn()){ // @yuan_xcore: IO/accumulative/XCore node or has ArcIn  can not be packed
            continue;
        }
        int degree = 0;
        dfgNodeIdDegreeOrder.push_back(nodeId);
        // std::cout << " name: " << node->name() << " op: "<<node->operation() << std::endl;
        // for(auto& outPort : node->outputEdges()){
        //     for(auto& outEdge : outPort.second){
        //         for(auto eid : outEdge.second){
        //             DFGEdge* edge = newDfg->edge(eid); 
        //             DFGNode* dstNode = newDfg->node(edge->dstId());
        //             if(!(newDfg->isIONode(edge->dstId()) || dstNode->accumulative() || dstNode->initSelection() || dstNode->operation() == "LUT")){
        //                 degree += 1;
        //             }
        //         }
        //     }
        // }
        // for(auto& inEdge : node->inputEdges()){
        //     for(auto& inEdgeSameWidth : inEdge.second){
        //         DFGEdge* edge = newDfg->edge(inEdgeSameWidth.second); 
        //         DFGNode* srcNode = newDfg->node(edge->srcId());
        //         if(!(newDfg->isIONode(edge->srcId()) || srcNode->accumulative() || srcNode->initSelection() || srcNode->operation() == "LUT")){
        //             degree += 1;
        //         }
        //     }
        //     // degree += inEdge.second.size();
        // }
        for(auto& outPort : node->outputEdges()){
            for(auto& outEdge : outPort.second){
                // degree += outEdge.second.size();
                if(outEdge.second.size() == 1){
                    degree += 2;
                }
            }
        }
        for(auto& inEdge : node->inputEdges()){
            // degree += inEdge.second.size();
            for(auto& edges: inEdge.second){
                DFGEdge* e = newDfg->edge(edges.second);
                DFGNode* srcNode = newDfg->node(e->srcId());
                // for(auto& srcInput : srcNode->inputEdges(cgWidth))
                degree += srcNode->numInputs(cgWidth);
                // degree += srcNode->inputEdges(cgWidth).size();
            }
        }
        // std::cout << " name: " << node->name() << " op: "<<node->operation() << " degree: "<<degree<<std::endl;
        degreeCnt[nodeId] = degree;
    }
    std::stable_sort(dfgNodeIdDegreeOrder.begin(), dfgNodeIdDegreeOrder.end(), [&](int a, int b){
        return degreeCnt[a] >  degreeCnt[b];
    });
    // std::random_shuffle(dfgNodeIdDegreeOrder.begin(), dfgNodeIdDegreeOrder.end());
    /*****
        second, we begin to pack based on the sorted nodes
    *****/
    int traverseTime = 0;
    int packPreTime = 0;
    int packSucTime = 0;
    while (!dfgNodeIdDegreeOrder.empty()) {
        if(numPackedNode == maxPackedNode) break;
        int nodeId = dfgNodeIdDegreeOrder.front();
        dfgNodeIdDegreeOrder.pop_front();
        // int nodeId = dfgNodeIdDegreeOrder.back();
        // dfgNodeIdDegreeOrder.pop_back();
        if(!newDfg->nodes().count(nodeId)) continue; //packing may merge some nodes
        while(true){//based on AA_pack, we should pack until there are no candidate nodes
            //select the candidate node for packing
            DFGNode* node = newDfg->node(nodeId);
            //@yuan: the following conditions means that this node is full
            // if(node->numInputs(cgWidth) >= 4) break;
            if(node->isPacked() && node->multiOps().size() >= 4){
                break;
            }
            traverseTime ++;
            int hasCandidate = 0; //@yuan: 0: no candidate; 1: precessor candidate; 2: subsucessor candidate
            DFGNode *candidatePreNode = nullptr;
            DFGNode *candidateSucNode = nullptr;
            std::set<DFGEdge*> internalEdges;// the internal edges will be merged after packing
            int maxAddInterEdgeLat = 0;//record the latest arrive latency of the input edge
            bool hasPacked = false;
            int bestAttractDegree = 0;
            std::map <int, int> tempInterLat;
            std::cout << "current node: " << node->name() << std::endl;
            if(node->numInputs(cgWidth) < 4){
                for(auto inEdges : node->inputEdges()){// find candidate nodes from processor
                    int width = inEdges.first;
                    for(auto edgeWithSameWidth: inEdges.second){
                        int edgeId = edgeWithSameWidth.second;
                        DFGEdge *edge = newDfg->edge(edgeId); 
                        DFGNode *srcNode = newDfg->node(edge->srcId());
                        maxAddInterEdgeLat = std::max(maxAddInterEdgeLat, edge->addDelay());
                        if(tempInterLat.count(srcNode->id())){
                            tempInterLat[srcNode->id()] = std::max(tempInterLat[srcNode->id()], edge->addDelay());
                        }else{
                            tempInterLat[srcNode->id()] = edge->addDelay();
                        }
                        if(edge->isBackEdge() || newDfg->isIONode(edge->srcId()) || srcNode->accumulative() || srcNode->initSelection() || srcNode->operation() == "LUT" || srcNode->operation() == "XCORE" || srcNode->operation() == "PASS" || srcNode->isConstArgIn()) continue; // can not pack two nodes have back-edge connection; can not pack the IOB/ACC/XCore node
                        // std::cout << "Src Node: " << srcNode->name() << std::endl;
                        auto srcOutEdgesPerPort = srcNode->outputEdges(width);// for now, one node has only 1 output port, therefore, the width is determined
                        if(srcOutEdgesPerPort.size() > 1 || srcOutEdgesPerPort.begin()->second.size() > 2) continue;
                        for(auto outEdges : srcOutEdgesPerPort){
                            if(outEdges.second.size() == 1){//the src node can only connect to current node
                                if(isCompatible(node, srcNode)){
                                    int currentAttractDegree = 1;
                                    for(auto& inEdge : srcNode->inputEdges()){
                                        for(auto& inEdgeSameWidth : inEdge.second){
                                            DFGEdge* srcEdge = newDfg->edge(inEdgeSameWidth.second); 
                                            DFGNode* preSrcNode = newDfg->node(srcEdge->srcId());
                                            if(!(isCompatible(preSrcNode, srcNode))){//the srcNode can not be packed with its precessor node, it more likely being pakced with current node
                                                currentAttractDegree += 3;
                                            }else if(isCompatible(preSrcNode, srcNode) && isCompatible(node, preSrcNode)&& preSrcNode->numOutputs(cgWidth) == 1){
                                                currentAttractDegree += 5;
                                            }
                                        }
                                        // degree += inEdge.second.size();
                                    }
                                    // std::cout << "currentAttractDegree: " << currentAttractDegree << std::endl;
                                    if(currentAttractDegree > bestAttractDegree){
                                        candidatePreNode = srcNode;
                                        internalEdges.clear();
                                        internalEdges.insert(edge);
                                        bestAttractDegree = currentAttractDegree;
                                        hasCandidate = 1;
                                    }
                                    // if(hasCandidate > 0) goto pack;
                                }
                            }else{
                                for(auto e : outEdges.second){
                                    if(e == edgeId) continue;
                                    DFGEdge *extraEdge = newDfg->edge(e); 
                                    // std::cout << "extraEdge->dstId(): " << extraEdge->dstId() << " id: " << id << std::endl;
                                    if(extraEdge->dstId() == nodeId && isCompatible(node, srcNode)){
                                        // std::cout << "Compatible 2~" << std::endl;
                                        int currentAttractDegree = 1;
                                        for(auto& inEdge : srcNode->inputEdges()){
                                            for(auto& inEdgeSameWidth : inEdge.second){
                                                DFGEdge* srcEdge = newDfg->edge(inEdgeSameWidth.second); 
                                                DFGNode* preSrcNode = newDfg->node(srcEdge->srcId());
                                                if(!(isCompatible(preSrcNode, srcNode))){//the srcNode can not be packed with its precessor node, it more likely being pakced with current node
                                                    currentAttractDegree += 3;
                                                }else if(isCompatible(preSrcNode, srcNode) && isCompatible(node, preSrcNode) && preSrcNode->numOutputs(cgWidth) == 1){
                                                    currentAttractDegree += 5;
                                                }
                                            }
                                            // degree += inEdge.second.size();
                                        }
                                        if(currentAttractDegree > bestAttractDegree){
                                            candidatePreNode = srcNode;
                                            internalEdges.clear();
                                            internalEdges.insert(edge);
                                            internalEdges.insert(extraEdge);
                                            bestAttractDegree = currentAttractDegree;
                                            hasCandidate = 1;
                                        }
                                        // if(hasCandidate > 0) goto pack;
                                        // internalEdges.insert(edge);
                                        // internalEdges.insert(extraEdge);
                                        // candidatePreNode = srcNode;
                                        // hasCandidate = true;
                                    }
                                }
                            }
                            // if(hasCandidate > 0) goto pack;
                        }
                    }
                }
                for(auto& outWidth : node->outputEdges()){// find candidate nodes from sucessor
                    //for now, the width of output edges for one node are same
                    for(auto& outEdges : outWidth.second){// for now, one node only has one output port
                        if(outEdges.second.size() > 2) break;
                        if(outEdges.second.size() == 1){
                            int edgeId = *outEdges.second.begin();
                            DFGEdge *edge = newDfg->edge(edgeId);   
                            DFGNode *dstNode = newDfg->node(edge->dstId());
                            if(node->numInputs(cgWidth) >= 4 && !dstNode->hasImm(cgWidth)) break; // no extra input port 
                            if(edge->isBackEdge() || newDfg->isIONode(edge->dstId()) || dstNode->accumulative() || dstNode->initSelection() || dstNode->operation() == "LUT" || dstNode->operation() == "PASS" || dstNode->operation() == "XCORE" || dstNode->isConstArgIn())
                                break;
                            if(isCompatible(node, dstNode)){
                                // std::cout << "Compatible 1~" << std::endl;
                                int currentAttractDegree = 2;
                                //for dst node, if it connect to output/store, it becomes more acttracted
                                // for(auto& inEdge : dstNode->inputEdges()){
                                //     for(auto& inEdgeSameWidth : inEdge.second){
                                //         DFGEdge* srcEdge = newDfg->edge(inEdgeSameWidth.second); 
                                //         DFGNode* extraSrcNode = newDfg->node(srcEdge->srcId());
                                //         if(!(newDfg->isIONode(srcEdge->srcId()) || extraSrcNode->accumulative() || extraSrcNode->initSelection() || extraSrcNode->operation() == "LUT")){
                                //             currentAttractDegree += 1;
                                //         }
                                //     }
                                //     // degree += inEdge.second.size();
                                // }
                                for(auto& outEdge : dstNode->outputEdges()){
                                    for(auto& outEdgeSameWidth : outEdge.second){
                                        if(outEdgeSameWidth.second.size() > 2){
                                            currentAttractDegree += 5;
                                            break;
                                        }else if(outEdgeSameWidth.second.size() == 2){
                                            std::vector<int> outEdgeId;
                                            for(auto e : outEdgeSameWidth.second){
                                                outEdgeId.push_back(e);
                                            }
                                            int firstEdgeId = outEdgeId[0];
                                            DFGEdge *firstEdge = newDfg->edge(firstEdgeId); 
                                            int secondEdgeId = outEdgeId[1];
                                            DFGEdge *secondEdge = newDfg->edge(secondEdgeId); 
                                            if(firstEdge->dstId() != secondEdge->dstId()){
                                                currentAttractDegree += 4;
                                                break;
                                            }                                
                                            for(auto e : outEdgeSameWidth.second){
                                                DFGEdge* dstNodeOutEdge = newDfg->edge(e); 
                                                DFGNode* next2Node = newDfg->node(dstNodeOutEdge->dstId());
                                                if(newDfg->isIONode(dstNodeOutEdge->dstId())){
                                                    currentAttractDegree += 4;
                                                }else if(!isCompatible(next2Node, dstNode)){
                                                    currentAttractDegree += 3;
                                                }
                                            }
                                        }else{                                    
                                            for(auto e : outEdgeSameWidth.second){
                                                DFGEdge* dstNodeOutEdge = newDfg->edge(e); 
                                                DFGNode* next2Node = newDfg->node(dstNodeOutEdge->dstId());
                                                if(newDfg->isIONode(dstNodeOutEdge->dstId())){
                                                    currentAttractDegree += 4;
                                                }else if(!isCompatible(next2Node, dstNode)){
                                                    currentAttractDegree += 3;
                                                }
                                                break;
                                            }
                                        }
                                    }
                                    // degree += inEdge.second.size();
                                }
                                // std::cout << "currentAttractDegree: " << currentAttractDegree << std::endl;
                                if(currentAttractDegree > bestAttractDegree){
                                    candidateSucNode = dstNode;
                                    internalEdges.clear();
                                    internalEdges.insert(edge);
                                    hasCandidate = 2;
                                    bestAttractDegree = currentAttractDegree;
                                }
                                // if(hasCandidate > 0) goto pack;
                            }
                        }else{//if the current node has 2 outputs, they must be connected to the same node 
                            std::vector<int> outEdgeId;
                            for(auto out : outEdges.second){
                                outEdgeId.push_back(out);
                            }
                            int firstEdgeId = outEdgeId[0];
                            DFGEdge *firstEdge = newDfg->edge(firstEdgeId); 
                            int secondEdgeId = outEdgeId[1];
                            DFGEdge *secondEdge = newDfg->edge(secondEdgeId); 
                            if(firstEdge->dstId() != secondEdge->dstId()) break;
                            DFGNode *dstNode = newDfg->node(firstEdge->dstId());
                            if(isCompatible(node, dstNode)){
                                int currentAttractDegree = 2;
                                for(auto& outEdge : dstNode->outputEdges()){
                                    for(auto& outEdgeSameWidth : outEdge.second){
                                        if(outEdgeSameWidth.second.size() > 2){
                                            currentAttractDegree += 5;
                                            break;
                                        }else if(outEdgeSameWidth.second.size() == 2){
                                            std::vector<int> outEdgeIds;
                                            for(auto e : outEdgeSameWidth.second){
                                                outEdgeIds.push_back(e);
                                            }
                                            int firstId = outEdgeIds[0];
                                            DFGEdge *Edge1 = newDfg->edge(firstId); 
                                            int secondId = outEdgeIds[1];
                                            DFGEdge *Edge2 = newDfg->edge(secondId); 
                                            if(Edge1->dstId() != Edge2->dstId()){
                                                currentAttractDegree += 4;
                                                break;
                                            }                                
                                            for(auto e : outEdgeSameWidth.second){
                                                DFGEdge* dstNodeOutEdge = newDfg->edge(e); 
                                                DFGNode* next2Node = newDfg->node(dstNodeOutEdge->dstId());
                                                if(newDfg->isIONode(dstNodeOutEdge->dstId())){
                                                    currentAttractDegree += 4;
                                                }else if(!isCompatible(next2Node, dstNode)){
                                                    currentAttractDegree += 3;
                                                }
                                                break;
                                            }
                                        }else{                                    
                                            for(auto e : outEdgeSameWidth.second){
                                                DFGEdge* dstNodeOutEdge = newDfg->edge(e); 
                                                DFGNode* next2Node = newDfg->node(dstNodeOutEdge->dstId());
                                                if(newDfg->isIONode(dstNodeOutEdge->dstId())){
                                                    currentAttractDegree += 4;
                                                }else if(!isCompatible(next2Node, dstNode)){
                                                    currentAttractDegree += 3;
                                                }
                                            }
                                        }
                                    }
                                    // degree += inEdge.second.size();
                                }
                                if(currentAttractDegree > bestAttractDegree){
                                    internalEdges.clear();
                                    internalEdges.insert(firstEdge);
                                    internalEdges.insert(secondEdge);
                                    candidateSucNode = dstNode;
                                    hasCandidate = 2;
                                    bestAttractDegree = currentAttractDegree;
                                }
                                // if(hasCandidate > 0) goto pack;
                            }
                        }
                        // if(hasCandidate > 0) goto pack;
                    }
                }
            }
            pack: if(hasCandidate > 0){
                if(!node->isPacked()) numPackedNode += 1;
                if(hasCandidate == 1){
                    packPreTime += 1;
                    DFGNode* newNode = new DFGNode();
                    newNode->setId(++maxNodeId);
                    newNode->setOperation(node->operation());//just save the last operation, for the packed node
                    newNode->setOpLatency(candidatePreNode->opLatency() + node->opLatency() - tempInterLat[candidatePreNode->id()]);
                    std::vector<std::string> multiOps;
                    multiOps.push_back(candidatePreNode->operation());
                    std::string name = candidatePreNode->operation();
                    if(node->isPacked()){
                        for(auto elem : node->multiOps()){
                            // std::cout << "op: " << elem << std::endl;
                            name += " " + elem;
                            multiOps.push_back(elem);
                        }
                    }else{
                        multiOps.push_back(node->operation());
                        name += " " + node->operation();
                    }
                    newNode->setPacked(true);
                    newNode->setMultiOps(multiOps);
                    newNode->setName(name + "_" +std::to_string(maxNodeId));
                    std::map<int, std::set<DFGEdge*>> externalSrcInEdges;// the input-edges for candidate node
                    for(auto inEdges : candidatePreNode->inputEdges()){
                        int width = inEdges.first;
                        for(auto edgeWithSameWidth: inEdges.second){
                            DFGEdge* e = new DFGEdge(++maxEdgeId);// the new edge connected to new packed node
                            DFGEdge* oldEdge = newDfg->edge(edgeWithSameWidth.second);
                            // std::cout << "candidate src edge id: " << edgeWithSameWidth.second << std::endl;
                            int dstId = newNode->id();
                            int dstPort = width > 1 ? oldEdge->dstPortIdx() : 4; // for the fine-grained input, there 4 coarse-grained inputs before it
                            int srcId = oldEdge->srcId();
                            int srcPort = oldEdge->srcPortIdx();
                            newNode->addInternalAttribute(newDfg->getEuType(candidatePreNode->operation()), "GPE", dstPort, oldEdge->dstPortIdx(), width);
                            std::cout <<"src name: " << newDfg->node(srcId)->name()<<" srcId: " << srcId << " srcPort: " << srcPort << " dstId: " << dstId << " dstPort: " << dstPort << " edge id: " << e->id()<< std::endl;
                            e->setEdge(width, srcId, srcPort, dstId, dstPort);
                            e->setBackEdge(oldEdge->isBackEdge());
                            e->setType(oldEdge->type());
                            e->setIterDist(oldEdge->iterDist());
                            e->setAddDelay(oldEdge->addDelay());
                            externalSrcInEdges[width].insert(e);
                        }
                    }
                    std::map<int, std::set<DFGEdge*>> externalDstInEdges;//the input-edges of current node
                    for(auto inEdges : node->inputEdges()){
                        int width = inEdges.first;
                        int numEdgeWithSameWidth = 0;
                        for(auto edgeWithSameWidth: inEdges.second){
                            std::cout << "edge id: " << edgeWithSameWidth.second << std::endl;
                            DFGEdge* oldEdge = newDfg->edge(edgeWithSameWidth.second);
                            if(internalEdges.count(oldEdge)) continue;
                            // int oldAddDelay = oldEdge->addDelay();
                            DFGEdge* e = new DFGEdge(++maxEdgeId);// the new edge connected to new packed node
                            int dstId = newNode->id();
                            int srcId = oldEdge->srcId();
                            int srcPort = oldEdge->srcPortIdx();
                            std::cout << "src: " << newDfg->node(srcId)->name() << " edge id: " << edgeWithSameWidth.second << std::endl;
                            int oldDstPort = oldEdge->dstPortIdx();//TODO: we need to record the original dstport for PnR
                            int newDstPort = 0;
                            if(width > 1){
                                int extraConst = 0;
                                if(candidatePreNode->hasImm(width)){
                                    if(node->hasImm(width)){
                                        if(node->has2ndImm()){
                                            newDstPort = candidatePreNode->numInputs(width) + numEdgeWithSameWidth + 1;
                                        }else{
                                            if(candidatePreNode->immValue(width) == node->immValue(width)){
                                                newDstPort = candidatePreNode->numInputs(width) + numEdgeWithSameWidth;
                                            }else{
                                                newDstPort = candidatePreNode->numInputs(width) + numEdgeWithSameWidth + 1;
                                            }
                                        }
                                    }else{
                                        newDstPort = candidatePreNode->numInputs(width) + numEdgeWithSameWidth;
                                    }
                                }else{
                                    newDstPort = candidatePreNode->numInputs(width) + numEdgeWithSameWidth + int(node->hasImm(width)) + int(node->has2ndImm());
                                }
                            }else{
                                newDstPort = 4;
                            }
                            if(node->isPacked()){//for current node is packed node, we should handle its original internal attribute
                                // bool hasAddedInternal = false;
                                for(auto& EuInputs : node->internalAttrMap()){
                                    std::string opName = EuInputs.first;
                                    for(auto& eachOperand : EuInputs.second){
                                        // std::cout << "opName: " << opName << " edge id: " << eachOperand.second.edgeId << " old edge id: " << oldEdge->id() << std::endl;
                                        if(eachOperand.second.srcPort == oldDstPort && eachOperand.second.width == width){
                                            int operand = eachOperand.first;
                                            int width = eachOperand.second.width;
                                            newNode->addInternalAttribute(opName, "GPE", newDstPort, operand, width);// GPE's newDstPort -> opName's operand
                                            // hasAddedInternal = true;
                                            // if(hasAddedInternal) goto addNew;
                                        }
                                    }
                                }
                                // addNew: if(!hasAddedInternal){
                                //     std::string latestOp = node->multiOps().back();
                                //     newNode->addInternalAttribute(newDfg->getEuType(latestOp), "GPE", newDstPort, operand, e->id(), width);
                                // }
                            }else{
                                newNode->addInternalAttribute(newDfg->getEuType(node->operation()), "GPE", newDstPort, oldDstPort, width);// GPE -> EU's oldDstPort(operand) 
                                // std::cout << "nopacked opName: " << newDfg->getEuType(node->operation()) << " edge id: " << e->id() << std::endl;
                            }
                            e->setEdge(width, srcId, srcPort, dstId, newDstPort);
                            e->setBackEdge(oldEdge->isBackEdge());
                            e->setType(oldEdge->type());
                            e->setIterDist(oldEdge->iterDist());
                            int biasInterLat = tempInterLat[candidatePreNode->id()];
                            // if(srcId == candidatePreNode->id()){
                            //     biasInterLat = tempInterLat[srcId];
                            // }else{
                            //     biasInterLat = 0;
                            // }
                            e->setAddDelay(oldEdge->addDelay() + candidatePreNode->opLatency() - biasInterLat);//additional delay equals to older additional delay + candidate node's operation's delay - maxAddInterEdgeLat
                            // e->setAddDelay(oldEdge->addDelay() + candidatePreNode->opLatency() - maxAddInterEdgeLat);//additional delay equals to older additional delay + candidate node's operation's delay - maxAddInterEdgeLat
                            // std::cout <<"externalDstInEdges: src name: " << newDfg->node(srcId)->name()<<" srcId: " << srcId << " srcPort: " << srcPort << " dstId: " << dstId << " dstPort: " << newDstPort << " add delay: " << e->addDelay() << std::endl;
                            externalDstInEdges[width].insert(e);
                            numEdgeWithSameWidth++;
                        }
                    }
                    //we need to handle the output edges here
                    //it's noting that the candidate node's output are completely connected to current node
                    //hence, there is no need to record these outputs explicitly
                    std::map<int, std::set<DFGEdge*>> externalDstOutEdges;
                    for(auto outEdges : node->outputEdges()){
                        int width = outEdges.first;
                        for(auto edgeWithSameWidthPort: outEdges.second){
                            int srcPort = edgeWithSameWidthPort.first;
                            for(auto edgeId : edgeWithSameWidthPort.second){
                                DFGEdge* oldEdge = newDfg->edge(edgeId);
                                DFGEdge* e = new DFGEdge(++maxEdgeId);// the new output edge
                                int dstId = oldEdge->dstId();
                                int srcId = newNode->id();
                                int dstPort = oldEdge->dstPortIdx();
                                e->setEdge(width, srcId, srcPort, dstId, dstPort);
                                e->setBackEdge(oldEdge->isBackEdge());
                                e->setType(oldEdge->type());
                                e->setIterDist(oldEdge->iterDist());
                                e->setAddDelay(oldEdge->addDelay());
                                externalDstOutEdges[width].insert(e);
                            }
                        }
                    } 
                    //we can assign the old inter-Eu connections within this loop     
                    for(auto& EuInputs : node->internalAttrMap()){
                        std::string opName = EuInputs.first;
                        for(auto& eachOperand : EuInputs.second){
                            if(eachOperand.second.srcName != "GPE" && eachOperand.second.srcName != "CONST"){
                                int operand = eachOperand.first;
                                int width = eachOperand.second.width;
                                newNode->addInternalAttribute(opName, eachOperand.second.srcName, 0, operand, width);// srcEu'0 port -> opName Eu's opreand
                            }
                        }
                    }
                    //finally, we handle the new inter-Eu connections 
                    std::cout << "new internal edge size: " << internalEdges.size() << std::endl;
                    for(auto edge : internalEdges){
                        int dstPort = edge->dstPortIdx();
                        int width = edge->bitWidth();
                        std::string latestOp;
                        if(node->isPacked()){
                            int opIdx = edge->addDelay(); //according to the added delay to this edge, we can find the corresponding op
                            auto multiop = node->multiOps();
                            latestOp = multiop[opIdx];
                            // std::cout << "latestOp: " << latestOp << " type: " << newDfg->getEuType(latestOp) << " node internal size: " << node->internalAttrMap().size()<< std::endl;
                            // std::cout << "candidate Eu: " << newDfg->getEuType(candidatePreNode->operation()) << std::endl;
                            auto internalAttr = node->internalAttrMap(newDfg->getEuType(latestOp));
                            for(auto elem : internalAttr){
                                if((elem.second.srcPort == dstPort) && (elem.second.width == width)){
                                    newNode->addInternalAttribute(newDfg->getEuType(latestOp), newDfg->getEuType(candidatePreNode->operation()), 0, elem.first, width);//elem.first is the operand
                                }
                            }
                        }else{
                            latestOp = node->operation();
                            newNode->addInternalAttribute(newDfg->getEuType(latestOp), newDfg->getEuType(candidatePreNode->operation()), 0, dstPort, width);// Candidate EU -> curent EU
                        }
                        // std::cout << "candidate node: " << candidateNode->name() << " to packed node: " << node->name() << " lastop: " << latestOp <<" type: "<< newDfg->getEuType(latestOp)<<std::endl; 
                    }
                    newDfg->addNode(newNode);
                    //set the constant for packed node
                    //for now, there are no fine-grained constant input for non-LUT node
                    if(node->hasImm(cgWidth)){
                        // std::set<int> idxTemp = node->immIdxs();
                        // newNode->clearImmMultiIdxs();
                        int constantType = 0; // 0: candidate has no constant; 1: the candidate's constant is the first constant; 2 : the candidate's constant is second constant
                        if(candidatePreNode->hasImm(cgWidth)){
                            newNode->setImm(cgWidth, std::make_pair(candidatePreNode->immIdx(cgWidth), node->immValue(cgWidth)));// the record imm Idx shift to right
                            if(candidatePreNode->immValue(cgWidth) == node->immValue(cgWidth)){
                                constantType = 1;
                                newNode->addInternalAttribute(newDfg->getEuType(candidatePreNode->operation()), "CONST", candidatePreNode->immIdx(cgWidth), candidatePreNode->immIdx(cgWidth), cgWidth);
                            }else{
                                constantType = 2;
                                if(node->has2ndImm()){// old node has seconde constant, using it
                                    newNode->addInternalAttribute(newDfg->getEuType(candidatePreNode->operation()), "Second_Const", candidatePreNode->numInputs(cgWidth), candidatePreNode->immIdx(cgWidth), cgWidth);
                                }else{
                                    newNode->addInternalAttribute(newDfg->getEuType(candidatePreNode->operation()), "Second_Const", candidatePreNode->numInputs(cgWidth), candidatePreNode->immIdx(cgWidth), cgWidth);
                                    newNode->set2ndImm(candidatePreNode->immValue(cgWidth));
                                    newNode->set2ndImmIdx(candidatePreNode->numInputs(cgWidth));
                                }
                            }
                        }else{
                            newNode->setImm(cgWidth, std::make_pair(candidatePreNode->numInputs(cgWidth), node->immValue(cgWidth)));// the record imm Idx shift to right
                        }
                        if(node->has2ndImm()){
                            newNode->set2ndImm(node->get2ndImm());
                            int extraIdx = constantType > 0 ? 0 : 1;// if candidate has no constant, all its inputs are variable, so the first constant is num; then secoond is num + 1
                            newNode->set2ndImmIdx(candidatePreNode->numInputs(cgWidth) + extraIdx);
                        }
                        //for old-node has constant, we connect all the Eus to this constant 
                        if(node->isPacked()){// the node has 2 constant, it must be packed node
                            for(auto& EuInputs : node->internalAttrMap()){
                                std::string opName = EuInputs.first;
                                for(auto& eachOperand : EuInputs.second){
                                    if(eachOperand.second.srcName == "CONST"){
                                        int operand = eachOperand.first;
                                        // int width = eachOperand.second.width;
                                        if(constantType > 0){
                                            newNode->addInternalAttribute(opName, "CONST", candidatePreNode->immIdx(cgWidth), operand, cgWidth);
                                        }else{
                                            newNode->addInternalAttribute(opName, "CONST", candidatePreNode->numInputs(cgWidth), operand, cgWidth);
                                        }
                                    }else if(eachOperand.second.srcName == "Second_Const"){// if one node has second constant, it must have 2 constants
                                        int operand = eachOperand.first;
                                        int extraIdx = constantType > 0 ? 0 : 1;// if candidate has no constant, all its inputs are variable, so the first constant is num; then secoond is num + 1
                                        newNode->addInternalAttribute(opName, "Second_Const", candidatePreNode->numInputs(cgWidth) + extraIdx, operand, cgWidth);
                                    }
                                }
                            }
                        }else{
                            newNode->addInternalAttribute(newDfg->getEuType(node->operation()), "CONST", candidatePreNode->numInputs(cgWidth), node->immIdx(cgWidth), cgWidth);
                        }
                        // if(candidatePreNode->hasImm(cgWidth)){
                        //     if(candidatePreNode->immValue(cgWidth) == node->immValue(cgWidth)){
                        //         newNode->addInternalAttribute(newDfg->getEuType(candidatePreNode->operation()), "CONST", candidatePreNode->numInputs(cgWidth), candidatePreNode->immIdx(cgWidth), cgWidth);
                        //     }else{
                        //         if(node->has2ndImm()){// old node has seconde constant, using it
                        //             newNode->addInternalAttribute(newDfg->getEuType(candidatePreNode->operation()), "Second_Const", candidatePreNode->numInputs(cgWidth) + 1, candidatePreNode->immIdx(cgWidth), cgWidth);
                        //         }else{
                        //             newNode->addInternalAttribute(newDfg->getEuType(candidatePreNode->operation()), "Second_Const", candidatePreNode->immIdx(cgWidth), candidatePreNode->immIdx(cgWidth), cgWidth);
                        //             newNode->set2ndImm(candidatePreNode->immValue(cgWidth));
                        //             newNode->set2ndImmIdx(candidatePreNode->immIdx(cgWidth));
                        //         }
                        //     }
                        // }
                    }else if(candidatePreNode->hasImm(cgWidth)){
                        newNode->setImm(cgWidth, std::make_pair(candidatePreNode->immIdx(cgWidth), candidatePreNode->immValue(cgWidth)));
                        newNode->addInternalAttribute(newDfg->getEuType(candidatePreNode->operation()), "CONST", candidatePreNode->immIdx(cgWidth), candidatePreNode->immIdx(cgWidth), cgWidth);
                        // newNode->setImmIdxs(candidatePreNode->immIdx(cgWidth));
                    }
                    //delete the old nodes
                    newDfg->delNode(candidatePreNode->id());
                    newDfg->delNode(node->id());
                    //add all the new edges
                    for(auto elem : externalSrcInEdges){
                        // std::cout << "externalSrcInEdges width: " << elem.first << " size: " << elem.second.size() <<std::endl;
                        for(auto edge: elem.second){
                            newDfg->addEdge(edge);
                        }
                    }
                    for(auto elem : externalDstInEdges){
                        // std::cout << "externalDstInEdges width: " << elem.first << " size: " << elem.second.size() <<std::endl;
                        for(auto edge: elem.second){
                            newDfg->addEdge(edge);
                        }
                    }
                    for(auto elem : externalDstOutEdges){
                        // std::cout << "externalDstOutEdges width: " << elem.first << " size: " << elem.second.size() <<std::endl;
                        for(auto edge: elem.second){
                            newDfg->addEdge(edge);
                        }
                    }
                    hasPacked = true;
                    nodeId = newNode->id();
                }else{// packing the successor node
                    packSucTime += 1;
                    DFGNode* newNode = new DFGNode();
                    newNode->setId(++maxNodeId);
                    newNode->setOperation(candidateSucNode->operation());//just save the last operation, for the packed node
                    // newNode->setOpLatency(candidateSucNode->opLatency() + node->opLatency() - maxAddInterEdgeLat);
                    //@yuan: an observation is, the edges connected to the successor node will not have addition delay
                    newNode->setOpLatency(candidateSucNode->opLatency() + node->opLatency());
                    std::vector<std::string> multiOps;// the order in this vector is the order of execution
                    std::string name;
                    if(node->isPacked()){
                        for(auto elem : node->multiOps()){
                            // std::cout << "op: " << elem << std::endl;
                            name += " " + elem;
                            multiOps.push_back(elem);
                        }
                    }else{
                        multiOps.push_back(node->operation());
                        name += " " + node->operation();
                    } 
                    name += " " + candidateSucNode->operation();
                    multiOps.push_back(candidateSucNode->operation());// this time, the candidate node is the the tail (the last one)
                    newNode->setPacked(true);
                    newNode->setMultiOps(multiOps);
                    newNode->setName(name + "_" +std::to_string(maxNodeId));
                    std::map<int, std::set<DFGEdge*>> externalSrcInEdges;
                    for(auto inEdges : node->inputEdges()){
                        int width = inEdges.first;
                        for(auto edgeWithSameWidth: inEdges.second){
                            // std::cout << "edge id: " << edgeWithSameWidth.second << std::endl;
                            DFGEdge* oldEdge = newDfg->edge(edgeWithSameWidth.second);
                            // int oldAddDelay = oldEdge->addDelay();
                            DFGEdge* e = new DFGEdge(++maxEdgeId);// the new edge connected to new packed node
                            int dstId = newNode->id();
                            int srcId = oldEdge->srcId();
                            int srcPort = oldEdge->srcPortIdx();
                            // std::cout << "src: " << newDfg->node(srcId)->name() << " edge id: " << edgeWithSameWidth.second << std::endl;
                            int oldDstPort = width > 1 ? oldEdge->dstPortIdx() : 4;//TODO: we need to record the original dstport for PnR//fxied bug: for the src node has fine-grained input (e.g., sel)
                            //using the old packed node internal attribute directly
                            if(node->isPacked()){
                                // bool hasAddedInternal = false;
                                for(auto& EuInputs : node->internalAttrMap()){
                                    std::string opName = EuInputs.first;
                                    for(auto& eachOperand : EuInputs.second){
                                        // std::cout << "opName: " << opName << " edge id: " << eachOperand.second.edgeId << " old edge id: " << oldEdge->id() << std::endl;
                                        if(eachOperand.second.srcPort == oldDstPort && eachOperand.second.width == width){
                                            int operand = eachOperand.first;
                                            int width = eachOperand.second.width;
                                            newNode->addInternalAttribute(opName, "GPE", oldDstPort, operand, width);
                                            // hasAddedInternal = true;
                                            // if(hasAddedInternal) goto addNew;
                                        }
                                    }
                                }
                            }else{
                                newNode->addInternalAttribute(newDfg->getEuType(node->operation()), "GPE", oldDstPort, oldEdge->dstPortIdx(), width);
                                // std::cout << "nopacked opName: " << newDfg->getEuType(node->operation()) << " edge id: " << e->id() << std::endl;
                            }
                            e->setEdge(width, srcId, srcPort, dstId, oldDstPort);
                            e->setBackEdge(oldEdge->isBackEdge());
                            e->setType(oldEdge->type());
                            e->setIterDist(oldEdge->iterDist());
                            e->setAddDelay(oldEdge->addDelay());
                            // std::cout <<"externalDstInEdges: src name: " << newDfg->node(srcId)->name()<<" srcId: " << srcId << " srcPort: " << srcPort << " dstId: " << dstId << " dstPort: " << newDstPort << " add delay: " << e->addDelay() << std::endl;
                            externalSrcInEdges[width].insert(e);
                        }
                    }
                    std::map<int, std::set<DFGEdge*>> externalDstInEdges;
                    for(auto inEdges : candidateSucNode->inputEdges()){
                        int width = inEdges.first;
                        int numEdgeWithSameWidth = 0;
                        for(auto edgeWithSameWidth: inEdges.second){//currently, the maximum number of input edges (coarse-grained) for candidate node is 1 (constant of one input)
                            // std::cout << "edge id: " << edgeWithSameWidth.second << std::endl;
                            DFGEdge* oldEdge = newDfg->edge(edgeWithSameWidth.second);
                            if(internalEdges.count(oldEdge)) continue;
                            // int oldAddDelay = oldEdge->addDelay(
                            DFGEdge* e = new DFGEdge(++maxEdgeId);// the new edge connected to new packed node
                            int dstId = newNode->id();
                            int srcId = oldEdge->srcId();
                            int srcPort = oldEdge->srcPortIdx();
                            // std::cout << "src: " << newDfg->node(srcId)->name() << " edge id: " << edgeWithSameWidth.second << std::endl;
                            int oldDstPort = oldEdge->dstPortIdx();//TODO: we need to record the original dstport for PnR
                            int newDstPort = 0;
                            if(width > 1){
                                newDstPort = node->numInputs(width) + numEdgeWithSameWidth;
                            }else{
                                newDstPort = 4;
                            }
                            newNode->addInternalAttribute(newDfg->getEuType(candidateSucNode->operation()), "GPE", newDstPort, oldDstPort, width);// actually, oldDstPort is the operand
                            e->setEdge(width, srcId, srcPort, dstId, newDstPort);
                            e->setBackEdge(oldEdge->isBackEdge());
                            e->setType(oldEdge->type());
                            e->setIterDist(oldEdge->iterDist());
                            e->setAddDelay(oldEdge->addDelay() + node->opLatency());//
                            // std::cout <<"externalDstInEdges: src name: " << newDfg->node(srcId)->name()<<" srcId: " << srcId << " srcPort: " << srcPort << " dstId: " << dstId << " dstPort: " << newDstPort << " add delay: " << e->addDelay() << std::endl;
                            externalDstInEdges[width].insert(e);
                            numEdgeWithSameWidth++;
                        }
                    }
                    //we need to handle the output edges here
                    //the outputs are all the candidate node's outputs
                    std::map<int, std::set<DFGEdge*>> externalDstOutEdges;
                    for(auto outEdges : candidateSucNode->outputEdges()){
                        int width = outEdges.first;
                        for(auto edgeWithSameWidthPort: outEdges.second){
                            int srcPort = edgeWithSameWidthPort.first;
                            for(auto edgeId : edgeWithSameWidthPort.second){
                                DFGEdge* oldEdge = newDfg->edge(edgeId);
                                DFGEdge* e = new DFGEdge(++maxEdgeId);// the new output edge
                                int dstId = oldEdge->dstId();
                                int srcId = newNode->id();
                                int dstPort = oldEdge->dstPortIdx();
                                e->setEdge(width, srcId, srcPort, dstId, dstPort);
                                e->setBackEdge(oldEdge->isBackEdge());
                                e->setType(oldEdge->type());
                                e->setIterDist(oldEdge->iterDist());
                                e->setAddDelay(oldEdge->addDelay());
                                externalDstOutEdges[width].insert(e);
                            }
                        }
                    }     
                    //we can assign the old inter-Eu connections within this loop     
                    for(auto& EuInputs : node->internalAttrMap()){
                        std::string opName = EuInputs.first;
                        for(auto& eachOperand : EuInputs.second){
                            if(eachOperand.second.srcName != "GPE" && eachOperand.second.srcName != "CONST"){
                                int operand = eachOperand.first;
                                int width = eachOperand.second.width;
                                newNode->addInternalAttribute(opName, eachOperand.second.srcName, 0, operand, width);
                            }
                        }
                    }
                    //finally, we handle the new inter-Eu connections 
                    // std::cout << "new internal edge size: " << internalEdges.size() << std::endl;
                    for(auto edge : internalEdges){
                        int dstPort = edge->dstPortIdx();
                        int width = edge->bitWidth();
                        std::string latestOp;
                        if(node->isPacked()){
                            auto multiop = node->multiOps();
                            latestOp = multiop.back();
                            newNode->addInternalAttribute(newDfg->getEuType(candidateSucNode->operation()), newDfg->getEuType(latestOp), 0, dstPort, width);
                            // std::cout << "latestOp: " << latestOp << " type: " << newDfg->getEuType(latestOp) << " node internal size: " << node->internalAttrMap().size()<< std::endl;
                            // std::cout << "candidate Eu: " << newDfg->getEuType(candidateSucNode->operation()) << std::endl;
                            // auto internalAttr = node->internalAttrMap(newDfg->getEuType(latestOp));
                            // for(auto elem : internalAttr){
                            //     if((elem.second.srcPort == dstPort) && (elem.second.width == width)){
                                    
                            //     }
                            // }
                        }else{
                            latestOp = node->operation();
                            newNode->addInternalAttribute(newDfg->getEuType(candidateSucNode->operation()), newDfg->getEuType(latestOp), 0, dstPort, width);
                        }
                    }
                    newDfg->addNode(newNode);
                    //set the constant for packed node
                    //for now, there are no fine-grained constant input for non-LUT node
                    // std::cout << "newNode input num: " << newNode->numInputs(cgWidth) << std::endl;
                    if(node->hasImm(cgWidth)){
                        newNode->setImm(cgWidth, std::make_pair(node->immIdx(cgWidth), node->immValue(cgWidth)));// just using the original idx
                        if(node->has2ndImm()){// just using the original idx
                            newNode->set2ndImm(node->get2ndImm());
                            newNode->set2ndImmIdx(node->get2ndImmIdx());
                        }
                        //for old-node has constant, we connect all the Eus to this constant 
                        if(node->isPacked()){// the node has 2 constant, it must be packed node
                            for(auto& EuInputs : node->internalAttrMap()){// just using the old attribute
                                std::string opName = EuInputs.first;
                                for(auto& eachOperand : EuInputs.second){
                                    if(eachOperand.second.srcName == "CONST"){
                                        int operand = eachOperand.first;
                                        // int width = eachOperand.second.width;
                                        newNode->addInternalAttribute(opName, "CONST", eachOperand.second.srcPort, operand, cgWidth);
                                    }else if(eachOperand.second.srcName == "Second_Const"){// if one node has second constant, it must have 2 constants
                                        int operand = eachOperand.first;
                                        newNode->addInternalAttribute(opName, "Second_Const", eachOperand.second.srcPort, operand, cgWidth);
                                    }
                                }
                            }
                        }else{
                            newNode->addInternalAttribute(newDfg->getEuType(node->operation()), "CONST", node->immIdx(cgWidth), node->immIdx(cgWidth), cgWidth);
                        }
                        if(candidateSucNode->hasImm(cgWidth)){
                            if(candidateSucNode->immValue(cgWidth) == newNode->immValue(cgWidth)){// one constan for new node is set
                                for(auto& EuInputs : newNode->internalAttrMap()){
                                    std::string opName = EuInputs.first;
                                    for(auto& eachOperand : EuInputs.second){
                                        if(eachOperand.second.srcName == "CONST"){
                                            // int operand = eachOperand.first;
                                            // int width = eachOperand.second.width;
                                            newNode->addInternalAttribute(newDfg->getEuType(candidateSucNode->operation()), "CONST", eachOperand.second.srcPort, candidateSucNode->immIdx(cgWidth), cgWidth);
                                        }
                                    }
                                }
                                // newNode->addInternalAttribute(newDfg->getEuType(candidateSucNode->operation()), "CONST", node->numInputs(cgWidth), candidateSucNode->immIdx(cgWidth), cgWidth);
                            }else{
                                if(node->has2ndImm()){// old node has seconde constant, using it
                                    for(auto& EuInputs : newNode->internalAttrMap()){
                                        std::string opName = EuInputs.first;
                                        for(auto& eachOperand : EuInputs.second){
                                            if(eachOperand.second.srcName == "Second_Const"){
                                                // int operand = eachOperand.first;
                                                // int width = eachOperand.second.width;
                                                newNode->addInternalAttribute(newDfg->getEuType(candidateSucNode->operation()), "Second_Const", eachOperand.second.srcPort, candidateSucNode->immIdx(cgWidth), cgWidth);
                                            }
                                        }
                                    }
                                    // newNode->addInternalAttribute(newDfg->getEuType(candidateSucNode->operation()), "Second_Const", node->numInputs(cgWidth) + 1, candidateSucNode->immIdx(cgWidth), cgWidth);
                                }else{//here, the current node has one constant, and the candidate node has one additional constant, hence it's required to add a second constant
                                    newNode->addInternalAttribute(newDfg->getEuType(candidateSucNode->operation()), "Second_Const", node->numInputs(cgWidth), candidateSucNode->immIdx(cgWidth), cgWidth);
                                    std::cout << "add second const idx: " << node->numInputs(cgWidth) << std::endl;
                                    newNode->set2ndImmIdx(node->numInputs(cgWidth));// a new port for this new constant
                                    newNode->set2ndImm(candidateSucNode->immValue(cgWidth));
                                }
                            }
                        }
                    }else if(candidateSucNode->hasImm(cgWidth)){// only candidate node has a constant
                        newNode->setImm(cgWidth, std::make_pair(node->numInputs(cgWidth), candidateSucNode->immValue(cgWidth)));
                        newNode->addInternalAttribute(newDfg->getEuType(candidateSucNode->operation()), "CONST", node->numInputs(cgWidth), candidateSucNode->immIdx(cgWidth), cgWidth);
                        // newNode->setImmIdxs(candidateNode->immIdx(cgWidth));
                    }
                    //delete the old nodes
                    newDfg->delNode(candidateSucNode->id());
                    newDfg->delNode(node->id());
                    //add all the new edges
                    for(auto elem : externalSrcInEdges){
                        // std::cout << "externalSrcInEdges width: " << elem.first << " size: " << elem.second.size() <<std::endl;
                        for(auto edge: elem.second){
                            newDfg->addEdge(edge);
                        }
                    }
                    for(auto elem : externalDstInEdges){
                        // std::cout << "externalDstInEdges width: " << elem.first << " size: " << elem.second.size() <<std::endl;
                        for(auto edge: elem.second){
                            newDfg->addEdge(edge);
                        }
                    }
                    for(auto elem : externalDstOutEdges){
                        // std::cout << "externalDstOutEdges width: " << elem.first << " size: " << elem.second.size() <<std::endl;
                        for(auto edge: elem.second){
                            newDfg->addEdge(edge);
                        }
                    }
                    hasPacked = true;
                    nodeId = newNode->id();
                    // break;
                }                                
            }else{// can not find a candidate node, break the loop and try next node
                if(hasPacked) numPackedNode += 1;
                break;
            }
        }
    }
    newDfg->topoSortNodes();
    std::cout << "internale attribute: " << std::endl;
    for(auto& elem : newDfg->nodes()){
        if(elem.second->isPacked()){
            std::cout << "packed node: " << elem.second->name() << std::endl;
            for(auto& EuInputs : elem.second->internalAttrMap()){
                std::string opName = EuInputs.first;
                for(auto& eachOperand : EuInputs.second){
                    std::cout << "src: " << eachOperand.second.srcName << " port: " << eachOperand.second.srcPort << " dst: " << opName << " operand: " << eachOperand.first << std::endl;
                    // if(eachOperand.second.srcName == "Second_Const"){
                    //     // int operand = eachOperand.first;
                    //     // int width = eachOperand.second.width;
                    //     newNode->addInternalAttribute(newDfg->getEuType(candidateSucNode->operation()), "Second_Const", eachOperand.second.srcPort, candidateSucNode->immIdx(cgWidth), cgWidth);
                    // }
                }
            }
        }
    }
    setDFG(newDfg, true);
    newDfg->dumpDFG(resultDir, true);
    newDfg->setNumPacked(numPackedNode);
    // std::cout << "numPackedNode: " << numPackedNode << " traverse Time: " << traverseTime << std::endl;
    std::cout << "numPackedNode: " << numPackedNode << " packpre: " << packPreTime << " packsuc: " << packSucTime<< std::endl;
}