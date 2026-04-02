
#include "dfg/dfg.h"
#include "rtlil/rtlil_ir.h"

std::string getAfterApostrophe(const std::string& str) {
    size_t pos = str.find('\'');
    if (pos != std::string::npos && pos + 1 < str.length()) {
        return str.substr(pos + 1);
    }
    return ""; 
}

std::string getAfterUnderScore(const std::string& str) {
    size_t pos = str.find('_');
    if (pos != std::string::npos && pos + 1 < str.length()) {
        return str.substr(pos + 1);
    }
    return ""; 
}

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
    std::set<int> edgeId;
    for(auto& inEdges : dfgNode->inputEdges()){
        for(auto elem : inEdges.second){
            edgeId.insert(elem.second);
        }
    }
    for(auto& outEdges : dfgNode->outputEdges()){
        for(auto outEdgesWithSameWidth : outEdges.second){
            for(auto elem : outEdgesWithSameWidth.second){
                edgeId.insert(elem);
            }
        }
    }
    for(auto& elem : edgeId){
        delEdge(elem);
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
    // std::cout << "once topo~" << std::endl;
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
    this->_numPckedNode = that._numPckedNode;
    this->_numArgIn = that._numArgIn;
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
    int criticalBackEdge;// the backedge causes the largest II
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
            // std::cout << "(lat+iterDist-1)/iterDist): " << (lat+iterDist-1)/iterDist << " _MII: " << _MII << std::endl;
            if(((lat+iterDist-1)/iterDist) >= _MII) criticalBackEdge = elem.first;
            _MII = std::max(_MII, (lat+iterDist-1)/iterDist);
        }
    }
    //delete the loop that is already satisfy the dependency by setting the _MII
    // std::cout << "criticalBackEdge: " << criticalBackEdge << std::endl;
    for(auto& elem : _backEdgeLoops){
        DFGEdge* realEdge = _edges[elem.first];
        int srcId = realEdge->srcId();
        int dstId = realEdge->dstId();
        if(realEdge->bitWidth() == 1 && isIONode(srcId) && isIONode(dstId) && criticalBackEdge != elem.first){//should delete the edges: 1. between memory nodes; 2. does not cause the largest _MII
            delEdge(elem.first);
            deleteBackEdgeLoop(elem.first);
            _backEdges.erase(elem.first);
            if(!getOutNodes().count(dstId)){
                DFGNode* dstNode = node(dstId);
                if(dstNode->inputEdges(1).size() == 0){
                    if(dstNode->operation() == "CINPUT"){
                        dstNode->setOperation("INPUT");
                    }else if(dstNode->operation() == "CLOAD"){
                        dstNode->setOperation("LOAD");
                    }
                }
            }
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
    // std::cout << "before delete edge size: " << edges().size()<< std::endl;
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
    // std::cout << "after delete edge size: " << edges().size()<< std::endl;
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
    // std::cout << "node0 name: " << node0->name() << " node1 name: " << node1->name() << std::endl;
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
// void DFG::partitionCheck(int maxBank){
//     while(true){
//         for(auto& elem :  multiportIO){ 


//         }
//     }
// }

void DFG::dumpDFG(std::string resultDir, bool isPack){//@hw_yuan_op
    std::string filename;
    if(isPack){
        filename = resultDir + "/packedDFG.dot";
    }else{
        filename = resultDir + "/finalDFG.dot";
    }
    std::ofstream ofs(filename);
    int dfgId = id();
    ofs << "Digraph G {\n";
    for(auto& elem : nodes()){
        auto Node = elem.second;
        //int nodeWidth = 32;
        auto name = Node->name();
        name.erase(std::remove(name.begin(), name.end(), '$'), name.end());
        std::string nodeOp = Node->operation();
        std::string quoteName = "\"" + name + "\"";
        //std::cout << "Node name: " << name << std::endl;
        if(Node->operation() == "LUT"){
            //nodeWidth = 1;
            int LUTsize = Node->LUTsize();
            std::string LUTconfig = Node->LUTconfig();
            LUTconfig.erase(std::remove(LUTconfig.begin(), LUTconfig.end(), '\''), LUTconfig.end());
            ofs << quoteName << "[" <<"opcode = " << nodeOp << ", id= " <<Node->id() << ", LUT_size = " << LUTsize <<", LUT_config = " << LUTconfig<< ", label = \"\\N\\nlat=" << Node->opLatency()<< "\"];\n";
        }else if(isIONode(elem.first)){
            std::string nodeName = Node->name();
            ofs << quoteName << "[" <<"opcode = " << nodeOp << ", id= " <<Node->id()<<", label = \"\\N\\nlat=" << Node->opLatency()<< "\"];\n";
            DFGIONode* ioNode = dynamic_cast<DFGIONode*>(Node);
            if(!ioNode->dependencyNodes().empty()){
                for(auto c : ioNode->dependencyNodes()){
                    std::string srcName = node(c)->name();
                    srcName.erase(std::remove(srcName.begin(), srcName.end(), '$'), srcName.end());
                    std::string quoteSrcName = "\"" + srcName + "\"";
                    ofs << quoteSrcName << "->" << quoteName ;
                    ofs << "[style = dashed, color = blue];\n";
                }
            }
        }
        else{
            ofs << quoteName << "[" <<"opcode = " << nodeOp  << ", id= " <<Node->id()<< ", label = \"\\N\\nlat=" << Node->opLatency()<< "\"];\n";
        }
        for(auto& inedge : Node->inputEdges()){
            int width = inedge.first;
            for(auto& edges: inedge.second){
                int portIndex = edges.first;
                DFGEdge* Edge = edge(edges.second);
                if(Edge->isMemEdge()) continue;
                int srcNodeId = Edge->srcId();
                bool isBackEdge = Edge->isBackEdge();
                //int srcIndex = edge->srcPortIdx();
                int operandIdx = Edge->dstPortIdx();
                int addDelay = Edge->addDelay();
                std::string color;
                if(width == 1){
                    color = "red";
                }else{
                    color = "black";
                }
                std::string srcName = node(srcNodeId)->name();
                srcName.erase(std::remove(srcName.begin(), srcName.end(), '$'), srcName.end());
                std::string quoteSrcName = "\"" + srcName + "\"";
                ofs << quoteSrcName << "->" << quoteName ;
                ofs << "[Width = " << width << ", operand = " << portIndex <<", color = " << color<< ", label =" << "\"op = " << operandIdx << "\\ndelay = " << addDelay;
                if(isBackEdge){
                    int iterDist = Edge->iterDist();
                    ofs << "\\niterDist = " << iterDist<<"\""<< ", backedge = 1, style = dashed, iterdist = " << iterDist<<"];\n";
                }else{
                    ofs<<"\""<< "];\n";
                }
            }
        }
        for (auto& elem : Node->bitWidths()){
                //std::cout << "bit width: "<< elem << std::endl;
            /*if(node->operation() == "lut"){
                std::cout << "bit width: "<< elem << std::endl;
            }*/
            if(Node->hasImm(elem)){
                std::string constname = "const_" + std::to_string(Node->id());
                std::string conName = "\"" + constname + "\"";
                int value = Node->imm(elem).second;
                ofs << conName << "[" <<"opcode = CONST, value = " << value << "];\n";
                int constPort  = Node->immIdx(elem);
                ofs << conName << "->" << quoteName ;
                ofs << "[" << "operand = " << constPort << ", label =" << "\"op = " << constPort << "\"];\n";
            }   
        }
        if(Node->has2ndImm()){
            std::string constname = "const_" + std::to_string(Node->id()) + "_2";
            std::string conName = "\"" + constname + "\"";
            int value = Node->get2ndImm();
            ofs << conName << "[" <<"opcode = CONST, value = " << value << "];\n";
            int constPort  = Node->get2ndImmIdx();
            ofs << conName << "->" << quoteName ;
            ofs << "[" << "operand = " << constPort << ", label =" << "\"op = " << constPort << "\"];\n";

        }
        //print the information of internal connections, just for debug
        // if(Node->isPacked()){
        //     std::cout << "packed node: " << Node->name() << " internal connection size: " << Node->internalAttrMap().size() <<" and connections: " << std::endl;
        //     for(auto& elem : Node->internalAttrMap()){
        //         std::string opName = elem.first;
        //         for(auto& eachPort: elem.second){
        //             std::cout << "src: " << eachPort.second.srcName << " port: " << eachPort.second.srcPort << " to dst: " << opName << " port: " <<  eachPort.first << " width: " << eachPort.second.width << std::endl;
        //         }
        //     }
        // }
    }
    ofs << "}\n";

}


//@hw_yuan: get the Eu's type for one specific operation, keep the same with hardware design
std::string DFG::getEuType(std::string op){		
	if (op == "ADD" || op == "SUB" || op == "ULE" || op == "ULT" || op == "UGT" || op == "UGE" || op == "SLT" || op == "SLE"|| op == "SGT" || op == "SGE") {
		return "ARITH";
	} else if (op == "AND" || op == "OR" || op == "NOT" || op == "XOR" || op == "XNOR" || op == "NE" || op == "EQ" || op == "PASS") {
		return "LOGIC";
	} else if (op == "SHL" || op == "LSHR" || op == "CSHL" || op == "CSHR" || op == "ASHR") {
		return "SHIFT";
	} else if (op == "FADD" || op == "FSUB" ){//@yuan_fp
		return "FADDSUB";
	}else if (op == "FEQ" || op == "FLT" || op == "FLE"){
		return "FCMP";
	} else if (op == "FEXADD" || op == "FEXSUB") {
		return "FEAS";
	} else if (op == "FDIV" || op == "FSQRT") {
		return "FDIVSQRT";
	}else{
		return op;
	}
}

//@hw_yuan_op: update the DFG, where the custom Op is replaced by the sub-DFG
void DFG::UpdateCustomOPandArgIn(bool viz, std::string resultDir){
    int maxNodeId = nodes().rbegin()->first; // std::map auto sort the key
    int maxEdgeId = edges().rbegin()->first;  
    std::vector<DFGNode*> customNode;
    for(auto& elem: nodes()){
        DFGNode* currentNode = elem.second;
        if(isIONode(currentNode->id()) || currentNode->accumulative() || currentNode->initSelection() || currentNode->operation() == "LUT"){ // these nodes never be custom OP
            continue;
        }
        //find the custom OP based on the operation
        std::string operation = currentNode->operation();
        if(operation.substr(0, 2) != "MY") continue;
        customNode.push_back(currentNode);
    }
    for(auto& currentNode : customNode){
        std::string operation = currentNode->operation();
        std::cout << "Begain to transform: " << operation << std::endl;
        std::string rtlilPath = "./CustomOP/Lib/" + operation + ".rtlil";
        std::ifstream ifs(rtlilPath);
        if(!ifs){
            std::cout << "Cannot open RTLIL file: " << operation << ".rtlil" << std::endl;
            exit(1);
        }
        RTLIL_IR* rtlil = new RTLIL_IR();
        std::string line;
        bool newCell = true;
        std::string cellName;
        std::string outName;//for each sub-DFG, there is only one output node, since the function can only have one returen value
        std::map<std::string, int> lineStart = {
            {"cell", 1},
            {"wire", 2},
            {"parameter", 3},
            {"connect", 4},
            {"end", 5},
        };
        while (getline(ifs, line)){
            line.erase(0, line.find_first_not_of(" "));
            std::string spLine;
            std::istringstream in(line);
            std::vector<std::string> pline;
            while(getline(in, spLine, ' ')){
                pline.push_back(spLine);
            }
            if(pline.empty()) // empty line
                continue;
            // std::cout << " " << std::endl;
            // for(auto elem : pline){
            //     std::cout << elem << std::endl;
            // }
            int caseValue = lineStart[pline[0]];  
            switch (caseValue)
            {
                case 1:{// current line start with "cell"
                    newCell = true; // start with cell means a new cell
                    std::string opName = pline[1].substr(1, pline[1].length() - 1);
                    std::transform(opName.begin(), opName.end(), opName.begin(), toupper);
                    cellName = pline[2];
                    // std::cout << "opName: " << opName << " cellName: " << cellName << std::endl;
                    Cell_IR* cell = new Cell_IR();
                    cell->setName(cellName);
                    cell->setOperation(opName);
                    rtlil->addCellIR(cell);
                    if(opName == "OUTPUT"){
                        outName = cellName;
                    }
                    break;
                }
                case 2:{ // current line start with "wire"
                    // std::cout << "a new wire: " << std::endl; 
                    // for(auto elem : pline){
                    //     std::cout << elem << std::endl;
                    // }
                    // std::cout << std::endl;
                    int width;
                    std::string wireName;
                    Wire_IR* wire = new Wire_IR();
                    //@yuan: different kinds of wires have different size of pline
                    if(pline.size() > 4){// coarse-grained input/output edge
                        width = std::atoi(pline[2].c_str());
                        wireName = pline[5];
                        if(pline[3] == "input"){
                            int inIdx = std::atoi(pline[4].c_str());
                            wire->setInputId(inIdx);
                            wire->setIsInput(true);
                        }else if(pline[3] == "output"){
                            wire->setIsOutput(true);
                        }
                    }else if(pline.size() > 2){
                        //fine-grained input/output edge
                        if(pline[1] == "input"){
                            width = 1;
                            wireName = pline[3];
                            int inIdx = std::atoi(pline[2].c_str());
                            wire->setInputId(inIdx);
                            wire->setIsInput(true);
                        }else if(pline[1] == "output"){
                            width = 1;
                            wireName = pline[3];
                            wire->setIsOutput(true);
                        }else if(pline[1] == "width"){// coarse-grained internal edge
                            width = std::atoi(pline[2].c_str());
                            wireName = pline[3];
                        }
                    }else{// fine grained internal edge
                        width = 1;
                        wireName = pline[1];

                    }
                    wire->setName(wireName);
                    wire->setWidth(width);
                    rtlil->addWireIR(wire);
                    break;
                }  
                case 3:{ // current line start with "parameter"
                    Cell_IR* cell = rtlil->getCellIR(cellName);
                    std::string parName;
                    int basePlineIdx = 1;
                    if(pline[1] == "signed"){
                        basePlineIdx += 1;
                    }
                    parName = pline[basePlineIdx].substr(1, pline[basePlineIdx].length() - 1);
                    // std::cout << parName << std::endl;
                    //TODO: maybe we can using LUT, making the CGRA more like FPGA
                    if(cell->operation() == "CONST"){ // const cell has 2 parameters: the const value; the outport width
                        if(parName == "VALUE"){
                            int pos = pline[basePlineIdx + 1].find_first_of('\'');
                            std::string value_s = pline[basePlineIdx + 1];
                            int64_t value = 0;
                            if(pos != -1){
                                value_s = pline[basePlineIdx + 1].substr(pos + 1, pline[basePlineIdx + 1].length() - 1);
                                value = std::stoll(value_s, 0, 2);
                            }else{
                                value = std::atoll(value_s.c_str());
                            }
                            //std::cout << "value_s: " << value_s << std::endl;
                            cell->setValue(value);
                        }else{// default has one outport
                            int width = std::atoi(pline[basePlineIdx + 1].c_str());
                            cell->setOutportWidth(0, width);
                        }
                    }else{// other cell's parameters
                        if(parName.length() > 5 && parName.substr(2, 6) == "WIDTH"){
                            char index = parName.at(0);
                            int width = std::atoi(pline[basePlineIdx + 1].c_str());
                            //std::cout << "index: " << index << std::endl;
                            if(int(index) >= 65 && int(index) < 89){ // inport parameter
                                if(cell->operation() == "INPUT"){ // input cell don't set input port
                                    break;
                                }    
                                cell->setInportWidth(int(index) - 65, width);
                            }else{// outport parameter
                                if(cell->operation() == "OUTPUT"){ // output cell don't set output port
                                    break;
                                }    
                                cell->setOutportWidth(int(index) - 89, width);
                            }
                        }else{//TODO: here is the configuration for XCore
                            // std::cout << "parName: "<<parName<<" pre parameter: " <<  pline[basePlineIdx + 1] << std::endl;
                            // std::cout << "paramater: " << pline[basePlineIdx + 1] << std::endl;
                            pline[basePlineIdx + 1].erase(std::remove(pline[basePlineIdx + 1].begin(), pline[basePlineIdx + 1].end(), '\"'), pline[basePlineIdx + 1].end());
                            cell->setParameters(parName, pline[basePlineIdx + 1]);
                        }
                    }
                    break;
                }
                case 4:{ // current line start with "connect"
                    Cell_IR* cell = rtlil->getCellIR(cellName);
                    char portName = pline[1].at(1);
                    int portASCII = int(portName);
                    if(cell->operation() == "INPUT" || cell->operation() == "CONST"){// input and const cell don't set the input wire
                        char portName = pline[1].at(1);
                        int portASCII = int(portName);
                        if(portASCII >= 89){// only set output port connections, port_name > Y
                            int portIndex = portASCII - 89;
                            std::string wireName = pline[2];
                            cell->addOutputWire(portIndex, wireName);
                            Wire_IR* wire = rtlil->getWireIR(wireName);
                            if(!wire){
                                std::cout << "Error! The output port of INPUT/CONST cell connects to an unknown wire!" << std::endl;
                                exit(1);
                            }else{
                                int portWidth = cell->getOutportWidth(portIndex); 
                                if(portWidth != wire->Width()){
                                    std::cout << "Error! The output port of INPUT/CONST cell connects to an unmatch width wire!" << std::endl;
                                    exit(1);
                                }
                                wire->setSrc(cellName, portIndex);
                            }
                        }
                    }else if(cell->operation() == "OUTPUT"){// output cell don't set the output wire
                        char portName = pline[1].at(1);
                        int portASCII = int(portName);
                        if(portASCII < 89){// only set input port connections, port_name < Y
                            int portIndex = portASCII - 65;
                            std::string wireName = pline[2];
                            cell->addInputWire(portIndex, wireName);
                            Wire_IR* wire = rtlil->getWireIR(wireName);
                            if(!wire){
                                std::cout << "Error! The input port of OUTPUT cell connects to an unknown wire!" << std::endl;
                                exit(1);
                            }else{
                                int portWidth = cell->getInportWidth(portIndex); 
                                if(portWidth != wire->Width()){
                                    std::cout << "Error! The input port of OUTPUT cell connects to an unmatch width wire!" << std::endl;
                                    exit(1);
                                }
                                wire->setDst(cellName, portIndex);
                            }
                        }
                    }else{// other cells both contain input and output wires
                        char portName = pline[1].at(1);
                        int portASCII = int(portName);
                        std::string wireName = pline[2];
                        Wire_IR* wire = rtlil->getWireIR(wireName);
                        if(!wire){
                            std::cout << "Error! The port of functional cell connects to an unknown wire!" << std::endl;
                            exit(1);
                        }
                        if(portASCII < 89){// set input wires
                            int portIndex = portASCII - 65;
                            int portWidth = cell->getInportWidth(portIndex); 
                            cell->addInputWire(portIndex, wireName);
                            if(portWidth != wire->Width()){
                                std::cout << "Error! The input port of functional cell connects to an unmatch width wire!" << std::endl;
                                exit(1);
                            }
                            wire->setDst(cellName, portIndex);
                        }else{// set output wires
                            int portIndex = portASCII - 89;
                            int portWidth = cell->getOutportWidth(portIndex); 
                            cell->addOutputWire(portIndex, wireName);
                            std::cout << "cell name: " << cellName << " portWidth: " << portWidth << " wire width: " << wire->Width() << std::endl;
                            if(portWidth != wire->Width()){
                                std::cout << "Error! The output port of functional cell connects to an unmatch width wire!" << std::endl;
                                exit(1);
                            }
                            wire->setSrc(cellName, portIndex);
                        }
                    }
                    break;
                }  
                case 5:{ // current line start with "end"
                    if(newCell == true){
                        newCell = false;
                        cellName = " ";
                    }
                    break;
                }    
                default:
                    break;
            }
        }
        std::cout << "Parsing the sub-DFG of " << operation << ", begin to merge it into the original DFG~"<< std::endl;
        std::cout << "current dfg size: " << nodes().size() << std::endl;
        //using BFS to replace the original custom node
        std::queue<std::string> cellQue; // the cell to be traversed
        std::map<std::string, int> visitedCell; // <cell name, corresponding node's id>
        std::map<int, std::set<DFGEdge*>> externalEdges; // <width, set<DFGEdge*>>
        std::unordered_set<std::string> cellInQueue;
        cellQue.push(outName);
        while(!cellQue.empty()){
            std::string currentName = cellQue.front();
            std::cout << "currentName" << currentName << std::endl;
            Cell_IR* cell = rtlil->getCellIR(currentName);
            DFGNode* newNode = new DFGNode();
            newNode->setId(++maxNodeId);
            newNode->setOperation(cell->operation());
            newNode->setName(cell->operation() +std::to_string(maxNodeId));
            if(cell->operation() != "OUTPUT"){
                addNode(newNode);
            }
            //@yuan_xcore: set the configuration of the xcore node based on the parameter
            if(cell->operation() == "XCORE"){
                xcoreAttr currentAttr;
                for(auto& param : cell->Parameters()){
                    // std::cout << "paramName: " << param.first << " parameter: " << param.second << std::endl;
                    std::string paraName = param.first;
                    if(paraName == "n"){
                        std::string nString = getAfterApostrophe(param.second);
                        // std::cout << "paramName: " << param.first << " real para: " << std::stoul(nString, nullptr, 2) << std::endl;
                        currentAttr.n = std::stoul(nString, nullptr, 2);
                    }else if(paraName == "float_flag"){
                        if(param.second == "0"){
                            currentAttr.float_flag = false;
                        }else{
                            currentAttr.float_flag = true;
                        }
                    }else if(paraName == "bias_sel"){
                        std::string biasSelString = getAfterApostrophe(param.second);
                        // std::cout << "paramName: " << param.first << " real para: " << std::stoul(biasSelString, nullptr, 2) << std::endl;
                        currentAttr.bias_sel = std::stoul(biasSelString, nullptr, 2);
                    }else if(paraName == "break_points"){
                        std::string breakPointString = getAfterApostrophe(param.second);
                        size_t length = breakPointString.length();
                        // std::cout << "paramName: " << param.first << " real para: " << breakPointString << std::endl;
                        const size_t chunkSize = 32;
                        for (size_t i = 0; i < length; i += chunkSize) {
                            std::string chunk = breakPointString.substr(i, chunkSize);
                            // std::cout << "i: " << i << " chunk: " << chunk << std::endl;
                            auto& breakPointsMap = currentAttr.break_points;
                            breakPointsMap[4 - i/32] = std::stoul(chunk, nullptr, 2); // should be reversed
                        }
                    }else if(paraName == "constant_bias"){
                        std::string biasString = getAfterApostrophe(param.second);
                        size_t length = biasString.length();
                        // std::cout << "paramName: " << param.first << " real para: " << biasString << std::endl;
                        const size_t chunkSize = 32;
                        for (size_t i = 0; i < length; i += chunkSize) {
                            std::string chunk = biasString.substr(i, chunkSize);
                            // std::cout << "i: " << i << " chunk: " << chunk << std::endl;
                            auto& biasMap = currentAttr.constant_bias;
                            biasMap[5 - i/32] = std::stoul(chunk, nullptr, 2); // should be reversed
                        }
                    }else if(paraName.at(0) == 'K'){
                        int index = std::stoi(getAfterUnderScore(paraName));
                        std::string kString = getAfterApostrophe(param.second);
                        // std::cout << "paramName: " << param.first << " index: " << index <<" real para: " << kString << std::endl;
                        auto& kMap = currentAttr.k_in;
                        kMap[index] = std::stoul(kString, nullptr, 2);
                    }else if(paraName.substr(0, 4) == "LOGC"){
                        int segIdx = std::stoi(getAfterUnderScore(paraName));
                        std::string logcString = getAfterApostrophe(param.second);
                        size_t length = logcString.length();
                        // std::cout << "paramName: " << param.first << " real para: " << logcString << std::endl;
                        const size_t chunkSize = 32;
                        for (size_t i = 0; i < length; i += chunkSize) {
                            std::string chunk = logcString.substr(i, chunkSize);
                            // std::cout << "i: " << i << " chunk: " << chunk << std::endl;
                            auto& logcMap = currentAttr.logc;
                            logcMap[segIdx][4 - i/32] = std::stoul(chunk, nullptr, 2); // should be reversed
                        }
                    }else if(paraName == "OPC"){
                        std::string opcString = getAfterApostrophe(param.second);
                        // std::cout << "paramName: " << param.first << " real para: " << std::stoul(opcString, nullptr, 2) << std::endl;
                        currentAttr.opc = std::stoul(opcString, nullptr, 2);
                    }
                }
                newNode->setXcoreAttr(currentAttr);
                if(currentAttr.opc == 4){// polynomial mode
                    if(currentAttr.float_flag){// polynomial in float-point requires 7 cycles
                        newNode->setOpLatency(7);
                    }else{// fixed point
                        newNode->setOpLatency(6);
                    }
                }else{
                    newNode->setOpLatency(Operations::latency(cell->operation()));
                }
            }
            visitedCell[currentName] = newNode->id();
            cellQue.pop();
            for(auto& inWire : cell->inputWires()){//return <input port, wire name>
                Wire_IR* wire = rtlil->getWireIR(inWire.second);
                int portIndex = inWire.first;
                int width = wire->Width();
                std::string srcCellName = wire->getSrc().first;
                Cell_IR* srcCell = rtlil->getCellIR(srcCellName);
                if(srcCell->operation() == "INPUT"){ // it means current Cell needs to connect to the src nodes of the custom op node
                    //connect current cell to the src node
                    // std::cout << "srcCellName: " << srcCellName << std::endl;
                    int opIdx = std::atoi(srcCellName.substr(7, srcCellName.length() - 1).c_str());
                    //@yuan: the src node maybe a constant
                    if(currentNode->hasImm(width) && currentNode->immIdx(width) == opIdx){
                        auto immValue = currentNode->immValue(width);
                        newNode->setImm(width, std::make_pair(opIdx, immValue));
                        continue;
                    }
                    DFGEdge* newEdge = new DFGEdge(++maxEdgeId);
                    DFGEdge * oldSrcEdge = edge(currentNode->inputEdge(width, opIdx));
                    int srcId = oldSrcEdge->srcId();
                    int srcPort = oldSrcEdge->srcPortIdx();
                    int dstId = newNode->id();
                    // std::cout <<"src name: " << node(srcId)->name()<<" srcId: " << srcId << " srcPort: " << srcPort << " dstId: " << dstId << " dstPort: " << opIdx << std::endl;
                    newEdge->setEdge(width, srcId, srcPort, dstId, portIndex);
                    newEdge->setBackEdge(oldSrcEdge->isBackEdge());
                    newEdge->setType(oldSrcEdge->type());
                    newEdge->setIterDist(oldSrcEdge->iterDist());
                    newEdge->setAddDelay(oldSrcEdge->addDelay());
                    externalEdges[width].insert(newEdge);
                }else if(srcCell->operation() == "CONST"){
                    newNode->setImm(width ,std::make_pair(portIndex,srcCell->Value()));
                }else{// the src cell is a normal computing cell
                    if(visitedCell.count(srcCellName)){// the src cell is already visited
                        int srcId = visitedCell[srcCellName];
                        DFGNode* srcNode = node(srcId);
                        int srcPort = wire->getSrc().second;
                        int dstId = newNode->id();
                        //important: one output wire may connect to different cells or a same cell but different ports
                        auto wireDist = wire->getDst();
                        for(auto& dstPort : wireDist[srcCellName]){
                            DFGEdge* newEdge = new DFGEdge(++maxEdgeId);// the new edge connected to new packed node
                            newEdge->setEdge(width, srcId, srcPort, dstId, dstPort);
                            addEdge(newEdge);
                        }
                    }else{// the src cell is not visited
                        if(!cellInQueue.count(srcCellName)){
                            cellQue.push(srcCellName);// just record to te queue, the wire will be recorded when handling the srcCell
                            cellInQueue.insert(srcCellName);
                        }
                    }
                }
            }
            for(auto& outWire : cell->outputWires()){
                Wire_IR* wire = rtlil->getWireIR(outWire.second);
                int outPortIndex = outWire.first;
                int width = wire->Width();
                auto wireDist = wire->getDst();
                for(auto& dst : wireDist){
                    std::string dstCellName =  dst.first;
                    Cell_IR* dstCell = rtlil->getCellIR(dstCellName);
                    if(dstCell->operation() == "OUTPUT"){// it's note that output cell has only one input wire, it means current Cell needs to connect to the dst nodes of the custom op node
                        for(auto& out : currentNode->outputEdges(width)){
                            int outputIdx = out.first;
                            for(auto& outEdgeId : out.second){
                                DFGEdge* oldOutEdge = edge(outEdgeId);
                                int dstId = oldOutEdge->dstId();
                                int dstPort = oldOutEdge->dstPortIdx();
                                int srcId = newNode->id();
                                DFGEdge* newEdge = new DFGEdge(++maxEdgeId);
                                newEdge->setEdge(width, srcId, outputIdx, dstId, dstPort);
                                newEdge->setBackEdge(oldOutEdge->isBackEdge());
                                newEdge->setType(oldOutEdge->type());
                                newEdge->setIterDist(oldOutEdge->iterDist());
                                newEdge->setAddDelay(oldOutEdge->addDelay());
                                externalEdges[width].insert(newEdge);
                            }
                        }
                    }else{
                        if(visitedCell.count(dstCellName)){// only the visited node has a cprresponding DFG node
                            int srcId = newNode->id();
                            int dstId = visitedCell[dstCellName];
                            DFGNode* dstNode = node(dstId);
                            for(auto& dstPort : dst.second){
                                DFGEdge* newEdge = new DFGEdge(++maxEdgeId);
                                newEdge->setEdge(width, srcId, outPortIndex, dstId, dstPort);
                                addEdge(newEdge);
                            }
                        }
                    }
                }
            }
            if(cell->operation() == "OUTPUT") delete newNode;
        }
        delNode(currentNode->id());
        rtlil->clearCellIR();
        rtlil->clearWireIR();
        for(auto& Edges : externalEdges){
            for(auto& Edge : Edges.second){
                addEdge(Edge);
            }
        }
    }
    std::cout << "Begin to Update the constant and ArgIn node!" << std::endl;
    //@yuan_argIn: first, we need to insert the pass node for two constant
    int cgWidth = 0;
    for(auto& elem : nodes()){
        DFGNode* currentNode = elem.second;
        if(isIONode(currentNode->id()) || currentNode->accumulative() || currentNode->initSelection() || currentNode->operation() == "LUT" || currentNode->operation() == "ARGIN"){ // these nodes never have two constants or two constant will not affect them
            continue;
        }
        if(cgWidth <= 1){
            for(auto width : currentNode->bitWidths()){
                if(width != 1){
                    cgWidth = width;
                    break;
                }
            }
        }
        std::cout << "currentNode: " << currentNode->name() << std::endl;
        auto inputedges = currentNode->inputEdges();
        // if(!inputedges.count(cgWidth)) continue;
        // if(currentNode->numInputs(cgWidth) == 0) continue;
        bool hasArgIn = false;
        int numArgIn = 0;
        int argInEdgeId = 0; 
        if(inputedges.count(cgWidth)){
            for(auto& e : currentNode->inputEdges(cgWidth)){//argIn should be coarse-grained
                int edgeId = e.second;
                DFGEdge* inEdge = edge(edgeId);
                DFGNode* srcNode = node(inEdge->srcId());
                if(srcNode->operation() == "ARGIN"){
                    hasArgIn = true;
                    argInEdgeId = edgeId;
                    numArgIn++;
                }
            }
        }
        if(currentNode->has2ndImm()){// two constants, insert the pass node at the second constant
            uint64_t immValue = currentNode->get2ndImm();
            int portIdx = currentNode->get2ndImmIdx();
            currentNode->set2ndImmIdx(-1); // it means delete the second constant for this node
            int srcPort = 0; // the only one port of the added pass node
            DFGNode* newNode = new DFGNode();
            newNode->setId(++maxNodeId);
            newNode->setName("pass"+std::to_string(maxNodeId));
            newNode->setOperation("PASS");
            newNode->addBitWidth(cgWidth);
            newNode->setImm(cgWidth ,std::make_pair(0,immValue));// the added pass node has constant now
            addNode(newNode);
            DFGEdge* newEdge = new DFGEdge(maxNodeId, currentNode->id());
            newEdge->setId(++maxEdgeId); 
            newEdge->setSrcPortIdx(srcPort);
            newEdge->setDstPortIdx(portIdx);
            newEdge->setBitWidth(cgWidth);
            addEdge(newEdge);
        }else if(numArgIn == 1 && currentNode->hasImm(cgWidth)){// one argIn, one constant;
            uint64_t immValue = currentNode->immValue(cgWidth);
            int immIdx = currentNode->immIdx(cgWidth);
            currentNode->clearImm(cgWidth);
            int srcPort = 0; // the only one port of the added pass node
            DFGNode* newNode = new DFGNode();
            newNode->setId(++maxNodeId);
            newNode->setName("pass"+std::to_string(maxNodeId));
            newNode->setOperation("PASS");
            newNode->addBitWidth(cgWidth);
            newNode->setImm(cgWidth ,std::make_pair(0,immValue));// the added pass node has constant now
            addNode(newNode);
            DFGEdge* newEdge = new DFGEdge(maxNodeId, currentNode->id());
            newEdge->setId(++maxEdgeId); 
            newEdge->setSrcPortIdx(srcPort);
            newEdge->setDstPortIdx(immIdx);
            newEdge->setBitWidth(cgWidth);
            addEdge(newEdge);
        }else if (numArgIn == 2){// two argIns, just insert one edge
            DFGEdge* oldEdge = edge(argInEdgeId);
            int srcId = oldEdge->srcId();
            int dstId = oldEdge->dstId();
            int srcPortIdx = oldEdge->srcPortIdx();
            int dstPortIdx = oldEdge->dstPortIdx();
            delEdge(argInEdgeId);
            DFGNode* newNode = new DFGNode();
            newNode->setId(++maxNodeId);
            newNode->setName("pass"+std::to_string(maxNodeId));
            newNode->setOperation("PASS");
            newNode->addBitWidth(cgWidth);
            addNode(newNode);
            DFGEdge* e1 = new DFGEdge(srcId, maxNodeId);//the edge between argIn and Pass
            e1->setId(++maxEdgeId); 
            e1->setSrcPortIdx(srcPortIdx);
            e1->setDstPortIdx(0);
            e1->setBitWidth(cgWidth);
            addEdge(e1);
            DFGEdge* e2 = new DFGEdge(maxNodeId, dstId);//the edge between Pass and CurrentNode
            e2->setId(++maxEdgeId); 
            e2->setSrcPortIdx(0);
            e2->setDstPortIdx(dstPortIdx);
            e2->setBitWidth(cgWidth);
            addEdge(e2);
        }
    }
    //after inserting pass node, we need to handle the argIn as constant
    //traverse the node
    for(auto& elem : nodes()){
        DFGNode* currentNode = elem.second;
        if(isIONode(currentNode->id()) || currentNode->accumulative() || currentNode->initSelection() || currentNode->operation() == "LUT" || currentNode->operation() == "ARGIN"){ // these nodes never have two constants or two constant will not affect them
            continue;
        }
        // std::cout << "currentNode: " << currentNode->name() << std::endl;
        auto inputedges = currentNode->inputEdges();
        if(!inputedges.count(cgWidth)) continue;
        for(auto& e : currentNode->inputEdges(cgWidth)){//argIn should be coarse-grained
            int edgeId = e.second;
            DFGEdge* inEdge = edge(edgeId);
            DFGNode* srcNode = node(inEdge->srcId());
            if(srcNode->operation() == "ARGIN"){
                currentNode->setIsArgIn(true);
                currentNode->setArgIdx(srcNode->getArgIdx());
                // std::cout << "currentNode: " << currentNode->name() << " ArgIdx: " << srcNode->getArgIdx() << std::endl; 
                currentNode->setImm(cgWidth, std::make_pair(inEdge->dstPortIdx(), (uint32_t)INT32_MAX));
            }
        }
    }
    std::set<int> argInId;
    for(auto& elem : nodes()){// delete the argIn nodes
        DFGNode* currentNode = elem.second;
        // std::cout << "currentNode: " << currentNode->name() << std::endl;
        if(currentNode->operation() == "ARGIN"){
            argInId.emplace(currentNode->id());
        }
    }
    for(auto& elem : argInId){// delete the argIn nodes
        delNode(elem);
    }
    // std::cout << "resultDir: " << resultDir << std::endl;
    dumpDFG(resultDir, false);
    // exit(0);

}