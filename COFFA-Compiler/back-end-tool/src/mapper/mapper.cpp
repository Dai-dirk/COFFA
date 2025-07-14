
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
    // int Nmax = _adg->iobToSpadBanks().begin()->second.size();
    int Nmax = std::min(8, (int)_adg->iobToSpadBanks().begin()->second.size());//@yuan: for test hw
    int maxBankNum = _adg->numIobNodes();
    //@yuan: we need to check whether the sum of N exceeds the architecture limitation
    if(!modify) setStartTime();
    _dfg->setOverflag(Bmax, DateinByte);
    _dfg->genBankingSolution(modify, Nmax, Bmax, maxBankNum);
    // _dfg->updateIterDist();//@yuan_ddp
    if(!modify) std::cout << "Partition Scheme generation time(s): " << runningTimeMS()/1000 << std::endl;
    // exit(0);
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
            if(dfgNode->operation() == "LUT"){
                if(gpeNode->hasLUT()){
                   candidatesCnt++; 
                }
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
bool Mapper::preMapCheck(ADG* adg, DFG* dfg){
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
    if(adg->numGpeNodes() < (dfg->nodes().size() - dfg->ioNodes().size() - dfg->lutNodes().size())){
        std::cout << "This DFG has too many computing nodes!\n";
        return false;
    }
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

// // map the DFG to the ADG
// bool Mapper::mapping(){

// }


// mapper with running time
bool Mapper::mapperTimed(){
    setStartTime();
    // check if the DFG can be mapped to the ADG according to the resources
    if(!preMapCheck(getADG(), getDFG())){
        return false;
    }
    // std::cout << "fine-grain mapping, before mapper!!" << std::endl;
    // exit(0);
    std::cout << "Pre-map checking passed!\n";
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
    bool res = mapperTimed();
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

