
#include "llvm_cdfg.h"

//#include "cgra.h"



LLVMCDFG::~LLVMCDFG()
{
    for(auto& elem : _nodes){
        delete elem.second;
    }
    for(auto& elem : _edges){
        delete elem.second;
    }
}


LLVMCDFGNode* LLVMCDFG::node(int id)
{
    if(_nodes.count(id)){
        return _nodes[id];
    }
    return NULL;
}

LLVMCDFGNode* LLVMCDFG::node(Instruction *ins)
{
    if(_insNodeMap.count(ins)){
        return _insNodeMap[ins];
    }
    return NULL;
}


void LLVMCDFG::addNode(LLVMCDFGNode *node)
{
    int id = node->id();
    if(_nodes.count(id)){
        return;
    }
    _nodes[id] = node;
    if(auto ins = node->instruction()){
        _insNodeMap[ins] = node;
    }
}

// create node according to instruction and add node
LLVMCDFGNode* LLVMCDFG::addNode(Instruction *ins)
{
    if(_insNodeMap.count(ins)){
        return _insNodeMap[ins];
    }
    // create new node
    LLVMCDFGNode *node = new LLVMCDFGNode(ins, this);
    if(dyn_cast<LoadInst>(ins)){
        _loadList.insert(node);
        _arrayName2LSNode[node->getLSArrayName()].first.insert(node);
    }else if(dyn_cast<StoreInst>(ins)){
        _storeList.insert(node);
        _arrayName2LSNode[node->getLSArrayName()].second.insert(node);
    }
    int id = 0;
    if(!_nodes.empty()){
        id = _nodes.rbegin()->first + 1;
    }
    node->setId(id);
    node->setBB(ins->getParent());
    _nodes[id] = node;
    _insNodeMap[ins] = node;
    return node;
}

// create node according to the custom instruction and add node
LLVMCDFGNode* LLVMCDFG::addNode(std::string customIns, BasicBlock *BB)
{
    LLVMCDFGNode *node = new LLVMCDFGNode(this);
    node->setCustomInstruction(customIns);
    int id = 0;
    if(!_nodes.empty()){
        id = _nodes.rbegin()->first + 1;
    }
    node->setId(id);
    node->setBB(BB);
    _nodes[id] = node;    
    return node;
}

// create node according to the custom instruction for pattern which is not with BB: TODO ?
LLVMCDFGNode* LLVMCDFG::addNode(std::string customIns)
{
    LLVMCDFGNode *node = new LLVMCDFGNode(this);
    node->setCustomInstruction(customIns);
    int id = 0;
    if(!_nodes.empty()){
        id = _nodes.rbegin()->first + 1;
    }
    node->setId(id);
    _nodes[id] = node;    
    return node;
}

//for multi-store of subtasks
LLVMCDFGNode* LLVMCDFG::addCpNode(Instruction* ins)
{
    // create new node
    LLVMCDFGNode *node = new LLVMCDFGNode(ins, this);
    if(dyn_cast<LoadInst>(ins)){
        _loadList.insert(node);
    }else if(dyn_cast<StoreInst>(ins)){
        _storeList.insert(node);
    }
    int id = 0;
    if(!_nodes.empty()){
        id = _nodes.rbegin()->first + 1;
    }
    node->setId(id);
    node->setBB(ins->getParent());
    _nodes[id] = node;
    //_insNodeMap[ins] = node;
    return node;
}

//for multi-store of subtasks
LLVMCDFGNode* LLVMCDFG::addCpNode(LLVMCDFGNode* node)
{
    LLVMCDFGNode *newnode = new LLVMCDFGNode(this);
    (*newnode) = (*node);
    int id = 0;
    if(!_nodes.empty()){
        id = _nodes.rbegin()->first + 1;
    }
    newnode->setId(id);
    _nodes[id] = newnode;
    return newnode;
}

void LLVMCDFG::delNode(LLVMCDFGNode *node)
{
    if(_loadList.count(node)){
        _loadList.erase(node);
    }
    if(_storeList.count(node)){
        _storeList.erase(node);
    }
    for(auto inputnode:node->inputNodes()){
        //errs() << "tem inputnode is "<<inputnode->getName()<<"\n";
        inputnode->delOutputNode(node);
    }
    for(auto outputnode:node->outputNodes()){
        outputnode->delInputNode(node);
    }
    int id = node->id();
    _nodes.erase(id);
    if(auto ins = node->instruction()){
        _insNodeMap.erase(ins);
    }
    auto inputEdges = node->inputEdges();
    for(int eid : inputEdges){
        delEdge(eid);
    }
    auto outputEdges = node->outputEdges();
    for(int eid : outputEdges){
        delEdge(eid);
    }
    delete node;
}


void LLVMCDFG::delNode(Instruction *ins)
{
    if(!_insNodeMap.count(ins)){
        return;
    }
    auto node = _insNodeMap[ins];
    for(auto inputnode:node->inputNodes()){
        inputnode->delOutputNode(node);
    }
    for(auto outputnode:node->outputNodes()){
        outputnode->delInputNode(node);
    }
    _nodes.erase(node->id());
    _insNodeMap.erase(ins);
    auto inputEdges = node->inputEdges();
    for(int eid : inputEdges){
        delEdge(eid);
    }
    auto outputEdges = node->outputEdges();
    for(int eid : outputEdges){
        delEdge(eid);
    }
    delete node;
}

void LLVMCDFG::delNodeTree(LLVMCDFGNode *node){
    int id = node->id();
    if(!_nodes.count(id)) return;
    auto teminputs = node->inputNodes();
    for(auto prenode : teminputs){
        if(prenode->outputEdges().size() == 1){
            //errs()<< "\tdelete tree of "<< prenode->getName()<<"\n";
            delNodeTree(prenode);
        }
    }
    errs()<< "\tdelete node "<<node->getName()<<"\n";
    delNode(node);
}

void LLVMCDFG::delNodeTree(Instruction *ins){
    if(!_insNodeMap.count(ins)){
        return;
    }
    auto node = _insNodeMap[ins];
    auto teminputs = node->inputNodes();
    for(auto prenode : teminputs){
        if(prenode->outputEdges().size() == 1){
            delNodeTree(prenode);
        }
    }
    delNode(node);
}

void LLVMCDFG::connectNodes(LLVMCDFGNode *src, LLVMCDFGNode *dst, int idx, EdgeType edgeType, bool isBackEdge){
    src->addOutputNode(dst, isBackEdge);
    dst->addInputNode(src, idx, isBackEdge);
    addEdge(src, dst, edgeType);
}


LLVMCDFGEdge* LLVMCDFG::edge(int id)
{
    if(_edges.count(id)){
        return _edges[id];
    }
    return NULL;
}


LLVMCDFGEdge* LLVMCDFG::edge(LLVMCDFGNode *src, LLVMCDFGNode *dst)
{
    for(auto eid : src->outputEdges()){
        assert(_edges.count(eid));
        auto outEdge = _edges[eid];
        if(outEdge->dst() == dst){
            return outEdge;
        }
    }
    return NULL;
}


void LLVMCDFG::addEdge(LLVMCDFGEdge *edge)
{
    int id = edge->id();
    if(!_edges.count(id)){
        _edges[id] = edge;
        edge->src()->addOutputEdge(id);
        edge->dst()->addInputEdge(id);
    }
}


LLVMCDFGEdge* LLVMCDFG::addEdge(LLVMCDFGNode *src, LLVMCDFGNode *dst, EdgeType type)
{
    int eid = 0;
    if(!_edges.empty()){
        eid = _edges.rbegin()->first + 1;
    }
    LLVMCDFGEdge *edge = new LLVMCDFGEdge(eid, type, src, dst);
    _edges[eid] = edge;
    src->addOutputEdge(eid);
    dst->addInputEdge(eid);
    return edge;
}



void LLVMCDFG::delEdge(LLVMCDFGEdge *edge)
{
    int id = edge->id();
    if(_edges.count(id)){
        _edges.erase(id);
    }
    edge->src()->delOutputEdge(id);
    edge->dst()->delInputEdge(id);
    edge->dst()->delInputNode(edge->src());
    edge->src()->delOutputNode(edge->dst());
    delete edge;    
}


void LLVMCDFG::delEdge(int id)
{
    if(!_edges.count(id)){
       return;
    }
    auto edge = _edges[id];
    edge->src()->delOutputEdge(id);
    edge->dst()->delInputEdge(id);
    edge->dst()->delInputNode(edge->src());
    edge->src()->delOutputNode(edge->dst());
    _edges.erase(id);
    delete edge;    
}


// get GEP node info 
std::string LLVMCDFG::GEPInfo(LLVMCDFGNode* node)
{
    assert(_GEPInfoMap.count(node));
    return _GEPInfoMap[node];
}


void LLVMCDFG::setGEPInfo(LLVMCDFGNode* node, std::string name)
{
    _GEPInfoMap[node] = name;
}


// get Input/Output node info 
std::string LLVMCDFG::IOInfo(LLVMCDFGNode* node)
{
    assert(_IOInfoMap.count(node));
    return _IOInfoMap[node];
}


void LLVMCDFG::setIOInfo(LLVMCDFGNode* node, std::string name)
{
    _IOInfoMap[node] = name;
    errs() << "setIOInfo???\n";
}


// initialize CDFG according to loopBBs
void LLVMCDFG::initialize()
{
    std::set<BasicBlock*> allBBs = _loopBBs;
    for(auto &elem : _loopentryBBs){
        allBBs.insert(elem.first);
    }
    for(auto &elem : _loopexitBBs){
        allBBs.insert(elem.second);
    }
    //FIXME: to handle the _taskIOBBs when loopsafetycheck
    // for(auto BB : _taskIOBBs){
    //     allBBs.insert(BB);
    // }
    outs() << ">>>>>>>>>>>>>allBBs: ";
    for(auto bb : allBBs){
        errs() << bb->getName() << ", ";
    }
    outs() << "\n";
    _allBBs = allBBs;
    outs() << ">>>>>>>>>>>>>initialize<<<<<<<<<<<<<\n";
    auto F = (*(allBBs.begin()))->getParent();//to check all ConstantExpr GEP
    for(auto &BB : *F){
        //errs()<< "check BB: ";BB.dump();
        for(auto &I : BB){
            int addressidx;
            GetElementPtrInst *GEP;
            Instruction *ins = &I;
            if(_ExInsList.count(ins)){
                continue;
            }
            if(auto Load = dyn_cast<LoadInst>(ins)){
                GEP = dyn_cast<GetElementPtrInst>(Load->getPointerOperand());
                addressidx = Load->getPointerOperandIndex();
            }else if(auto Store = dyn_cast<StoreInst>(ins)){
                GEP = dyn_cast<GetElementPtrInst>(Store->getPointerOperand());
                addressidx = Store->getPointerOperandIndex();
            }else{
                continue;
            }
            // errs()<< "check LS: ";ins->dump();
            if(ConstantExpr * CE = dyn_cast<ConstantExpr>(ins->getOperand(addressidx))){
                errs()<<"LS has ConstantExpr GEP: "; ins->dump();
                std::vector<Value *> Indices;
                for (unsigned index = 1; index < CE->getNumOperands(); ++index) {
                    Indices.push_back (CE->getOperand(index));
                }
                auto pointeeType = dyn_cast<PointerType>(CE->getOperand(0)->getType()->getScalarType())->getElementType();
                pointeeType->dump();
                GEP = GetElementPtrInst::CreateInBounds(pointeeType,CE->getOperand(0),Indices,CE->getName(),ins);
                ins->replaceUsesOfWith(CE, GEP);
            }
        }
    }

    outs() << ">>>>>>>>>>>>>ExInsList: ";
    for(auto exIns : _ExInsList){
        exIns->dump();
    }
    outs() << "\n";
    // Create CDFG nodes
    for(auto BB : allBBs){
        for(auto &I : *BB){
            if(_ExInsList.count(&I)){
                errs() << "_ExInsList ins: ";(&I)->dump();
                continue;
            }
            addNode(&I);
        }
    }
    // for(auto iter = PHItoLevel.begin(); iter != PHItoLevel.end(); iter++){
    //     auto Ins = iter->first;
    //     addNode(Ins);
    // }
    // create connections
    for(auto BB : allBBs){
        for(auto &I : *BB){
            if(_ExInsList.count(&I)){
                continue;
            }
            Instruction *ins = &I;
            LLVMCDFGNode *node = this->node(ins);            
            // find out-of-loop connections or constant operands for non-phi nodes
            if(!dyn_cast<PHINode>(ins)){
                int idx = 0;
                for(Use &pred : ins->operands()){
                    // add input nodes
                    LLVMCDFGNode *inputNode = NULL;
                    if(dyn_cast<GetElementPtrInst>(ins) && idx == 0){
                        idx++;
                        continue;
                    }
                    if(ConstantInt *CI = dyn_cast<ConstantInt>(pred)){
                        if(dyn_cast<GetElementPtrInst>(ins)){ // GEP Constant will be handled in the handleGEPNodes
                            idx++;
                            continue;
                        }
                        inputNode = addNode("CONST", node->BB());
                        inputNode->setConstVal(CI->getSExtValue());
                    }else if(ConstantFP* FP = dyn_cast<ConstantFP>(pred)){
                        if(dyn_cast<GetElementPtrInst>(ins)){ // GEP Constant will be handled in the handleGEPNodes
                            idx++;
                            continue;
                        }
                        int constant = 0;
                        if(FP->getType()->getTypeID() == Type::DoubleTyID){
                            constant = (DataType)FP->getValueAPF().convertToDouble();
                        }else if(FP->getType()->getTypeID() == Type::FloatTyID){
                            constant = (DataType)FP->getValueAPF().convertToFloat();
                        }else{
                            assert(false && "bad FP type");
                        }
                        inputNode = addNode("CONST", node->BB());
                        inputNode->setConstVal(constant);
                    }else if(Instruction *predIns = dyn_cast<Instruction>(pred)){
                        if(!allBBs.count(predIns->getParent()) || _ExInsList.count(predIns)){ // out of loop BB
                            if(auto LI = dyn_cast<LoadInst>(predIns)){
                                inputNode = addNodeTree(predIns);
                            }else{
                                inputNode = getInputNode(predIns, BB);
                            }
                        }
                    }else if(Argument *arg = dyn_cast<Argument>(pred)){
                        if(!pred->getType()->isPointerTy()){
                            inputNode = getInputNode(arg, BB);
                            inputNode->setCustomInstruction("ArgIn");
                            for(auto iter : _funcArgs){
                                if(arg == iter.second){
                                    inputNode->setArgIn(iter.first);
                                    break;
                                }
                            }
                        }
                    }
                    EdgeType type = EDGE_TYPE_DATA;
                    if(pred->getType()->getPrimitiveSizeInBits() == 1){ //  single-bit operand
                        type = EDGE_TYPE_CTRL;
                    }    
                    if(inputNode){
                        int opidx = idx;
                        if(dyn_cast<SelectInst>(ins) && (idx > 0)){
                            opidx--;
                        }
                        node->addInputNode(inputNode, opidx);
                        inputNode->addOutputNode(node);
                        addEdge(inputNode, node, type);
                    }
                    idx++;
                }
            }
            if(dyn_cast<PHINode>(ins))
                    ins->dump(); 
            for(User *succ : ins->users()){               
                if(Instruction *succIns = dyn_cast<Instruction>(succ)){
                    if(_ExInsList.count(succIns)){
                        continue;
                    }
                    if(dyn_cast<PHINode>(ins)){ 
                        errs()<<"\t";succIns->dump();
                    }
                    LLVMCDFGNode *succNode = NULL;
                    bool isBackEdge = false;
                    if(!allBBs.count(succIns->getParent()) || _ExInsList.count(succIns)){ // out of loop BB
                        errs() << "[BugCheck]: " << "current ins: "; ins->dump();
                        errs() << "[BugCheck]: " << "succ ins: "; succIns->dump();
                        succNode = getOutputNode(succIns, BB);
                        succNode->setInstruction(succIns);
                        //succNode->setInstruction(succIns);//record output direct ins
                    }else if(dyn_cast<PHINode>(succIns)){
                        continue; // not connected here, instead in handlePHINodes
                    }else{
                        succNode = this->node(succIns);
                        // // only if the successor is PHINode, the back edge can exist.
                        // // the following condition should never be true
                        // if(BB != succIns->getParent()){ // not in the same BB
                        //     std::pair<const BasicBlock*, const BasicBlock*> bbp(BB, succIns->getParent());
                        //     if(std::find(_backEdgeBBPs.begin(), _backEdgeBBPs.end(), bbp) != _backEdgeBBPs.end()){
                        //         isBackEdge = true;
                        //     }
                        // }
                    }
                    // get the input index
                    int idx = -1;
                    // reorder the operand of SELECT and STORE node
                    if(auto SI = dyn_cast<SelectInst>(succIns)){
                        if(ins == dyn_cast<Instruction>(SI->getCondition())){
                            idx = 2;
                        }else if(ins == dyn_cast<Instruction>(SI->getTrueValue())){
                            idx = 0;
                        }else if(ins == dyn_cast<Instruction>(SI->getFalseValue())){
                            idx = 1;
                        }else{
                            assert(false);
                        }
                    }else if(auto SI = dyn_cast<StoreInst>(succIns)){
                        if(ins == dyn_cast<Instruction>(SI->getValueOperand())){
                            idx = 0;
                        }else{
                            idx = 1;
                        }
                    }else{
                        for(int i = 0; i < succIns->getNumOperands(); i++){
                            if(succNode->getInputPort(i) == node){//when check the same port, continue to avoid break too early
                                continue;
                            }
                            if(ins == dyn_cast<Instruction>(succIns->getOperand(i))){
                                idx = i;
                                break;
                            }
                        }
                    }   
                    EdgeType type = EDGE_TYPE_DATA;
                    if(ins->getType()->getPrimitiveSizeInBits() == 1){ //  single-bit operand
                        type = EDGE_TYPE_CTRL;
                    }                 
                    node->addOutputNode(succNode, isBackEdge);
                    succNode->addInputNode(node, idx, isBackEdge);
                    addEdge(node, succNode, type);
                }
            }
        }
    }  

    //find output node & check if store
    auto temnode = _nodes;
    for(auto &elem : temnode){
        auto node = elem.second;
        if(node->customInstruction() == "OUTPUT"){
            errs() << node->getName(); node->instruction()->dump();
            assert(node->inputNodes().size() == 1 && "OUTPUT has only 1 input");
            // auto result = node->inputNodes()[0];
            // Instruction* resultins = result->instruction();
            // assert(resultins != NULL);
            /// TODO: how to do in multiTask
            // errs() << "For output: "<<node->getName()<<"\n";
            // auto outputset = addOutputTree(result->instruction());//TODO: ret output maybe deleted
            // for(auto output:outputset){
            //     LLVMCDFGNode* outputNode = output.first;
            //     Instruction* ins = outputNode->instruction();
            //     ins->dump();
            //     result->addOutputNode(outputNode);
            //     outputNode->addInputNode(result, output.second);
            //     EdgeType type = EDGE_TYPE_DATA;
            //     if(resultins->getType()->getPrimitiveSizeInBits() == 1){
            //         type = EDGE_TYPE_CTRL;
            //     }
            //     addEdge(result, output.first, EDGE_TYPE_DATA);
            //     Instruction* outins = output.first->instruction();
            //     assert(outins != NULL);
            // }
            // errs()<< "\n";
            // result->delOutputNode(node);///aviod access this node latter
            // delNode(node);
        }
        // else if(node->customInstruction() == "INPUT"){
        //     //auto tempoutput = node->outputNodes();
        //     for(auto output:node->outputNodes()){
        //         Instruction* ins = output->instruction();
        //         assert(ins != NULL);
        //         int idx = output->getInputIdx(node);
        //         auto operand = ins->getOperand(idx);
        //         auto newnode = addNodeTree(operand);
        //         if(newnode == NULL)
        //             continue;
        //         newnode->addOutputNode(output);
        //         output->addInputNode(newnode, idx);
        //         EdgeType type = EDGE_TYPE_DATA;
        //         if(operand->getType()->getPrimitiveSizeInBits() == 1){
        //             type = EDGE_TYPE_CTRL;
        //         }
        //         addEdge(newnode, output, type);
        //     }
        //     delNode(node);
        // }
        
    }


    errs() << "print _IOInfoMap: ";
    for(auto iter : _IOInfoMap){
        errs() << iter.second << "; ";
    }
    errs() << "\n";
}





// root nodes : no output nodes
std::vector<LLVMCDFGNode *> LLVMCDFG::getRoots()
{
	std::vector<LLVMCDFGNode *> rootNodes;
    for(auto &elem : _nodes){
        if(elem.second->outputNodes().size() == 0){
            rootNodes.push_back(elem.second);
        }
    }
	return rootNodes;
}

// leaf nodes : no input nodes
std::vector<LLVMCDFGNode *> LLVMCDFG::getLeafs()
{
	std::vector<LLVMCDFGNode *> leafNodes;
    for(auto &elem : _nodes){
        if(elem.second->inputNodes().size() == 0){
            leafNodes.push_back(elem.second);
        }
    }
	return leafNodes;
}

// leaf nodes in a BB : no input nodes in this BB or is INPUT/PHI
std::vector<LLVMCDFGNode *> LLVMCDFG::getLeafs(BasicBlock *BB)
{
	std::vector<LLVMCDFGNode *> leafNodes;
    for(auto &elem : _nodes){
        auto node = elem.second;
        if(node->BB() != BB || node->customInstruction() == "INPUT"){
            continue;
        }
        if(node->instruction() && dyn_cast<PHINode>(node->instruction())){
            continue;
        }
        bool flag = true;
        for(auto &inNode : node->inputNodes()){
            if(inNode->BB() == BB && inNode->customInstruction() != "INPUT"){
                if(inNode->instruction() == NULL || !dyn_cast<PHINode>(inNode->instruction())){
                    flag = false;
                    break;
                }
            }
        }
        if(flag){
            leafNodes.push_back(node);
        }
    }
	return leafNodes;
}




// get loop start node. If not exist, create one.
LLVMCDFGNode* LLVMCDFG::getLoopStartNode(BasicBlock *BB)
{
    if(_loopStartNodeMap.count(BB)){
        return _loopStartNodeMap[BB];
    }
    // create new node and add node
    LLVMCDFGNode *node = addNode("LOOPSTART", BB);
    // int cnt = _loopStartNodeMap.size();
    _loopStartNodeMap[BB] = node;
    // node->setConstVal(cnt);
    return node;
}


// get loop exit node. If not exist, create one.
LLVMCDFGNode* LLVMCDFG::getLoopExitNode(BasicBlock *BB)
{
    if(_BB2ExitNodeMap.count(BB)){
        return _BB2ExitNodeMap[BB];
    }
    // create new node and add node
    LLVMCDFGNode *node = addNode("LOOPEXIT", BB);
    // int cnt = _BB2ExitNodeMap.size();
    _BB2ExitNodeMap[BB] = node;
    // node->setConstVal(cnt);
    return node;
}

// get input node. If not exist, create one.
LLVMCDFGNode* LLVMCDFG::getInputNode(Value *ins, BasicBlock *BB)
{
    if(_ioNodeMap.count(ins)){
        return _ioNodeMap[ins];
    }
    // create new node and add node
    LLVMCDFGNode *node = addNode("INPUT", BB);
    _ioNodeMap[ins] = node;
    // _ioNodeMapReverse[node] = ins;
    setIOInfo(node, ins->getName().str());
    return node;
}


// get out loop store node. If not exist, create one.
LLVMCDFGNode* LLVMCDFG::getOutputNode(Value *ins, BasicBlock *BB)
{
    if(_ioNodeMap.count(ins)){
        return _ioNodeMap[ins];
    }
    // create new node and add node
    LLVMCDFGNode *node = addNode("OUTPUT", BB);
    _ioNodeMap[ins] = node;
    // _ioNodeMapReverse[node] = ins;
    setIOInfo(node, ins->getName().str());
    return node;
}


/*
 * This function
 * 1) connects PHI nodes with its parents by analyzing PHI instruction[value,basic block][value, basic block]
 * 2) transfer PHI nodes to SELECT nodes
 * 3) add LOOPSTART control node, INPUT data node
 *
 * */
void LLVMCDFG::handlePHINodes() 
{
	std::vector<LLVMCDFGNode*> phiNodes;
    auto temnodes = _nodes;
	for(auto &elem : temnodes){
        auto &node = elem.second;
        bool isSafetyCheck = false;
		if(node->instruction() && dyn_cast<PHINode>(node->instruction()) && node->customInstruction() != "SELECT"){
            phiNodes.push_back(node);
		}
	}

    // errs() << "_safetyMap:\n";
    // for(auto safe : _safetyMap){
    //     errs() << "level " << safe.first << ": ";
    //     for(auto elem : safe.second){
    //         errs() << elem.first->getName() << " " << elem.second << "; ";
    //     }
    //     errs() << "\n";
    // }
/*
    for(auto &elem : temnodes){
        auto &node = elem.second;
        if(node->instruction() && dyn_cast<PHINode>(node->instruction()) && node->customInstruction() != "SELECT"){
            phiNodes.push_back(node);
        }
    }
*/
    // PHI Node to SELECT node map
    std::map<LLVMCDFGNode*, LLVMCDFGNode*> phi2selectMap;
    // Phi node and its PHI parent before got the above map
    std::map<LLVMCDFGNode*, LLVMCDFGNode*> phi2phiParentMap;
    
	for(LLVMCDFGNode* node : phiNodes){
		PHINode* PHI = dyn_cast<PHINode>(node->instruction());
        outs() << "PHI :: "; PHI->dump();
        ///TODO: is this right to skip PHI?
        // if(!_loopBBs.count(PHI->getParent())){
        //     outs() << "Out of whole loop PHI, delete;\n";
        //     delNode(PHI);
        //     continue;
        // }
        ///TODO: is this right to skip PHI? how to generalize this
        // if((!_loopBBs.count(PHI->getParent())) && (PHItoLevel.count(PHI))){
        //     outs() << "Outter index PHI, generate SELECT;\n";
        //     int idxlevel = PHItoLevel[PHI];
        //     BasicBlock* BB = PHI->getIncomingBlock(0);///TODO::any exception?
        //     auto outputs = node->outputNodes();
        //     std::map<LLVMCDFGNode*, int> nodestoInputidx;
        //     for(auto itr = outputs.begin(); itr != outputs.end(); itr++){
        //         nodestoInputidx[*itr] = (*itr)->getInputIdx(node);
        //     }
        //     delNode(PHI);
        //     auto selNode = addIdxCycle(PHI);//selNode means SELECT here
        //     LLVMCDFGNode* temnode;
        //     for(auto itr = outputs.begin(); itr != outputs.end(); itr++){
        //         int i = 0;
        //         temnode = *itr;
        //         int inputidx = nodestoInputidx[temnode];
        //         if(temnode->getInputPort(inputidx) == selNode){
        //             continue;
        //         }
        //         temnode->addInputNode(selNode, inputidx);
        //         selNode->addOutputNode(temnode);
        //         addEdge(selNode, temnode, EDGE_TYPE_DATA);
        //     }
        //     //create idx level start
        //     LLVMCDFGNode *startnode = addNode("LOOPSTART", BB);
        //     selNode->addInputNode(startnode, 2);
        //     startnode->addOutputNode(selNode);
        //     addEdge(startnode, selNode, EDGE_TYPE_CTRL);

        //     outs() << "new SELECT node = " << selNode->getName() << "\n";
        //     outs() << selNode->getName() << " has inputs: "<<selNode->inputNodes().size()<<"\n";
            
        //     continue;
        // }
		assert(node->inputNodes().empty());
        std::vector<std::pair<LLVMCDFGNode*, LLVMCDFGNode*>> phiParents; // <value-node, control-node>>
        bool hasLoopstart = false;
        int loopstartIdx;
        CondVal branchCond = UNCOND;
		for (int i = 0; i < PHI->getNumIncomingValues(); ++i) {
			BasicBlock* bb = PHI->getIncomingBlock(i);
            bool outterIncoming = true;
			Value* V = PHI->getIncomingValue(i);
			outs() << "IncomingValue :: "; V->dump();
			LLVMCDFGNode* previousCtrlNode = NULL;
            for(int level = 0; level < nestloops().size(); level++){
                if(_LoopexclusiveBBs[level].count(bb)){
                    if(_LoopexclusiveBBs[level].count(node->BB())){
                        outterIncoming = false;
                    }
                    break;
                }
            }
			std::pair<BasicBlock*,BasicBlock*> bbPair = std::make_pair(bb, node->BB());// should be loop entry
			if(outterIncoming && (_loopentryBBs.count(bbPair) || _loopexitBBs.count(bbPair))){ // predecessor not in loopBBs		
                errs() << node->getName() << " has outter predecessor\n";
				// assert(_loopentryBBs.count(bbPair) || _loopexitBBs.count(bbPair)); 
                LLVMCDFGNode *startNode = getLoopStartNode(bb);
				previousCtrlNode = startNode;
                hasLoopstart = true;
                loopstartIdx = i;
			}else{ // within the loopBBs
				BranchInst* BRI = cast<BranchInst>(bb->getTerminator());
				LLVMCDFGNode* BRNode = this->node(BRI);
				if(!BRI->isConditional()){
                    auto &inNodes = BRNode->inputNodes();
                    ///TODO:this is a very abnormal loop with a subsequent unconditional branch to PHINode
                    if(inNodes.size() == 0){
                        errs() << "unconditional jump\n";
                        previousCtrlNode = BRNode;
                    }else{
                        assert(inNodes.size() == 1); // only one conditional predecessor after createCTRLORTree
                        previousCtrlNode = inNodes[0];
                    }
				}else{
					assert(BRNode->inputNodes().size() == 1); // only one condition predecessor
                    LLVMCDFGNode *ctrlNode = BRNode->inputNodes()[0];
                    if(BRI->getSuccessor(0) == node->BB()){ // true condition
                        branchCond = TRUE_COND;
                        previousCtrlNode = ctrlNode;
                    }else{ // false condition
                        assert(BRI->getSuccessor(1) == node->BB());
                        branchCond = FALSE_COND;
                        // find the CTRLNOT node
                        for(auto outNode : ctrlNode->outputNodes()){
                            if(outNode->customInstruction() == "CTRLNOT"){
                                previousCtrlNode = outNode;
                                break;
                            }
                        }
                    }
				}
			}
			assert(previousCtrlNode != NULL);
            outs() << "previousCTRLNode : " << previousCtrlNode->getName() << "\n";
            ///DO NOT connect LoopIdx Safety control
            bool skipThisParent = false;
            for(auto iter : _safetyMap){
                for(auto safetyBB : iter.second){
                    CondVal intoCond = safetyBB.second;
                    BasicBlock* invalidBBbr = safetyBB.first;
                    assert(intoCond != UNCOND && "BAD condition == UNCOND");
                    intoCond = (intoCond == TRUE_COND) ? FALSE_COND : TRUE_COND;
                    // if(bb == invalidBBbr && branchCond == intoCond){
                    if(bb == invalidBBbr){
                        skipThisParent = true;
                        break;
                    }
                }
                
            }
            if(skipThisParent){
                continue;
            }
            // get operand value
            LLVMCDFGNode* phiParent = NULL;
            
            if(ConstantInt* CI = dyn_cast<ConstantInt>(V)){
				int constant = CI->getSExtValue();
                phiParent = addNode("CONST", bb);
                phiParent->setConstVal(constant);
			}else if(ConstantFP* FP = dyn_cast<ConstantFP>(V)){
                int constant = 0;
                if(FP->getType()->getTypeID() == Type::DoubleTyID){
                    constant = (DataType)FP->getValueAPF().convertToDouble();
                }else if(FP->getType()->getTypeID() == Type::FloatTyID){
                    constant = (DataType)FP->getValueAPF().convertToFloat();
                }
                phiParent = addNode("CONST", bb);
                phiParent->setConstVal(constant);
			}else if(UndefValue *UND = dyn_cast<UndefValue>(V)){
                phiParent = addNode("CONST", bb);
                phiParent->setConstVal(0);
			}else if(Argument *ARG = dyn_cast<Argument>(V)){
                phiParent = addNode("ArgIn", bb);
                for(auto iter : _funcArgs){
                    if(ARG == iter.second){
                        phiParent->setArgIn(iter.first);
                        break;
                    }
                }
                //phiParent->setConstVal(0);
			}else{								
                if(Instruction* ins = dyn_cast<Instruction>(V)){
                    phiParent = this->node(ins);
                    if(dyn_cast<PHINode>(ins)){
                        if(!_loopBBs.count(ins->getParent()) && ins->getNumOperands() == 1){//to handle initial from outter lcssa phi
                            V = ins->getOperand(0);
                            if (ConstantInt *CI = dyn_cast<ConstantInt>(V))
                            {
                                int constant = CI->getSExtValue();
                                phiParent = addNode("CONST", bb);
                                phiParent->setConstVal(constant);
                            }
                            else if (ConstantFP *FP = dyn_cast<ConstantFP>(V))
                            {
                                DataType constant = (DataType)FP->getValueAPF().convertToFloat();
                                phiParent = addNode("CONST", bb);
                                phiParent->setConstVal(constant);
                            }
                        }
                        else if(phi2selectMap.count(phiParent)){ // PHI parent has been transformed to SELECT node
                            phiParent = phi2selectMap[phiParent];
                        }else{ // record unresolved PHI parent
                            phi2phiParentMap[node] = phiParent;
                        }                       
                    }				                    
				}
				if(phiParent == NULL){ //not found
                    phiParent = getInputNode(V, bb);
				}
            }
            phiParents.push_back(std::make_pair(phiParent, previousCtrlNode));
        }
        for(int i = 0; i + 1 < phiParents.size(); i += 2){
            // connect two parents to a SELECT node
            // operand 0 : true data; 1 : false data; 2 : condition (conditional node of parent 1)
            auto &pp1 = phiParents[i];
            auto &pp2 = phiParents[i+1];
            // create a SELECT node
            LLVMCDFGNode *selNode = addNode("SELECT", node->BB());
            //if(!PHItoLevel.count(PHI))
                selNode->setInstruction(PHI);///set Instruction if this SELECT is not index
            outs() << "new SELECT node = " << selNode->getName() << "\n";
            errs()<<"\t"<<selNode->getName()<< " in "<< node->BB()->getName() <<"\n";
            errs()<<"\tparent1: "<<pp1.first->getName()<< " in "<< pp1.first->BB()->getName() <<"\n";
            errs()<<"\tparent2: "<<pp2.first->getName()<< " in "<< pp2.first->BB()->getName() <<"\n";
            bool isBackEdge1 = false;
            //backedge is depend on incomingBB, but not which BB having the incomingValue 
            std::pair<const BasicBlock*, const BasicBlock*> bbp1 = std::make_pair(PHI->getIncomingBlock(i), node->BB());
            if(std::find(_backEdgeBBPs.begin(), _backEdgeBBPs.end(), bbp1) != _backEdgeBBPs.end()){
                isBackEdge1 = true;
                errs()<<"\tparent1 to Sel is backedge!!!\n";
            }
            bool isBackEdge2 = false;
            //backedge is depend on incomingBB, but not which BB having the incomingValue 
            std::pair<const BasicBlock*, const BasicBlock*> bbp2 = std::make_pair(PHI->getIncomingBlock(i+1), node->BB());
            if(std::find(_backEdgeBBPs.begin(), _backEdgeBBPs.end(), bbp2) != _backEdgeBBPs.end()){
                isBackEdge2 = true;
                errs()<<"\tparent2 to Sel is backedge!!!\n";
            }
            // swap two operands
            // 1. the second operand is loop initial value
            // 2. the second control node have more successive BBs
            // 3. the first control node is CTRLNOT  
            bool swapOperand = hasLoopstart && loopstartIdx == 1; 
            if(!swapOperand){
                int succBBNum1 = _succBBsMap[pp1.second->BB()].size();
                int succBBNum2 = _succBBsMap[pp2.second->BB()].size();
                bool isNot1 = pp1.second->customInstruction() == "CTRLNOT";
                bool isNot2 = pp2.second->customInstruction() == "CTRLNOT";
                swapOperand = (succBBNum1 < succBBNum2) || ((succBBNum1 == succBBNum2) && isNot1 && !isNot2);
            }
            if(swapOperand){
                std::swap(isBackEdge1, isBackEdge2);
                std::swap(pp1,pp2);
            }
            selNode->addInputNode(pp1.first, 0, isBackEdge1); // true data
            selNode->addInputNode(pp2.first, 1, isBackEdge2); // false data
            LLVMCDFGNode* selConNode = pp1.second;
            //add CTRLAND
            if(!hasLoopstart){    
                auto DependentNode = _BBctrlDependentNode[pp1.second->BB()];//get temsel true condition
                errs() << "temBB: " << pp1.second->BB()->getName() << "\n";
                for(auto iter:DependentNode){
                    errs() << iter.first->getName() << ", " << iter.first->BB()->getName() << "; ";
                }
                errs() << "\n";
                assert(DependentNode.size() < 2);
                LLVMCDFGNode* conditionBBCtrl = (*(DependentNode.begin())).first;
                if(DependentNode.size() == 1 && pp1.second != conditionBBCtrl){
                    LLVMCDFGNode* CTRLAND = addNode("CTRLAND", pp1.second->BB());
                    CTRLAND->addInputNode(conditionBBCtrl, 0);
                    conditionBBCtrl->addOutputNode(CTRLAND);
                    addEdge(conditionBBCtrl, CTRLAND, EDGE_TYPE_CTRL);
                    CTRLAND->addInputNode(pp1.second, 1);
                    pp1.second->addOutputNode(CTRLAND);
                    addEdge(pp1.second, CTRLAND, EDGE_TYPE_CTRL);
                    selConNode = CTRLAND;
                }
            }
            assert(selConNode != NULL);
            selNode->addInputNode(selConNode, 2); // condition
            pp1.first->addOutputNode(selNode, isBackEdge1); 
            pp2.first->addOutputNode(selNode, isBackEdge2);
            selConNode->addOutputNode(selNode);
            addEdge(pp1.first, selNode, EDGE_TYPE_DATA);
            addEdge(pp2.first, selNode, EDGE_TYPE_DATA);
            addEdge(selConNode, selNode, EDGE_TYPE_CTRL);
            if(i+2 == phiParents.size()){ // last pair, don not crete new conditional node
                // add new parent to vector
                phiParents.push_back(std::make_pair(selNode, (LLVMCDFGNode *)NULL));
            }else{
                // create new conditional node: CTRLOR (cond1 | cond2)
                LLVMCDFGNode *newCondNode = addNode("CTRLOR", node->BB());
                outs() << "new CTRLOR node = " << newCondNode->getName() << "\n";
                newCondNode->addInputNode(pp1.second, 0);
                newCondNode->addInputNode(pp2.second, 1);
                pp1.second->addOutputNode(newCondNode);
                pp2.second->addOutputNode(newCondNode);
                addEdge(pp1.second, newCondNode, EDGE_TYPE_CTRL);
                addEdge(pp2.second, newCondNode, EDGE_TYPE_CTRL);
                // add new parent to vector
                phiParents.push_back(std::make_pair(selNode, newCondNode));
            }
            //outs() << selNode->getName() << " has inputs: "<<selNode->inputNodes().size()<<"\n";
        }
        // connect last node to the successor nodes of the phi node
        LLVMCDFGNode *lastNode = phiParents.rbegin()->first;
        errs()<<"\tlastNode is: "<<lastNode->getName()<<"\n";
        errs()<<"\ttemNode is: "<<node->getName()<<"\n";
        std::vector<std::pair<LLVMCDFGNode*, int>> addedNodePortPairs;
        for(auto succ : node->outputNodes()){
            bool isBackEdge = (succ->isInputBackEdge(node) | lastNode->isInputBackEdge(node));
            int idx;
            ///TODO: (code seems silly) to connect correct input idx when two same inputnodes existing
            for(int i = 0; i < succ->inputNodes().size(); i++){
                if(succ->getInputPort(i) == node){
                    auto tempair = std::make_pair(succ, i);
                    if(std::find(addedNodePortPairs.begin(), addedNodePortPairs.end(), tempair) == addedNodePortPairs.end()){
                        idx = i;
                        addedNodePortPairs.push_back(tempair);
                        break;
                    }
                }
            }
            ///TODO: lcssa PHI isbackedge may wrong
            if(isBackEdge){
                errs()<<"\t\tlastNode to "<<succ->getName()<<" is backedge\n";
            }else{
                errs()<<"\t\tlastNode to "<<succ->getName()<<" is not backedge\n";
            }
            lastNode->addOutputNode(succ, isBackEdge);
            succ->addInputNode(lastNode, idx, isBackEdge);
            addEdge(lastNode, succ, EDGE_TYPE_DATA);
        }
        // map the PHI Node to the select node
        phi2selectMap[node] = lastNode;
        // outs() << "remove PHI node = " << node->getName() << "\n";
        // delNode(node);
    }
    // replace the unresolved PHI parent
    for(auto &elem : phi2phiParentMap){
        LLVMCDFGNode *phinode = elem.first;
        LLVMCDFGNode *phiparent = elem.second;
        LLVMCDFGNode *selparent = phi2selectMap[phiparent];
        errs() << "phinode: " << phinode->getName() << "\n";
        bool isBackEdge = phinode->isInputBackEdge(phiparent); 
        // delete old connections
        int idx = phinode->delInputNode(phiparent);
        // delEdge(edge(phiparent, phinode));
        // add new connections           
        phinode->addInputNode(selparent, idx, isBackEdge);
        selparent->addOutputNode(phinode, isBackEdge);
        addEdge(selparent, phinode, EDGE_TYPE_DATA);   
    }
    // remove all phi nodes and update _insNodeMap
    for(auto &elem : phi2selectMap){
        Instruction* PHI = elem.first->instruction();
        outs() << "remove PHI node = " << elem.first->getName() << "\n";
        delNode(elem.first);
        _insNodeMap[PHI] = elem.second;
    }

}


// add mask AND node behind the Shl node with bytewidth less than MAX_DATA_BYTES
void LLVMCDFG::addMaskAndNodes()
{
    std::vector<LLVMCDFGNode*> shlNodes;
    for(auto &elem : _nodes){
        auto node = elem.second;
        auto ins = node->instruction();
        if(ins && ins->getOpcode() == Instruction::Shl){
            shlNodes.push_back(node);
        }
    }
    for(auto node : shlNodes){
        auto ins = node->instruction();
        BasicBlock *BB = node->BB();
        int bytes = ins->getType()->getIntegerBitWidth() / 8;
        if(bytes < MAX_DATA_BYTES){
            LLVMCDFGNode *andNode = addNode("AND", BB);
            DataType maskVal = (1 << (8 * bytes)) - 1;
            LLVMCDFGNode *constNode = addNode("CONST", BB);
            constNode->setConstVal(maskVal);
            andNode->addInputNode(node, 0);
            andNode->addInputNode(constNode, 1);            
            constNode->addOutputNode(andNode);
            auto outNodes = node->outputNodes();
            for(auto outNode : outNodes){
                node->delOutputNode(outNode);
                int idx = outNode->delInputNode(node);
                delEdge(edge(node, outNode));
                andNode->addOutputNode(outNode);
                outNode->addInputNode(andNode, idx);
                addEdge(andNode, outNode, EDGE_TYPE_DATA);
            }
        }        
    }
}

// handle conversion operations including trunc, zext, sext
void LLVMCDFG::handleConversionNodes()
{
    for(auto &elem : _nodes){
        auto node = elem.second;
        Instruction *ins = node->instruction();
        BasicBlock *BB = node->BB();
        if(ins){
            switch (ins->getOpcode()){
                #ifdef ALL_COARSE
                case Instruction::Trunc:{
                    TruncInst *TI = dyn_cast<TruncInst>(ins);
                    auto bitWidth = TI->getDestTy()->getIntegerBitWidth();
                    DataType mask = (DataType)(1 << bitWidth) - 1;
                    LLVMCDFGNode *constNode = addNode("CONST", BB);
                    constNode->setConstVal(mask);
                    node->addInputNode(constNode, 1);            
                    constNode->addOutputNode(node);      
                    addEdge(constNode, node, EDGE_TYPE_DATA);  
                    break;    
                }        
                case Instruction::SExt:{
                    SExtInst *SI = dyn_cast<SExtInst>(ins);
                    auto srcBitWidth = SI->getSrcTy()->getIntegerBitWidth();
                    //auto dstBitWidth = SI->getDestTy()->getIntegerBitWidth();
                    //DataType constVal = (DataType)((dstBitWidth << 8) | srcBitWidth);
                    DataType constVal = (DataType)(srcBitWidth);
                    LLVMCDFGNode *constNode = addNode("CONST", BB);
                    constNode->setConstVal(constVal);
                    node->addInputNode(constNode, 1);            
                    constNode->addOutputNode(node);      
                    addEdge(constNode, node, EDGE_TYPE_DATA);        
                    break;       
                }    
                case Instruction::ZExt:{
                    // LLVMCDFGNode *constNode = addNode("CONST", BB);
                    // constNode->setConstVal(0);
                    // node->addInputNode(constNode, 1);            
                    // constNode->addOutputNode(node);      
                    // addEdge(constNode, node, EDGE_TYPE_DATA);  
                    auto ZExtIn = node->getInputPort(0);
                    delEdge(edge(ZExtIn, node));
                    auto outputs = node->outputNodes();
                    for(auto output : outputs){
                        int idx = output->getInputIdx(node);
                        delEdge(edge(node, output));
                        connectNodes(ZExtIn, output, idx, EDGE_TYPE_DATA);
                    }
                    break;
                }
                #endif
                //for ZQL: to do calculation of address
                case Instruction::PtrToInt:{
                    auto PtrToIntIn = node->getInputPort(0);
                    delEdge(edge(PtrToIntIn, node));
                    auto outputs = node->outputNodes();
                    for(auto output : outputs){
                        int idx = output->getInputIdx(node);
                        delEdge(edge(node, output));
                        connectNodes(PtrToIntIn, output, idx, EDGE_TYPE_DATA);
                    }
                    break;
                }
                default: break;
            }
        }
    }
}

// fuse constant input node with the mul output node for acc node 
void LLVMCDFG::fuseAccConstNode()
{
    auto nodes = _nodes;
    for(auto &elem : nodes){
        auto node = elem.second;
        if(node->isAcc() && node->outputNodes().size() == 1){
            auto op0 = node->getInputPort(0);
            if(op0->customInstruction() == "CONST"){
                auto dst = node->outputNodes()[0];
                Instruction *dstIns = dst->instruction();
                auto dstCustomIns = dst->customInstruction();
                bool isMul = false;
                bool isShl = false;
                if(dstCustomIns == "MUL"){
                    isMul = true;
                }else if(dstCustomIns == "SHL"){
                    isShl = true;
                }else if(dstIns){
                    auto opc = dstIns->getOpcode();
                    if(opc == Instruction::Mul){
                        isMul = true;
                    }else if(opc == Instruction::Shl){
                        isShl = true;
                    }
                }
                if(isMul || (isShl && dst->getInputIdx(node) == 0)){
                    int otherIdx = 1-dst->getInputIdx(node);
                    auto otherOp = dst->getInputPort(otherIdx);
                    if(otherOp->customInstruction() == "CONST"){
                        auto otherConstVal = otherOp->constVal();
                        auto accConstVal = op0->constVal();
                        // fuse constant
                        //@yuan&lh: fix bug
                        if(isMul){
                            accConstVal *= otherConstVal;
                            node->setInitial(node->getInitial() * otherConstVal);
                        }else{
                            accConstVal <<= otherConstVal;
                            node->setInitial(node->getInitial() << otherConstVal);
                        }
                        op0->setConstVal(accConstVal);
                        auto dstdsts = dst->outputNodes();
                        for(auto dstdst:dstdsts){
                            // delete node and edges
                            int idx = dstdst->delInputNode(dst);
                            node->addOutputNode(dstdst);
                            dstdst->addInputNode(node, idx);
                            addEdge(node, dstdst, EDGE_TYPE_DATA);
                        }
                        delNode(dst);
                    }
                }
                
            }
        }
    }
}



// get the offset of each element in the struct
std::map<int, int> LLVMCDFG::getStructElemOffsetMap(StructType *ST)
{
    std::map<int, int> elemOffsetMap;
    int offset = 0;
    int idx = 0;
    for(Type *type : ST->elements()){
        elemOffsetMap[idx] = offset;
        offset += DL->getTypeAllocSize(type);
        idx++;
    }
    return elemOffsetMap;
}


// transfer GetElementPtr(GEP) node to MUL/ADD/Const tree
void LLVMCDFG::handleGEPNodes()
{
    std::vector<LLVMCDFGNode*> GEPNodes;
    for(auto &elem : _nodes){
        auto node = elem.second;
        auto ins = node->instruction();
        if(ins && dyn_cast<GetElementPtrInst>(ins)){
            GEPNodes.push_back(node);            
        }
    }
    for(auto node : GEPNodes){
        outs() << "----come across GEP: " << node->getName() <<"\n";
        if(node->customInstruction() == "OUTPUT"){
            errs() << "the GEP is an outter instruction, continue\n";
            continue;
        }
        //delete alloca node
        ///TODO: maybe need to extract alloca information
        for(auto inputNode : node->inputNodes()){
            Instruction* ins = inputNode->instruction();
            if(ins != NULL){
                if(auto allocaIns = dyn_cast<AllocaInst>(ins)){
                    errs() << "delete its alloca tree\n";
                    delNodeTree(inputNode);
                }
            }
        }
        auto ins = node->instruction();
        GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(ins);
        ins->dump();
        setGEPInfo(node, GEP->getPointerOperand()->getName().str());
        Type *currType = GEP->getSourceElementType();
        std::vector<std::pair<Value*, int>> varOperandSizes; // <non-constant-operand, element-size>
        int offset = 0;
        int NumOperands = ins->getNumOperands();
        if(NumOperands == 1){ // only one pointer, no index, so GEP should be a Const node
            // base address, should be provided by the scheduler that allocate the address space for the memory
            outs() << "GEP has only one pointer: constant base address\n";
        }else{ // have indexes, get the operand
            for(int i = 1; i < NumOperands; i++){
                outs() << "Operand " << i << ": ";
                Value *V = ins->getOperand(i);
                V->dump();
                outs() << "currType: ";
                currType->dump();
                // check if this operand is constant
                bool isConst = false;
                int constVal;
                // Instruction *I;
                if(ConstantInt *constIns = dyn_cast<ConstantInt>(V)){
                    isConst = true;
                    constVal = constIns->getSExtValue();
                    outs() << "Const: " << constVal << ", ";
                }else{
                    // I = dyn_cast<Instruction>(V);
                    outs() << "Variable, ";
                }
                // get the type and elemnt size of this operand
                int elemSize = 0;
                if(i == 1){ // source element index
                    elemSize = DL->getTypeAllocSize(currType);
                    if(isConst){
                        offset += constVal * elemSize;
                    }                   
                }else if(StructType *ST = dyn_cast<StructType>(currType)){
                    std::map<int, int> elemOffsetMap = getStructElemOffsetMap(ST);
                    assert(isConst); // the operand should be constant
                    offset += elemOffsetMap[constVal];
                    currType = ST->getElementType(constVal);
                    outs() << "StructType, ";
                }else if(ArrayType *AT = dyn_cast<ArrayType>(currType)){                    
                    currType = AT->getElementType();
                    elemSize = DL->getTypeAllocSize(currType);
                    outs() << "ArrayType, ";
                    if(isConst){
                        offset += constVal * elemSize;
                    }
                }else{
                    outs() << "Other type\n";
                    assert(false);
                }
                outs() << "\n";
                if(!isConst){
                    varOperandSizes.push_back(std::make_pair(V, elemSize));
                }
            }
        }
        // construct MUL/ADD/Const tree
        std::vector<LLVMCDFGNode*> newNodes;
        // create MUL nodes
        for(auto &elem : varOperandSizes){
            Instruction *ins = dyn_cast<Instruction>(elem.first);
            LLVMCDFGNode *preNode = this->node(ins); // predecessor node
            if(preNode == NULL){ // out of loop node
                preNode = _ioNodeMap[elem.first];
                outs() << "Out of loop ";
            }
            // elem.first->dump();
            outs() << "preNode: " << preNode->getName() << "\n";
            // delete old connection
            preNode->delOutputNode(node);
            node->delInputNode(preNode);
            delEdge(edge(preNode, node));
            // create new nodes and edges
            auto mulNode = addNode("MUL", node->BB());
            auto constNode = addNode("CONST", node->BB());
            constNode->setConstVal(elem.second);
            preNode->addOutputNode(mulNode);
            mulNode->addInputNode(preNode, 0);
            constNode->addOutputNode(mulNode);
            mulNode->addInputNode(constNode, 1);
            addEdge(preNode, mulNode, EDGE_TYPE_DATA);
            addEdge(constNode, mulNode, EDGE_TYPE_DATA);
            newNodes.push_back(mulNode);
        }
        // create ADD nodes
        for(int i = 0; i + 1 < newNodes.size(); i += 2){
            auto n1 = newNodes[i];
            auto n2 = newNodes[i+1];
            auto newNode = addNode("ADD", node->BB());
            n1->addOutputNode(newNode);
            n2->addOutputNode(newNode);
            newNode->addInputNode(n1, 0);
            newNode->addInputNode(n2, 1);
            addEdge(n1, newNode, EDGE_TYPE_DATA);
            addEdge(n2, newNode, EDGE_TYPE_DATA);
            newNodes.push_back(newNode);
        }
        // connect the last node to GEP node and set constant
        outs() << "Total offset: " << offset << "\n";
        node->setConstVal(offset);
        if(!newNodes.empty()){
            auto lastNewNode = *(newNodes.rbegin());
            lastNewNode->addOutputNode(node);
            node->addInputNode(lastNewNode, 0);
            addEdge(lastNewNode, node, EDGE_TYPE_DATA);
        }
        // the flollowing solution will delete GEP node
        LLVMCDFGNode* GEPAdd = NULL;
        if(node->inputNodes().size() == 2){
            GEPAdd = addNode("ADD", node->BB());
            int idx = 0;
            for(auto inputNode : node->inputNodes()){
                inputNode->addOutputNode(GEPAdd);
                errs() << "inputNode "<< inputNode->getName() << "is port "<< node->getInputIdx(inputNode) << " of "<< node->getName() <<"\n";
                GEPAdd->addInputNode(inputNode, idx);
                addEdge(inputNode, GEPAdd, EDGE_TYPE_DATA);
                idx++;
            }
        }else if(node->inputNodes().size() == 1){
            GEPAdd = *(node->inputNodes().begin());
        }else if(node->inputNodes().size() == 0){
            GEPAdd = addNode("CONST", node->BB());
            GEPAdd->setConstVal(node->constVal());
            errs() << "const address access\n";
        }else{
            assert(0);
        }
        int LSoffset = 0;
        if(node->constVal() != 0){///add const offset of GEP
            LSoffset = node->constVal();
        }
        //connect output of GEP to GEPAdd
        assert(GEPAdd != NULL);
        for(auto outputNode : node->outputNodes()){
            auto ins = outputNode->instruction();
            /// TODO: how to do in multiTask
            if(ins == NULL && outputNode->customInstruction() == "OUTPUT"){
                errs() << outputNode->getName() << ": the output is outter node, continue\n";
                continue;
            }
            if(ins != NULL && (dyn_cast<LoadInst>(ins) || dyn_cast<StoreInst>(ins))){
                outputNode->setLSoffset(node->constVal());
            }
            outputNode->addInputNode(GEPAdd, outputNode->delInputNode(node));
            GEPAdd->addOutputNode(outputNode);
            addEdge(GEPAdd, outputNode, EDGE_TYPE_DATA);
        }
        delNode(node);
    }
}

///TODO: tied forloop in the same level
// add loop exit nodes
std::map<int, LLVMCDFGNode*> LLVMCDFG::addLoopExitNodes()
{
    // std::map<int, std::set<LLVMCDFGNode*>> loop2exitNodesMap;
    std::map<int, LLVMCDFGNode*> loop2exitNodesMap;
    for(auto &elem : _loopexitBBs){
        BasicBlock *srcBB = elem.first;
        BasicBlock *dstBB = elem.second;        
        LLVMCDFGNode *ctrlNode;
        bool isTrueCond;
        int level = -1;
        for(auto &iter : _LoopexclusiveBBs){
            if(iter.second.count(srcBB)){
                level = iter.first;
                break;
            }
        }
        assert(level != -1);
        BranchInst *BRI = cast<BranchInst>(srcBB->getTerminator());
        if(BRI->isConditional()){
            ctrlNode = node(dyn_cast<Instruction>(BRI->getCondition()));
            // check true/false condition
            for(int i = 0; i < BRI->getNumSuccessors(); i++){
                if(dstBB == BRI->getSuccessor(i)){
                    isTrueCond = (i == 0);
                    break;
                }
            }
        }else{ // find the control node in the dominating BB
            BasicBlock *domBB = DT->getNode(srcBB)->getIDom()->getBlock();
            BranchInst *domBRI = cast<BranchInst>(domBB->getTerminator());
            assert(domBRI->isConditional());
            ctrlNode = node(dyn_cast<Instruction>(domBRI->getCondition()));
            // check true/false condition
            for(int i = 0; i < domBRI->getNumSuccessors(); i++){
                if(DT->dominates(domBRI->getSuccessor(i), srcBB)){
                    isTrueCond = (i == 0);
                    break;
                }
            }
        }        
        // create LOOPEXIT node and connect ctrlNode to it
        auto exitNode = getLoopExitNode(srcBB);
        if(!isTrueCond){
            // find the CTRLNOT node
            bool hasNotNode = false;
            for(auto outNode : ctrlNode->outputNodes()){
                if(outNode->customInstruction() == "CTRLNOT"){
                    hasNotNode = true;
                    ctrlNode = outNode;
                    break;
                }
            }
            if(!hasNotNode){
                auto NotNode = addNode("CTRLNOT", ctrlNode->BB());
                connectNodes(ctrlNode, NotNode, 0, EDGE_TYPE_CTRL); 
                ctrlNode = NotNode;
            }
        }
        outs() << "Control node: " << ctrlNode->getName();
        outs() << ", exit node: " << exitNode->getName() << "\n";
        exitNode->addInputNode(ctrlNode, 0);
        ctrlNode->addOutputNode(exitNode);
        addEdge(ctrlNode, exitNode, EDGE_TYPE_CTRL);
        loop2exitNodesMap[level] = exitNode;
    }
    return loop2exitNodesMap;
}



// remove redundant nodes, e.g. Branch
void LLVMCDFG::removeRedundantNodes()
{
    auto nodes = _nodes;
    // remove redundant nodes iteratively until no node to be remove
    bool removed = true;
    while(removed){
        std::vector<LLVMCDFGNode*> rmNodes;
        for(auto &elem : _nodes){
            auto node = elem.second;
            Instruction *ins = node->instruction();
            std::string customIns = node->customInstruction();
            if(node->inputNodes().empty()){ // no input node
                if(!(ins && dyn_cast<LoadInst>(ins)) && customIns != "CONST" && customIns != "LOOPSTART" && customIns != "INPUT" && customIns != "ArgIn"){ //  && !node->isLSaffine()
                    rmNodes.push_back(node);
                    continue;
                }
            } 
            if(node->outputNodes().empty()){ // no output node
                if(!(ins && dyn_cast<StoreInst>(ins)) && customIns != "LOOPEXIT" && customIns != "OUTPUT" && customIns != "TASKEXIT"){ //  && !node->isLSaffine()
                    rmNodes.push_back(node);
                    continue;
                }
            }
            // if(node->isLSaffine() && customIns != "OUTPUT"){ // for temp test, to be removed later
            //     bool toRm = true;
            //     auto strides = node->getLSstride();
            //     for(auto &elem : strides){  // strides are all 0
            //         if(elem.second != 0){
            //             toRm = false;
            //             break;
            //         }
            //     }
            //     if(toRm){
            //         rmNodes.push_back(node);
            //         continue;
            //     }
            // }
        }
        removed = !rmNodes.empty();
        outs() << "remove node: ";
        for(auto node : rmNodes){
            outs() << node->getName() << ", \n\t";
            delNode(node);            
        }
        outs() << "\n";
    }
}


// assign final node name
void LLVMCDFG::assignFinalNodeName()
{
    for(auto &elem : _nodes){
        auto node = elem.second;
        Instruction *ins = node->instruction();
        std::string customIns = node->customInstruction();
        if(!customIns.empty()){
            if(customIns == "SELECT"){
                customIns = "SEL";
            }else if(customIns == "InitSELECT" || customIns == "CInitSELECT"){
                customIns = (customIns == "InitSELECT") ? "ISEL" : "CISEL";
                auto op0 = node->getInputPort(0);
                auto op1 = node->getInputPort(1);
                node->setInputIdx(op0, 1);
                node->setInputIdx(op1, 0);
            }else if(customIns.size() > 4 && customIns.substr(0, 4) == "CTRL"){
                customIns = customIns.substr(4);
            }
            node->setFinalInstruction(customIns);
        }else{
            switch (ins->getOpcode()){
            case Instruction::Add:
                node->setFinalInstruction("ADD");
                break;
            case Instruction::FAdd:
                node->setFinalInstruction("FADD");
                break;
            case Instruction::Sub:
                node->setFinalInstruction("SUB");
                break;
            case Instruction::FSub:
                node->setFinalInstruction("FSUB");
                break;
            case Instruction::Mul:
                node->setFinalInstruction("MUL");
                break;
            case Instruction::FMul:
                node->setFinalInstruction("FMUL");
                break;
            case Instruction::UDiv:
            case Instruction::SDiv:
            case Instruction::FDiv:
                // errs() << "Do not support Div Instructions\n";
                // assert(false);
                // break;
                node->setFinalInstruction("DIV");
                break;
            case Instruction::URem:
            case Instruction::SRem:
            case Instruction::FRem:
                if(_suppRem){
                    node->setFinalInstruction("REM");
                    break;
                }else{
                    errs() << "Do not support Rem Instructions\n";
                    assert(false);
                    break;
                }
            case Instruction::Shl:
                node->setFinalInstruction("SHL");
                break;
            case Instruction::LShr:
                node->setFinalInstruction("LSHR");
                break;
            case Instruction::AShr:
                node->setFinalInstruction("ASHR");
                break;
            case Instruction::And:
                node->setFinalInstruction("AND");
                break;
            case Instruction::Or:
                node->setFinalInstruction("OR");
                break;
            case Instruction::Xor:
                node->setFinalInstruction("XOR");
                break;
            case Instruction::Load:
                if(node->isLSaffine())
                    if(node->getInputPort(1)!=NULL){
                        node->setInputIdx(node->getInputPort(1), 0);
                        node->setFinalInstruction("CInput");
                    }
                    else
                        node->setFinalInstruction("Input");
                else{
                    if(node->getInputPort(1)!=NULL)
                        node->setFinalInstruction("CLOAD");
                    else
                        node->setFinalInstruction("LOAD");
                }
                break;
            case Instruction::Store:
                if(node->isLSaffine())
                    if(node->getInputPort(2)!=NULL){
                        node->setInputIdx(node->getInputPort(2), 1);
                        node->setFinalInstruction("COutput");
                    }
                    else
                        node->setFinalInstruction("Output");
                else{
                    if(node->getInputPort(2)!=NULL)
                        node->setFinalInstruction("CSTORE");
                    else
                        node->setFinalInstruction("STORE");
                }
                break;
            case Instruction::GetElementPtr:
                if(node->inputNodes().empty()){
                    node->setFinalInstruction("CONST");
                }else{
                    node->setFinalInstruction("ADD");
                }                
                break;
            case Instruction::Trunc:{
                node->setFinalInstruction("AND");             
                break;    
            }        
            case Instruction::SExt:{
                node->setFinalInstruction("SEXT");
                break;       
            }    
            case Instruction::ZExt:{
            #ifdef ALL_COARSE
                node->setFinalInstruction("OR");         
            #else
                node->setFinalInstruction("ZExt");         
            #endif
                break;
            }
            case Instruction::ICmp:{
                CmpInst *CI = dyn_cast<CmpInst>(ins);
                switch(CI->getPredicate()){
                case CmpInst::ICMP_EQ:
                    node->setFinalInstruction("EQ");
                    break;
                case CmpInst::ICMP_NE:
                    node->setFinalInstruction("NE");
                    break;
                case CmpInst::ICMP_SGE:{
                    //node->setFinalInstruction("SGE");
                    node->setFinalInstruction("SLE");
                    LLVMCDFGNode* opnode0 = node->getInputPort(0), *opnode1 = node->getInputPort(1);
                    node->setInputIdx(opnode0, 1);
                    node->setInputIdx(opnode1, 0);
                    break;
                }
                case CmpInst::ICMP_UGE:{
                    //node->setFinalInstruction("UGE");
                    node->setFinalInstruction("ULE");
                    LLVMCDFGNode* opnode0 = node->getInputPort(0), *opnode1 = node->getInputPort(1);
                    node->setInputIdx(opnode0, 1);
                    node->setInputIdx(opnode1, 0);
                    break;
                }
                case CmpInst::ICMP_SGT:{
                    //node->setFinalInstruction("SGT");
                    node->setFinalInstruction("SLT");
                    LLVMCDFGNode* opnode0 = node->getInputPort(0), *opnode1 = node->getInputPort(1);
                    node->setInputIdx(opnode0, 1);
                    node->setInputIdx(opnode1, 0);
                    break;
                }
                case CmpInst::ICMP_UGT:{
                    //node->setFinalInstruction("UGT");
                    node->setFinalInstruction("ULT");
                    LLVMCDFGNode* opnode0 = node->getInputPort(0), *opnode1 = node->getInputPort(1);
                    node->setInputIdx(opnode0, 1);
                    node->setInputIdx(opnode1, 0);
                    break;
                }
                case CmpInst::ICMP_SLE:
                    node->setFinalInstruction("SLE");
                    break;
                case CmpInst::ICMP_ULE:
                    node->setFinalInstruction("ULE");
                    break;
                case CmpInst::ICMP_SLT:
                    node->setFinalInstruction("SLT");
                    break;
                case CmpInst::ICMP_ULT:
                    node->setFinalInstruction("ULT");
                    break;
                default:
                    assert(false);
                    break;
                }
                break;
            }
            case Instruction::Select:
                node->setFinalInstruction("SEL");
                break;
            case Instruction::FNeg:{
                node->setInputIdx(node->inputNodes()[0], 1);
                LLVMCDFGNode* constZero = addNode("CONST", node->BB());
                constZero->setConstVal(0);
                connectNodes(constZero, node, 0, EDGE_TYPE_DATA);
                node->setFinalInstruction("FSUB");
                break;
            }
            case Instruction::PHI:
                //node->setFinalInstruction("SELECT");
                break;

            //////// jhlou add 20250418
            case Instruction::Call:{
                CallInst* call_ins = dyn_cast<CallInst>(ins);
                assert(call_ins != nullptr);

                Function* calledFunc = call_ins->getCalledFunction();
                if (!calledFunc)  assert(false);

                //// Get this hardware implemented op's name from 
                //// the attributes of this function.
                //// We already add the "__CGRA_Hardware_Op" attribute
                //// in ParseHardwareImplementFunction().
                std::string funcOpName = "";
                if(calledFunc->hasFnAttribute("__CGRA_Hardware_Op")){
                    funcOpName = calledFunc->getFnAttribute("__CGRA_Hardware_Op").getValueAsString().str();
                } else{
                    assert(false && "Not a hardware-implemented op."); 
                }

                assert(funcOpName != "" && "Is not a hardware defined call op.");
                node->setFinalInstruction(funcOpName);
                break;
            }
            //////// End of jhlou add 20250418

            default:
                ins->dump();
                assert(false);
                break;
            }
        }
    }
}

///to cast constant value to Int
int castConstInt(Constant* V){
    int integer;
    if (ConstantInt *CI = dyn_cast<ConstantInt>(V))
    {
        integer = CI->getSExtValue();
    }
    else if (ConstantFP *FP = dyn_cast<ConstantFP>(V))
    {
        integer = (DataType)FP->getValueAPF().convertToFloat();
    }
    else{
        assert(0&&"fail in casting const value to Int");
    }
    return integer;
}

///to get the constant operand of a BinaryOperation(first:OperandIndex; second:IntValue)
std::pair<int, int> BinaryConstOp(Instruction *Bi)
{
    int OperandIndex;
    int constOp;
    assert(Bi && (Bi->getNumOperands() == 2));
    if (auto V = dyn_cast<Constant>(Bi->getOperand(0)))
    {
        constOp = castConstInt(V);
        OperandIndex = 0;
        //errs() << "\t" << "Instructions: " << Bi->getName().str() << "\n";
    }
    else if (auto V = dyn_cast<Constant>(Bi->getOperand(1)))
    {
        constOp = castConstInt(V);
        OperandIndex = 1;
        //errs() << "\t" << "Instructions: " << Bi->getName().str() << "\n";
    }else{
        return std::make_pair(-1, 0);
        assert(0&&"do not have Constant");
    }
    return std::make_pair(OperandIndex, constOp);
}

///to get the constant operand of a BinaryOperation(first:OperandIndex; second:IntValue)
///OperandIndex: 
///        0----first operand is invariable
///        1----second operand is invariable
///       -1----do not have invariable operator
///        2----both operands are invariable
std::pair<int, varType> BinaryConstOp(LLVMCDFGNode* Bn, std::map<LLVMCDFGNode *, std::string> IOInfoMap)
{
    int OperandIndex = -1;
    varType constOp = 0;
    int nonConst = 0;
    assert(Bn && (Bn->inputNodes().size() == 2));
    for(int i = 0; i < 2; i++){
        auto BnInputNode = Bn->getInputPort(i);
        if (BnInputNode->hasConst())
        {
            constOp = BnInputNode->constVal();
            OperandIndex = i;
            //errs() << "\t" << "Instructions: " << Bi->getName().str() << "\n";
        }else if (BnInputNode->hasArgIn()){
            constOp = "arg" + std::to_string(BnInputNode->argNum());
            OperandIndex = i;
        }else if (IOInfoMap.count(BnInputNode)){
            constOp = IOInfoMap[BnInputNode];
            OperandIndex = i;
        }else{
            nonConst++;
        }
    }
    // assert(nonConst > 0 && "two operators are both invariable");
    if(nonConst == 0){
        errs() << Bn->getName() << " has two invariable operators\n";
        OperandIndex = 2;
    }
    return std::make_pair(OperandIndex, constOp);
}

LLVMCDFGNode* LLVMCDFG::addNodeTree(Value* opnode){
    for (auto iter : _funcArgs)
    {
        auto arg = iter.second;
        if (arg == opnode)
        {
            LLVMCDFGNode* newnode = addNode("ArgIn");
            return newnode;
        }
    }
    if(auto CT = dyn_cast<Constant>(opnode)){
        ///TODO:bugs when coming accross address of global reg
        LLVMCDFGNode* newnode = addNode("CONST");
        newnode->setConstVal(castConstInt(CT));
        return newnode;
    //PHI will be handled in arraystride()
    }else if(auto Phi = dyn_cast<PHINode>(opnode)){
        //LLVMCDFGNode *newnode = addNode("SELECT", Phi->getParent());
        return node(Phi);
    }else if(auto Load = dyn_cast<LoadInst>(opnode)){
        LLVMCDFGNode *newnode = addNode(Load);
        if(dyn_cast<GetElementPtrInst>(Load->getOperand(0))){
            LLVMCDFGNode* prenode = addNodeTree(Load->getOperand(0));
            prenode->addOutputNode(newnode);
            newnode->addInputNode(prenode, 0);
            addEdge(prenode, newnode, EDGE_TYPE_DATA);
        }else{
            ///TODO: is it right?
            newnode->setCustomInstruction("INREG");
        }
        return newnode;
    }else if(auto GEP = dyn_cast<GetElementPtrInst>(opnode)){
        LLVMCDFGNode *newnode = addNode(GEP);
        int dimension = GEP->getNumIndices();
        auto teminputs = newnode->inputNodes();
        for(auto prenode : teminputs){
            if(prenode->customInstruction() == "INPUT"){
                delNode(prenode);
            }
        }
        for(int i = 2; i <= dimension; i++){
            auto operand = GEP->getOperand(i);
            LLVMCDFGNode* prenode;
            if(auto Phi = dyn_cast<PHINode>(operand)){//PHI direct connnect to GEP handle here
                auto inputs = newnode->inputNodes();
                prenode = addIdxCycle(Phi);
                if(prenode->customInstruction().empty() && _loopBBs.count(Phi->getParent()))///innermost phi
                    continue;
            }else if(dyn_cast<Constant>(operand)){//handled in handleGEP
                continue;
            }else{
                prenode = addNodeTree(operand);
            }
            if(newnode->getInputPort(i) == prenode)//have been added
                continue;
            prenode->addOutputNode(newnode);
            newnode->addInputNode(prenode, i);
            addEdge(prenode, newnode, EDGE_TYPE_DATA);
        }
        return newnode;
    }else if (auto ins = dyn_cast<Instruction>(opnode))//TODO:outter action in DFG may be wrong
    {
        LLVMCDFGNode *newnode = node(ins);
        if(newnode != NULL)
            return newnode;
        else
            newnode = addNode(ins);
        for(int i = 0; i < ins->getNumOperands(); i++){
            auto operand = ins->getOperand(i);
            if(dyn_cast<PHINode>(operand))//get PHI operand.deal in arraystride
                continue;
            LLVMCDFGNode* prenode = addNodeTree(operand);
            prenode->addOutputNode(newnode);
            newnode->addInputNode(prenode, i);
            addEdge(prenode, newnode, EDGE_TYPE_DATA);
        }
        return newnode;
    }else{
        return NULL;
    }
}

std::set<std::pair<LLVMCDFGNode*, int>> LLVMCDFG::addOutputTree(Value* user){
    std::set<std::pair<LLVMCDFGNode*, int>> resultset;
    auto ins = dyn_cast<Instruction>(user);
    for(auto succ : user->users()){
        if(auto succIns = dyn_cast<Instruction>(succ)){
            if(_allBBs.count(succIns->getParent()) && !_ExInsList.count(succIns)){
                continue;
            }else if(auto Phi = dyn_cast<PHINode>(succIns)){//single PHI is just pass
                if(Phi->getNumOperands() != 1)///only consider lcssa new PHI:TODO ?
                    continue;
                errs() << "find phi:";Phi->dump();
                auto outputset = addOutputTree(Phi);
                resultset.insert(outputset.begin(), outputset.end());
            }else if(auto Store = dyn_cast<StoreInst>(succIns)){
                errs() << "find store:";Store->dump();
                auto storeNode = addNode(Store);
                int idx = -1;
                for(int i = 0; i < Store->getNumOperands(); i++){
                    if(auto operandins = dyn_cast<Instruction>(Store->getOperand(i))){
                        if(operandins == ins){
                            idx = i;
                            continue;
                        }
                    }
                    // auto newNode = addNodeTree(Store->getOperand(i));
                    // newNode->addOutputNode(storeNode);
                    // storeNode->addInputNode(newNode, i);
                    // addEdge(newNode, storeNode, EDGE_TYPE_DATA);
                    LLVMCDFGNode* newNode = NULL;
                    if(i == 1 && dyn_cast<GetElementPtrInst>(Store->getOperand(i)))
                        newNode = addNodeTree(Store->getOperand(i));
                    if(newNode != NULL){
                        newNode->addOutputNode(storeNode);
                        storeNode->addInputNode(newNode, i);
                        addEdge(newNode, storeNode, EDGE_TYPE_DATA);
                    }
                }
                resultset.insert(std::make_pair(storeNode, idx));
            }else if(dyn_cast<BranchInst>(succIns)||dyn_cast<ReturnInst>(succIns)){
                succIns->dump();///TODO:can not handle like:ret...
                continue;
            }else{
                succIns->dump();
                auto detectedflag = (node(succIns) != NULL);
                auto succnode = addNode(succIns);
                int idx = -1;
                for(int i = 0; i < succIns->getNumOperands(); i++){
                    if(auto operandins = dyn_cast<Instruction>(succIns->getOperand(i))){
                        if(operandins == ins){
                            idx = i;
                            continue;
                        }else if(dyn_cast<PHINode>(operandins)){
                            continue;
                        }else if(operandins != ins && node(operandins) == NULL){
                            auto opnode = addNodeTree(operandins);
                            opnode->addOutputNode(succnode);
                            succnode->addInputNode(opnode);
                            addEdge(opnode, succnode, EDGE_TYPE_DATA);
                            succnode->setInputIdx(opnode, i);
                        }
                    }
                    else if(dyn_cast<Constant>(succIns->getOperand(i))){
                        if(!detectedflag){
                            auto opnode = addNodeTree(succIns->getOperand(i));
                            opnode->addOutputNode(succnode);
                            succnode->addInputNode(opnode);
                            addEdge(opnode, succnode, EDGE_TYPE_DATA);
                            succnode->setInputIdx(opnode, i);
                        }
                    }else{
                        assert(0&&"what's wrong");
                    }
                }
                resultset.insert(std::make_pair(succnode, idx));
                auto outputset = addOutputTree(succIns);
                for(auto temoutput:outputset){
                    Instruction* temoutputins = temoutput.first->instruction();
                    int idx=-1;
                    for(auto i = 0; i<temoutputins->getNumOperands(); i++){
                        if(temoutputins->getOperand(i) == succIns){
                            idx = i;
                            break;
                        }
                    }
                    ///TODO:error
                    // assert(idx>=0);
                    if(edge(succnode, temoutput.first)==NULL || temoutput.first->getInputIdx(succnode) != idx){
                        succnode->addOutputNode(temoutput.first);
                        temoutput.first->addInputNode(succnode);
                        addEdge(succnode, temoutput.first, EDGE_TYPE_DATA);
                        temoutput.first->setInputIdx(succnode, idx);
                    }
                }
            }
        }else{
            succ->dump();
            assert(0&&"come assross what in add OutputTree");
            return resultset;
        }
    }
    return resultset;
}

LLVMCDFGNode* LLVMCDFG::addIdxCycle(PHINode* Phi){
    if(Phi->getNumIncomingValues() > 2){
        assert(0&&"what phi =_=");
    }
    auto exist = node(Phi);
    if(exist != NULL)
        return exist;
    LLVMCDFGNode* SELECT = addNode(Phi);
    SELECT->setCustomInstruction("SELECT");
    for(int i = 0; i < Phi->getNumIncomingValues(); i++){
        BasicBlock* bb = Phi->getIncomingBlock(i);
		Value* V = Phi->getIncomingValue(i);
		outs() << "IncomingValue :: "; V->dump();
        if(auto C = dyn_cast<Constant>(V)){///TODO:very puny constrain
            LLVMCDFGNode* phiTree = addNodeTree(V);
            SELECT->addInputNode(phiTree);
            SELECT->setInputIdx(phiTree, i);
            phiTree->addOutputNode(SELECT);
            addEdge(phiTree, SELECT, EDGE_TYPE_DATA);
        }else if(auto ins = dyn_cast<Instruction>(V)){
            LLVMCDFGNode* phiTree = node(ins);
            if(phiTree == NULL)
                phiTree = addNodeTree(V);
            SELECT->addInputNode(phiTree);
            SELECT->setInputIdx(phiTree, i);
            phiTree->addOutputNode(SELECT);
            auto phiTreeins = phiTree->instruction();
            int Sel2phiTreeidx = -1;
            for(int i = 0; i < phiTreeins->getNumOperands(); i++){
                if(phiTreeins->getOperand(i) == Phi){
                    Sel2phiTreeidx = i;
                    break;
                }
            }
            assert(Sel2phiTreeidx != -1);
            phiTree->setInputIdx(SELECT, Sel2phiTreeidx);
            addEdge(phiTree, SELECT, EDGE_TYPE_DATA);
            addEdge(SELECT, phiTree, EDGE_TYPE_DATA);
            phiTree->setOutputBackEdge(SELECT, true);
            
        }
    }
    return SELECT;
}


PHINode* InstoPHI(Instruction* src){
    for (int i = 0; i < src->getNumOperands(); i++){
        if(auto ins = dyn_cast<Instruction>(src->getOperand(i))){
            if (auto Phi = dyn_cast<PHINode>(ins)){
                return Phi;
            }
            else{
                return InstoPHI(ins);
            }
        }
        else{
            continue;
        }
    }
    return NULL;
}


///analyze loop index
void LLVMCDFG::LoopIdxAnalyze(){
    errs() << "----------------analyze Loop Idx\n";
    _nestLoopisAffine = true;
    std::map<int, llvm::Loop *> nestloops = _nestloops;
    int total_level = nestloops.size();
    std::set<BasicBlock*> detected; //detected loops' BBs shouldn't be checked by outter loop
    std::map<int, std::set<BasicBlock*>> LoopexclusiveBBs;
    std::map<int, int> strides;
    //std::map<int, std::pair<int, int>> bounds;
    std::map<int, std::pair<varType, varType>> bounds;
    std::map<int, varType> counts;

    bounds.clear();
    strides.clear();
    
    //get every nests' idx stride & bound
    for(int level = 0; level < total_level; level++){
        ///get all basicblocks of this loop nest
        std::set<BasicBlock*> temNestBBs;
        temNestBBs.insert(nestloops[level]->getBlocks().begin(), nestloops[level]->getBlocks().end());
        BasicBlock* exitingBB = nestloops[level]->getExitingBlock();
        assert(exitingBB != NULL);
        Instruction* exitingins = exitingBB->getTerminator();
        auto branch = dyn_cast<BranchInst>(exitingins);
        CmpInst* exitcmp;
        PHINode* idxPhi = NULL;

        errs() << "nest: " << level << " have blocks: ";
        for(auto BI = temNestBBs.begin(); BI != temNestBBs.end(); BI++){
            BasicBlock *temBB = *BI;
            errs() << temBB->getName().str() << " ";
        }
        
        errs() << "\n"; 
        errs() << "exiting block is: " << exitingBB->getName().str() <<"\n";
        errs() << "exit branch is: "; branch->dump();
        //errs() << "Canonical Induction Variable is: ";nestloops[level]->getCanonicalInductionVariable()->dump();


        if(auto cmp = dyn_cast<CmpInst>(branch->getOperand(0))){
            exitcmp = cmp;
            errs() << "exit cmp is: "; exitcmp->dump();
        }

        ///get all basicblocks of this loop nest
        idxPhi = InstoPHI(exitcmp);
        for(auto BI = temNestBBs.begin(); BI != temNestBBs.end(); BI++){
            BasicBlock *temBB = *BI;
            if (detected.find(temBB) == detected.end())
            {
                LoopexclusiveBBs[level].insert(temBB);
                for (auto II = temBB->begin(); II != temBB->end(); II++)
                {
                    Instruction *ins = &*II;
                    if (auto Phi = dyn_cast<PHINode>(ins))
                    {
                        if(Phi != idxPhi){
                            continue;
                        }
                        PHItoLevel[Phi] = level;
                    }
                }
            }
        } 
        detected.insert(temNestBBs.begin(),temNestBBs.end());
        _LoopexclusiveBBs = LoopexclusiveBBs;
        // for(int level = 0; level < LoopexclusiveBBs.size(); level++){
        //     errs() << "level"<<level <<" has: ";
        //     for(auto elem:LoopexclusiveBBs[level]){
        //         errs() << elem->getName() <<", ";
        //     }
        //     errs() << ";\n";
        // }

        Instruction *idx_gen_oprand;
        Value* InitialIVValue, *FinalIVValue;
        // bool bothConst = false;//to identify if InitialIVValue and FinalIVValue are both constant
        if(!nestloops[level]->getBounds(*SE).hasValue()){
            for (int i = 0; i < idxPhi->getNumOperands(); i++){
                auto incomingBB = idxPhi->getIncomingBlock(i);
                errs() << "incoming value:"; idxPhi->getIncomingValue(i)->dump();
                if(temNestBBs.find(incomingBB) == temNestBBs.end()){
                    InitialIVValue = idxPhi->getIncomingValue(i);
                }else{
                    if(dyn_cast<Instruction>(idxPhi->getIncomingValue(i))){
                        idx_gen_oprand = dyn_cast<Instruction>(idxPhi->getIncomingValue(i));
                        FinalIVValue = exitcmp->getOperand(1);
                    }else if(auto V = dyn_cast<ConstantInt>(idxPhi->getIncomingValue(i))){
                        idx_gen_oprand = NULL;
                        FinalIVValue = V;
                    }else{
                        assert(false && "what incoming?");
                    }
                }
            }
        }else{
            auto LoopBound = (nestloops[level]->getBounds(*SE).getPointer());
            idx_gen_oprand = &(LoopBound->getStepInst());
            InitialIVValue = (&(LoopBound->getInitialIVValue()));
            FinalIVValue = (&(LoopBound->getFinalIVValue()));
        }
        //errs() << "getBoundFuntion: level step: "<< nestloops[level]->getBounds(*SE).getPointer()->getStepInst().getName().str() << "\n";
        errs() << "InitialIVValue: " << InitialIVValue->getName() << "\n";
        errs() << "FinalIVValue: " << FinalIVValue->getName() << "\n";
        //errs() << "idx_gen_oprand: " << idx_gen_oprand->getName() << "\n";
        if (idx_gen_oprand != NULL)
        {
            //stride
            if(idx_gen_oprand->getOpcode() != Instruction::Add && idx_gen_oprand->getOpcode() != Instruction::Sub){
                errs() << "\t" << idxPhi->getName().str() << " had non-affine operation\n";
                this->_nestLoopisAffine = false;
            }else{
                if (auto V = dyn_cast<Constant>(idx_gen_oprand->getOperand(0)))
                {
                    if(idx_gen_oprand->getOpcode() != Instruction::Sub){
                        int stride = castConstInt(V);
                        errs() << "\t" << "Instructions: " << idx_gen_oprand->getName().str() << "\n";
                        strides[level] = stride;
                    }
                }
                else if (auto V = dyn_cast<ConstantInt>(idx_gen_oprand->getOperand(1)))
                {
                    int stride = castConstInt(V);
                    errs() << "\t"<< "Instructions: " << idx_gen_oprand->getName().str() << "\n";
                    strides[level] = stride;
                }
                else{
                    errs() << "\t" << idxPhi->getName().str() << " have affine operation, but no constant operands\n";
                }
                if(idx_gen_oprand->getOpcode() == Instruction::Sub){
                    strides[level] = -strides[level];
                }
            }
            //bound
            if(auto CV = dyn_cast<ConstantInt>(InitialIVValue)){
                bounds[level].first = castConstInt(CV);
            }else{
                errs() << "\tinitial value is not ConstantInt\n";
                bool findArgIn = false;
                for(auto iter : _funcArgs){
                    auto arg = iter.second;
                    if(arg == InitialIVValue){
                        errs() << "\tInitialIVValue is the "<< iter.first << " input param of this functuion\n";
                        //FindLoopIdxSafetyBr(level, InitialIVValue);
                        bounds[level].first = "arg" + std::to_string(iter.first);
                        findArgIn = true;
                        break;
                    }
                }
                if(!findArgIn){
                    if(auto InitialIVIns = dyn_cast<Instruction>(InitialIVValue)){
                        BasicBlock* InitialIVBB = InitialIVIns->getParent();
                        if(_loopBBs.find(InitialIVBB) == _loopBBs.end()){
                            bounds[level].second = InitialIVIns->getName().str();
                        }else{
                            errs() << "\tFinalIVValue is not loop invariable\n";
                            _nestLoopisAffine = false;
                        }
                    }else{
                        InitialIVValue->dump();
                        _nestLoopisAffine = false;
                    }
                }
            }
            if(auto CV = dyn_cast<ConstantInt>(FinalIVValue)){
                bounds[level].second = castConstInt(CV);
            }else{
                errs() << "\tfinal value is not ConstantInt\n";
                bool findArgIn = false;
                for(auto iter : _funcArgs){
                    auto arg = iter.second;
                    if(arg == FinalIVValue){
                        errs() << "\tFinalIVValue is the "<< iter.first << " input param of this functuion\n";
                        //FindLoopIdxSafetyBr(level, FinalIVValue);
                        bounds[level].second = "arg" + std::to_string(iter.first);
                        findArgIn = true;
                        break;
                    }
                }
                if(!findArgIn){
                    if(auto FinalIVIns = dyn_cast<Instruction>(FinalIVValue)){
                        BasicBlock* FinalIVBB = FinalIVIns->getParent();
                        if(_loopBBs.find(FinalIVBB) == _loopBBs.end()){
                            bounds[level].second = FinalIVIns->getName().str();
                        }else{
                            errs() << "\tFinalIVValue is not loop invariable\n";
                            _nestLoopisAffine = false;
                        }
                    }else{
                        FinalIVValue->dump();
                        assert(false && "FinalIVValue comes from error location");
                    }
                    // _nestLoopisAffine = false;
                }
            }
            auto tempair = std::make_pair(InitialIVValue, FinalIVValue);
            _loopboundMap[level] = tempair;
            bool NormalJumpout = true;
            bool JumpoutCond;
            if(branch->getSuccessor(0) == idxPhi->getParent()){///cmp is false, loop jumpout
                JumpoutCond = false;
                errs() << "\t"<< exitcmp->getName().str()<<" is true, loop repeat; opt-analyzed bounds[level].second = "<<bounds[level].second.to_string() << "\n";
            }else{///cmp is true, loop jumpout
                JumpoutCond = true;
                errs() << "\t"<< exitcmp->getName().str()<<" is true, loop jumpout;\n";
                errs() << "\topt-analyzed: bounds[level].second = "<<bounds[level].second.to_string() << "\n";
            }
            if(exitcmp->getOperand(0) != idxPhi  && exitcmp->getOperand(0) != idx_gen_oprand){
                NormalJumpout = false;
                errs() << "\t"<< exitcmp->getName().str()<<" do not compare loopidx or its inc value "<<bounds[level].second.to_string() << "\n";
            }
            auto lb = bounds[level].first;
            auto rb = bounds[level].second;
            int stride = strides[level];
            if(NormalJumpout){
                switch(exitcmp->getPredicate()){
                    case CmpInst::ICMP_EQ:{
                        if(!JumpoutCond){
                            counts[level] = 1;
                            //assert(false&&"what?jumpout");
                        }else{
                            if(stride > 0){
                                counts[level] = (rb - 1 - lb) / stride + 1;
                            }else if(stride < 0){
                                counts[level] = (rb + 1 - lb) / stride + 1;
                            }
                        }
                        break;
                    }
                    case CmpInst::ICMP_NE:{
                        if(JumpoutCond){
                            counts[level] = 1;
                            // assert(false&&"what?jumpout");
                        }else{
                            if(stride > 0){
                                counts[level] = (rb - 1 - lb) / stride + 1;
                            }else if(stride < 0){
                                counts[level] = (rb + 1 - lb) / stride + 1;
                            }
                        }
                        break;
                    }
                    case CmpInst::ICMP_SLE:
                    case CmpInst::ICMP_ULE:{
                        if(JumpoutCond){
                            //mod1
                            counts[level] = (rb + 1 - lb) / stride + 1;
                        }else{
                            //mod2
                            counts[level] = (rb - lb) / stride + 1;
                        }
                        break;
                    }
                    case CmpInst::ICMP_SGE:
                    case CmpInst::ICMP_UGE:{
                        if(JumpoutCond){
                            //mod3
                            counts[level] = (rb - 1 - lb) / stride + 1;
                        }else{
                            //mod4
                            counts[level] = (rb - lb) / stride + 1;
                        }
                        break;
                    }
                    case CmpInst::ICMP_SLT:
                    case CmpInst::ICMP_ULT:{
                        if(JumpoutCond){
                            //mod4
                            counts[level] = (rb - lb) / stride + 1;
                        }else{
                            //mod3
                            counts[level] = (rb - 1 - lb) / stride + 1;
                        }
                        break;
                    }
                    case CmpInst::ICMP_SGT:
                    case CmpInst::ICMP_UGT:{
                        if(JumpoutCond){
                            //mod2
                            counts[level] = (rb - lb) / stride + 1;
                        }else{
                            //mod1
                            counts[level] = (rb + 1 - lb) / stride + 1;
                        }
                        break;
                    }
                    default:
                        assert(false);
                        break;
                }
                if(exitcmp->getOperand(0) == idx_gen_oprand){
                    bounds[level].second = (counts[level]-1) * stride + lb;
                }else if(exitcmp->getOperand(0) == idxPhi){
                    bounds[level].second = counts[level] * stride + lb;
                    counts[level] = counts[level]+1;
                }
            }else{
                assert(false&& "abnormal jumpout");
            }
        }else{
            int left, right;
            if(auto CV1 = dyn_cast<ConstantInt>(InitialIVValue))
                left = castConstInt(CV1); 
            if(auto CV2 = dyn_cast<ConstantInt>(FinalIVValue))
                right = castConstInt(CV2);
            strides[level] = right - left;
            bounds[level].first = left;
            bounds[level].second = right;
            counts[level] = 2;
        }
    }

    // load stride information when loopsAffine is true
    // if loop is not affine--analyze END
    if(isloopsAffine()){
        setLoopsAffineStrides(strides);
    }else{
        _loop2exitNodesMap.clear();
        _loop2exitNodesMap = addLoopExitNodes();
        return;
    }
    
    //print loop index information
    for(int i = 0; i < getLoopsAffineStrides().size(); i++){
        int stride = strides[i];
        varType left = bounds[i].first;
        varType right = bounds[i].second;
        // if(abs(stride)==1 || (right - left)%stride == 0)
        //     bounds[i].second -= stride;
        // else
        //     count+=1;
        /*
        if(bounds[i].first > bounds[i].second){
            int tem = bounds[i].first;
            bounds[i].first = bounds[i].second;
            bounds[i].second = tem;
        }
        */
        errs() << "loop level " << i << " stride " << getLoopsAffineStrides()[i] << " per cycle\n";
        errs() << "\tleftbound is " << bounds[i].first.to_string() << "\n";
        errs() << "\trightbound is " << bounds[i].second.to_string() << "\n";
        setLoopsAffineBounds(i,bounds[i].first, bounds[i].second);
        setLoopsAffineCounts(i,counts[i]);
    }
}

///recursively find array assess stride & bound
/*
std::vector<PHINode*> LLVMCDFG::arrayStride(Value* opnode, std::map<int, std::pair<double,std::pair<double,double>>>* factortable){
    
    //if return {} && factortable->clear() means non-affine operation
    if(factortable == NULL || factortable->empty()){
        if(dyn_cast<PHINode>(opnode)||dyn_cast<Constant>(opnode)){
            return {};
        }else if(auto ins = dyn_cast<Instruction>(opnode)){
            LLVMCDFGNode* temnode = node(ins);
            checkInsInputs(ins);
            for(int i = 0; i < ins->getNumOperands(); i++){
                auto preopnode = ins->getOperand(i);
                arrayStride(preopnode, NULL);
            }
        }
        if(factortable != NULL){factortable->clear();}
        return {};
    }
    if(dyn_cast<Constant>(opnode)){
        ///mul constant will be covered in following
        errs() << "\t\thas a constant address\n";
        return {};
    }else if(auto phi = dyn_cast<PHINode>(opnode)){
        std::vector<llvm::PHINode *> singlephi;
        ///TODO: can have conditional load/store?
        if(!PHItoLevel.count(phi)){
            factortable->clear();
            return {};
        }
        int level = PHItoLevel[phi];
        singlephi.push_back(phi);
        if((*factortable)[level].first == 0) (*factortable)[level].first = getLoopsAffineStrides(level);
        return singlephi;
    }else if(dyn_cast<StoreInst>(opnode)||dyn_cast<LoadInst>(opnode)){
        errs() << "\t\thas a read address\n";
        factortable->clear();
        return {};
    }else{
        Instruction* temins = dyn_cast<Instruction>(opnode);
        //errs()<<"\tcome accross ins:";temins->dump();
        int temopcode = temins->getOpcode();
        if(temins->isBinaryOp()){
            std::pair<int, int> OperandIndextoConst = BinaryConstOp(temins);
            //checkInsInputs(temins);//flatened DFG need no checkInsInputs
            switch (temopcode)
            {
            case Instruction::Mul :{
                if(OperandIndextoConst.second == 0){
                    factortable->clear();
                    arrayStride(temins->getOperand(0), NULL);
                    arrayStride(temins->getOperand(1), NULL);
                    return {};
                }else{
                    auto innerphis = arrayStride(temins->getOperand(1-OperandIndextoConst.first), factortable);
                    for(auto phi : innerphis){
                        int level = PHItoLevel[phi];
                        if((*factortable)[level].first == 0) (*factortable)[level].first = getLoopsAffineStrides(level);
                        (*factortable)[level].first *= OperandIndextoConst.second;
                        (*factortable)[level].second.first *= OperandIndextoConst.second;
                        (*factortable)[level].second.second *= OperandIndextoConst.second;
                    }
                    return innerphis;
                }
            }
            case Instruction::UDiv:
            case Instruction::SDiv:
            case Instruction::FDiv:{
                if(OperandIndextoConst.second == 0){
                    factortable->clear();
                    arrayStride(temins->getOperand(0), NULL);
                    arrayStride(temins->getOperand(1), NULL);
                    return {};
                }if(OperandIndextoConst.first == 0){
                    factortable->clear();
                    arrayStride(temins->getOperand(0), NULL);
                    arrayStride(temins->getOperand(1), NULL);
                    return {};
                }else{
                    auto innerphis = arrayStride(temins->getOperand(0), factortable);
                    for(auto phi : innerphis){
                        int level = PHItoLevel[phi];
                        if((*factortable)[level].first == 0) (*factortable)[level].first = getLoopsAffineStrides(level);
                        (*factortable)[level].first /= OperandIndextoConst.second;
                        (*factortable)[level].second.first /= OperandIndextoConst.second;
                        (*factortable)[level].second.second /= OperandIndextoConst.second;                        
                    }
                    return innerphis;
                }
            }
            ///TODO:div & Lshl may be unaffine when not divisible
            case Instruction::Shl :{
                if(OperandIndextoConst.second == 0){
                    factortable->clear();
                    arrayStride(temins->getOperand(0), NULL);
                    arrayStride(temins->getOperand(1), NULL);
                    return {};
                }if(OperandIndextoConst.first == 0){
                    factortable->clear();
                    arrayStride(temins->getOperand(0), NULL);
                    arrayStride(temins->getOperand(1), NULL);
                    return {};
                }else{
                    auto innerphis = arrayStride(temins->getOperand(0), factortable);
                    for(auto phi : innerphis){
                        int level = PHItoLevel[phi];
                        if((*factortable)[level].first == 0) (*factortable)[level].first = getLoopsAffineStrides(level);
                        //errs()<< "shl: " << (int)(*factortable)[level].first<<"<<"<<OperandIndextoConst.second<<"!!!!!!\n";
                        (*factortable)[level].first = (int)(*factortable)[level].first << OperandIndextoConst.second;
                        //errs()<<"result is: "<< (int)(*factortable)[level].first<<"!!!!\n";
                        (*factortable)[level].second.first = (int)(*factortable)[level].second.first << OperandIndextoConst.second;
                        (*factortable)[level].second.second = (int)(*factortable)[level].second.second << OperandIndextoConst.second;
                    }
                    return innerphis;
                }
            }
            case Instruction::LShr :{
                if(OperandIndextoConst.second == 0){
                    factortable->clear();
                    arrayStride(temins->getOperand(0), NULL);
                    arrayStride(temins->getOperand(1), NULL);
                    return {};
                }if(OperandIndextoConst.first == 0){
                    factortable->clear();
                    arrayStride(temins->getOperand(0), NULL);
                    arrayStride(temins->getOperand(1), NULL);
                    return {};
                }else{
                    auto innerphis = arrayStride(temins->getOperand(0), factortable);
                    for(auto phi : innerphis){
                        int level = PHItoLevel[phi];
                        if((*factortable)[level].first == 0) (*factortable)[level].first = getLoopsAffineStrides(level);
                        (*factortable)[level].first /= varPow(2, OperandIndextoConst.second);
                        (*factortable)[level].second.first /= varPow(2, OperandIndextoConst.second);
                        (*factortable)[level].second.second /= varPow(2, OperandIndextoConst.second);
                    }
                    return innerphis;
                }
            }
            case Instruction::Add :{
                if(OperandIndextoConst.second == 0){
                    auto temphis0 = arrayStride(temins->getOperand(0), factortable);
                    auto temphis1 = arrayStride(temins->getOperand(1), factortable);
                    temphis1.insert(temphis1.end(), temphis0.begin(), temphis0.end());///TODO:10*(i+i) ???
                    return temphis1;
                }else{
                    auto innerphis = arrayStride(temins->getOperand(1-OperandIndextoConst.first), factortable);
                    if(innerphis.empty()){//may add something else
                        factortable->clear();
                        return {};
                    }
                    auto phi = *innerphis.begin();//only add once
                    int level = PHItoLevel[phi];
                    (*factortable)[level].second.first += OperandIndextoConst.second;
                    (*factortable)[level].second.second += OperandIndextoConst.second;
                    
                    return innerphis;
                }
            }
            case Instruction::Or :{
                if(OperandIndextoConst.second == 0){//unkonw condition
                    //assert(0&&"unknow condition about Or operation");
                    factortable->clear();
                    arrayStride(temins->getOperand(0), NULL);
                    arrayStride(temins->getOperand(1), NULL);
                    return {};
                }else{
                    auto innerphis = arrayStride(temins->getOperand(1-OperandIndextoConst.first), factortable);
                    for(auto phi : innerphis){//TODO:any else condition besides add?
                        int level = PHItoLevel[phi];
                        (*factortable)[level].second.first += OperandIndextoConst.second;
                        (*factortable)[level].second.second += OperandIndextoConst.second;
                    }
                    return innerphis;
                }
            }
            case Instruction::Sub :{
                if(OperandIndextoConst.second == 0){
                    auto temphis0 = arrayStride(temins->getOperand(0), factortable);
                    auto temphis1 = arrayStride(temins->getOperand(1), factortable);
                    for(auto phi : temphis1){///to Negate subtrahends
                        int level = PHItoLevel[phi];
                        if((*factortable)[level].first == 0) (*factortable)[level].first = 1;
                        (*factortable)[level].first = -(*factortable)[level].first;
                    }
                    temphis1.insert(temphis1.end(), temphis0.begin(), temphis0.end());
                    return temphis1;
                }else if(OperandIndextoConst.first == 0){
                    auto temphis1 = arrayStride(temins->getOperand(1), factortable);
                    auto phi = *temphis1.begin();///to Negate subtrahends
                                                ///constant only once
                        int level = PHItoLevel[phi];
                        if((*factortable)[level].first == 0) (*factortable)[level].first = 1;
                        (*factortable)[level].first = -(*factortable)[level].first;
                        (*factortable)[level].second.first = OperandIndextoConst.second - (*factortable)[level].second.first;
                        (*factortable)[level].second.second =  OperandIndextoConst.second - (*factortable)[level].second.second;
                }else{
                    auto innerphis = arrayStride(temins->getOperand(0), factortable);
                    auto phi = *innerphis.begin();///constant only once
                        int level = PHItoLevel[phi];
                        (*factortable)[level].second.first -= OperandIndextoConst.second;
                        (*factortable)[level].second.second -= OperandIndextoConst.second;
                    return innerphis;
                }
            }
            default:{
                factortable->clear();
                return {};
                }
            }
        }else{///when will is not BinaryOp??!
            if(auto temins = dyn_cast<ZExtInst>(opnode)){
                errs()<<"finally across ZExt";temins->dump();
                auto innerphis = arrayStride(temins->getOperand(0), factortable);
                return innerphis;
            }else{
                assert(0&&"what ins?");
            }
        }
    }
}
*/

///recursively find array assess stride & bound
std::vector<int> LLVMCDFG::arrayStride(LLVMCDFGNode* opnode, std::map<int, std::pair<varType,std::pair<varType, varType>>>* factortable){
    auto ins = opnode->instruction();
    //if return {} && factortable->clear() means non-affine operation
    if(factortable == NULL || factortable->empty()){
        if(factortable != NULL){factortable->clear();}
        return {};
    }
    if(opnode->hasConst()){
        ///mul constant will be covered in following
        errs() << "\t\thas a constant address\n";
        return {};
    }else if(ins != NULL && (dyn_cast<PHINode>(ins))){
        auto phi = dyn_cast<PHINode>(ins);
        std::vector<int> singleLevel;
        int level = -1;
        varType stride = 0;
        varType start = 0;
        ///TODO: can have conditional load/store?
        if(opnode->customInstruction() == "ACC"){
            auto ACCinput = opnode->getInputPort(0);
            int CummuInitialLevel = opnode->getCummuInitialLevel();
            int CummuIntervalLevel = opnode->getCummuIntervalLevel();
            start = opnode->getInitial();
            if(CummuInitialLevel == CummuIntervalLevel){
                errs() << "\t\tACC: " << opnode->getName() << " is the same as level " << CummuInitialLevel << "\n";
                level = CummuInitialLevel;
                if(ACCinput->hasConst()){
                    stride = ACCinput->constVal();
                }else if(_IOInfoMap.count(ACCinput)){
                    stride = _IOInfoMap[ACCinput];
                }else{
                    errs() << opnode->getName() << " does not have invariable operand\n";
                    factortable->clear();
                    return {};
                }
            }
        }
        if(level == -1){
            factortable->clear();
            return {};
        }
        singleLevel.push_back(level);
        if((*factortable)[level].first == 0) {
            (*factortable)[level].first = stride;
            (*factortable)[level].second.first /= getLoopsAffineStrides(level);
            (*factortable)[level].second.first *= stride;
            (*factortable)[level].second.second /= getLoopsAffineStrides(level);
            (*factortable)[level].second.second *= stride;
            varType offset = (*factortable)[level].second.first - start;
            (*factortable)[level].second.first = start;
            (*factortable)[level].second.second -= offset;
            if(opnode->isAccFirst()){
                (*factortable)[level].second.first += stride;
                (*factortable)[level].second.second += stride;
            }
        }
        return singleLevel;
    }else if(ins != NULL && (dyn_cast<StoreInst>(ins)||dyn_cast<LoadInst>(ins))){
        errs() << "\t\thas a read address\n";
        factortable->clear();
        return {};
    }else{
        int temopcode;
        if(ins != NULL){
            temopcode = ins->getOpcode();
        }else{
            if(opnode->customInstruction() == "ADD"){
                temopcode = Instruction::Add;
            }else if(opnode->customInstruction() == "MUL"){
                temopcode = Instruction::Mul;
            }
        }
        
        if(opnode->inputNodes().size() == 2){
            std::pair<int, varType> OperandIndextoConst = BinaryConstOp(opnode, _IOInfoMap);
            //checkInsInputs(temins);//flatened DFG need no checkInsInputs
            if(OperandIndextoConst.first == 2){
                errs() << opnode->getName() << " is totally invariable operator\n";
                factortable->clear();
                arrayStride(opnode->getInputPort(0), NULL);
                arrayStride(opnode->getInputPort(1), NULL);
                return {};
            }
            switch (temopcode)
            {
            case Instruction::Mul :{
                if(OperandIndextoConst.first == -1){
                    factortable->clear();
                    arrayStride(opnode->getInputPort(0), NULL);
                    arrayStride(opnode->getInputPort(1), NULL);
                    return {};
                }else{
                    varType coefficient = OperandIndextoConst.second;
                    auto innerLevels = arrayStride(opnode->getInputPort(1-OperandIndextoConst.first), factortable);
                    for(auto level : innerLevels){
                        (*factortable)[level].first *= coefficient;
                        (*factortable)[level].second.first *= coefficient;
                        (*factortable)[level].second.second *= coefficient;
                    }
                    return innerLevels;
                }
            }
            case Instruction::UDiv:
            case Instruction::SDiv:
            case Instruction::FDiv:{
                if(OperandIndextoConst.first != 1 || OperandIndextoConst.first == -1){
                    factortable->clear();
                    arrayStride(opnode->getInputPort(0), NULL);
                    arrayStride(opnode->getInputPort(1), NULL);
                    return {};
                }else{
                    varType coefficient = OperandIndextoConst.second;
                    auto innerLevels = arrayStride(opnode->getInputPort(0), factortable);
                    for(auto level : innerLevels){
                        (*factortable)[level].first /= coefficient;
                        (*factortable)[level].second.first /= coefficient;
                        (*factortable)[level].second.second /= coefficient;                        
                    }
                    return innerLevels;
                }
            }
            ///TODO:div & Lshl may be unaffine when not divisible
            case Instruction::Shl :{
                if(OperandIndextoConst.first != 1 || OperandIndextoConst.first == -1){
                    factortable->clear();
                    arrayStride(opnode->getInputPort(0), NULL);
                    arrayStride(opnode->getInputPort(1), NULL);
                    return {};
                }else{
                    varType coefficient = OperandIndextoConst.second;
                    auto innerLevels = arrayStride(opnode->getInputPort(0), factortable);
                    for(auto level : innerLevels){
                        //errs()<< "shl: " << (int)(*factortable)[level].first<<"<<"<<Coefficient<<"!!!!!!\n";
                        (*factortable)[level].first = (*factortable)[level].first * varPow(2, coefficient);
                        //errs()<<"result is: "<< (int)(*factortable)[level].first<<"!!!!\n";
                        (*factortable)[level].second.first = (*factortable)[level].second.first * varPow(2, coefficient);
                        (*factortable)[level].second.second = (*factortable)[level].second.second * varPow(2, coefficient);
                    }
                    return innerLevels;
                }
            }
            case Instruction::LShr :{
                if(OperandIndextoConst.first != 1 || OperandIndextoConst.first == -1){
                    factortable->clear();
                    arrayStride(opnode->getInputPort(0), NULL);
                    arrayStride(opnode->getInputPort(1), NULL);
                    return {};
                }else{
                    varType coefficient = OperandIndextoConst.second;
                    auto innerLevels = arrayStride(opnode->getInputPort(0), factortable);
                    for(auto level : innerLevels){
                        (*factortable)[level].first /= varPow(2, coefficient);
                        (*factortable)[level].second.first /= varPow(2, coefficient);
                        (*factortable)[level].second.second /= varPow(2, coefficient);
                    }
                    return innerLevels;
                }
            }
            case Instruction::Add :{
                if(OperandIndextoConst.first == -1){
                    auto temphis0 = arrayStride(opnode->getInputPort(0), factortable);
                    auto temphis1 = arrayStride(opnode->getInputPort(1), factortable);
                    temphis1.insert(temphis1.end(), temphis0.begin(), temphis0.end());
                    if(!factortable->empty())
                        return temphis1;
                    else
                        return {};
                }else{
                    varType coefficient = OperandIndextoConst.second;
                    auto innerLevels = arrayStride(opnode->getInputPort(1-OperandIndextoConst.first), factortable);
                    if(innerLevels.empty()){//may add something else
                        factortable->clear();
                        return {};
                    }
                    int level = *innerLevels.begin();//only add once
                    (*factortable)[level].second.first += coefficient;
                    (*factortable)[level].second.second += coefficient;
                    
                    return innerLevels;
                }
            }
            case Instruction::Or :{
                if(OperandIndextoConst.first == -1){//unknow condition
                    //assert(0&&"unknow condition about Or operation");
                    // factortable->clear();
                    // arrayStride(opnode->getInputPort(0), NULL);
                    // arrayStride(opnode->getInputPort(1), NULL);
                    // return {};
                    auto temphis0 = arrayStride(opnode->getInputPort(0), factortable);
                    auto temphis1 = arrayStride(opnode->getInputPort(1), factortable);
                    temphis1.insert(temphis1.end(), temphis0.begin(), temphis0.end());
                    if(!factortable->empty())
                        return temphis1;
                    else
                        return {};
                }else{
                    varType coefficient = OperandIndextoConst.second;
                    auto loopbound = _loopsAffineBounds;
                    auto loopstride = _loopsAffineStrides;
                    auto innerLevels = arrayStride(opnode->getInputPort(1-OperandIndextoConst.first), factortable);
                    for(auto level : innerLevels){//TODO:any else condition besides add?
                        // if(loopbound[level].first == 0 && loopstride[level] > coefficient){
                        int stride;
                        if((*factortable)[level].first.value.index() == 0){
                            stride = std::get<int>((*factortable)[level].first.value);
                        }
                        else if((*factortable)[level].first.value.index() == 1){
                            stride = std::get<double>((*factortable)[level].first.value);
                        }else{
                            assert(false && "bad stride");
                        }
                        if((*factortable)[level].second.first == 0 && stride > coefficient){
                            (*factortable)[level].second.first += coefficient;
                            (*factortable)[level].second.second += coefficient;
                        }else{
                            factortable->clear();
                            arrayStride(opnode->getInputPort(0), NULL);
                            arrayStride(opnode->getInputPort(1), NULL);
                            return {};
                        }
                    }
                    return innerLevels;
                }
            }
            case Instruction::Sub :{
                if(OperandIndextoConst.first == -1){
                    auto temLevels0 = arrayStride(opnode->getInputPort(0), factortable);
                    auto temLevels1 = arrayStride(opnode->getInputPort(1), factortable);
                    if(!factortable->empty()){
                        for(auto level : temLevels1){///to Negate subtrahends
                            //stride
                            if((*factortable)[level].first == 0){
                                (*factortable)[level].first = 1;
                            }
                            (*factortable)[level].first = 0-(*factortable)[level].first;
                            //bound
                            (*factortable)[level].second.first = 0 - (*factortable)[level].second.first;
                            (*factortable)[level].second.second =  0 - (*factortable)[level].second.second;
                        }
                        temLevels1.insert(temLevels1.end(), temLevels0.begin(), temLevels0.end());
                        return temLevels1;
                    }else{
                        return {};
                    }
                }else if(OperandIndextoConst.first == 0){
                    varType coefficient = OperandIndextoConst.second;
                    auto temLevels1 = arrayStride(opnode->getInputPort(1), factortable);
                    if(!factortable->empty()){
                        for(auto level : temLevels1){///to Negate subtrahends
                            //stride
                            if((*factortable)[level].first == 0){
                                (*factortable)[level].first = 1;
                            }
                            (*factortable)[level].first = 0-(*factortable)[level].first;
                            //bound
                            (*factortable)[level].second.first = 0 - (*factortable)[level].second.first;
                            (*factortable)[level].second.second =  0 - (*factortable)[level].second.second;
                        }
                        int onelevel = *temLevels1.begin();///to Negate subtrahends
                                                ///constant only once
                        //bound
                        (*factortable)[onelevel].second.first = coefficient + (*factortable)[onelevel].second.first;
                        (*factortable)[onelevel].second.second =  coefficient + (*factortable)[onelevel].second.second;
                        return temLevels1;
                    }else{
                        factortable->clear();
                        return {};
                    }
                }else{
                    varType coefficient = OperandIndextoConst.second;
                    auto innerLevels = arrayStride(opnode->getInputPort(0), factortable);
                    int level = *innerLevels.begin();///constant only once
                        (*factortable)[level].second.first -= coefficient;
                        (*factortable)[level].second.second -= coefficient;
                    return innerLevels;
                }
            }
            default:{
                factortable->clear();
                return {};
                }
            }
        }else{///when will is not BinaryOp??!

            if(auto temins = dyn_cast<ZExtInst>(ins)){
                errs()<<"finally across ZExt";temins->dump();
                auto innerphis = arrayStride(opnode->getInputPort(0), factortable);
                return innerphis;
            }else{
                factortable->clear();
                return {};
                assert(0&&"what ins?");
            }
        }
    }
}

/*
///to get the Branch of this loop 
BranchInst *LooptoBranch(std::set<llvm::BasicBlock *> detected, std::set<llvm::BasicBlock *> temNestBBs){
    for(auto BI = temNestBBs.begin(); BI != temNestBBs.end(); BI++){
        BasicBlock *temBB = *BI;
        if(detected.find(temBB) == detected.end()){
            for(auto II = temBB->begin(); II != temBB->end(); II++){
                Instruction *ins = &*II;
                if(auto br = dyn_cast<BranchInst>(ins)){
                    if(br->getNumOperands() == 3){
                        errs() << "yes: "<< br->getOperand(0)->getName().str() <<"\n";
                        return br;
                    }
                }
            }
        }
    }
    return NULL;
}
*/


// remove GEP nodes(when pattern)
void LLVMCDFG::handleAffineLSNodes(){
    bool allaffine = true;
    std::set<LLVMCDFGNode*> GEPsToDelnode, GEPsToDeltree;
    std::map<int, LLVMCDFGNode *> nodes = _nodes;
    for(auto &elem : nodes){
        auto node = elem.second;
        Instruction *ins = node->instruction();
        std::string customIns = node->customInstruction();
        if(node->isLSaffine()){
            // get pattern access offset
            auto LSbounds = node->getLSbounds();
            varType offset = LSbounds[0] + node->getLSoffset();
            // errs() <<"LSbounds[0]: " << LSbounds[0].to_string() << "\n";
            errs() << node->getName() << ": " << node->getLSArrayName() <<" offset: " << offset.to_string() << "\n";
            node->setLSoffset(offset);
            // delete address claculation
            errs() << node->getName() << " is affine, delete its GEP tree\n";
            auto teminputs = node->inputNodes();
            for(auto pre : teminputs){
                if(dyn_cast<LoadInst>(ins)){
                    GEPsToDeltree.insert(node->getInputPort(0));
                }else if(dyn_cast<StoreInst>(ins)){
                    GEPsToDeltree.insert(node->getInputPort(1));
                }
            }
        }else if(ins != NULL){
            if(dyn_cast<LoadInst>(ins) || dyn_cast<StoreInst>(ins)){
                allaffine = false;
                auto temp =node->inputNodes();
                for(auto pre : temp){
                    auto preins = pre->instruction();
                    if(preins != NULL){
                        if(dyn_cast<GetElementPtrInst>(preins)){
                            errs() << node->getName() << " is not affine, delete its GEP node\n";
                            //delete GEP & connect lastmul of address claculation with Load/Store
                            if(pre->inputNodes().size() == 0)
                                continue;
                            auto lastmul = *(pre->inputNodes().begin());
                            lastmul->addOutputNode(node);
                            node->addInputNode(lastmul, node->getInputIdx(pre));
                            addEdge(lastmul, node, EDGE_TYPE_DATA);
                            GEPsToDelnode.insert(pre);
                        }
                    }
                }
            } 
        }
    }
    ///finally del to avoid muiti LSU using the same GEP
    for(auto del:GEPsToDelnode){
        if(del != NULL)
            delNode(del);
    }
    for(auto deltree:GEPsToDeltree){
        if(deltree != NULL)
            delNodeTree(deltree);
    }
}

//to judge if a node is inline ADD or Custom ADD
bool isADDNode(LLVMCDFGNode* node){
    auto ins = node->instruction();
    bool isADD = false;
    if(ins == NULL){
        if(node->customInstruction() == "ADD"){
            isADD = true;
        }
    }
    else if(ins->getOpcode() == Instruction::Add || ins->getOpcode() == Instruction::FAdd){
        isADD = true;
    }
    return isADD;
}

void LLVMCDFG::transAddOpSelectTree(){
    outs() << ">>>>>>>>>>transform add result SELECT tree to add operand SELECT tree\n";
    std::map<int, LLVMCDFGNode *> nodes = _nodes;
    for (auto &elem : nodes){
        auto node = elem.second;
        Instruction* ins = node->instruction();
        if(node->customInstruction() == "SELECT"){
            outs() << "come across node: "<< node->getName() <<":\n";
            std::map<LLVMCDFGNode*, LLVMCDFGNode*> selectTree;
            std::vector<LLVMCDFGNode*> op;
            op.push_back(node->getInputPort(0));
            op.push_back(node->getInputPort(1));
            int Addidx = -1;
            bool firstADD = isADDNode(op[0]);
            bool secondADD = isADDNode(op[1]);
            bool bothAdd =  secondADD && firstADD;
            LLVMCDFGNode *sameOperand = NULL;
            if (bothAdd)
            {
                auto op0inputs = op[0]->inputNodes();
                if (std::count(op0inputs.begin(), op0inputs.end(), op[1]->getInputPort(0)))
                {
                    sameOperand = op[1]->getInputPort(0);
                }
                else if (std::count(op0inputs.begin(), op0inputs.end(), op[1]->getInputPort(1)))
                {
                    sameOperand = op[1]->getInputPort(1);
                }
            }
            else
            {
                if (firstADD)
                {
                    Addidx = 0;
                }
                else if (secondADD)
                {
                    Addidx = 1;
                }
                if (Addidx == -1)
                {
                    continue;
                }
                auto Addinputs = op[Addidx]->inputNodes();
                if (std::count(Addinputs.begin(), Addinputs.end(), op[1 - Addidx]))
                {
                    sameOperand = op[1 - Addidx];
                    auto NewADD = addNode("ADD", sameOperand->BB());
                    auto NewConst0 = addNode("CONST", sameOperand->BB());
                    NewConst0->setConstVal(0);
                    sameOperand->addOutputNode(NewADD);
                    NewConst0->addOutputNode(NewADD);
                    NewADD->addOutputNode(node);
                    NewADD->addInputNode(sameOperand, 0);
                    NewADD->addInputNode(NewConst0, 1);
                    node->addInputNode(NewADD, 1 - Addidx);
                    delEdge(edge(sameOperand, node));
                    addEdge(NewConst0, NewADD, EDGE_TYPE_DATA);
                    addEdge(sameOperand, NewADD, EDGE_TYPE_DATA);
                    addEdge(NewADD, node, EDGE_TYPE_DATA);
                    op[1 - Addidx] = NewADD;
                }
            }
            if (sameOperand == NULL)
            {
                // do not have the same add operand input
                continue;
            }
            selectTree[node] = node; // mark this SELECT node by this
            std::map<LLVMCDFGNode *, std::set<LLVMCDFGNode *>> treeNodes2Outs;
            std::map<LLVMCDFGNode *, LLVMCDFGNode *> selOut2selTreeNode;
            std::vector<LLVMCDFGNode *> SelsStack;
            SelsStack.insert(SelsStack.begin(), node);
            while (!SelsStack.empty())
            {
                LLVMCDFGNode *temSel = SelsStack.back();
                SelsStack.pop_back();
                auto temOutputs = temSel->outputNodes();
                for (auto outputNode : temOutputs)
                {
                    int idx = outputNode->getInputIdx(temSel);
                    if (outputNode->customInstruction() == "SELECT")
                    {
                        LLVMCDFGNode *anotherSelOp = outputNode->getInputPort(1 - idx);
                        Instruction *anotherSelOpIns = anotherSelOp->instruction();
                        if (anotherSelOp->customInstruction() == "SELECT" && selectTree.count(anotherSelOp))
                        {
                            SelsStack.insert(SelsStack.begin(), outputNode);
                            selectTree[outputNode] = NULL;
                            treeNodes2Outs[selOut2selTreeNode[outputNode]].erase(outputNode);
                            continue;
                        }
                        else if (isADDNode(anotherSelOp))
                        {
                            if (anotherSelOp->getInputIdx(sameOperand) != -1)
                            {
                                selectTree[outputNode] = anotherSelOp;
                                SelsStack.insert(SelsStack.begin(), outputNode);
                                continue;
                            }
                        }
                        else
                        {
                            selOut2selTreeNode[outputNode] = temSel;
                        }
                    }
                    treeNodes2Outs[temSel].insert(outputNode);
                }
            }
            /// transform the tree
            for (auto &treeNodeElem : selectTree)
            {
                auto treeNode = treeNodeElem.first;
                auto ADDNode = treeNodeElem.second;
                bool firstNode = false;
                if (ADDNode == NULL)
                {
                    continue;
                }
                if (ADDNode == treeNode)
                {
                    firstNode = true;
                    ADDNode = op[0];
                }
                int idx = treeNode->getInputIdx(ADDNode);
                auto addOpNode = ADDNode->getInputPort(1 - ADDNode->getInputIdx(sameOperand));
                bool isbackedge = ADDNode->isOutputBackEdge(treeNode);
                if (ADDNode->outputNodes().size() == 1)
                {
                    delNode(ADDNode);
                    treeNode->addInputNode(addOpNode, idx, isbackedge);
                    addOpNode->addOutputNode(treeNode, isbackedge);
                    addEdge(addOpNode, treeNode, EDGE_TYPE_DATA);
                }
                else
                {
                    delEdge(edge(ADDNode, treeNode));
                    treeNode->addInputNode(addOpNode, idx, isbackedge);
                    addOpNode->addOutputNode(treeNode, isbackedge);
                    addEdge(addOpNode, treeNode, EDGE_TYPE_DATA);
                }
                if (firstNode)
                {
                    ADDNode = op[1];
                    isbackedge = ADDNode->isOutputBackEdge(treeNode);
                    int idx = treeNode->getInputIdx(ADDNode);
                    auto addOpNode = ADDNode->getInputPort(1 - ADDNode->getInputIdx(sameOperand));
                    if (ADDNode->outputNodes().size() == 1)
                    {
                        delNode(ADDNode);
                        treeNode->addInputNode(addOpNode, idx, isbackedge);
                        addOpNode->addOutputNode(treeNode, isbackedge);
                        addEdge(addOpNode, treeNode, EDGE_TYPE_DATA);
                    }
                    else
                    {
                        delEdge(edge(ADDNode, treeNode));
                        treeNode->addInputNode(addOpNode, idx, isbackedge);
                        addOpNode->addOutputNode(treeNode, isbackedge);
                        addEdge(addOpNode, treeNode, EDGE_TYPE_DATA);
                    }
                }
                /// add ADD node for out of this tree
                if (treeNodes2Outs.count(treeNode) && treeNodes2Outs[treeNode].size() > 0)
                {
                    auto NewADD = addNode("ADD", treeNode->BB());
                    NewADD->addInputNode(treeNode, 1);
                    NewADD->addInputNode(sameOperand, 0);
                    treeNode->addOutputNode(NewADD);
                    sameOperand->addOutputNode(NewADD);
                    addEdge(treeNode, NewADD, EDGE_TYPE_DATA);
                    addEdge(sameOperand, NewADD, EDGE_TYPE_DATA);
                    for (auto extraOut : treeNodes2Outs[treeNode])
                    {
                        bool isbackedge = treeNode->isOutputBackEdge(extraOut);
                        int idx = extraOut->getInputIdx(treeNode);
                        delEdge(edge(treeNode, extraOut));
                        extraOut->addInputNode(NewADD, idx, isbackedge);
                        NewADD->addOutputNode(extraOut, isbackedge);
                        addEdge(NewADD, extraOut, EDGE_TYPE_DATA);
                    }
                }
            }
        }
    }
}

// analyze SELECT tree from backedge node
bool analyzeSelectTree(LLVMCDFGNode* node, LLVMCDFGNode* rootnode, std::set<LLVMCDFGNode*> &selectTree, bool &promisingCycle){
    // errs() << "temnode is: " << node->getName()<<", root node: " << rootnode->getName() << "\n";
    bool isReachable = false;
   
    for(auto inputNode: node->inputNodes()){
        if(node->isInputBackEdge(inputNode)){
            continue;
        }
        bool selCondIn = node->customInstruction() == "SELECT" && node->getInputIdx(inputNode) == 2;
        
        if(inputNode == rootnode || selectTree.count(inputNode)){
            isReachable = true;
        }else if(selCondIn || (!isADDNode(inputNode) && node->customInstruction() != "SELECT")){
            if(analyzeSelectTree(inputNode, rootnode, selectTree, promisingCycle)){
                // errs() << "temNode is " << node->getName() << "; inputNode is " << inputNode->getName() << "\n";
                // errs() << "[WARNING] promsing cycle! can not extracted as ACC-series\n";
                promisingCycle = true;
                isReachable = true;
                continue;
            }
        }else if(analyzeSelectTree(inputNode, rootnode, selectTree, promisingCycle)){
            isReachable = true;
        }
    }
    if(isReachable){
        
        errs() << "\t{" << node->getName() << "} is reachable!\n";
        selectTree.insert(node);
    }
    return isReachable;
}

///traverse all initialSels & generate ACC node
void LLVMCDFG::generateACC(std::set<LLVMCDFGNode *> &InitialSels){
    std::set<LLVMCDFGNode*> rmnodes;
    auto temSet = InitialSels;
    for (auto node : temSet)
    {
        LLVMCDFGNode *initSel = node;
        bool isACC = false;
        auto Initial = node->getInputPort(0);
        auto pre = node->getInputPort(1);
        errs() << "\n\tcurrent pre is: " << pre->getName() << "\n";
        if (pre->isOutputBackEdge(node))
        {
            if (isADDNode(pre))
            {
                //to check is promising cycle after ACC transformation
                std::vector<LLVMCDFGNode*> toVisit;
                bool hasPromisingCycle = false;
                for(auto addOp:pre->inputNodes()){
                    if(addOp != node){
                        toVisit.push_back(addOp);
                    }
                }
                while(!toVisit.empty()){
                    auto temNode = toVisit.back();
                    toVisit.pop_back();
                    // errs() << "temNode is: " << temNode->getName() << "\n";
                    if(std::find(temNode->inputNodes().begin(), temNode->inputNodes().end(), node) != temNode->inputNodes().end()){
                        hasPromisingCycle = true;
                        break;
                    }
                    // toVisit.insert(toVisit.end(), temNode->inputNodes().begin(), temNode->inputNodes().end());
                    for(auto inputNode : temNode->inputNodes()){
                        if(!temNode->isInputBackEdge(inputNode)){
                            toVisit.push_back(inputNode);
                        }
                    }
                }
                if(hasPromisingCycle){
                    continue;
                }
                // to check add chain and find cycleout(if don't have cycleout, no ACC)
                auto temoutputedges = node->outputEdges();
                LLVMCDFGNode *cycleout = NULL;
                LLVMCDFGNode *ADDpre = NULL;
                bool findcycle = false;
                bool legalexchange = true;
                for (auto addpre : pre->inputNodes())
                {
                    legalexchange = true;
                    errs() << "tem ADD pre: " << addpre->getName() << "\n";
                    std::vector<LLVMCDFGNode *> todetect;
                    todetect.push_back(addpre);
                    while (!findcycle && !todetect.empty())
                    {
                        auto temnode = todetect.back();
                        todetect.pop_back();
                        auto temins = temnode->instruction();
                        if (temnode != initSel)
                        {
                            if (!isADDNode(temnode))
                            {
                                continue;
                            }
                        }
                        for (auto prenode : temnode->inputNodes())
                        {
                            if (prenode == initSel)
                            {
                                findcycle = true;
                                break;
                            }
                            todetect.push_back(prenode);
                        }
                        if (findcycle)
                        {
                            cycleout = temnode;
                            ADDpre = addpre;
                        }
                    }
                    if (findcycle)
                    {
                        errs() << "find cycle out: " << cycleout->getName() << "!!!!!!!!!!!!\n";
                        break;
                    }
                }
                // then handle related nodes to generate ACC
                if (cycleout == NULL){
                    continue;
                }
                else{
                    isACC = true;
                    if (cycleout != pre && legalexchange)
                    { // exchange add oprands
                    ///TODO: if there is other outputs from the middle of addchain
                        errs() << "cycleout: " << cycleout->getName() << "; backedge pre: " << pre->getName() << "\n";
                        LLVMCDFGNode *transoperand;
                        int preidx, cycleoutidx;
                        for (auto preinput : pre->inputNodes())
                        {
                            errs() << "\ttem backedge pre's input is: " << preinput->getName() << "\n";
                            if (preinput == ADDpre)
                            {
                                errs() << "\t\tdetected: pass\n";
                                continue;
                            }
                            else
                            {
                                errs() << "\t\tnot in cycle: record as transoperand\n";
                                transoperand = preinput;
                                preidx = pre->getInputIdx(preinput);
                            }
                        }
                        cycleoutidx = cycleout->getInputIdx(node);
                        node->addOutputNode(pre);
                        pre->addInputNode(node, preidx);
                        addEdge(node, pre, EDGE_TYPE_DATA);

                        cycleout->addInputNode(transoperand, cycleoutidx);
                        transoperand->addOutputNode(cycleout);
                        addEdge(transoperand, cycleout, EDGE_TYPE_DATA);

                        node->delOutputNode(cycleout);
                        cycleout->delInputNode(node);
                        auto temedge = edge(node, cycleout);
                        delEdge(temedge);

                        transoperand->delOutputNode(pre);
                        pre->delInputNode(transoperand);
                        temedge = edge(transoperand, pre);
                        delEdge(temedge);
                    }
                    delEdge(edge(Initial, node));
                    for (auto findinc : pre->inputNodes())
                    {
                        if (findinc == node)
                            continue;
                        else
                        {
                            node->addInputNode(findinc, 0);
                            findinc->addOutputNode(node);
                            addEdge(findinc, node, EDGE_TYPE_DATA);
                        }
                    }
                    errs() << "node->outputEdges(): " << node->outputEdges().size() << "**********\n";
                    if (node->outputEdges().size() == 1) 
                    {                                    // only output to addself
                        node->setAccFirst();
                        // pre->delInputNode(node);//delete node form add's IOmap
                        pre->delOutputNode(node);
                        for (auto addsucc : pre->outputNodes())
                        {
                            if (pre->isOutputBackEdge(addsucc))
                            { // TODO:is here right to skip ever backedge?
                                continue;
                            }
                            addsucc->addInputNode(node);
                            node->addOutputNode(addsucc);
                            addsucc->setInputIdx(node, addsucc->getInputIdx(pre));
                            addEdge(node, addsucc, EDGE_TYPE_DATA);
                        }
                        errs() << " SELECT has no other outputs---delete add calculate: " << pre->getName() << "\n";
                        node->delInputNode(pre);
                        rmnodes.insert(pre);
                    }
                    else if(pre->outputEdges().size() == 1)
                    {
                        errs() << "SELECT has succ: ";
                        for (auto succ : node->outputNodes())
                        {
                            errs() << succ->getName() << " ";
                        }
                        errs() << "\n";
                        errs() << "ADD has no other outputs---delete add calculate: " << pre->getName() << "\n";
                        node->delInputNode(pre);
                        rmnodes.insert(pre);
                    }else{
                        delEdge(edge(pre, node));
                        errs() << "ADD and SELECT both have other outputs\n";
                    }
                }
            }else if(pre->hasConst()){
                isACC = true;
                edge(pre, node)->setType(EDGE_TYPE_DATA);
                node->setInputIdx(pre, 0);
                node->setInputBackEdge(pre, false);
                pre->setOutputBackEdge(node, false);
            }else if (pre == node){
                isACC = true;
                auto constantZero = addNode("CONST", node->BB());
                constantZero->setConstVal(0);
                delEdge(edge(Initial, node));
                delEdge(edge(node, pre));
                connectNodes(constantZero, node, 0, EDGE_TYPE_DATA, false);
            }
            else{
                continue;
            }
        }
        if(isACC){
            //handle initial(const: set constVal; others: set 0 as initial, ADD original initial variable)
            if(Initial->hasConst()){
                node->setInitial(Initial->constVal());
                rmnodes.insert(Initial);
            }else{
                node->setInitial(0);
                auto InitialADD = addNode("ADD", Initial->BB());
                auto ACCOutputs = node->outputNodes();
                for(auto ACCOutput:ACCOutputs){
                    int idx = ACCOutput->getInputIdx(node);
                    delEdge(edge(node, ACCOutput));
                    addEdge(InitialADD, ACCOutput, EDGE_TYPE_DATA);
                    InitialADD->addOutputNode(ACCOutput);
                    ACCOutput->addInputNode(InitialADD, idx);
                }
                InitialADD->addInputNode(Initial, 0);
                Initial->addOutputNode(InitialADD);
                addEdge(Initial, InitialADD, EDGE_TYPE_DATA);
                InitialADD->addInputNode(node, 1);
                node->addOutputNode(InitialADD);
                addEdge(node, InitialADD, EDGE_TYPE_DATA);
            }
            // set ACC and CustomInstruction
            node->setAcc();
            node->setCustomInstruction("ACC");
            InitialSels.erase(node);
            // extract pattern of ACC
            int levels = _LoopexclusiveBBs.size();
            varType count = 1, interval = 1, repeat = 1;
            int initialLevel = levels, addLevel;
            auto initialBB = node->BB();
            auto addBB = pre->BB();
            for (int i = 0; i < levels; i++)
            {
                if (_LoopexclusiveBBs[i].count(addBB))
                {
                    addLevel = i;
                }
                if (_LoopexclusiveBBs[i].count(initialBB))
                {
                    initialLevel = i;
                }
            }
            assert(addLevel <= initialLevel);
            for (int i = 0; i < levels; i++)
            {
                if (i < addLevel)
                    interval *= _loopsAffineCounts[i];
                else if (i <= initialLevel)
                    count *= _loopsAffineCounts[i];
                else
                    repeat *= _loopsAffineCounts[i];
            }
            node->setAccPattern(count, interval, repeat);
        }
        if (node->outputEdges().size() == 0)
        { // if SELECT is dangling
            errs() << node->getName() << " is dangling\n";
            rmnodes.insert(node);
            continue;
        }
    }

    errs()<< "final rm::\n";
    for(auto rm : rmnodes){
        errs() << "remove node: "<<rm->getName()<< "\n";
        delNode(rm);
    }
}

///traverse all initialSels & generate CACC
void LLVMCDFG::generateCACC(std::set<LLVMCDFGNode *> &InitialSels){
    outs() << "----------handle conditional ACC(CACC)\n";
    for (auto node : InitialSels)//check if SELECT nodes left forming conditional select
    {
        LLVMCDFGNode *SelParent1, *ADDNode;
        LLVMCDFGNode *Initial = node->getInputPort(0);
        errs() << "\t" << node->getName() << " has initial\n";
        SelParent1 = node->getInputPort(1);
        errs() << "SelParent1 is " << SelParent1->getName() << "\n";
        for (auto input : node->inputNodes())
        {
            errs() << "input: " << input->getName() << "\n";
        }
        assert(SelParent1 != NULL);
        if (SelParent1->customInstruction() != "SELECT")
        {
            errs() << "\t" << node->getName() << "'s parent lefe is not SLECET\n";
            continue;
        }
        if (!(SelParent1->isOutputBackEdge(node)))
        {
            errs() << "\t" << node->getName() << "'s parent is not backedge next cycle\n";
            continue;
        }
        if (edge(node, SelParent1) == NULL)
        {
            errs() << "\t" << node->getName() << "'s parent do not selct it\n";
            continue;
        }
        /// now locate potential SELECTI(initialized) & its parent is SELECT too & parent to it is backedge
        /// TODO: Is Backedge always here?
        std::vector<LLVMCDFGNode *> temoutputs = node->outputNodes();
        bool findCACC = false;
        LLVMCDFGNode *cycleout;
        LLVMCDFGNode *preADD;
        bool legalexchange = true;
        // list to record nodes in the SELECT backedge cycle
        std::set<LLVMCDFGNode *> detected;
        // std::vector<std::pair<LLVMCDFGNode*, LLVMCDFGNode*>> node2parentRecord;
        for (auto selectout : temoutputs)
        {
            // to recode each node's parent (in detected path)
            std::vector<std::pair<LLVMCDFGNode *, LLVMCDFGNode *>> node2parentRecord;
            legalexchange = true;
            /// TODO:only add?; only single add?
            if (!isADDNode(selectout))
            {                          // only consider serial addition
                legalexchange = false; // TODO:how to detect cycle path RIGHT?
                continue;
            }
            // else if(selectout->instruction()->getOpcode() != Instruction::Add){
            //     legalexchange = false;
            //     continue;
            // }
            else
            {
                errs() << "tem SELECT out: " << selectout->getName() << "\n";
                std::vector<LLVMCDFGNode *> todetect = selectout->outputNodes();
                for (auto output : todetect)
                {
                    node2parentRecord.push_back(std::make_pair(output, selectout)); // record initial parent pair
                }
                while (!findCACC && !todetect.empty())
                {
                    auto temnode = todetect.back();
                    detected.insert(temnode);
                    todetect.pop_back();
                    if (temnode == SelParent1)
                    { // when find SelParent1 in cycle, its inputnode in path will be preADD
                        findCACC = true;
                        for (auto nodepair : node2parentRecord)
                        {
                            if (nodepair.first == SelParent1)
                            {
                                preADD = nodepair.second;
                                errs() << "find preADD: " << preADD->getName() << "!!!!!!!!!!!!\n";
                                break;
                            }
                        }
                        break;
                    }
                    // else if (temnode->instruction() == NULL)
                    // {
                    //     continue;
                    // }
                    // else if (temnode->instruction()->getOpcode() != Instruction::Add)
                    // {
                    //     // legalexchange = false;
                    //     continue;
                    // }
                    else
                    {
                        if (!isADDNode(temnode)){
                            continue;
                        }
                        for (auto nextoutput : temnode->outputNodes())
                        {
                            node2parentRecord.push_back(std::make_pair(nextoutput, temnode));
                            todetect.push_back(nextoutput);
                        }
                    }
                }
            }
            if (findCACC)
            {
                errs() << "find cycle out: " << selectout->getName() << "!!!!!!!!!!!!\n";
                cycleout = selectout;
                detected.insert(cycleout); // if find cycle, add cycleout into detected list
                break;
            }
        }
        if (findCACC)
        {
            assert(cycleout != NULL && preADD != NULL);
            if (cycleout != preADD && legalexchange)
            { // exchange add oprands
                errs() << "cycleout: " << cycleout->getName() << "; backedge preADD: " << preADD->getName() << "\n";
                LLVMCDFGNode *transoperand;
                int preidx, cycleoutidx;
                for (auto preinput : preADD->inputNodes())
                {
                    errs() << "\ttem backedge preADD's input is: " << preinput->getName() << "\n";
                    if (detected.count(preinput))
                    {
                        errs() << "\t\tdetected: pass\n";
                        continue;
                    }
                    else
                    {
                        errs() << "\t\tnot in cycle: record as transoperand\n";
                        transoperand = preinput;
                        preidx = preADD->getInputIdx(preinput);
                    }
                }
                cycleoutidx = cycleout->getInputIdx(node);
                node->addOutputNode(preADD);
                preADD->addInputNode(node, preidx);
                addEdge(node, preADD, EDGE_TYPE_DATA);

                cycleout->addInputNode(transoperand, cycleoutidx);
                transoperand->addOutputNode(cycleout);
                addEdge(transoperand, cycleout, EDGE_TYPE_DATA);

                node->delOutputNode(cycleout);
                cycleout->delInputNode(node);
                auto temedge = edge(node, cycleout);
                delEdge(temedge);

                transoperand->delOutputNode(preADD);
                preADD->delInputNode(transoperand);
                temedge = edge(transoperand, preADD);
                delEdge(temedge);
            }

            /// transform {2 SELCET & 1 ADD} to CACC
            LLVMCDFGNode *condition = SelParent1->getInputPort(2);
            if (SelParent1->getInputIdx(preADD) == 1)
            { // add CTRLNOT after condition if opidx 1 correspond add result
                if(condition->customInstruction() == "CTRLNOT"){
                    condition->inputNodes()[0];
                }
                else{   
                    auto ctrlnot = addNode("CTRLNOT", condition->BB());
                    condition->addOutputNode(ctrlnot);
                    ctrlnot->addInputNode(condition, 0);
                    addEdge(condition, ctrlnot, EDGE_TYPE_CTRL);
                    condition = ctrlnot;
                }
            }
            condition->addOutputNode(node);
            node->addInputNode(condition, 1); // CACC Ctrl Operand idx is 1
            addEdge(condition, node, EdgeType::EDGE_TYPE_CTRL);
            ///set acc_first (reserve node when output both exit after ISEL & condSel) 
            if (node->outputEdges().size() == 2)
            { // only output to addself
                node->setAccFirst();
                SelParent1->delOutputNode(node);
                for (auto result : SelParent1->outputNodes())
                { // connect result out to CACC
                    if (result != node)
                    {
                        node->addOutputNode(result);
                        result->addInputNode(node, result->getInputIdx(SelParent1));
                        addEdge(node, result, EDGE_TYPE_DATA);
                    }
                }
                errs() << " SELECT has no other outputs---delete condSEL: " << SelParent1->getName() << "\n";
                // trans all conection of SelParent1 to SELECT; then delnode
                delNode(SelParent1);
            }
            else if (SelParent1->outputEdges().size() == 1)
            {
                errs() << "SELECT has succ: ";
                for (auto succ : node->outputNodes())
                {
                    errs() << succ->getName() << " ";
                }
                errs() << "\n";
                errs() << "ADD has no other outputs---delete condSEL: " << SelParent1->getName() << "\n";
                delNode(SelParent1);
            }
            else
            {
                delEdge(edge(SelParent1, node));
                errs() << "ADD and SELECT both have other outputs\n";
            }
            

            ADDNode = preADD;
            for (auto addend : ADDNode->inputNodes())
            {
                if (addend != node)
                {
                    addend->addOutputNode(node);
                    node->addInputNode(addend, 0);
                    addEdge(addend, node, EDGE_TYPE_DATA);
                }
            }
            if (ADDNode->outputNodes().size() == 0)
            { /// ADD result only output to SelParent; then delnode
                delNode(ADDNode);
            }
            // handle initial(const: set constVal; others: set 0 as initial, ADD original initial variable)
            if (Initial->hasConst())
            {
                node->setInitial(Initial->constVal());
                delNode(Initial);
            }
            else
            {
                node->setInitial(0);
                auto InitialADD = addNode("ADD", Initial->BB());
                auto ACCOutputs = node->outputNodes();
                for (auto ACCOutput : ACCOutputs)
                {
                    int idx = ACCOutput->getInputIdx(node);
                    delEdge(edge(node, ACCOutput));
                    addEdge(InitialADD, ACCOutput, EDGE_TYPE_DATA);
                    InitialADD->addOutputNode(ACCOutput);
                    ACCOutput->addInputNode(InitialADD, idx);
                }
                InitialADD->addInputNode(Initial, 0);
                Initial->addOutputNode(InitialADD);
                addEdge(Initial, InitialADD, EDGE_TYPE_DATA);
                InitialADD->addInputNode(node, 1);
                node->addOutputNode(InitialADD);
                addEdge(node, InitialADD, EDGE_TYPE_DATA);
            }

            // rename CACC
            node->setCustomInstruction("CACC");
            node->setAcc();
            // extract pattern of ACC
            int levels = _LoopexclusiveBBs.size();
            varType count = 1, interval = 1, repeat = 1;
            int initialLevel = levels, addLevel;
            auto initialBB = node->BB();
            auto addBB = ADDNode->BB();
            for (int i = 0; i < levels; i++)
            {
                if (_LoopexclusiveBBs[i].count(addBB))
                {
                    addLevel = i;
                }
                if (_LoopexclusiveBBs[i].count(initialBB))
                {
                    initialLevel = i;
                }
            }
            assert(addLevel <= initialLevel);
            for (int i = 0; i < levels; i++)
            {
                if (i < addLevel)
                    interval *= _loopsAffineCounts[i];
                else if (i <= initialLevel)
                    count *= _loopsAffineCounts[i];
                else
                    repeat *= _loopsAffineCounts[i];
            }
            node->setAccPattern(count, interval, repeat);
        }
    }
}


void LLVMCDFG::handleSelectNodes(){
    std::map<int, LLVMCDFGNode *> nodes = _nodes;
    std::set<LLVMCDFGNode*> InitialSels;
    std::set<LLVMCDFGNode*> rmnodes;
    std::set<LLVMCDFGNode*> skipSel;
    //find initial SELECT for multi-level loop
    for (auto &elem : _nodes)
    {
        auto node = elem.second;
        if(skipSel.count(node)){
            continue;
        }
        if (node->customInstruction() == "SELECT")
        {
            errs() << "check SELECT: " << node->getName() << ": "; node->instruction()->dump();
            errs() << "\t"<< node->getName() << " has inputs: "<< node->inputNodes().size() << ": ";
            bool hasinitial = false;
            LLVMCDFGNode* Initial = NULL;//record INPUT if exist
            auto teminputs = node->inputNodes();
            for (auto pre : teminputs){
                errs() << pre->getName() << " ";
            }
            //input nodes idx problem
            assert(teminputs.size() == 3);
            auto pre = node->getInputPort(1);
            auto pre0 = node->getInputPort(0);
            ///if SELECT node controled by LOOPSTART to InitSELECT
            if (node->getInputPort(2)->customInstruction() == "LOOPSTART")
            {
                Initial = pre0;
                hasinitial = true;
                node->setInitial(0);
                delEdge(edge(node->getInputPort(2), node));
                node->setCustomInstruction("InitSELECT");
                InitialSels.insert(node);
                // extract refresh interval of InitSELECT
                int levels = _LoopexclusiveBBs.size();
                varType interval = 1, count = 1, repeat = 1;
                int initialLevel, iterateLevel;
                auto initialBB = node->BB();
                auto iterateBB = pre->BB();
                // errs() << "initial BB: " << initialBB->getName() << " initial ins: "; node->instruction()->dump();
                // errs() << "iterate BB: " << iterateBB->getName() << " iterate ins: "; pre->instruction()->dump(); 
                for (int i = 0; i < levels; i++)
                {
                    if (_LoopexclusiveBBs[i].count(initialBB))
                    {
                        initialLevel = i;
                    }
                    if (_LoopexclusiveBBs[i].count(iterateBB))
                    {
                        iterateLevel = i;
                    }
                }
                // if(initialLevel == levels - 1){
                //     interval = 0;//initial only once
                // }
                assert(initialLevel>=iterateLevel);
                node->setCummuLevels(initialLevel, iterateLevel);
                for (int i = 0; i < levels; i++)
                {
                    if (i < iterateLevel)
                        interval *= _loopsAffineCounts[i];
                    else if(i <= initialLevel)
                        count *= _loopsAffineCounts[i];
                    else
                        repeat *= _loopsAffineCounts[i];
                }
                node->setAccPattern(count,interval,repeat);
            }
            //assert(preins != NULL && "what's wrong with backedge");
            //start from initial, find select with LOOPSTART down
            std::map<LLVMCDFGNode *, int> initSels;
            LLVMCDFGNode *initSel = node;
            for (int i = 0; i < _LoopexclusiveBBs.size(); i++)
            {
                if (_LoopexclusiveBBs[i].count(initSel->BB()))
                {
                    initSels[initSel] = i;
                    errs() << "\toriginal initsel of loop " << i << " \n";
                    break;
                }
            }
            while (1)
            {
                bool findinitSel = false;
                for (auto initout : initSel->outputNodes())
                {
                    if (initout->customInstruction() == "SELECT")
                    {
                        if (initout->getInputPort(2)->customInstruction() == "LOOPSTART" && initout->getInputIdx(initSel) == 0)
                        {
                            if (pre->isOutputBackEdge(initout))
                            {
                                errs() << "\t\ttem init SELECT is "<< initout->getName() << "\n";
                                skipSel.insert(initout);
                                rmnodes.insert(initout);
                                auto outputNodes = initout->outputNodes();//copy
                                for(auto succInitout : outputNodes){
                                    int idx = succInitout->getInputIdx(initout);
                                    EdgeType type = edge(initout, succInitout)->type();
                                    delEdge(edge(initout, succInitout));
                                    initSel->addOutputNode(succInitout);
                                    succInitout->addInputNode(initSel, idx);
                                    addEdge(initSel, succInitout, type);
                                }
                                delEdge(edge(initSel, initout));
                                findinitSel = true;
                                break;
                            }
                        }
                    }else if(initout->customInstruction() == "InitSELECT" && initout->getInputIdx(initSel) == 0){
                        if (pre->isOutputBackEdge(initout))
                        {
                            errs() << "\t\ttem init SELECT is "<< initout->getName() << "\n";
                            skipSel.insert(initout);
                            rmnodes.insert(initout);
                            auto outputNodes = initout->outputNodes();//copy
                            for(auto succInitout : outputNodes){
                                int idx = succInitout->getInputIdx(initout);
                                EdgeType type = edge(initout, succInitout)->type();
                                delEdge(edge(initout, succInitout));
                                initSel->addOutputNode(succInitout);
                                succInitout->addInputNode(initSel, idx);
                                addEdge(initSel, succInitout, type);
                            }
                            delEdge(edge(initSel, initout));
                            findinitSel = true;
                            break;
                        }
                    }
                }
                if (!findinitSel)
                {
                    break;
                }
            }
            errs() << "---final initSel: " << node->getName() << "\n";
        }
    }
    for(auto rm : rmnodes){
        errs() << "remove node: "<<rm->getName()<< "\n";
        if(rm->customInstruction() == "InitSELECT"){
            InitialSels.erase(rm);
        }
        delNode(rm);
    }
    rmnodes.clear();

    if(_noACC){
        return;
    }
    printDOT("afterInitialSel.dot");

    ///TODO: multi reset value
    ///Detect Conditional Dual Initial
    outs() << "------Detect Conditional Dual Initial\n";
    //key(std::pair) 
    //----first:node to transform; 
    //----second:the node corresponding value to Dual Initial
    //value(std::vector) 
    //----LLVMCDFGNodes of contions
    std::map<std::pair<LLVMCDFGNode*, LLVMCDFGNode*>, std::vector<LLVMCDFGNode*>> DualInit2ConditionPath;
    for (auto node : InitialSels){
        //auto node = *&elem;
        errs() << "--------analyze root node {" << node->getName() << "}----\n";
        LLVMCDFGNode* DualInitValue, *DualInitSel, *DualInitSelPre;
        assert(node->isInputBackEdge(node->getInputPort(1)));
        auto backedgeNode = node->getInputPort(1);
        std::vector<LLVMCDFGNode*> treeNodeStack;
        //analyze SELECT tree from backedge node and record in selectTree
        std::set<LLVMCDFGNode*> selectTree;
        bool promisingCycle = false;
        analyzeSelectTree(backedgeNode, node, selectTree, promisingCycle);
        if(promisingCycle){
            continue;
        }
        selectTree.insert(node);
        outs() << "selectTree has nodes: ";
        for(auto selectTreenode:selectTree){
            outs() << selectTreenode->getName() << " ";
        }
        outs() << "\n";
        treeNodeStack.push_back(backedgeNode);
        bool legal = true;
        bool hasDualInit = false;
        while(!treeNodeStack.empty())
        {
            auto temNode = treeNodeStack.back();
            treeNodeStack.pop_back();
            if(temNode == node)
                continue;
            if(selectTree.count(temNode)){
                if(isADDNode(temNode)){
                    while(temNode->getInputIdx(node) == -1 && legal){
                        LLVMCDFGNode* preADD;
                        LLVMCDFGNode* opNotinTree;
                        int preADDidx;
                        bool Addop0inTree = selectTree.count(temNode->getInputPort(0));
                        bool Addop1inTree = selectTree.count(temNode->getInputPort(1));
                        assert(!(Addop0inTree && Addop1inTree));
                        if(Addop0inTree){
                            preADD = temNode->getInputPort(0);
                            opNotinTree = temNode->getInputPort(1);
                            preADDidx = 0;
                        }else{
                            preADD = temNode->getInputPort(1);
                            opNotinTree = temNode->getInputPort(0);
                            preADDidx = 1;
                        }
                        if(isADDNode(preADD)){
                            LLVMCDFGNode* prepreADD;
                            int prepreADDidx;
                            Addop0inTree = selectTree.count(preADD->getInputPort(0));
                            Addop1inTree = selectTree.count(preADD->getInputPort(1));
                            assert(!(Addop0inTree && Addop1inTree));
                            if(Addop0inTree){
                                prepreADD = preADD->getInputPort(0);
                                prepreADDidx = 0;
                            }else{
                                prepreADD = preADD->getInputPort(1);
                                prepreADDidx = 1;
                            }
                            delEdge(edge(opNotinTree, temNode));
                            delEdge(edge(prepreADD, preADD));
                            temNode->addInputNode(prepreADD, 1-preADDidx);
                            prepreADD->addOutputNode(temNode);
                            addEdge(prepreADD, temNode, EDGE_TYPE_DATA);
                            preADD->addInputNode(opNotinTree, prepreADDidx);
                            opNotinTree->addOutputNode(preADD);
                            addEdge(opNotinTree, preADD, EDGE_TYPE_DATA);
                        }else{
                            legal = false;
                        }
                        selectTree.erase(preADD);
                    }
                    if(temNode->getInputIdx(node) != -1){
                        int inTreeidx = 1-temNode->getInputIdx(node);
                        treeNodeStack.push_back(temNode->getInputPort(inTreeidx));
                    }else{
                        legal = false;
                    }
                }else if(temNode->customInstruction() == "SELECT"){
                    bool Selop0inTree = selectTree.count(temNode->getInputPort(0));
                    bool Selop1inTree = selectTree.count(temNode->getInputPort(1));
                    LLVMCDFGNode* preNode;//in Tree prenode
                    LLVMCDFGNode* opNotinTree;//not in Tree prenode
                    int inTreeidx;
                    if(Selop0inTree && Selop1inTree){
                        treeNodeStack.push_back(temNode->getInputPort(0));
                        treeNodeStack.push_back(temNode->getInputPort(1));
                        continue;
                    }
                    //SELECT has input out of TREE---->conditional DualInit
                    else if(Selop0inTree){
                        preNode = temNode->getInputPort(0);
                        opNotinTree = temNode->getInputPort(1);
                        inTreeidx = 0;
                    }else{
                        preNode = temNode->getInputPort(1);
                        opNotinTree = temNode->getInputPort(0);
                        inTreeidx = 1;
                    }
                    treeNodeStack.push_back(preNode);
                    if(temNode != backedgeNode){
                        assert(temNode->outputNodes().size() == 1);
                    }
                    DualInitValue = opNotinTree;
                    DualInitSel = temNode;
                    DualInitSelPre = preNode;
                    hasDualInit = true;
                }else{
                    legal = false;
                }
            }else{
                continue;
            }
        }//while end
        //if(legal && hasDualInit && hardwaresupport){
        if(legal && hasDualInit){
            auto preDualInitPathNode = DualInitValue;
            auto DualInitPathNode = DualInitSel;
            errs() << "-------conclude DualInitPath Nodes\n";
            while(1){
                errs() << "temp DualInitPath is: " << DualInitPathNode->getName() <<\
                "; outputNodes: " << DualInitPathNode->outputNodes().size() <<"\n";
                int idx = DualInitPathNode->getInputIdx(preDualInitPathNode);
                auto condition = DualInitPathNode->getInputPort(2);
                if (idx == 1){
                    if(condition->customInstruction() == "CTRLNOT"){
                        condition = condition->inputNodes()[0];
                    }
                    else{
                        bool hasNot = false;
                        for (auto ctrlnot : condition->outputNodes())
                        {
                            if (ctrlnot->customInstruction() == "CTRLNOT")
                            {
                                condition = ctrlnot;
                                hasNot = true;
                                break;
                            }
                        }
                        if(!hasNot){
                            auto NewCtrlNot = addNode("CTRLNOT", condition->BB());
                            connectNodes(condition, NewCtrlNot, 0, EDGE_TYPE_CTRL);
                            condition = NewCtrlNot;
                        }
                    }
                }
                auto DualInitpair = std::make_pair(node, DualInitValue);
                DualInit2ConditionPath[DualInitpair].push_back(condition);
                if(DualInitPathNode == backedgeNode){
                    break;
                }
                assert(DualInitPathNode->outputNodes().size() == 1 && "what's wrong with SELECT tree");
                auto outputNode = DualInitPathNode->outputNodes()[0];
                DualInitPathNode = outputNode;
            }
            //LLVMCDFGNode* DualInitValue, *DualInitSel, *DualInitSelPre;
            if(DualInitSel == backedgeNode){
                for(auto out:DualInitSel->outputNodes()){
                    bool isBackEdge = DualInitSel->isOutputBackEdge(out);
                    int idx = out->getInputIdx(DualInitSel);
                    connectNodes(DualInitSelPre, out, idx, EDGE_TYPE_DATA, isBackEdge);
                }
            }
            else{    
                assert(DualInitSel->outputNodes().size() == 1);
                LLVMCDFGNode* outputNode = DualInitSel->outputNodes()[0];
                int idx = outputNode->getInputIdx(DualInitSel);
                errs() << "tem DualInitSel: " << DualInitSel->getName() << " idx = "<<idx<<"\n";
                connectNodes(DualInitSelPre, outputNode, idx, EDGE_TYPE_DATA);
            }
            delNode(DualInitSel);
        }
    }
    simplifyCtrlLogic();
    transAddOpSelectTree(); 
    
    printDOT("afterDetectDualInitial.dot");
    generateACC(InitialSels);

    // printDOT("afterGenerateACC.dot");

    generateCACC(InitialSels);

    printDOT("before_ACC2CACC.dot");
    ///ACC to CACC
    for (auto &elem : _nodes){
        auto node = elem.second;
        if(node->customInstruction() == "ACC"){
            errs() << "tem ACC in is: " << node->getName() << "\n";
            bool findZeroConst = false;
            LLVMCDFGNode* ACCIn = node->inputNodes()[0];
            std::vector<LLVMCDFGNode*> SelsStack;
            std::vector<LLVMCDFGNode*> Conditionpath;
            LLVMCDFGNode* remainPathNode = NULL;
            if(ACCIn->customInstruction() == "SELECT"){
                SelsStack.push_back(ACCIn);
            }
            while (!SelsStack.empty() && !findZeroConst)
            {
                LLVMCDFGNode* temNode = SelsStack.back();
                SelsStack.pop_back();
                errs() << "\ttem SELECT is: " << temNode->getName() << "\n";
                for(int i = 0; i < 2; i++){
                    auto inputNode = temNode->getInputPort(i);
                    if(inputNode->customInstruction() == "SELECT"){
                        SelsStack.push_back(inputNode);
                    }else if(inputNode->customInstruction() == "CONST" && inputNode->constVal() == 0){
                        findZeroConst = true;
                        remainPathNode = inputNode;
                        if(temNode->outputNodes().size() == 1){
                            auto outputNode = temNode->outputNodes()[0];
                            int idx = outputNode->getInputIdx(temNode);
                            rmnodes.insert(temNode);
                            temNode->getInputPort(1-i)->addOutputNode(outputNode);
                            outputNode->addInputNode(temNode->getInputPort(1-i), idx);
                            addEdge(temNode->getInputPort(1-i), outputNode, EDGE_TYPE_DATA);
                        }
                        ///set CACC
                        node->setCustomInstruction("CACC");
                    }
                }
            }
            if(remainPathNode == NULL){
                continue;
            }
            errs() << "-------conclude remainPath Nodes\n";
            while(remainPathNode != ACCIn){
                errs() << "temp remainPathNode is: " << remainPathNode->getName() <<"; outputNodes: " << remainPathNode->outputNodes().size() <<"\n";
                assert(remainPathNode->outputNodes().size() == 1 && "what's wrong with SELECT tree");
                auto temOutput = remainPathNode->outputNodes()[0];
                int idx = temOutput->getInputIdx(remainPathNode);
                auto condition = temOutput->getInputPort(2);
                if (idx == 1){
                    if(condition->customInstruction() == "CTRLNOT"){
                        condition = condition->inputNodes()[0];
                    }
                    else{    
                        bool hasNot = false;
                        for (auto ctrlnot : condition->outputNodes())
                        {
                            if (ctrlnot->customInstruction() == "CTRLNOT")
                            {
                                condition = ctrlnot;
                                hasNot = true;
                                break;
                            }
                        }
                        if(!hasNot){
                            auto NewCtrlNot = addNode("CTRLNOT", condition->BB());
                            connectNodes(condition, NewCtrlNot, 0, EDGE_TYPE_CTRL);
                            condition = NewCtrlNot;
                        }
                    }
                }
                remainPathNode = temOutput;
                Conditionpath.push_back(condition);
            }
            assert(Conditionpath.size() > 0);
            LLVMCDFGNode* finalCond;
            if(Conditionpath.size() == 1){
                finalCond = Conditionpath[0];
            }else{
                LLVMCDFGNode* NewCtrlAnd;
                LLVMCDFGNode* and0 = Conditionpath[0];
                LLVMCDFGNode* and1 = Conditionpath[1];
                for(int i = 1; i < Conditionpath.size(); i++){
                    NewCtrlAnd = addNode("CTRLAND", node->BB());
                    NewCtrlAnd->addInputNode(and0, 0);
                    and0->addOutputNode(NewCtrlAnd);
                    addEdge(and0, NewCtrlAnd, EDGE_TYPE_CTRL);
                    NewCtrlAnd->addInputNode(and1, 1);
                    and1->addOutputNode(NewCtrlAnd);
                    addEdge(and1, NewCtrlAnd, EDGE_TYPE_CTRL);
                    and0 = NewCtrlAnd;
                    and1 = Conditionpath[i+1];
                }
                finalCond = NewCtrlAnd;
            }
            LLVMCDFGNode* finalNot = NULL;
            if(finalCond->customInstruction() == "CTRLNOT"){
                finalCond = finalCond->inputNodes()[0];
            }
            else{   
                for (auto ctrlnot : finalCond->outputNodes())
                {
                    if (ctrlnot->customInstruction() == "CTRLNOT")
                    {
                        finalNot = ctrlnot;
                        break;
                    }
                }
                if(finalNot == NULL){
                    finalNot = addNode("CTRLNOT", node->BB());
                    finalCond->addOutputNode(finalNot);
                    finalNot->addInputNode(finalCond, 0);
                    addEdge(finalCond, finalNot, EDGE_TYPE_CTRL);
                    finalCond = finalNot;
                }
            }
            node->addInputNode(finalCond, 1);
            finalCond->addOutputNode(node);
            addEdge(finalCond, node, EDGE_TYPE_CTRL);
        }
    }
    for(auto rmnode:rmnodes){
        delNode(rmnode);
    }
    rmnodes.clear();

    // printDOT("brforeCDIACC.dot");
    ///ACC/CACC to CDIACC
    errs() << "--------transform ACC/CACC to CDIACC\n";
    for(auto &elem : DualInit2ConditionPath){
        auto node = elem.first.first;
        auto DualInitValue = elem.first.second;
        auto Conditionpath = elem.second;
        errs() << "temNode is: " << node->getName() << "\n";
        assert(node->customInstruction() == "ACC" || node->customInstruction() == "CACC");
        assert(Conditionpath.size() > 0);
        LLVMCDFGNode* finalCond;
        if(Conditionpath.size() == 1){
            finalCond = Conditionpath[0];
        }else{
            LLVMCDFGNode* NewCtrlAnd;
            LLVMCDFGNode* and0 = Conditionpath[0];
            LLVMCDFGNode* and1 = Conditionpath[1];
            for(int i = 1; i < Conditionpath.size(); i++){
                NewCtrlAnd = addNode("CTRLAND", node->BB());
                NewCtrlAnd->addInputNode(and0, 0);
                and0->addOutputNode(NewCtrlAnd);
                addEdge(and0, NewCtrlAnd, EDGE_TYPE_CTRL);
                NewCtrlAnd->addInputNode(and1, 1);
                and1->addOutputNode(NewCtrlAnd);
                addEdge(and1, NewCtrlAnd, EDGE_TYPE_CTRL);
                and0 = NewCtrlAnd;
                and1 = Conditionpath[i+1];
            }
            finalCond = NewCtrlAnd;
        }
        if(node->customInstruction() == "CACC"){
            node->setInputIdx(node->getInputPort(1), 2);
        }else{
            LLVMCDFGNode* NewConst_1 = addNode("CONST", node->BB());
            NewConst_1->setConstVal(1);
            connectNodes(NewConst_1, node, 2, EDGE_TYPE_CTRL);
        }
        connectNodes(finalCond, node, 3, EDGE_TYPE_CTRL);
        connectNodes(DualInitValue, node, 1, EDGE_TYPE_DATA);
        node->setCustomInstruction("CDIACC");
        if(DualInitValue->hasConst()){
            node->setDualInitVal(DualInitValue->constVal());
        }
    }
}

    // generate MAC nodes
    void LLVMCDFG::genMACNodes(){
        std::map<int, LLVMCDFGNode *> nodes = _nodes;
        for(auto &elem : nodes){
            LLVMCDFGNode* node = elem.second;
            if(node->customInstruction() == "ACC"){
                auto inNode = node->getInputPort(0);
                unsigned int opcode = -1;
                if(inNode->instruction() != NULL){
                    opcode = inNode->instruction()->getOpcode();
                }else if(inNode->customInstruction() == "MUL"){
                    opcode = Instruction::Mul;
                }
                if(opcode == Instruction::Mul || opcode == Instruction::FMul){
                    auto op0 = inNode->getInputPort(0);
                    auto op1 = inNode->getInputPort(1);
                    errs() << "ACC is: " << node->getName() <<", MUL is: "<< inNode->getName() << "\n";
                    if(inNode->outputNodes().size() == 1)
                        delNode(inNode);
                    else
                        delEdge(edge(inNode, node));
                    node->addInputNode(op0, 0);
                    op0->addOutputNode(node);
                    addEdge(op0, node, EDGE_TYPE_DATA);
                    node->addInputNode(op1, 1);
                    op1->addOutputNode(node);
                    addEdge(op1, node, EDGE_TYPE_DATA);
                    node->setCustomInstruction("MAC");
                }
            }
        }
    }

// print DOT file of CDFG
void LLVMCDFG::printDOT(std::string fileName) {
#ifndef DUMP_TMP_CDFG
    return;
#endif
    printTimes += 1;
	std::ofstream ofs;
	ofs.open(fileName.c_str());
    ofs << "Digraph G {\n";
    // nodes
	assert(_nodes.size() != 0);
    for(auto &elem : _nodes){
        auto node = elem.second;
        auto name = node->getName();
        auto ins = node->instruction();
        ofs << name << "[label = \"" << name;
        //add address name of LSU
        if(ins != NULL){
            if(dyn_cast<LoadInst>(ins) || dyn_cast<StoreInst>(ins))
            ofs << ", ArrayName=" << node->getLSArrayName();
            //errs() << "sueecss land\n";
        }
        if(node->isLSaffine()){
            ofs << ", Stride=[ ";
            auto stride = node->getLSstride();
            for(int i = 0; i < stride.size(); i++){
                ofs << i<< ", " << stride[i].to_string() <<"; ";
            }
            ofs << "], Bounds=[ ";
            auto LSbounds = node->getLSbounds();
            for(int dim = 0; dim < 1; dim++){
                ofs << "(" << LSbounds[0].to_string()<<", "<<LSbounds[1].to_string()<<")*"<<LSbounds[2].to_string()<<"; ";
            }
            ofs << "]";
        }
        if(node->hasConst()){
            ofs << ", Const=" << node->constVal();
        }
        ofs << "\", shape = box, color = black];\n";
    }
	// edges
    std::map<std::pair<LLVMCDFGNode*, LLVMCDFGNode*>, int> edgestack;    
    for(auto &elem : _edges){
        auto edge = elem.second;
        auto srcName = edge->src()->getName();
        auto dstName = edge->dst()->getName();
        ofs << srcName << " -> " << dstName;
        // errs() << srcName << " -> " << dstName;
        auto type = edge->type();
        if(type == EDGE_TYPE_CTRL || type == EDGE_TYPE_TEND){
            ofs << "[color = red";
        }else if(type == EDGE_TYPE_MEM){
            ofs << "[color = blue";
        }else{
            ofs << "[color = black";
        }
        bool isBackEdge = edge->src()->isOutputBackEdge(edge->dst());
        if(isBackEdge){
            ofs << ", style = dashed";
        }else{
            ofs << ", style = bold";
        }
        int opIdx = edge->dst()->getInputIdx(edge->src());
        auto pair = std::make_pair(edge->src(), edge->dst());
        if(edgestack.count(pair)){
            opIdx = 1 - edgestack[pair];
        }else{
            edgestack[pair] = opIdx;
        }
        ofs << ", label = \"Op=" << opIdx;
        if(type == EDGE_TYPE_MEM){
            ofs << ", iterdist = " << edge->dst()->getSrcDep(edge->src()).distance;
        }
        if(type == EDGE_TYPE_TEND){
            ofs << ", TaskEnd";
        }
        ofs << "\"];\n";
    }
	ofs << "}\n";
	ofs.close();
}

void LLVMCDFG::printHierarchyDOT(std::string fileName) {
#ifndef DUMP_TMP_CDFG
    return;
#endif
	std::ofstream ofs;
	ofs.open(fileName.c_str());
    ofs << "digraph G {\n";
    // nodes
	assert(_nodes.size() != 0);
    int levels = nestloops().size();
    for(int level = levels-1; level >= 0; level--){
        ofs << "subgraph cluster_" << level << "{\n";
        ofs << "style = \"dashed\";\n";
        ofs << "label = \"level" << level <<"\";\n";
        for(auto &elem : _nodes){
            auto node = elem.second;
            auto name = node->getName();
            auto ins = node->instruction();
            if(!_LoopexclusiveBBs[level].count(node->BB())){
                continue;
            }
            ofs << name << "[label = \"" << name;
            //add address name of LSU
            if(ins != NULL){
                if(dyn_cast<LoadInst>(ins) || dyn_cast<StoreInst>(ins))
                ofs << ", ArrayName=" << node->getLSArrayName();
                //errs() << "sueecss land\n";
            }
            if(node->isLSaffine()){
                ofs << ", Stride=[ ";
                auto stride = node->getLSstride();
                for(int i = 0; i < stride.size(); i++){
                    ofs << i<< ", " << stride[i].to_string() <<"; ";
                }
                ofs << "], Bounds=[ ";
                auto LSbounds = node->getLSbounds();
                for(int dim = 0; dim < 1; dim++){
                    ofs << "(" << LSbounds[0].to_string()<<", "<<LSbounds[1].to_string()<<")*"<<LSbounds[2].to_string()<<"; ";
                }
                ofs << "]";
            }
            if(node->hasConst()){
                ofs << ", Const=" << node->constVal();
            }
            ofs << "\", shape = box, color = black];\n";
        }
    }
    for(int level = levels-1; level >= 0; level--){
        ofs << "}\n";
    }

	// edges
    std::map<std::pair<LLVMCDFGNode*, LLVMCDFGNode*>, int> edgestack;    
    for(auto &elem : _edges){
        auto edge = elem.second;
        auto srcName = edge->src()->getName();
        auto dstName = edge->dst()->getName();
        ofs << srcName << " -> " << dstName;
        auto type = edge->type();
        if(type == EDGE_TYPE_CTRL){
            ofs << "[color = red";
        }else if(type == EDGE_TYPE_MEM){
            ofs << "[color = blue";
        }else{
            ofs << "[color = black";
        }
        bool isBackEdge = edge->src()->isOutputBackEdge(edge->dst());
        if(isBackEdge){
            ofs << ", style = dashed";
        }else{
            ofs << ", style = bold";
        }
        int opIdx = edge->dst()->getInputIdx(edge->src());
        auto pair = std::make_pair(edge->src(), edge->dst());
        if(edgestack.count(pair)){
            opIdx = 1 - edgestack[pair];
        }else{
            edgestack[pair] = opIdx;
        }
        ofs << ", label = \"Op=" << opIdx;
        if(type == EDGE_TYPE_MEM){
            ofs << ", iterdist = " << edge->dst()->getSrcDep(edge->src()).distance;
        }
        ofs << "\"];\n";
    }
	ofs << "}\n";
	ofs.close();
}

// enforce print DOT file of CDFG
void LLVMCDFG::enforceprintDOT(std::string fileName) {
	std::ofstream ofs;
	ofs.open(fileName.c_str());
    ofs << "Digraph G {\n";
    // nodes
	assert(_nodes.size() != 0);
    for(auto &elem : _nodes){
        auto node = elem.second;
        auto name = node->getName();
        auto ins = node->instruction();
        ofs << name << "[label = \"" << name;
        //add address name of LSU
        if(ins != NULL){
            if(dyn_cast<LoadInst>(ins) || dyn_cast<StoreInst>(ins))
            ofs << ", ArrayName=" << node->getLSArrayName();
            //errs() << "sueecss land\n";
        }
        if(node->isLSaffine()){
            ofs << ", Stride=[ ";
            auto stride = node->getLSstride();
            for(int i = 0; i < stride.size(); i++){
                ofs << i<< ", " << stride[i].to_string() <<"; ";
            }
            ofs << "], Bounds=[ ";
            auto LSbounds = node->getLSbounds();
            for(int dim = 0; dim < 1; dim++){
                ofs << "(" << LSbounds[0].to_string()<<", "<<LSbounds[1].to_string()<<")*"<<LSbounds[2].to_string()<<"; ";
            }
            ofs << "]";
        }
        if(node->hasConst()){
            ofs << ", Const=" << node->constVal();
        }
        ofs << "\", shape = box, color = black];\n";
    }
	// edges
    std::map<std::pair<LLVMCDFGNode*, LLVMCDFGNode*>, int> edgestack;    
    for(auto &elem : _edges){
        auto edge = elem.second;
        auto srcName = edge->src()->getName();
        auto dstName = edge->dst()->getName();
        ofs << srcName << " -> " << dstName;
        // errs() << srcName << " -> " << dstName;
        auto type = edge->type();
        if(type == EDGE_TYPE_CTRL || type == EDGE_TYPE_TEND){
            ofs << "[color = red";
        }else if(type == EDGE_TYPE_MEM){
            ofs << "[color = blue";
        }else{
            ofs << "[color = black";
        }
        bool isBackEdge = edge->src()->isOutputBackEdge(edge->dst());
        if(isBackEdge){
            ofs << ", style = dashed";
        }else{
            ofs << ", style = bold";
        }
        int opIdx = edge->dst()->getInputIdx(edge->src());
        auto pair = std::make_pair(edge->src(), edge->dst());
        if(edgestack.count(pair)){
            opIdx = 1 - edgestack[pair];
        }else{
            edgestack[pair] = opIdx;
        }
        ofs << ", label = \"Op=" << opIdx;
        if(type == EDGE_TYPE_MEM){
            ofs << ", iterdist = " << edge->dst()->getSrcDep(edge->src()).distance;
        }
        if(type == EDGE_TYPE_TEND){
            ofs << ", TaskEnd";
        }
        ofs << "\"];\n";
    }
	ofs << "}\n";
	ofs.close();
}


void LLVMCDFG::printAffineDOT(std::string fileName) {
    printTimes += 1;
    errs() << "-------------print final affine DOT\n";
	std::ofstream ofs;
    auto loops = _nestloops;
    std::map<std::string, int> minoffset;
    std::map<std::string, int> maxsize;
    std::map<std::string, int> multiportMap;
    //aligent load/store (min) offset
    for(auto &elem : _nodes){
        auto node = elem.second;
        auto ins = node->instruction();
        if(ins != NULL){
            if(node->getLSoffset().index() == 0){
                int LSoffset = std::get<int>(node->getLSoffset().value);
                if(dyn_cast<LoadInst>(ins) || dyn_cast<StoreInst>(ins)){
                    auto arrayname = node->getLSArrayName();
                    // errs() << "node: " << node->getName() << "; arrayName: " << arrayname << "\n";
                    // errs() << "offset: " << LSoffset << "\n";
                    if(!minoffset.count(arrayname)){
                        minoffset[arrayname] = LSoffset;
                    }else if(minoffset[arrayname] > LSoffset){
                        minoffset[arrayname] = LSoffset;
                    }
                }
            }
        }
    }
    // int idx = 0;
    //aligent load/store (max) size
    for(auto &elem : _nodes){
        auto node = elem.second;
        auto ins = node->instruction();
        if(ins != NULL){
            if(dyn_cast<LoadInst>(ins) || dyn_cast<StoreInst>(ins)){
                auto LoadBound = node->getLSbounds();
                auto arrayname = node->getLSArrayName();
                multiportMap[arrayname]++;
                varType size = 0;
                // if(node->isLSaffine()){
                //     size += LoadBound[1] - LoadBound[0];
                //     size += LoadBound[2];
                // }else{
                // errs() << "node: " << node->getName() << "; arrayName: " << arrayname << "\n";
                // errs() << "LoadBound[0]: " << LoadBound[0].to_string() << "; LoadBound[1]: " << LoadBound[1].to_string() <<  "; LoadBound[2]: " << LoadBound[2].to_string() << "\n";
                size = LoadBound[1]-LoadBound[0]+LoadBound[2];
                if(size.index() == 2){
                    continue;
                }
                //}
                int sizeint = std::get<int>(size.value);
                int LSoffset = std::get<int>(node->getLSoffset().value);
                if(minoffset[arrayname] < LSoffset){
                    sizeint += (LSoffset-minoffset[arrayname]);
                }
                if(!maxsize.count(arrayname)){
                    maxsize[arrayname] = sizeint;
                }else if(maxsize[arrayname] < sizeint){
                    maxsize[arrayname] = sizeint;
                }
            }
        }
    }
	ofs.open(fileName.c_str());
    ofs << "Digraph G {\n";
    // nodes
	assert(_nodes.size() != 0);
    for(auto &elem : _nodes){
        auto node = elem.second;
        auto name = node->getName();
        auto ins = node->instruction();
        ofs << name<<"[opcode=" << node->finalInstruction();
        if(node->isAcc()){
            auto patternmap = node->getAccPattern();
            ofs << ", acc_params=\""<< node->getInitial();
            ofs << ", " << patternmap["count"].to_string();
            ofs << ", " << patternmap["interval"].to_string();      
            ofs << ", " << patternmap["repeat"].to_string() <<"\"";
            ofs << ", acc_first=" << node->isAccFirst();
            if(node->hasConstDualInit()){
                ofs << ", second_initVal=" << node->getDualInitVal();
            }
        }
        else if(node->hasConst()){
            ofs << ", label=\""<<name<< ", Const="<< node->constVal() << "\"";
            ofs << ", value="<<node->constVal();
        }else if(node->hasArgIn()){
            ofs << ", argNum=" << node->argNum();
        }
        else if(node->isLSaffine()){
            std::string arrayname = node->getLSArrayName();
            ofs << ", ref_name=\"" << arrayname<<"\"";\
            auto LSbounds = node->getLSbounds();
            varType size = 0;
            varType offset;
            varType reduceoffset;
            // errs() << "node: " << node->getName() << "; arrayName: " << arrayname << "\n";
            if(minoffset.count(arrayname) && (dyn_cast<LoadInst>(ins) || dyn_cast<StoreInst>(ins))){
                offset = minoffset[arrayname];
            }else{
                offset = node->getLSoffset();
            }
            if(maxsize.count(arrayname) && (dyn_cast<LoadInst>(ins) || dyn_cast<StoreInst>(ins))){
                size = maxsize[arrayname];
            }
            else{
                size += (LSbounds[1] - LSbounds[0]);
                size += LSbounds[2];
            }
            //physical offset MUST >= 0
            if(0 > offset){
                size -= offset;
                reduceoffset = node->getLSstart();
                offset = 0;
            }else{
                if(offset % BITWIDTH == 0){
                    //offset = node->getLSoffset();
                    reduceoffset = node->getLSstart() - offset;
                }else{
                    size += offset%BITWIDTH;
                    offset = offset-offset%BITWIDTH;
                    reduceoffset = node->getLSstart() - offset;
                }
            }
            
            if(node->getLSstart() == "")///LS totally fixed
                reduceoffset = node->getLSoffset() - offset;
            // errs()<<arrayname<<" LSstart: "<<node->getLSstart().to_string()<<"\n";
            // errs()<<arrayname<<" node->getLSoffset(): "<<node->getLSoffset().to_string()<<"\n";
            // errs()<<arrayname<<" reduceoffset: "<<reduceoffset.to_string()<<"\n";
            ofs << ", size=\""<<size.to_string()<<"\"";
            ofs << ", offset=\"" << offset.to_string() <<", ";
            //assert(node->getLSstart()>=0);
            ofs <<reduceoffset.to_string()<<"\"";
            ofs << ", pattern=\"";
            auto stride = node->getLSstride();
            varType resetstride = 0;
            for(int i = 0; i < stride.size(); i++){
                //if(i == 0 || stride[i] == 0)
                //    ofs << stride[i] <<", " << getLoopsAffineCounts(i);
                //else
                ofs << (stride[i]-resetstride).to_string() <<", " << getLoopsAffineCounts(i).to_string();
                resetstride += (getLoopsAffineCounts(i)-1)*stride[i];
                if(i < stride.size()-1)
                    ofs << ", ";
            }
            ofs << "\"";
            ofs << ", multiport=" << node->getMultiPort();
            #ifdef MEM_PARTITION
            ofs << ", controlstep=" << node->getCtrlStep();
            if(!node->getMultiPort() == 0){
                auto BNPair = getMemPartition(arrayname);
                ofs << ", B=" << BNPair.first << ", N=" << BNPair.second;
            }
            #endif
        }else if(node->finalInstruction() == "ISEL"){
            auto patternmap = node->getAccPattern();
            ofs << ", isel_params=\"";
            ofs << patternmap["count"].to_string();
            ofs << ", " << patternmap["interval"].to_string();  
            ofs << ", " << patternmap["repeat"].to_string() <<"\"";
        }
        else if(ins != NULL){
            if(dyn_cast<LoadInst>(ins) || dyn_cast<StoreInst>(ins)){
                varType size = 0;
                varType offset;
                varType reduceoffset;
                varType cycle = 1;
                auto LSBounds = node->getLSbounds();
                std::string arrayname = node->getLSArrayName();
                if(isloopsAffine()){
                    auto LoopsCounts = getLoopsAffineCounts();
                    for(int level = 0; level < LoopsCounts.size(); level++){
                        cycle *= LoopsCounts[level];
                    }
                }else{
                    cycle = MAX_CYCLE;
                }
                if(minoffset.count(arrayname)){
                    ///TODO: if right?
                    if(node->getLSoffset().index() != 2){
                        int LSoffset = std::get<int>(node->getLSoffset().value);
                        if(LSoffset > minoffset[arrayname]){
                            offset = minoffset[arrayname];
                        }else{
                            offset = LSoffset;
                        }
                    }
                }else{
                    offset = node->getLSoffset();
                }
                if(maxsize.count(arrayname)){
                    size = maxsize[arrayname] - offset;
                }else{ 
                    size = (LSBounds[1]-LSBounds[0]+LSBounds[2]) - offset;
                }
                if((offset%BITWIDTH).index() == 0){
                    int remainder = std::get<int>((offset%BITWIDTH).value);
                    if(remainder == 0){
                        //offset = node->getLSoffset();
                        reduceoffset = 0;
                    }else{
                        size += remainder;
                        reduceoffset = remainder;
                        offset = offset - remainder;
                    }

                }else{
                    size += offset%BITWIDTH;
                    reduceoffset = offset%BITWIDTH;
                    offset = offset-offset%BITWIDTH;
                }
                ofs << ", ref_name=\"" << arrayname<<"\"";
                ofs << ", size=\""<<size.to_string()<<"\"";
                ofs << ", offset=\""<<offset.to_string()<<", "<<reduceoffset.to_string()<<"\"";
                ofs << ", cycles=\""<<cycle.to_string()<<"\"";
                if(node->getMultiPort() != -1){
                    ofs << ", multiport=\"" << node->getMultiPort() << "\"";;
                }
            }
        }
        ofs << "];\n";
        //ofs << ", shape = box, color = black];\n";
    }
	// edges
    std::map<std::pair<LLVMCDFGNode*, LLVMCDFGNode*>, int> edgestack;    
    for(auto &elem : _edges){
        auto edge = elem.second;
        auto srcName = edge->src()->getName();
        auto dstName = edge->dst()->getName();
        auto type = edge->type();
        ofs << srcName << " -> " << dstName;
        int opIdx = edge->dst()->getInputIdx(edge->src());
        auto pair = std::make_pair(edge->src(), edge->dst());
        if(edgestack.count(pair)){// Square operation
            opIdx = 1 - edgestack[pair];
        }else{
            edgestack[pair] = opIdx;
        }
        ofs << "[operand = " << opIdx;
        if(type == EDGE_TYPE_CTRL || type == EDGE_TYPE_TEND){
            ofs << ", Width = 1";
            ofs << " ,color = red";
        }else if(type == EDGE_TYPE_MEM){
            ofs << " ,color = blue";
        }else{
            ofs << ", Width = " << 32;
            ofs << " ,color = black";
        }
        bool isBackEdge = edge->src()->isOutputBackEdge(edge->dst());
        if(isBackEdge){
            ofs << ", style = dashed, constraint=false, backedge=1";
        }else{
            ofs << ", style = bold";
        }
        if(type == EDGE_TYPE_MEM){
            auto DepInfo = edge->dst()->getSrcDep(edge->src());
            if(edge->hasIterDist()){
                ofs << ", iterdist = " << edge->getIterDist();
            }else{
                if(DepInfo.isConstDist == true){
                    ofs << ", iterdist = " << DepInfo.distance;
                }else{
                    ofs << ", iterdist = 1";
                }
            }
            // if(DepInfo.isConstDist == true){
            //     ofs << ", iterdist = " << DepInfo.distance;
            // }else{
            //     ofs << ", iterdist = 1";
            // }
            ofs << ", type = ";
            if(DepInfo.distance == 0){
                ofs << "1"; //the dependence of the same address accesses in once iteration (need P&R)
            }else{
                ofs << "0"; //the loop dependent memory dependence (donot need P&R)
            }
        }
        ofs << ", label = \"Op=" << opIdx;
        if(type == EDGE_TYPE_MEM){
            auto DepInfo = edge->dst()->getSrcDep(edge->src());
            if(edge->hasIterDist()){
                ofs << ", iterdist = " << edge->getIterDist();
            }else{
                if(DepInfo.isConstDist == true){
                    ofs << ", iterdist = " << DepInfo.distance;
                }else{
                    ofs << ", iterdist = 1";
                }
            }
            // if(DepInfo.isConstDist == true){
            //     ofs << ", iterdist = " << DepInfo.distance;
            // }else{
            //     ofs << ", iterdist = 1";
            // }
            ofs << ", type = ";
            if(DepInfo.distance == 0){
                ofs << "1"; //the dependence of the same address accesses in once iteration (need P&R)
            }else{
                ofs << "0"; //the loop dependent memory dependence (donot need P&R)
            }
        }
        if(type == EDGE_TYPE_TEND){
            ofs << ", TaskEnd";
        }
        ofs << "\"];\n";
    }
	ofs << "}\n";
	ofs.close();
}

//print subgraph as a cluster in a WHOLE DFG
void LLVMCDFG::printAsSubTask(std::ofstream &ofs, int kernel) {
    outs() << "------print Sub Task kernel " << kernel << "-------\n";
    auto loops = _nestloops;
    std::string kernelStr = std::to_string(kernel);
    std::map<Value*, int> minoffset;
    std::map<Value*, int> maxsize;
    //aligent load (min) offset
    for(auto &elem : _nodes){
        auto node = elem.second;
        auto ins = node->instruction();
        if(ins != NULL){
            if(node->getLSoffset().index() == 0){
                int LSoffset = std::get<int>(node->getLSoffset().value);
                if(dyn_cast<LoadInst>(ins)){
                    auto arrayname = node->getLSArrayName();
                    auto addrValue = node->getLSaddress();
                    if(!minoffset.count(addrValue)){
                        minoffset[addrValue] = LSoffset;
                    }else if(minoffset[addrValue] > LSoffset){
                        minoffset[addrValue] = LSoffset;
                    }
                }
            }
        }
    }
    if(!getminOffsetMap().empty()){
        minoffset = getminOffsetMap();
    }
    //aligent load (max) size
    for(auto &elem : _nodes){
        auto node = elem.second;
        auto ins = node->instruction();
        if(ins != NULL){
            if(dyn_cast<LoadInst>(ins)){
                auto LoadBound = node->getLSbounds();
                auto arrayname = node->getLSArrayName();
                auto addrValue = node->getLSaddress();
                varType size = 0;
                // if(node->isLSaffine()){
                //     size += LoadBound[1] - LoadBound[0];
                //     size += LoadBound[2];
                // }else{
                size = LoadBound[1]-LoadBound[0]+LoadBound[2];
                if(size.index() == 2){
                    continue;
                }
                //}
                int sizeint = std::get<int>(size.value);
                int LSoffset = std::get<int>(node->getLSoffset().value);
                if(minoffset[addrValue] < LSoffset){
                    size += (LSoffset-minoffset[addrValue]);
                }
                if(!maxsize.count(addrValue)){
                    maxsize[addrValue] = sizeint;
                }else if(maxsize[addrValue] < sizeint){
                    maxsize[addrValue] = sizeint;
                }
            }
        }
    }

    if(!getmaxSizeMap().empty()){
        maxsize = getmaxSizeMap();
    }
    
    ofs << "subgraph cluster_" + kernelStr + " {\n";
    ofs << "label = \"subgraph_" + kernelStr + "\";\n";
    // nodes
	assert(_nodes.size() != 0);
    for(auto &elem : _nodes){
        auto node = elem.second;
        auto name = node->getName()+"_"+std::to_string(kernel);
        auto ins = node->instruction();
        ofs << name<<"[opcode=" << node->finalInstruction();
        if(node->getDependenceFlag() != -1){
            ofs << ", dependency = "<< node->getDependenceFlag();
        }
        if(node->isAcc()){
            auto patternmap = node->getAccPattern();
            ofs << ", acc_params=\""<< node->getInitial();
            ofs << ", " << patternmap["count"].to_string();
            ofs << ", " << patternmap["interval"].to_string();      
            ofs << ", " << patternmap["repeat"].to_string() <<"\"";
            ofs << ", acc_first=" << node->isAccFirst();
        }
        else if(node->hasConst()){
            ofs << ", label=\""<<name<< ", Const="<< node->constVal() << "\"";
            ofs << ", value="<<node->constVal();
        }else if(node->hasArgIn()){
            ofs << ", argNum=" << node->argNum();
        }
        else if(node->isLSaffine()){
            std::string arrayname = node->getLSArrayName();
            auto addrValue = node->getLSaddress();
            ofs << ", ref_name=\"" << arrayname<<"\"";
            // ofs << "opcode=";
            // if(dyn_cast<LoadInst>(ins))
            //     ofs << "input, ref_name=\"" << arrayname<<"\"";
            // else if(dyn_cast<StoreInst>(ins))
            //     ofs << "output, ref_name=\"" << arrayname<<"\"";
            auto LSbounds = node->getLSbounds();
            varType size = 0;
            varType size_all = 0;
            varType offset;
            varType reduceoffset;
            if(minoffset.count(addrValue) && (dyn_cast<LoadInst>(ins) || dyn_cast<StoreInst>(ins))){
                offset = minoffset[addrValue];
            }else{
                offset = node->getLSoffset();
            }
            size += (LSbounds[1] - LSbounds[0]);
            size += LSbounds[2];
            if(maxsize.count(addrValue) && (dyn_cast<LoadInst>(ins) || dyn_cast<StoreInst>(ins))){
                size_all = maxsize[addrValue];
            }else{
                size_all = size;
            }
            if(offset % BITWIDTH == 0){
                //offset = node->getLSoffset();
                reduceoffset = node->getLSstart() - offset;
            }else{
                size += offset%BITWIDTH;
                size_all += offset%BITWIDTH;
                offset = offset-offset%BITWIDTH;
                reduceoffset = node->getLSstart() - offset;
            }
            if(node->getLSstart() == "")///LS totally fixed
                reduceoffset = node->getLSoffset() - offset;
            // errs()<<arrayname<<" LSstart: "<<node->getLSstart().to_string()<<"\n";
            // errs()<<arrayname<<" node->getLSoffset(): "<<node->getLSoffset().to_string()<<"\n";
            // errs()<<arrayname<<" reduceoffset: "<<reduceoffset.to_string()<<"\n";
            ofs << ", size=\""<<size.to_string()<<"\"";
            ofs << ", size_all=\""<<size_all.to_string()<<"\"";
            ofs << ", offset=\"" << offset.to_string() <<", ";
            //assert(node->getLSstart()>=0);
            ofs <<reduceoffset.to_string()<<"\"";
            ofs << ", pattern=\"";
            auto stride = node->getLSstride();
            varType resetstride = 0;
            for(int i = 0; i < stride.size(); i++){
                //if(i == 0 || stride[i] == 0)
                //    ofs << stride[i] <<", " << getLoopsAffineCounts(i);
                //else
                ofs << (stride[i]-resetstride).to_string() <<", " << getLoopsAffineCounts(i).to_string();
                resetstride += (getLoopsAffineCounts(i)-1)*stride[i];
                if(i < stride.size()-1)
                    ofs << ", ";
            }
            ofs << "\"";
        }
        // else if(node->customInstruction() == "SELECT"){
        //     ofs << "opcode=sel";
        // }
        else if(node->finalInstruction() == "ISEL"){
            auto patternmap = node->getAccPattern();
            ofs << ", isel_params=\"";
            ofs << patternmap["count"].to_string();
            ofs << ", " << patternmap["interval"].to_string();      
            ofs << ", " << patternmap["repeat"].to_string() <<"\"";
        }
        else if(node->customInstruction() != ""){
            if(node->hasConst()){
                ofs << ", value=" << node->constVal();
            }
        }
        else if(ins != NULL){
            if(dyn_cast<LoadInst>(ins) || dyn_cast<StoreInst>(ins)){
                auto LSBounds = node->getLSbounds();
                auto LoopsCounts = getLoopsAffineCounts();
                varType size = 0;
                varType offset;
                varType reduceoffset;
                varType cycle = 1;
                std::string arrayname = node->getLSArrayName();
                auto addrValue = node->getLSaddress();
                for(int level = 0; level < LoopsCounts.size(); level++){
                    cycle *= LoopsCounts[level];
                }
                if(minoffset.count(addrValue)){
                    ///TODO: if right?
                    if(node->getLSoffset().index() != 2){
                        int LSoffset = std::get<int>(node->getLSoffset().value);
                        if(LSoffset > minoffset[addrValue]){
                            offset = minoffset[addrValue];
                        }else{
                            offset = LSoffset;
                        }
                    }
                }else{
                    offset = node->getLSoffset();
                }
                if(maxsize.count(addrValue)){
                    size = maxsize[addrValue] - offset;
                }else{ 
                    size = (LSBounds[1]-LSBounds[0]+LSBounds[2]) - offset;
                }
                if((offset%BITWIDTH).index() == 0){
                    int remainder = std::get<int>((offset%BITWIDTH).value);
                    if(remainder == 0){
                        //offset = node->getLSoffset();
                        reduceoffset = 0;
                    }else{
                        size += remainder;
                        reduceoffset = remainder;
                        offset = offset - remainder;
                    }

                }else{
                    size += offset%BITWIDTH;
                    reduceoffset = offset%BITWIDTH;
                    offset = offset-offset%BITWIDTH;
                }
                ofs << ", ref_name=\"" << arrayname<<"\"";
                ofs << ", size=\""<<size.to_string()<<"\"";
                ofs << ", offset=\""<<offset.to_string()<<", "<<reduceoffset.to_string()<<"\"";
                ofs << ", cycles=\""<<cycle.to_string()<<"\"";
            }
        }
        ofs << "];\n";
        //ofs << ", shape = box, color = black];\n";
    }
	// edges
    std::map<std::pair<LLVMCDFGNode*, LLVMCDFGNode*>, int> edgestack;    
    for(auto &elem : _edges){
        auto edge = elem.second;
        auto srcName = edge->src()->getName()+"_"+std::to_string(kernel);
        auto dstName = edge->dst()->getName()+"_"+std::to_string(kernel);
        auto type = edge->type();
        ofs << srcName << " -> " << dstName;
        int opIdx = edge->dst()->getInputIdx(edge->src());
        auto pair = std::make_pair(edge->src(), edge->dst());
        if(edgestack.count(pair)){// Square operation
            opIdx = 1 - edgestack[pair];
        }else{
            edgestack[pair] = opIdx;
        }
        ofs << "[operand = " << opIdx;
        if(type == EDGE_TYPE_CTRL){
            ofs << ", Width = 1";
            ofs << " ,color = red";
        }else if(type == EDGE_TYPE_MEM){
            ofs << " ,color = blue";
        }else{
            ofs << ", Width = " << 32;
            ofs << " ,color = black";
        }
        bool isBackEdge = edge->src()->isOutputBackEdge(edge->dst());
        if(isBackEdge){
            ofs << ", style = dashed, constraint=false, backedge=1";
        }else{
            ofs << ", style = bold";
        }
        ofs << ", label = \"Op=" << opIdx;
        if(type == EDGE_TYPE_MEM){
            ofs << ", iterdist = " << edge->dst()->getSrcDep(edge->src()).distance;
        }
        ofs << "\"];\n";
    }
	ofs << "}\n";
}

// generate CDFG
void LLVMCDFG::generateCDFG()
{
    outs() << "########################################################\n";
    outs() << "Generate CDFG Started\n";
    // initialized CDFG
    printDOT(std::to_string(printTimes) + "_" + _name + "_init.dot");

    auto nestloops = this->nestloops();
    errs() << nestloops.size() << "\n";

    LoopIdxAnalyze();

    // Connect control dependent node pairs among BBs
    outs() << ">>>>>> Connect control dependent node pairs among BBs\n";
    connectCtrlDepBBs();
    printDOT(std::to_string(printTimes) + "_" + _name + "_after_connectCtrlDepBBs.dot");

    // insert Control NOT node behind the node with FALSE_COND control output edge
    outs() << ">>>>>> Insert CTRLNOT node behind the node with FALSE_COND control output edge\n";
    insertCtrlNotNodes();
    printDOT(std::to_string(printTimes) + "_" + _name + "_after_insertCtrlNotNodes.dot");
    // transfer the multiple control predecessors (input nodes) into a inverted OR tree 
    // with the root connected to a node and leaves connected to control predecessors
    outs() << ">>>>>> Transfer multiple control predecessors (input nodes) into a inverted OR tree\n";
    createCtrlOrTree();
    printDOT(std::to_string(printTimes) + "_" + _name + "_after_createCtrlOrTree.dot");

    outs() << ">>>>>> Transfer PHINode to SELECT nodes\n";
    handlePHINodes(); 
    printDOT(std::to_string(printTimes) + "_" + _name + "_after_handlePHINodes.dot");

    simplifyCtrlLogic();

    outs() << ">>>>>> Transfer GEP node to MUL/ADD/Const tree\n";
    handleGEPNodes();
    printDOT(std::to_string(printTimes) + "_" + _name + "_after_handleGEPNodes.dot");

    if(!_noACC){
        transAddOpSelectTree();
        printDOT(std::to_string(printTimes) + "_" + _name + "_after_transAddOpSelectTree.dot");
    }

    outs() << ">>>>>> Handle select Node\n";
    handleSelectNodes(); 
    printDOT(std::to_string(printTimes) + "_" + _name + "_after_handleSelectNodes.dot");

    if(!_noACC){
        if(!_noMAC){
            outs() << ">>>>>> Generate MAC Node\n";
            genMACNodes(); 
        }
        //printDOT(std::to_string(printTimes) + "_" + _name + "_after_genMACNodes.dot");
    }

    accessAnalyze();

    if(!_noPattern && isloopsAffine()){
        outs() << ">>>>>> handle affine Access Node\n";
        handleAffineLSNodes(); 
        printDOT(std::to_string(printTimes) + "_" + _name + "_after_handleAffineLSNodes.dot");
    }

    if(!isloopsAffine()){
        outs() << ">>>>>> Add task level control\n";
        addTaskCTRL();
        //printDOT(std::to_string(printTimes) + "_" + _name + "_after_addTaskCTRL.dot");
    }

    outs() << ">>>>>> Remove redundant nodes at first time\n";
    removeRedundantNodes();

    outs() << ">>>>>> add mask AND node behind the Shl node with bytewidth less than MAX_DATA_BYTE\n";
    addMaskAndNodes();

    outs() << ">>>>>> handle conversion operations including trunc, zext, sext\n";
    handleConversionNodes();
    printDOT(std::to_string(printTimes) + "_" + _name + "_after_handleConversionNodes.dot");

    outs() << ">>>>>> fuse constant input node with the mul output node for acc node\n";
    fuseAccConstNode();
    printDOT(std::to_string(printTimes) + "_" + _name + "_after_fuseAccConstNode.dot");

    outs() << ">>>>>> simplify the control logic\n";
    simplifyCtrlLogic();
    printDOT(std::to_string(printTimes) + "_" + _name + "_after_simplifyCtrlLogic.dot");

    outs() << ">>>>>> Remove redundant nodes\n";
    removeRedundantNodes();
    printDOT(std::to_string(printTimes) + "_" + _name + "_after_removeRedundantNodes.dot");

    outs() << ">>>>>> add edge between two nodes that have memory dependence\n";
    addMemDepEdges();
    printDOT(std::to_string(printTimes) + "_" + _name + "_after_addMemDepEdges.dot");

    if(!_noPattern && isloopsAffine()){
        std::chrono::time_point<std::chrono::steady_clock> _start;
        _start = std::chrono::steady_clock::now();
        updateIterDist();
        auto end = std::chrono::steady_clock::now();
        auto runningTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(end-_start).count();
        std::cout << "IterDisttence Solving Time: " << runningTimeMs << " (ms)" << std::endl;
    }

    outs() << ">>>>>> traverse all conflict memory accesses and find out conflict partition\n";
    memPartition();

    outs() << ">>>>>> Assign final node name\n";
    assignFinalNodeName();
    printDOT(std::to_string(printTimes) + "_" + _name + "_after_assignFinalNodeName.dot");
    printAffineDOT("affine.dot");
    //printAffineDOT("affine_"+std::to_string(printTimes) + "_" + _name + ".dot");

    outs() << "Generate CDFG Ended\n";
    outs() << "########################################################\n";


}