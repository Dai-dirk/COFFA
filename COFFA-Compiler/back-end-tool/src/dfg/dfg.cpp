
#include "dfg/dfg.h"

DFG::DFG(){}

DFG::~DFG()
{
    for(auto& elem : _nodes){
        delete elem.second;
    }
    for(auto& elem : _edges){
        delete elem.second;
    }
}


DFGNode* DFG::node(int id){
    if(_nodes.count(id)){
        return _nodes[id];
    } else {
        return nullptr;
    }  
}


DFGEdge* DFG::edge(int id){
    if(_edges.count(id)){
        return _edges[id];
    } else {
        return nullptr;
    }  
}


void DFG::addNode(DFGNode* node){
    int id = node->id();
    _nodes[id] = node;
}


void DFG::addEdge(DFGEdge* edge){
    int id = edge->id();
    _edges[id] = edge;
    int srcId = edge->srcId();
    int dstId = edge->dstId();
    int srcPort = edge->srcPortIdx();
    int dstPort = edge->dstPortIdx();
    int bits = edge->bitWidth();
    if(srcId == _id){ // source is input port
        addInput(bits, srcPort, std::make_pair(dstId, dstPort));
        addInputEdge(bits, srcPort, id);
    } else {
        DFGNode* src = node(srcId);
        assert(src);
        src->addOutput(bits, srcPort, std::make_pair(dstId, dstPort));
        src->addOutputEdge(bits, srcPort, id);
        if(isIONode(srcId)){
            src->addBitWidth(bits);
        }        
    }
    if(dstId == _id){ // destination is output port
        addOutput(bits, dstPort, std::make_pair(srcId, srcPort));
        addOutputEdge(bits, dstPort, id);
    } else{       
        DFGNode* dst = node(dstId);
        assert(dst);
        dst->addInput(bits, dstPort, std::make_pair(srcId, srcPort));
        dst->addInputEdge(bits, dstPort, id);
        dst->addBitWidth(bits);
    }
}


void DFG::delNode(int id){
    DFGNode* dfgNode = node(id);
    for(auto bits : dfgNode->bitWidths()){
        for(auto& elem : dfgNode->inputEdges(bits)){
            delEdge(elem.second);
        }
        for(auto& elem : dfgNode->outputEdges(bits)){
            for(auto eid : elem.second){
                delEdge(eid);
            }        
        }
    }
    _nodes.erase(id);
    delete dfgNode;
}


void DFG::delEdge(int id){
    DFGEdge* e = edge(id);
    int srcId = e->srcId();
    int dstId = e->dstId();
    int srcPortIdx = e->srcPortIdx();
    int dstPortIdx = e->dstPortIdx();
    int bits = e->bitWidth();
    if(srcId == _id){
        delInputEdge(bits, srcPortIdx, id);
        delInput(bits, srcPortIdx, std::make_pair(dstId, dstPortIdx));
    }else{
        DFGNode* srcNode = node(srcId);       
        srcNode->delOutputEdge(bits, srcPortIdx, id);
        srcNode->delOutput(bits, srcPortIdx, std::make_pair(dstId, dstPortIdx));
    }
    if(dstId == _id){
        delOutputEdge(bits, dstPortIdx);
        delOutput(bits, dstPortIdx);
    }else{
        DFGNode* dstNode = node(dstId);
        dstNode->delInputEdge(bits, dstPortIdx);
        dstNode->delInput(bits, dstPortIdx);
    }
    _edges.erase(id);
    delete e;
}


// In nodes: INPUT node, LOAD node without input
std::set<int> DFG::getInNodes(){
    std::set<int> inNodes;
    for(int ioNodeId : _ioNodes){
        DFGNode *ioNode = _nodes[ioNodeId];
        std::string opName = ioNode->operation();
        if((opName == "INPUT") || (opName == "LOAD" && (ioNode->inputs().size() == 0))){
            inNodes.insert(ioNodeId);
        }
    }
    return inNodes;
}

// Out nodes: OUTPUT/STORE node
std::set<int> DFG::getOutNodes(){
    std::set<int> outNodes;
    for(int ioNodeId : _ioNodes){
        DFGNode *ioNode = _nodes[ioNodeId];
        std::string opName = ioNode->operation();
        if((opName == "OUTPUT") || (opName == "COUTPUT") || (opName == "STORE") || (opName == "CSTORE") || (opName == "TSTORE")|| (opName == "TCSTORE")){
            outNodes.insert(ioNodeId);
        }
    }
    return outNodes;
}
// Out nodes: OUTPUT/STORE node
std::set<int> DFG::getEndNodes(){
    std::set<int> endNodes;
    for(auto & elem : nodes()){
        auto dfgNode = elem.second;
        std::string opName = dfgNode->operation();
        int dfgNodeId = dfgNode->id();
        if((opName == "OUTPUT") || (opName == "COUTPUT") || (opName == "STORE") || (opName == "CSTORE")|| (opName == "TSTORE")|| (opName == "TCSTORE")){
            endNodes.insert(dfgNodeId);
            continue;
        }
        bool onlyhasBackout = true; //@yuan: for the node that only has backedge output
        for(auto & outs : dfgNode->outputEdges()){ // every bit-width
            for(auto & out : outs.second){ // every out-port
                for(auto & outEdgeId : out.second){
                    if(!_edges[outEdgeId]->isBackEdge()){
                        onlyhasBackout = false;
                        break;
                    }
                }
                if(!onlyhasBackout) break;
            }
            if(!onlyhasBackout) break;
        }
        if(onlyhasBackout){
            endNodes.insert(dfgNodeId);
        }
    }
    return endNodes;
}

bool DFG::isMultiportIoNode(int id){
    if(isIONode(id)){
        DFGIONode *ionode = dynamic_cast<DFGIONode*>(node(id));
        return _multiportIOs.count(ionode->memRefName());
    }
    return false;
}

// sort dfg nodes in topological order
// depth-first search
void DFG::dfs(DFGNode* node, std::map<int, bool>& visited){
    int nodeId = node->id();
    if(visited.count(nodeId) && visited[nodeId]){
        return; // already visited
    }
    visited[nodeId] = true;
    for(auto& ins : node->inputs()){
        for(auto& in : ins.second){
            int inNodeId = in.second.first;
            if(inNodeId == _id){ // node connected to DFG input port
                continue;
            }
            int inEdgeId = node->inputEdge(ins.first,in.first); // input-index
            int srcNodeId = edge(inEdgeId)->srcId();
            if(edge(inEdgeId)->isBackEdge()){ // skip back edge
                // std::cout << "skipp backedge~~~" << std::endl;
                continue;
            }
            // std::cout << "into innode dfs: " << _nodes[inNodeId]->name() << std::endl;
            dfs(_nodes[inNodeId], visited); // visit input node
        }
    }
    // std::cout << "topo node add: " << node->name() << std::endl;
    _topoNodes.push_back(nodeId);
}

// sort dfg nodes in topological order
void DFG::topoSortNodes(){
    _topoNodes.clear();
    std::map<int, bool> visited; // node visited status
    for(auto& outNodeId : getEndNodes()){
        dfs(_nodes[outNodeId], visited); // visit output node
    }
}



// ====== operators >>>>>>>>>>
// DFG copy
DFG& DFG::operator=(const DFG& that){
    if(this == &that) return *this;
    this->_id = that._id;
    this->_bitWidths = that._bitWidths;
    this->_inputNames = that._inputNames;
    this->_outputNames = that._outputNames;
    this->_inputs = that._inputs;
    this->_outputs = that._outputs;
    this->_inputEdges = that._inputEdges;
    this->_outputEdges = that._outputEdges;
    this->_ioNodes = that._ioNodes;
    this->_topoNodes = that._topoNodes;
    this->_lutNodes = that._lutNodes;
    this->_CG_Width = that._CG_Width;
    this->_MII = that._MII;
    this->_MPII = that._MPII;
    this->_backEdgeLoops = that._backEdgeLoops;
    this->_backEdges = that._backEdges;
    this->_multiportIOs = that._multiportIOs;
    this->_multiportIOSteps = that._multiportIOSteps;
    this->_multiportIObr = that._multiportIObr;
    this->_multiportBnakingSolutions = that._multiportBnakingSolutions;
    for(auto& elem : that._nodes){
        int id = elem.first;
        DFGNode* node;
        if(that._ioNodes.count(id)){
            DFGIONode *ioNode = new DFGIONode();
            *ioNode = *(dynamic_cast<DFGIONode*>(elem.second));
            node = ioNode;
        }else{
            node = new DFGNode();
            *node = *(elem.second);
        }        
        this->_nodes[id] = node;
    }
    // this->_edges = that._edges;
    for(auto& elem : that._edges){
        int id = elem.first;
        DFGEdge* edge = new DFGEdge();
        *edge = *(elem.second);
        this->_edges[id] = edge;
    }
    return *this;
}



void DFG::print(){
    printGraph();
    for(auto& elem : _nodes){
        elem.second->print();
    }
}


//DFS for cycles
void DFG::DFS_loop(int headId, int currentId,
    std::list<DFGEdge*>* t_erasedEdges, std::list<DFGEdge*>* t_currentCycle,
    std::list<std::list<DFGEdge*>*>* t_cycles) {
  for (auto edge: edges()) {
    if (std::find(t_erasedEdges->begin(), t_erasedEdges->end(), edge.second) != t_erasedEdges->end())
      continue;
    //if the edge connect to IOB, there is no cycle uses the edge
    if(isIONode(edge.second->dstId()))
      continue;
    // check whether the Id is equal
    if (edge.second->srcId() == currentId) {
      // skip the visited nodes/edges:
      if (std::find(t_currentCycle->begin(), t_currentCycle->end(), edge.second) != t_currentCycle->end()) {
        continue;
      }
      t_currentCycle->push_back(edge.second);

      if (edge.second->dstId() == headId) {
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        std::cout << "[detected one loop]: ";
        std::cout << node(headId)->name();
        std::list<DFGEdge*>* temp_cycle = new std::list<DFGEdge*>();
        for (DFGEdge* currentEdge: *t_currentCycle) {
          temp_cycle->push_back(currentEdge);
          std::cout << " -> " ;
          std::cout << node(currentEdge->dstId())->name() ;
        }
        std::cout << std::endl;
        t_erasedEdges->push_back(edge.second);
        edge.second->setBackEdge(true);
        t_cycles->push_back(temp_cycle);
        t_currentCycle->remove(edge.second);
      } else {
        DFS_loop(headId, edge.second->dstId(), t_erasedEdges, t_currentCycle, t_cycles);
      }
    }
  }
  if (t_currentCycle->size()!=0) {
    t_currentCycle->pop_back();
  }
}

//new get all the cycles
void DFG::getLoops() {
  std::list<std::list<DFGEdge*>*>* cycleLists = new std::list<std::list<DFGEdge*>*>();
  std::list<DFGEdge*>* currentCycle = new std::list<DFGEdge*>();
  std::list<DFGEdge*>* erasedEdges = new std::list<DFGEdge*>();
  cycleLists->clear();
  for (auto node: nodes()) {//DFS for all the cycles
    currentCycle->clear();
    DFS_loop(node.first, node.first, erasedEdges, currentCycle, cycleLists);
  }
  //int cycleID = 0;
  for (std::list<DFGEdge*>* cycle: *cycleLists) {
    _loops.push_back(cycle);
  }
//   return cycleLists;
}


// multiport Load/Store
void DFG::detectMultiportIOs(){
    // std::map<std::string, std::pair<std::vector<int>, std::vector<int>>> tmpMultiportIOs;
    // _multiportIOs.clear();
    _multiportIObr.clear();
    _multiportIOSteps.clear();
    std::map<std::string, std::map<int, std::set<int>>> _eachBrnodes; // <mem-name, <branch, <node-id>>>
    // std::map<std::string, std::set<int>> _multiportConflict;
    std::set<std::string> _partitionMemName;
    // auto outNodeIds = getOutNodes(); // OUTPUT/STORE nodes
    for(int id : _ioNodes){
        DFGIONode* ionode = dynamic_cast<DFGIONode*>(node(id));
        if(ionode->MultiportType() > 0){
            auto memName = ionode->memRefName();
            // std::cout << "array name: " << memName << std::endl;
            // if(outNodeIds.count(id)){
            //     _multiportIOs[memName].second.push_back(id);
            // }else{
            //     _multiportIOs[memName].first.push_back(id);
            // }
            int branch = ionode->Branch();
            // std::cout << "branch: " << branch << std::endl;
            if(_multiportIObr.count(memName)){
                if(_multiportIObr[memName] < branch){
                    _multiportIObr[memName] = branch;
                }
            }else{
                _multiportIObr[memName] = branch;
            }
            if(ionode->MultiportType() > 1 && !_partitionMemName.count(memName)){ //@yuan: the IOB with partitioned memory
                _partitionMemName.emplace(memName);
            }
            if(ionode->MultiportType() > 0){
                int step = ionode->ControlStep();
                _multiportIOSteps[memName][step].push_back(id);
            }
            _eachBrnodes[memName][branch].emplace(id);
        }        
    }
    int MPII = 1;
    for(auto &elem: _multiportIOs){
        int curBrmax = INT32_MIN;
        if(_partitionMemName.count(elem.first)) continue;
        for(auto &br: _eachBrnodes[elem.first]){
            int num = br.second.size();
            curBrmax = std::max(curBrmax, num);
        }
        std::cout << "array: " << elem.first << " curBrmax: " << curBrmax << std::endl;
        assert(curBrmax >= 1);
        // _multiportIOs[elem.first] = elem.second;
        MPII = std::max(curBrmax, MPII);
    }
    for(auto &elem : _multiportIOSteps){
        int accessII = elem.second.size();
        assert(accessII >= 1);
        MPII = std::max(accessII, MPII);
    }
    _MPII = MPII;
    std::cout << "_MPII: " << _MPII << std::endl;
    _MII = std::max(_MII, MPII);
}



int DFG::getMultiportNum(std::string name){ 
    auto ids = _multiportIOs[name];
    return ids.first.size() + ids.second.size();
}


int DFG::getMultiportNum(DFGNode* node, std::string name){ 
    auto ids = _multiportIOs[name];
    int multiportSize = ids.first.size() + ids.second.size();
    auto IONode = dynamic_cast<DFGIONode*>(node);
    int N = IONode->NumMultiportBank();
    return std::max(N, multiportSize);
}

// detect loops based on backedge
void DFG::detectBackEdgeLoops(){
    _backEdgeLoops.clear();
    _backEdges.clear();
    for(auto elem : _edges){
        int curBackEdgeId = elem.first;
        DFGEdge *curBackEdge = elem.second;
        if(!curBackEdge->isBackEdge()){
            continue;
        }
        curBackEdge->setrealBackEdge(false);
        _backEdges.insert(curBackEdgeId);
        int headNodeId = curBackEdge->dstId(); // loop head
        int tailNodeId = curBackEdge->srcId(); // loop tail     
        int width = curBackEdge->bitWidth();
        // std::cout << "detect edge from: " << node(tailNodeId)->name() << " -> " << node(headNodeId)->name() << " width: " << width  << " Id: " << curBackEdgeId<< std::endl;
        std::vector<int> edgeStack;
        std::map<int, bool> visited;
        edgeStack.push_back(curBackEdgeId);
        while(!edgeStack.empty()){
            int topEid = *edgeStack.rbegin();
            DFGEdge *topEdge = _edges[topEid];
            int srcNodeId = topEdge->srcId();
            bool found = false;
            // std::cout << "srcnode: " << node(srcNodeId)->name() << std::endl;
            for(auto & Inegdes : _nodes[srcNodeId]->inputEdges()){
                // if(Inegdes.first != width) continue;
                for(auto &elem : Inegdes.second){ // find next edges
                    int eid = elem.second;
                    DFGEdge *edge = _edges[eid];
                    // std::cout << "src detect edge from: " << node(edge->srcId())->name() << " -> " << node(edge->dstId())->name() << " width: " << width <<" Id: " << eid<< std::endl;
                    if(!edge->isBackEdge() && (!visited.count(eid) || !visited[eid])){
                        edgeStack.push_back(eid);                    
                        if(edge->srcId() == headNodeId){ // find a loop
                            auto loop = edgeStack;
                            std::reverse(loop.begin(), loop.end());                        
                            std::stringstream ss;
                            ss << "Detected a loop: ";
                            for(int loopedge : loop){
                                int loopSrcNodeId = _edges[loopedge]->srcId();
                                ss << _nodes[loopSrcNodeId]->name() << " -> ";                           
                            }
                            ss << "<-";
                            spdlog::warn("{0}", ss.str()); 
                            loop.pop_back();
                            _backEdgeLoops[curBackEdgeId].push_back(loop); // record loop, do not record the backedge
                            visited[eid] = true;
                            edgeStack.pop_back();
                        }else{
                            found = true;
                            break;
                        }                    
                    }
                }
                if(found) break;
            }    
            if(!found){
                visited[*edgeStack.rbegin()] = true;
                edgeStack.pop_back();
            }  
        }
    }
    // std::cout << "~~~~~~~~~~~~~~~~~" << std::endl;
    // set MII
    // for(auto &elem: _backEdgeLoops){
    //     std::cout << "backedge id: " << elem.first << std::endl;
    // }
    for(auto &elem: _backEdgeLoops){
        // std::cout << "backedge id: " << elem.first << std::endl; 
        DFGEdge * realEdge = _edges[elem.first];
        int iterDist = realEdge->iterDist();    
        iterDist = std::max(iterDist, 1); // >= 1  
        realEdge->setrealBackEdge(true); 
        // std::cout << "iterDist: " << iterDist << std::endl;
        for(auto &loop : elem.second){ // calculate loop latency only considering operation latency 
            // std::cout << "loop size:  " << loop.size() << std::endl;
            int lat = _nodes[_edges[*loop.begin()]->srcId()]->opLatency();
            // std::cout << "begin lat: " << lat << std::endl;
            for(int eid : loop){
                lat += _nodes[_edges[eid]->dstId()]->opLatency();
            }
            // std::cout << "lat: " << lat << std::endl;
            _MII = std::max(_MII, (lat+iterDist-1)/iterDist);
        }
    }
    spdlog::warn("MII = {0}", _MII); 
    // std::cout << "_MPII: " << _MPII << std::endl;
    // exit(0);
}

// delete back edge loop
void DFG::deleteBackEdgeLoop(int backedgeId){
    auto backLoops = backEdgeLoops();
    if(backLoops.count(backedgeId)){
        backLoops.erase(backedgeId);
    }
}
// delete memory-dependent edge
void DFG::deleteMemEdge(){
    std::cout << "before delete edge size: " << edges().size()<< std::endl;
    for(auto iobNode : ioNodes()){
        DFGIONode* IOnode = dynamic_cast<DFGIONode*>(node(iobNode));
        // std::cout << "nodename: " << IOnode->name() << " edge size: " << IOnode->inputEdges().size() << std::endl;
        for(auto& inEdge : IOnode->inputEdges()){
            if(inEdge.first != 1) continue;
            for(auto& elem : inEdge.second){
                int EdgeId = elem.second;
                DFGEdge* e = edge(EdgeId);
                if(e->isMemEdge()){
                    delEdge(EdgeId);
                }else if(e->isBackEdge() && e->isDontTouch()){ //@yuan: we also need to delete the edges with long iteration-distance
                    delEdge(EdgeId);
                }
            }
        }
    }
    std::cout << "after delete edge size: " << edges().size()<< std::endl;
}



//@yuan: generate all the banking solutions
void DFG::genBankingSolution(bool modify, int Nmax, int Bmax, int maxBank){
    _multiportIOs.clear();
    std::map<std::string, std::vector<int>> multiportIO;
    auto outNodeIds = getOutNodes(); // OUTPUT/STORE nodes
    int dataWidthinByte = CGWidth() / 8;
    for(int id : _ioNodes){
        DFGIONode* ionode = dynamic_cast<DFGIONode*>(node(id));
        if(ionode->MultiportType() > 0){
            auto memName = ionode->memRefName();
            // std::cout << "array name: " << memName << std::endl;
            if(outNodeIds.count(id)){
                _multiportIOs[memName].second.push_back(id);
            }else{
                _multiportIOs[memName].first.push_back(id);
            }
            if(!modify)
                multiportIO[memName].push_back(id);
        }        
    }
    //@yuan: we should using memory duplication when the spm space is available 
    std::set<std::string> visited_name;
    for(int id : _ioNodes){
        DFGIONode* ionode = dynamic_cast<DFGIONode*>(node(id));
        auto memName = ionode->memRefName();
        if(ionode->MultiportType() > 0 && !visited_name.count(memName)){
            if(_multiportIOs[memName].second.size() == 0){//@yuan: we consider the situation that the array only has input nodes  
                int dataSize = ionode->memSize();
                if(dataSize / dataWidthinByte > Bmax){//@yuan: the array with a too big size to duplicate
                    visited_name.emplace(memName);
                    continue;
                }
                for(auto& elem : multiportIO[memName]){
                    DFGIONode* eachNode = dynamic_cast<DFGIONode*>(node(elem));
                    eachNode->setMultiportType(0);
                }
                visited_name.emplace(memName);
                _multiportIOs.erase(memName);
                multiportIO.erase(memName);
                // // then, we should judge whether using partition or duplication
                // auto BS_queue = _multiportBnakingSolutions[memName];
                // auto bankSolutionTop = BS_queue.top();
                // BS_queue.pop();
                // auto bankSolutionNext = BS_queue.top();
                // if(bankSolutionTop.II != 1 || bankSolutionNext.II != 1){//@yuan: partition can not find a solution with II = 1, using duplication can have better II
                //     for(auto& elem : multiportIO[memName]){
                //         DFGIONode* eachNode = dynamic_cast<DFGIONode*>(node(elem));
                //         eachNode->setMultiportType(0);
                //     }
                //     visited_name.emplace(memName);
                //     _multiportIOs.erase(memName);
                //     multiportIO.erase(memName);
                // }
            }
        }        
    }
    if(!modify){//@yuan: modify DFG doesn't affect the IOB Node
        // int B_MAX = 4096; int N_MAX = 8;
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
        // std::cout << pyCMD << "\n";
        PyRun_SimpleString(pyCMD.c_str());
        _multiportBnakingSolutions.clear();
        _numPyInit = 1;
        //pattern: std::vector<std::pair<int, int>>
        for(auto& elem :  multiportIO){ //@yuan: generate all the N,B,II for each array
            auto arrayName = elem.first;
            auto confLSNodeIDs = elem.second;
            std::cout << "Handling array ------- " << arrayName << "\n";
            std::cout << "array num ------- " << confLSNodeIDs.size() << "\n";
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
            // vector<L/SNode pairs>
            // L/SNode pairs means the numbers (in this loop) of conflicting memory access
            
            for(int N = 1; N <= Nmax; N *= 2){
            // for(int N = 1; N <= 1; N *= 2){ //@yuan: for test
                bool exceedSize =false;
                bool badPartition =false;
                for(int B = 1; B <= Bmax; B *= 2){
                    if(exceedSize) break;
                    bool noConflict = true;
                    std::vector<std::pair<int, int>> ConflictTable;
                    for(int i = 0; i < confLSNodeIDs.size(); i++){
                        auto Node0 = dynamic_cast<DFGIONode*>(node(confLSNodeIDs[i]));
                        int dataSize = Node0->memSize();
                        // std::cout << "dataSize: " << dataSize/dataWidthinByte << std::endl;
                        if(((dataSize/dataWidthinByte) <= N * B)){//@yuan: meaningless partition
                            exceedSize = true;
                        }else if((dataSize/dataWidthinByte) > N * Bmax){
                            badPartition = true;
                            break;
                        }
                        auto Pattern0 = Node0->pattern();
                        // std::cout << "node name: " << Node0->name() << " pattern size: " << Pattern0.size() << std::endl;
                        int offset0 = Node0->memOffset() + Node0->reducedMemOffset();
                        auto ABC_0 = getABC(Pattern0);
                        auto srcOp = Node0->operation();
                        int srcWoPattern = srcOp == "LOAD" || srcOp == "STORE" || srcOp == "CLOAD" || srcOp == "CSTORE"|| srcOp == "TLOAD" || srcOp == "TSTORE"|| srcOp == "TCLOAD" || srcOp == "TCSTORE";
                        for(int j = i+1; j < confLSNodeIDs.size(); j++){
                            //@yuan:if the node is load/store with undetermined pattern, it conflict with any other node
                            if(srcWoPattern){
                                ConflictTable.push_back(std::make_pair(i, j));
                                noConflict = false;
                                continue;
                            }
                            auto Node1 = dynamic_cast<DFGIONode*>(node(confLSNodeIDs[j]));
                            auto dstOp = Node1->operation();
                            int dstWoPattern = dstOp == "LOAD" || dstOp == "STORE" || dstOp == "CLOAD" || dstOp == "CSTORE"|| dstOp == "TLOAD" || dstOp == "TSTORE"|| dstOp == "TCLOAD" || dstOp == "TCSTORE";
                            //@yuan:if the node is load/store with undetermined pattern, it conflict with any other node, meanwhile N = 1 means all the nodes are conflict
                            if(dstWoPattern || N == 1){
                                ConflictTable.push_back(std::make_pair(i, j));
                                noConflict = false;
                                continue;
                            }
                            //@yuan: if there is flow-depedency between two nodes, we think they are conflict
                            // if(hasFlowDependency(Node0, Node1)){
                            //     ConflictTable.push_back(std::make_pair(i, j));
                            //     noConflict = false;
                            //     continue;
                            // }
                            auto Pattern1 = Node1->pattern();
                            int offset1 = Node1->memOffset() + Node1->reducedMemOffset();
                            auto ABC_1 = getABC(Pattern1);
                            int coffs[10] = {0};
                            int counts[3] = {0};
                            for(int l = 0; l < Pattern0.size(); l++){
                                coffs[l] = ABC_0[l] / dataWidthinByte;
                                coffs[l+dataWidthinByte] = ABC_1[l] / dataWidthinByte;
                                counts[l] = Pattern0[l].second;
                            }
                            coffs[3] = offset0 / dataWidthinByte;
                            coffs[7] = offset1 / dataWidthinByte;
                            coffs[8] = N;
                            coffs[9] = B;
                            if(conflictpolytope(coffs, counts)){
                            // if(true){//@yuan: for test COFFA without partition
                                ConflictTable.push_back(std::make_pair(i, j));
                                noConflict = false;
                            }
                        }
                    }
                    if(badPartition) break;
                    BankingSolution currBS;
                    currBS.B = B;
                    currBS.N = N;
                    if(noConflict){
                        currBS.II = 1;
                        currBS.scheduledSteps[0].insert(currBS.scheduledSteps[0].begin(), confLSNodeIDs.begin(), confLSNodeIDs.end());
                    }else{
                        currBS.II = graph_color_for_II(confLSNodeIDs.size(), ConflictTable);
                        std::map<int, int> Mem2CtrlStep = graph_color_for_CtrlStep(confLSNodeIDs.size(), ConflictTable);
                        for(auto &elem : Mem2CtrlStep){
                            int nodeID = confLSNodeIDs[elem.first];
                            // std::cout << "tem Node: " << LSNode->getName() << "\n";
                            // std::cout << "elem.first: " << elem.first;
                            // std::cout << " elem.second: " << elem.second;
                            currBS.scheduledSteps[elem.second].push_back(nodeID);
                        }
                    }
                    _multiportBnakingSolutions[arrayName].push(currBS);
                    if( N == 1) break; //@yuan: for 1 bank, this means no partition
                }
            }
            // std::cout << "multiport Bnaking Solutions ------- " << arrayName << "\n";
            auto BS_queue = _multiportBnakingSolutions[arrayName];
            if(BS_queue.empty()){
                std::cout << "[Fusion Error] The size of array " << arrayName << " is too big to have a feasible partition!" << std::endl;
                exit(0);
            }
            // while(!BS_queue.empty()){
            //     auto elem = BS_queue.top();
            //     BS_queue.pop();
            //     std::cout << "II: " << elem.II << " N: " << elem.N << " B: " << elem.B << "\n";
            // }
            //@yuan: we should update the N,B,step for the first time
            // updateBankingSettings(arrayName, true);
        }
        visited_name.clear();
        for(auto& elem : multiportIO){
            std::string refname = elem.first;
            if(!visited_name.count(refname)){
                visited_name.emplace(refname);
                updateBankingSettings(refname, true);
            }
            // for(auto& id: elem.second){
            //     DFGIONode* ionode = dynamic_cast<DFGIONode*>(node(id));
            //     ionode->setMultiportType(0);
            // }
        }
        //8、结束python接口初始化
        Py_Finalize();
        //@yuan: we need to check whether the sum of N exceeds the architecture limitation. and then update the solution
        while(true){
            int curBankNum = 0;
            std::string maxArray;//@yuan: the array with maximum N
            int maxN = 0;//used maximum N
            int maxII = 0;//maximum AII
            for(auto& elem : multiportIO){
                BankingSolution curSolution = getCurrBankingSolution(elem.first);
                int curN = curSolution.N;
                int curII = curSolution.II;
                curBankNum += curSolution.N;
                if(curN > maxN){
                    maxArray = elem.first;
                    maxN = curN;
                    maxII = curII;
                }else if (curII > maxII){
                    maxArray = elem.first;
                    maxN = curN;
                    maxII = curII;
                }
            }
            if(_multiportBnakingSolutions[maxArray].size() == 1){
                break;
            }
            for(auto& elem : ioNodes()){
                DFGIONode* ionode = dynamic_cast<DFGIONode*>(node(elem));
                if(ionode->MultiportType() == 0){
                    curBankNum += 1;
                }
            }
            if(curBankNum > maxBank){//@yuan: the used bank exceeds the upper bound
                updateBankingSettings(maxArray, false);
            }else{
                break;
            }
            // std::cout << "curBankNum: " << curBankNum << std::endl;
        }
        // std::cout << "final soulution: " << "\n";
        // for(auto& elem : multiportIO){
        //     BankingSolution curSolution = getCurrBankingSolution(elem.first);
        //     int curN = curSolution.N;
        //     int curII = curSolution.II;
        //     int curB = curSolution.B;
        //     std::cout << "name: " << elem.first << "II: " << curII<< " N: " << curN << " B: " << curB << std::endl;
        // }
        // _multiportIOs.clear(); 
        // for(auto& elem : multiportIO){
        //     std::string refname = elem.first;
        //     for(auto& id: elem.second){
        //         DFGIONode* ionode = dynamic_cast<DFGIONode*>(node(id));
        //         ionode->setMultiportType(0);
        //     }
        // }

    }
}

//@yuan: update the N, B, Control step for the array
void DFG::updateBankingSettings(std::string memName, bool isFirst){
    if(!_multiportBnakingSolutions.count(memName))
        return;
    if(_multiportBnakingSolutions[memName].size() == 1) //@yuan: only 1 solution left
        return;
    if(!_multiportIOs.count(memName))
        return;
    if(_multiportIOSteps.count(memName)){
        _multiportIOSteps[memName].clear();
    }
    std::cout << "update partition!!!!!" << std::endl;
    if(!isFirst) _multiportBnakingSolutions[memName].pop();
    BankingSolution currentSolution =  _multiportBnakingSolutions[memName].top();
    int N = currentSolution.N;
    int B = currentSolution.B;
    int MII = currentSolution.II;
    int type = 1;
    if(N > 1 && MII == 1){//@yuan: partition with no conflict 
        type = 3;
    }else if(N > 1 && MII > 1){//@yuan: partition with conflict 
        type = 2;
    }
    auto shceduleResult = currentSolution.scheduledSteps;
    std::set<int> ids;
    for(auto& elem : _multiportIOs[memName].first){ //@yuan: set the control step for input nodes
        ids.emplace(elem);
        DFGIONode* ionode = dynamic_cast<DFGIONode*>(node(elem));
        ionode->setNumMultiportBank(N);
        ionode->setMultiportBankSize(B);
        ionode->setMultiportType(type);
        bool isSet = false;
        for(auto& step : shceduleResult){
            for(auto& id : step.second){
                if(id == elem){
                    ionode->setControlStep(step.first);
                    _multiportIOSteps[memName][step.first].push_back(elem);
                    isSet = true;
                    break;
                }
            }
            if(isSet) break;
        }
    }   
    for(auto& elem : _multiportIOs[memName].second){ //@yuan: set the control step for output nodes
        ids.emplace(elem);
        DFGIONode* ionode = dynamic_cast<DFGIONode*>(node(elem));
        ionode->setNumMultiportBank(N);
        ionode->setMultiportBankSize(B);
        ionode->setMultiportType(type);
        bool isSet = false;
        for(auto& step : shceduleResult){
            for(auto& id : step.second){
                if(id == elem){
                    ionode->setControlStep(step.first);
                    _multiportIOSteps[memName][step.first].push_back(elem);
                    isSet = true;
                    break;
                }
            }
            if(isSet) break;
        }
    }
    
    //@yuan: update the II
    _MII = std::max(_MII, MII); 
    _MPII = std::max(_MPII, MII);
    // std::cout << "update schedule II: " << _MII << std::endl;
}

void DFG::setOverflag(int maxB, int DateinByte){
    for(int id : _ioNodes){
        DFGIONode* ionode = dynamic_cast<DFGIONode*>(node(id));
        int memSize = ionode->memSize() / DateinByte;
        if(memSize > maxB){
            ionode->setOversie(true);
        }
    }
}

bool DFG::hasFlowDependency(DFGIONode* node0, DFGIONode* node1){
    if(!getOutNodes().count(node0->id()) && !getOutNodes().count(node1->id())){//@yuan: for two input/output nodes, there is no flow-dependency
        return false;
    }else if(getOutNodes().count(node0->id()) && getOutNodes().count(node1->id())){
        return false;
    }
    std::cout << "node0 name: " << node0->name() << " node1 name: " << node1->name() << std::endl;
    if(!getOutNodes().count(node0->id())){ //@yuan: node0 is input node
        for(auto& edges : node0->inputEdges()){
            if(edges.first != 1) continue;
            for(auto& elem : edges.second){ //then, the control edge comes from node1
                DFGEdge * Edge = edge(elem.second);
                if(Edge->isBackEdge() && Edge->srcId() == node1->id()){
                    return true;
                }
            }
        }
    }else{//@yuan: node1 is input node
        for(auto& edges : node1->inputEdges()){
            if(edges.first != 1) continue;
            for(auto& elem : edges.second){ //then, the control edge comes from node0
                DFGEdge * Edge = edge(elem.second);
                if(Edge->isBackEdge() && Edge->srcId() == node0->id()){
                    return true;
                }
            }
        }
    }
    return false;
}

void DFG::updateIterDist(){
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
    // std::cout << pyCMD << "\n";
    PyRun_SimpleString(pyCMD.c_str());
    // if(_numPyInit == 0){
    //     // 1、初始化python接口  
    //     Py_Initialize();
    //     if(!Py_IsInitialized()){
    //         std::cout << "python init fail\n";
    //     }
    //     // 2、初始化python系统文件路径，保证可以访问到 .py文件
    //     PyRun_SimpleString("import sys");
    //     std::string str1 = "sys.path.append('";
    //     std::string str2 = PROJECT_PATH;
    //     std::string str3 = "/src/Py_Tools')";
    //     std::string pyCMD = str1 + str2 + str3;
    //     // std::cout << pyCMD << "\n";
    //     PyRun_SimpleString(pyCMD.c_str());
    // }
    int dataWidthinByte = CGWidth() / 8;
    for(auto elem : edges()){
        DFGEdge* edge = elem.second;
        if(edge->isDynamicDist()){
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
            DFGIONode* srcNode = dynamic_cast<DFGIONode*>(node(edge->srcId()));
            DFGIONode* dstNode = dynamic_cast<DFGIONode*>(node(edge->dstId()));
            auto PatternSrc = srcNode->pattern();
            auto PatternDst = dstNode->pattern();
            auto ABC_Src = getABC(PatternSrc);
            auto ABC_Dst = getABC(PatternDst);
            int offsetSrc = srcNode->memOffset() + srcNode->reducedMemOffset();
            int offsetDst = dstNode->memOffset() + dstNode->reducedMemOffset();
            int coffs[8] = {0};
            int counts[3] = {0};
            //@yuan_ddp: the input node first
            for(int l = 0; l < PatternDst.size(); l++){
                coffs[l] = ABC_Src[l] / dataWidthinByte;
                coffs[l+4] = ABC_Dst[l] / dataWidthinByte;
                counts[l] = PatternDst[l].second;
            }
            coffs[3] = offsetSrc / dataWidthinByte;
            coffs[7] = offsetDst / dataWidthinByte;
            int minIterDist = minimumIterDistGen(coffs, counts);
            std::cout << "Setting iteration distance for edge from: "<<srcNode->name() << " to " << dstNode->name() << " with: " << minIterDist << std::endl; 
            edge->setIterDist(minIterDist);
        }
    }
    // exit(0);
    //8、结束python接口初始化
//    Py_Finalize();

}

// bool DFG::hasFlowDependency(DFGIONode* node0, DFGIONode* node1){
//     if(!getOutNodes().count(node0->id()) && !getOutNodes().count(node1->id())){//@yuan: for two input/output nodes, there is no flow-dependency
//         return false;
//     }else if(getOutNodes().count(node0->id()) && getOutNodes().count(node1->id())){
//         return false;
//     }
//     if(!getOutNodes().count(node0->id())){ //@yuan: node0 is input node
//         for(auto& elem : node0->inputEdges(1)){ //then, the control edge comes from node1
//             DFGEdge * Edge = edge(elem.second);
//             if(Edge->isBackEdge() && Edge->srcId() == node1->id()){
//                 return true;
//             }
//         }
//     }else{//@yuan: node1 is input node
//         for(auto& elem : node1->inputEdges(1)){ //then, the control edge comes from node0
//             DFGEdge * Edge = edge(elem.second);
//             if(Edge->isBackEdge() && Edge->srcId() == node0->id()){
//                 return true;
//             }
//         }
//     }
//     return false;
// }
// void DFG::partitionCheck(int maxBank){
//     while(true){
//         for(auto& elem :  multiportIO){ 


//         }
//     }
// }