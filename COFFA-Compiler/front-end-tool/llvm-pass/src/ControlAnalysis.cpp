#include "llvm_cdfg.h"

// return a map of basicblocks to their real control dependent (recursive) predecessors with the respective control value
std::map<BasicBlock*, std::set<std::pair<BasicBlock*, CondVal>>> LLVMCDFG::getCtrlDepPredBBs() {
    //to record exitingblock for each level
    std::map<BasicBlock*, int> exitingBlocks;
    for(auto iter : nestloops()){
       BasicBlock* exitingBB = iter.second->getExitingBlock();
       assert(exitingBB != NULL);
       exitingBlocks[exitingBB] = iter.first;
    }
	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> res;
    // BFS to find all the recursive predecessors except the out-of-loop and back-edge ones for each BB
	for(BasicBlock* BB : _allBBs){
        int temlevel;
        for(auto iter : _LoopexclusiveBBs){
            if(iter.second.count(BB)){
                temlevel = iter.first;
                break;
            }
        }
		std::queue<BasicBlock*> q;
		q.push(BB);
		std::map<BasicBlock*,std::set<CondVal>> visited;
		while(!q.empty()){
			BasicBlock* curr = q.front(); q.pop();
			for (auto it = pred_begin(curr); it != pred_end(curr); ++it){
				BasicBlock* predecessor = *it;
				if(_allBBs.find(predecessor) == _allBBs.end()){
					continue; // no need to care for out of loop BBs.
				}
				std::pair<const BasicBlock*,const BasicBlock*> bbPair = std::make_pair(predecessor, curr);
				if(std::find(_backEdgeBBPs.begin(), _backEdgeBBPs.end(), bbPair) != _backEdgeBBPs.end()){
					continue; // no need to traverse backedges;
				}
				CondVal cv;
				assert(predecessor->getTerminator());
				BranchInst* BRI = cast<BranchInst>(predecessor->getTerminator());
                // get control value
				if(!BRI->isConditional() || exitingBlocks.count(predecessor)){
					cv = UNCOND;
				}else{
					for (int i = 0; i < BRI->getNumSuccessors(); ++i) {
						if(BRI->getSuccessor(i) == curr){
							if(i==0){
								cv = TRUE_COND;
							}else if(i==1){
								cv = FALSE_COND;
							}else{
								assert(false);
							}
						}
					}
				}
				visited[predecessor].insert(cv);
				q.push(predecessor);
			}
		}

		outs() << "BasicBlock : " << BB->getName() << " :: CtrlDependentBBs = ";
		for(std::pair<BasicBlock*,std::set<CondVal>> pair : visited){
			BasicBlock* bb = pair.first;
			std::set<CondVal> brOuts = pair.second;
			outs() << bb->getName();
			if(brOuts.count(TRUE_COND)){
				res[BB].insert(std::make_pair(bb, TRUE_COND)); // TRUE control dependent predecessor BB
				outs() << "(TRUE),";
			}
			if(brOuts.count(FALSE_COND)){
				res[BB].insert(std::make_pair(bb, FALSE_COND)); // FALSE control dependent predecessor BB
				outs() << "(FALSE),";
			}
		}
		outs() << "\n";
	} // res[BB] = set<preBB, TRUE/FALSE_COND>

    // remove extra <preBB, TRUE/FALSE_COND>
	std::map<BasicBlock*, std::set<std::pair<BasicBlock*, CondVal>>> refinedRes;
	for(auto &pair : res){
		std::set<std::pair<BasicBlock*,CondVal>> tobeRemoved;
		BasicBlock* currBB = pair.first;
		outs() << "BasicBlock : " << currBB->getName() << " :: RefinedCtrlDependentBBs = ";
		for(auto &bbVal : pair.second){
			BasicBlock* depBB = bbVal.first;
			for(auto &p2 : res[depBB]){
				tobeRemoved.insert(p2);
			}
		}
        // if currBB and depBB have the same <preBB, TRUE/FALSE_COND>, remove from res[BB]
		for(auto &bbVal : pair.second){
			if(tobeRemoved.find(bbVal) == tobeRemoved.end()){
				outs() << bbVal.first->getName();
				outs() << ((bbVal.second == TRUE_COND)? "(TRUE)," : "(FALSE),");
				refinedRes[currBB].insert(bbVal);
			}
		}
		outs() << "\n";
	}

    // remove preBB with both TRUE_COND and FALSE_COND from refinedRes
	std::map<BasicBlock*,std::set<std::pair<BasicBlock*,CondVal>>> finalRes;
    for(auto &pair : refinedRes){
		BasicBlock* currBB = pair.first;
		outs() << "BasicBlock : " << currBB->getName() << " :: FinalCtrlDependentBBs = ";
		std::set<std::pair<BasicBlock*, CondVal>> bbValPairs = pair.second; // auto-sorted, first prority : BasicBlock*, second : CondVal
        assert(bbValPairs.size() > 0);
        bool changed = true;
        while(changed){
            changed = false;
            for(auto it = bbValPairs.begin(), ie = --bbValPairs.end(); it != ie;){
                auto bb1 = it->first;
                auto old_it = it;
                auto next_it = ++it;
                if(bb1 == next_it->first){
                    bbValPairs.erase(old_it, ++next_it); // remove TRUE_COND and FALSE_COND preBBs
                    if(finalRes.count(bb1)){ // already got the final control dependent BBs
                        for(auto bbp : finalRes[bb1]){
                            bbValPairs.insert(bbp);
                        }
                    }else if(refinedRes.count(bb1)){ // add control dependent BBs from refinedRes[bb1]
                        for(auto bbp : refinedRes[bb1]){
                            bbValPairs.insert(bbp);
                        }
                    }
                    changed = (bbValPairs.size() > 0); // if still have element, continue
                    break;
                }
            }
        }
        if(bbValPairs.size() > 0){
            finalRes[currBB] = bbValPairs;
            for(auto &bbVal: bbValPairs){
			    outs() << bbVal.first->getName();
			    outs() << ((bbVal.second == TRUE_COND)? "(TRUE)," : "(FALSE),");
		    }
        }
        outs() << "\n";
    }
    errs() << "safety loopboundMap\n";
    std::vector<BasicBlock*> safetyBBs;
    for(auto iter: _loopboundMap){
        int level = iter.first;
        errs() << "level " << level << ": \n";
        Loop* temloop = nestloops()[level];
        BasicBlock* header = temloop->getHeader();
        auto safetyCtrlBBs = finalRes[header];
        for(auto elem : safetyCtrlBBs){
            outs() << elem.first->getName();
			outs() << ((elem.second == TRUE_COND)? "(TRUE)," : "(FALSE),");
            _safetyMap[level].push_back(elem);
            safetyBBs.push_back(elem.first);
        }
        // finalRes[header].clear();
        errs() << "\n";
    }

    if(!_loopSafety){
        for(auto &iter : finalRes){
            auto parentBBs = iter.second;
            for(auto elem : parentBBs){
                if(std::find(safetyBBs.begin(), safetyBBs.end(), elem.first) != safetyBBs.end()){
                    iter.second.erase(elem);
                }
            }
        }
    }
	return finalRes;
}


// get the control dependent nodes in a BB, including unconditional BranchInst, StoreInst, OUTPUT
// only TRUE_COND, STORE can be performed
std::vector<LLVMCDFGNode*> LLVMCDFG::getCtrlDepNodes(BasicBlock *BB)
{
    std::vector<LLVMCDFGNode*> res;
    for(auto &elem : _nodes){
        auto node = elem.second;
        if(node->BB() != BB){
            continue;
        }
        if(Instruction *ins = node->instruction()){
            if(BranchInst *BRI = dyn_cast<BranchInst>(ins)){
                if(!BRI->isConditional()){
                    res.push_back(node);
                }
            }else if(dyn_cast<StoreInst>(ins)){
                res.push_back(node);
            }else if(dyn_cast<LoadInst>(ins)){
                res.push_back(node);
            }
        }
        ///TODO: maybe consider regOut here
        // else if(node->customInstruction() == "OUTPUT"){
        //     res.push_back(node);
        // }
    }
    return res;
}

// add CTRLAND each CTRL path's node to tem preBB
std::set<std::pair<LLVMCDFGNode *, CondVal>> LLVMCDFG::ANDctrlPathNodes(std::map<BasicBlock*, std::set<std::pair<BasicBlock*, CondVal>>> &CtrlDepPredBBs, BasicBlock* BB){
    
    if(_BBctrlDependentNode.count(BB)){
        return _BBctrlDependentNode[BB];
    }

    std::set<std::pair<LLVMCDFGNode *, CondVal>> result;
    result.clear();
    errs() << "temBB is " << BB->getName() << "\n";
    for(auto &ctrlDepPredBB : CtrlDepPredBBs[BB]){
        BasicBlock* preBB = ctrlDepPredBB.first;
        CondVal cond = ctrlDepPredBB.second;
		BranchInst* BRI = cast<BranchInst>(preBB->getTerminator());
        LLVMCDFGNode* BRNode = node(BRI);
        LLVMCDFGNode* preBRNode = BRNode->inputNodes()[0];
        std::set<std::pair<LLVMCDFGNode *, CondVal>> ANDNodes = ANDctrlPathNodes(CtrlDepPredBBs, preBB);
        if(ANDNodes.size() > 0){
            std::set<std::pair<LLVMCDFGNode *, CondVal>> finalANDNodes;
            for(auto &elem:ANDNodes){
                BasicBlock* sourceBB = elem.first->BB();
                CondVal sourceCond = elem.second;
                std::pair<llvm::BasicBlock *, CondVal> compPair;
                if(sourceCond == TRUE_COND){
                    compPair = std::make_pair(sourceBB, FALSE_COND);
                }else if(sourceCond == FALSE_COND){
                    compPair = std::make_pair(sourceBB, TRUE_COND);
                }else{
                    assert(false&&"why uncond?");
                }
                if(!CtrlDepPredBBs[BB].count(compPair)){
                    finalANDNodes.insert(elem);
                }
            }
            if(finalANDNodes.size() > 0){
                ///TOCHECK:is it right to let the CTRLAND belong to preBRNode->BB()??
                LLVMCDFGNode* CTRLAND = addNode("CTRLAND", preBRNode->BB());
                errs() << "newCTRLAND: " << CTRLAND->getName() << "\n";
                for(auto preBBCtrl : finalANDNodes){
                    CTRLAND->addInputNode(preBBCtrl.first, -1, false, preBBCtrl.second);
                    preBBCtrl.first->addOutputNode(CTRLAND, false, preBBCtrl.second);
                    addEdge(preBBCtrl.first, CTRLAND, EDGE_TYPE_CTRL);
                }
                CTRLAND->addInputNode(preBRNode, 0, false, cond);
                preBRNode->addOutputNode(CTRLAND, false, cond);
                addEdge(preBRNode, CTRLAND, EDGE_TYPE_CTRL);
                preBRNode = CTRLAND;
                cond = TRUE_COND;
            }
        }
        errs() << BB->getName() << "'s preNode is " << preBRNode->getName() << "\n";
        _BBctrlDependentNode[BB].insert(std::make_pair(preBRNode, cond));
    }
    result = _BBctrlDependentNode[BB];
    return result;
}

///find safety branch of variable Idx and modify dependent node
void LLVMCDFG::FindLoopIdxSafetyBr(int level, Value* bound){
    Loop* temloop = nestloops()[level];
    BasicBlock* header = temloop->getHeader();
    auto ctrlNodes = _BBctrlDependentNode[header];
    // errs() << "temHeader is " << header->getName() << "\n";
    // errs() << "temBound is "; bound->dump();
    //assert(!ctrlNodes.empty());///the outter most header may have no CtrlNodes
    ///TODO: is it safe? directly consider the ctrlNode as the safetycmp?
    if(!ctrlNodes.empty()){
        errs() << "ctrlNodes are not empty\n"; 
        _BBctrlDependentNode[header].clear();
        outs() << "Find LoopIdx Safety of level "<< level <<"\n";
    }
    
    // for(auto iter : ctrlNodes){
    //     bool findbound = false;
    //     Instruction* safetycmp =  iter.first->instruction();
    //     errs() << "ctrl node: " << iter.first->getName() << "from ins: ";
    //     assert(safetycmp != NULL);
    //     assert(dyn_cast<CmpInst>(safetycmp));
    //     safetycmp->dump();
    //     for(int i = 0; i < safetycmp->getNumOperands(); i++){
    //         if(bound == safetycmp->getOperand(i)){
    //             findbound = true;
    //             _safetyMap[level] = std::make_pair(dyn_cast<CmpInst>(safetycmp), iter.second);
    //             break;
    //         }
    //     }
    //     if(findbound){
    //         _BBctrlDependentNode[header].erase(iter);
    //         outs() << "Find LoopIdx Safety of level "<< level <<"\n";
    //     }
    // }

    // outs() << "header "<< header->getName() << "'s final ctrlNodes are ";
    // for(auto iter : _BBctrlDependentNode[header]){
    //     outs() << iter.first->getName() << ", " << iter.second << ";";
    // }
    // outs() << "\n";
}

// Connect control dependent node pairs among BBs
void LLVMCDFG::connectCtrlDepBBs(){
    // get real control dependent (recursive) predecessors with the respective control value
	std::map<BasicBlock*, std::set<std::pair<BasicBlock*, CondVal>>> CtrlDepPredBBs = getCtrlDepPredBBs();
	for(auto &BBelem : CtrlDepPredBBs){
		BasicBlock* currBB = BBelem.first;
        errs() << "current BB is " << currBB->getName() << "\n";
        //add CTRLAND each CTRL path's node to tem preBB & update _BBctrlDependentNode
        auto currBBctrlNodes = ANDctrlPathNodes(CtrlDepPredBBs, currBB);
	}
    // printDOT("beforeFindSafety.dot");
    for(auto iter: _loopboundMap){
        int level = iter.first;
        auto InitialIVValue = iter.second.first;
        auto FinalIVValue = iter.second.second;
        // FindLoopIdxSafetyBr(level, InitialIVValue);
        // FindLoopIdxSafetyBr(level, FinalIVValue);
    }

    for(auto &BBelem : CtrlDepPredBBs){
		BasicBlock* currBB = BBelem.first;
        errs() << "current BB is " << currBB->getName() << "\n";
		std::vector<LLVMCDFGNode*> ctrlDepNodes = getCtrlDepNodes(currBB);
        auto currBBctrlNodes = _BBctrlDependentNode[currBB];
        for(auto &elem:currBBctrlNodes){
            LLVMCDFGNode* preBRNode = elem.first;
            Instruction* preBRCMP = preBRNode->instruction();
            CondVal cond = elem.second;
            BasicBlock* preBB = preBRNode->BB();
            outs() << "ConnectBB :: From " << preBRNode->getName() << "(srcBB = " << preBB->getName() << ")" << ", To ";
			for(LLVMCDFGNode* succNode : ctrlDepNodes){
                int  idx = -1;
				outs() << succNode->getName() << ", ";
                //set condition port of CSTORE and CLOAD
                if(succNode->instruction() != NULL){
                    if(dyn_cast<StoreInst>(succNode->instruction())){
                        idx = 2;
                    }
                    if(dyn_cast<LoadInst>(succNode->instruction())){
                        idx = 1;
                    }
                }
                preBRNode->addOutputNode(succNode, false, cond);
                succNode->addInputNode(preBRNode, idx, false, cond); // donot care the index
                addEdge(preBRNode, succNode, EDGE_TYPE_CTRL);
			}
            outs() << "\n";
        }
	}
    //add complete condition of built-in select node
    for(auto &elem : _nodes){
        LLVMCDFGNode* node = elem.second;
        Instruction* ins = node->instruction();
        if(ins != NULL && dyn_cast<SelectInst>(ins)){
            node->setCustomInstruction("SELECT");
            LLVMCDFGNode* directCTRLNode = node->getInputPort(2);
            BasicBlock* selectBB = node->BB();
            auto currBBctrlNodes = _BBctrlDependentNode[selectBB];
            if(!currBBctrlNodes.empty()){
                LLVMCDFGNode* CTRLAND = addNode("CTRLAND", selectBB);
                delEdge(edge(directCTRLNode, node));
                node->addInputNode(CTRLAND, 2, false);
                CTRLAND->addOutputNode(node);
                addEdge(CTRLAND, node, EDGE_TYPE_CTRL);
                CTRLAND->addInputNode(directCTRLNode, 0, false, TRUE_COND);
                directCTRLNode->addOutputNode(CTRLAND);
                addEdge(directCTRLNode, CTRLAND, EDGE_TYPE_CTRL);
                for(auto toAnd : currBBctrlNodes){
                    LLVMCDFGNode* toAndNode = toAnd.first;
                    CondVal toAndCond = toAnd.second;
                    CTRLAND->addInputNode(toAndNode, -1, false, toAndCond);
                    toAndNode->addOutputNode(CTRLAND);
                    addEdge(toAndNode, CTRLAND, EDGE_TYPE_CTRL);
                }
            }
        }
    }
}


// insert Control NOT node behind the condition node of the Branch node
void LLVMCDFG::insertCtrlNotNodes()
{
    outs() << "insertCtrlNotNodes STARTED!\n";
    // for(auto &elem : _nodes){
    //     LLVMCDFGNode* node = elem.second;
    //     std::set<LLVMCDFGNode*> falseOutNodes;
    //     for(auto outNode : node->outputNodes()){
    //         if(node->getOutputCondVal(outNode) == FALSE_COND){
    //             falseOutNodes.insert(outNode);
    //         }
    //     }
        // if(falseOutNodes.empty()){
        //     continue;
        // }
        // // create CTRLNOT node
        // LLVMCDFGNode* notNode = addNode("CTRLNOT", node->BB());
		// outs() << "newNOTNode = " << notNode->getName() << "\n";
        // for(auto outNode : falseOutNodes){
        //     // add new connections
        //     bool isBackEdge = node->isOutputBackEdge(outNode);
        //     notNode->addOutputNode(outNode, isBackEdge, TRUE_COND);
        //     outNode->addInputNode(notNode, isBackEdge, TRUE_COND);
        //     addEdge(notNode, outNode, EDGE_TYPE_CTRL);
        //     // delete old connections
        //     node->delOutputNode(outNode);
        //     outNode->delInputNode(node);
        // }
    ///TODO: does here need to be _allBBs
    for(auto BB : _loopBBs){       
        BranchInst* BRI = cast<BranchInst>(BB->getTerminator());
        if(!BRI->isConditional()){
            continue;
        }
        Instruction *condIns = dyn_cast<Instruction>(BRI->getCondition()); // conditional predecessor
        LLVMCDFGNode *node = this->node(condIns);
        outs() << "insert CTRLNOT node behind " << node->getName() << ", ";
        // create CTRLNOT node
        LLVMCDFGNode* notNode = addNode("CTRLNOT", node->BB());
		outs() << "newNOTNode = " << notNode->getName() << "\n";  
        std::set<LLVMCDFGNode*> falseOutNodes;                
        for(auto outNode : node->outputNodes()){
            if(node->getOutputCondVal(outNode) == FALSE_COND){
                falseOutNodes.insert(outNode);
            }
        }
        for(auto outNode : falseOutNodes){
            bool isBackEdge = node->isOutputBackEdge(outNode); //  should be false
            // delete old connections
            node->delOutputNode(outNode);
            int idx = outNode->delInputNode(node);
            // add new connections           
            notNode->addOutputNode(outNode, isBackEdge, TRUE_COND);
            outNode->addInputNode(notNode, idx, isBackEdge, TRUE_COND);
            addEdge(notNode, outNode, EDGE_TYPE_CTRL);            
        }
        // delete old edges
        auto temp = node->outputEdges();
        for(auto eid : temp){
            auto outEdge = edge(eid);
             outs() << "outEdge NULL?(1 is empty)" << (outEdge == NULL) << "\n";
             outs() << "outEdge source: " << outEdge->src()->getName() << "\n"; //
            auto dstNode = outEdge->dst();
            if(falseOutNodes.count(dstNode)){
                delEdge(outEdge);
            }
        }
        node->addOutputNode(notNode, false, FALSE_COND);
        notNode->addInputNode(node, 0, false, FALSE_COND);
        addEdge(node, notNode, EDGE_TYPE_CTRL);
    }
///replace set of CTRLNodes by CTRLNOT
    for(auto &BB2preCTRLNODE : _BBctrlDependentNode){
        auto currBB = BB2preCTRLNODE.first;
        errs() << "current BB is: " << currBB->getName() << "\n";
        auto preCTRLs = BB2preCTRLNODE.second;
        for(auto &elem : preCTRLs){
            auto preCTRL = elem.first;
            CondVal cond = elem.second;
            errs() << "\ttem preCTRL is: "; 
            if(preCTRL->instruction()!=NULL)
                preCTRL->instruction()->dump();
            else{
                errs() << preCTRL->getName() << "\n";
            }
            if(cond == TRUE_COND){
                continue;
            }else if(cond == FALSE_COND){
                _BBctrlDependentNode[currBB].erase(std::make_pair(preCTRL, FALSE_COND));
                for(auto outputnode : preCTRL->outputNodes()){
                    if(outputnode->customInstruction() == "CTRLNOT"){
                        _BBctrlDependentNode[currBB].insert(std::make_pair(outputnode, TRUE_COND));
                        break;
                    }
                }
            }else{
                assert(0 && "what's wrong CTRLNOT");
            }
        }
    }
    outs() << "insertCtrlNotNodes ENDED!\n";
}


// transfer the multiple control predecessors (input nodes) into a inverted OR tree 
// with the root connected to a node and leaves connected to control predecessors
void LLVMCDFG::createCtrlOrTree() 
{
	outs() << "createCtrlOrTree STARTED!\n";
    std::map<LLVMCDFGNode*, std::set<LLVMCDFGNode*>> condParentsMap; // conditional parents Map
	for(auto &elem : _nodes){
        LLVMCDFGNode* node = elem.second;		
		for(LLVMCDFGNode* par : node->inputNodes()){
            auto cond = node->getInputCondVal(par);
            if(node->customInstruction() == "CTRLAND"){
                if(node->getInputIdx(par) == -1){
                    condParentsMap[node].insert(par);
                }
            }else{
                if( cond != UNCOND){
                    condParentsMap[node].insert(par);
                    // condParents.insert(par);
                }
            }
		}
    }
    for(auto &elem : condParentsMap){
        LLVMCDFGNode* node = elem.first;
        Instruction* ins = node->instruction();
        int idx = -1;
        if(ins != NULL){
            if(dyn_cast<LoadInst>(ins)){
                idx = 1;
            }else if(dyn_cast<StoreInst>(ins)){
                idx = 2;
            }
        }
        auto &condParents = elem.second; // conditional parents
        // create CTRLOR tree
		if(condParents.size() > 1){
			std::queue<LLVMCDFGNode*> q;
			for(auto pp : condParents){
				q.push(pp);
			}
			while(!q.empty()){
				auto pp1 = q.front(); q.pop();
				if(!q.empty()){
					auto pp2 = q.front(); q.pop();
					outs() << "Connecting pp1 = " << pp1->getName() << ", ";
					outs() << "pp2 = " << pp2->getName() << ", ";                   
                    // add CTRLOR node
                    LLVMCDFGNode* orNode = addNode("CTRLOR", node->BB());
					outs() << "newORNode = " << orNode->getName() << "\n";                    
					bool isPP1BackEdge = node->isInputBackEdge(pp1);
					bool isPP2BackEdge = node->isInputBackEdge(pp2);
                    assert(node->getInputCondVal(pp1) == TRUE_COND); // the FALSE_COND should be transformed to TRUE_COND before calling this function
                    assert(node->getInputCondVal(pp2) == TRUE_COND);
                    // delete old connections
                    pp1->delOutputNode(node);
                    node->delInputNode(pp1);
                    pp2->delOutputNode(node);
                    node->delInputNode(pp2);     
                    delEdge(edge(pp1, node));     
                    delEdge(edge(pp2, node));  
                    // add new connections
                    pp1->addOutputNode(orNode, isPP1BackEdge, TRUE_COND);
                    orNode->addInputNode(pp1, 0, isPP1BackEdge, TRUE_COND);
                    pp2->addOutputNode(orNode, isPP2BackEdge, TRUE_COND);
                    orNode->addInputNode(pp2, 1, isPP2BackEdge, TRUE_COND);
                    orNode->addOutputNode(node, false, TRUE_COND);
                    node->addInputNode(orNode, idx, false, TRUE_COND);
                    addEdge(pp1, orNode, EDGE_TYPE_CTRL);
                    addEdge(pp2, orNode, EDGE_TYPE_CTRL);
                    addEdge(orNode, node, EDGE_TYPE_CTRL);   
                            
					q.push(orNode);
				}else{
					outs() << "Alone node = " << pp1->getName() << "\n";
				}
			}
		}
	}
	outs() << "createCtrlOrTree ENDED!\n";
    for(auto &elem : _nodes){
        auto node = elem.second;
        if(node->customInstruction() == "CTRLOR"){
            BasicBlock* currBB = node->BB();
            errs() << "---come accross " << node->getName() << "current BB is: " << node->BB()->getName() << "\n";
            ///TODO: how about "Alone node"
            _BBctrlDependentNode[currBB].clear();
            _BBctrlDependentNode[currBB].insert(std::make_pair(node, TRUE_COND));
        }
        if(node->customInstruction() == "CTRLAND"){
            auto unsetOp = node->getInputPort(-1);
            node->setInputIdx(unsetOp, 1);
        }
    }
}

// simplify the control logic
// AND/OR node connected to more than one NOT nodes
void LLVMCDFG::simplifyCtrlLogic()
{
    bool flag = true;
    while(flag){ // scan interatively
        flag = false;
        auto nodes = _nodes;
        for (auto &elem : nodes)
        {
            auto node = elem.second;
            auto BB = node->BB();
            auto customIns = node->customInstruction();
            if(customIns == "SELECT"){//check out all SLELECT nodes with two same operand
                errs() << "come accross " << node->getName() << "\n";
                errs() << "port0: " << node->getInputPort(0)->getName() << " port1: " << node->getInputPort(1)->getName() << "\n";
                if(node->getInputPort(0) == node->getInputPort(1)){
                    LLVMCDFGNode* preNode = node->getInputPort(0);
                    auto succNodes = node->outputNodes();
                    for(auto succ:succNodes){
                        bool isbackedge = succ->isInputBackEdge(node);
                        int idx = succ->delInputNode(node);
                        auto edgetype = edge(node, succ)->type();
                        connectNodes(preNode, succ, idx, edgetype, isbackedge);
                    }
                    delNode(node);
                    flag = true;
                }
            }
            else if (customIns == "CTRLAND" || customIns == "CTRLOR")
            {
                if(node->outputNodes().empty()){
                    continue;
                }
                auto op0 = node->getInputPort(0);
                auto op1 = node->getInputPort(1);
                auto dst = node->outputNodes()[0];
                bool isNotOp0 = op0->customInstruction() == "CTRLNOT";
                bool isNotOp1 = op1->customInstruction() == "CTRLNOT";
                bool isNotDst = dst->customInstruction() == "CTRLNOT";
                if (isNotOp0 + isNotOp1 + isNotDst >= 2)
                {
                    if((isNotOp0 && op0->outputNodes().size() > 1) | (isNotOp1 && op1->outputNodes().size() > 1) | (node->outputNodes().size() > 1)){
                        continue;
                    }
                    if (customIns == "CTRLAND")
                    {
                        node->setCustomInstruction("CTRLOR");
                    }
                    else
                    {
                        node->setCustomInstruction("CTRLAND");
                    }
                    if (isNotOp0)
                    {
                        auto opop = op0->getInputPort(0);
                        // delete node and edges
                        delNode(op0);
                        opop->addOutputNode(node);
                        node->addInputNode(opop, 0);
                        addEdge(opop, node, EDGE_TYPE_CTRL);
                    }
                    else
                    {
                        op0->delOutputNode(node);
                        node->delInputNode(op0);
                        delEdge(edge(op0, node));
                        LLVMCDFGNode *notNode = addNode("CTRLNOT", BB);
                        op0->addOutputNode(notNode);
                        notNode->addInputNode(op0, 0);
                        notNode->addOutputNode(node);
                        node->addInputNode(notNode, 0);
                        addEdge(op0, notNode, EDGE_TYPE_CTRL);
                        addEdge(notNode, node, EDGE_TYPE_CTRL);
                    }
                    if (isNotOp1)
                    {
                        auto opop = op1->getInputPort(0);
                        // delete node and edges
                        delNode(op1);
                        opop->addOutputNode(node);
                        node->addInputNode(opop, 1);
                        addEdge(opop, node, EDGE_TYPE_CTRL);
                    }
                    else
                    {
                        op1->delOutputNode(node);
                        node->delInputNode(op1);
                        delEdge(edge(op1, node));
                        LLVMCDFGNode *notNode = addNode("CTRLNOT", BB);
                        op1->addOutputNode(notNode);
                        notNode->addInputNode(op1, 0);
                        notNode->addOutputNode(node);
                        node->addInputNode(notNode, 1);
                        addEdge(op1, notNode, EDGE_TYPE_CTRL);
                        addEdge(notNode, node, EDGE_TYPE_CTRL);
                    }
                    if (isNotDst)
                    {
                        auto dstdst = dst->outputNodes()[0];
                        // delete node and edges
                        int idx = dstdst->delInputNode(dst);
                        delNode(dst);
                        node->addOutputNode(dstdst);
                        dstdst->addInputNode(node, idx);
                        addEdge(node, dstdst, EDGE_TYPE_CTRL);
                    }
                    else
                    {
                        node->delOutputNode(dst);
                        int idx = dst->delInputNode(node);
                        delEdge(edge(node, dst));
                        LLVMCDFGNode *notNode = addNode("CTRLNOT", BB);
                        node->addOutputNode(notNode);
                        notNode->addInputNode(node, 0);
                        notNode->addOutputNode(dst);
                        dst->addInputNode(notNode, idx);
                        addEdge(node, notNode, EDGE_TYPE_CTRL);
                        addEdge(notNode, dst, EDGE_TYPE_CTRL);
                    }
                    flag = true;
                    break;
                }
            }
        }
    }
}

// add task level control
void LLVMCDFG::addTaskCTRL(){
    //Transform special operators
    std::set<LLVMCDFGNode*> unPatternNodes;
    std::map<int, LLVMCDFGNode*> trueLoopCTRLNodes;
    int totalLevel = _nestloops.size();
    trueLoopCTRLNodes[0] = _loop2exitNodesMap[0];
    for(int i = 1; i < totalLevel; i++){
        auto newCTRLAND = addNode("CTRLAND");
        connectNodes(trueLoopCTRLNodes[i-1], newCTRLAND, 0, EDGE_TYPE_CTRL);
        connectNodes(_loop2exitNodesMap[i], newCTRLAND, 1, EDGE_TYPE_CTRL);
        trueLoopCTRLNodes[i] = newCTRLAND;
    }
    //restore ADD of idx ACC-series when acc_first = 1
    if(!_noACC){
        for(auto &elem : PHItoLevel){
            Instruction* idxPhi = elem.first;
            LLVMCDFGNode* idxPhiNode = node(idxPhi);
            std::string cusIns = idxPhiNode->customInstruction();
            assert(cusIns == "ACC" || cusIns == "CACC" || cusIns == "CIACC" || cusIns == "CDIACC");
            if(idxPhiNode->isAccFirst()){
                auto addOpNode = idxPhiNode->getInputPort(0);
                auto ADDNode = addNode("ADD", idxPhiNode->BB());
                auto outputNodes = idxPhiNode->outputNodes();
                for(auto output : outputNodes){
                    int portidx = output->getInputIdx(idxPhiNode);
                    delEdge(edge(idxPhiNode, output));
                    connectNodes(ADDNode, output, portidx, EDGE_TYPE_DATA);
                }
                connectNodes(addOpNode, ADDNode, 0, EDGE_TYPE_DATA);
                connectNodes(idxPhiNode, ADDNode, 1, EDGE_TYPE_DATA);
                idxPhiNode->cleanAccFirst();
            }
        }
    }
    //indicate the END exitnode
    auto endExit = (*(trueLoopCTRLNodes.rbegin())).second;
    for(auto &elem:_nodes){
        auto node = elem.second;
        std::string cusIns = node->customInstruction();
        if(cusIns == "ACC" || cusIns == "CACC" || cusIns == "CIACC" || cusIns == "CDIACC" ||  cusIns == "InitSELECT"){
            unPatternNodes.insert(node);
            node->setAccPattern(0, 0, 0);
        }
    }
    for(auto node:unPatternNodes){
        std::string cusIns = node->customInstruction();
        auto cummuLevels = node->getCummuLevels();
        int initialLevel = cummuLevels.first;
        int iterateLevel = cummuLevels.second;
        assert(initialLevel>=iterateLevel);
        bool noEn = (iterateLevel == 0), noInit = (initialLevel == totalLevel-1);
        LLVMCDFGNode* enCond = NULL;
        LLVMCDFGNode* initCond = NULL;
        if(cusIns == "ACC"){
            if(!noEn){
                enCond = trueLoopCTRLNodes[initialLevel-1];
            }
            if(!noInit){
                initCond = trueLoopCTRLNodes[initialLevel];
            }
        }else if(cusIns == "CACC"){
            LLVMCDFGNode* oriEnNode = node->getInputPort(1);
            if(noEn){
                enCond = oriEnNode;
                noEn = false;
            }else{
                delEdge(edge(oriEnNode, node));
                enCond = trueLoopCTRLNodes[initialLevel-1];
                auto ctrlAND = addNode("CTRLAND", enCond->BB());
                connectNodes(enCond, ctrlAND, 0, EDGE_TYPE_CTRL);
                connectNodes(oriEnNode, ctrlAND, 1, EDGE_TYPE_CTRL);
                enCond = ctrlAND;
            }
            if(!noInit){
                initCond = trueLoopCTRLNodes[initialLevel];
            }
        }else if(cusIns == "CIACC"){
            LLVMCDFGNode* oriEnNode = node->getInputPort(1);
            LLVMCDFGNode* oriInitNode = node->getInputPort(2);
            if(noEn){
                enCond = oriEnNode;
                noEn = false;
            }else{
                delEdge(edge(oriEnNode, node));
                enCond = trueLoopCTRLNodes[initialLevel-1];
                auto ctrlAND = addNode("CTRLAND", enCond->BB());
                connectNodes(enCond, ctrlAND, 0, EDGE_TYPE_CTRL);
                connectNodes(oriEnNode, ctrlAND, 1, EDGE_TYPE_CTRL);
                enCond = ctrlAND;
            }
            if(noInit){
                initCond = oriInitNode;
                noInit = false;
            }else{
                delEdge(edge(oriInitNode, node));
                initCond = trueLoopCTRLNodes[initialLevel];
                auto ctrlAND = addNode("CTRLAND", enCond->BB());
                connectNodes(initCond, ctrlAND, 0, EDGE_TYPE_CTRL);
                connectNodes(oriInitNode, ctrlAND, 1, EDGE_TYPE_CTRL);
                initCond = ctrlAND;
            }
        }else if(cusIns == "InitSELECT"){
            if(!noEn){
                enCond = trueLoopCTRLNodes[initialLevel-1];
            }
            if(!noInit){
                initCond = trueLoopCTRLNodes[initialLevel];
            }
        }
        else if(cusIns == "CDIACC"){
            ///TODO: how to deal with CDIACC in this condition?
            continue;
        }
        //connect control signals & set the final instruction
        if(cusIns == "InitSELECT"){
            node->setCustomInstruction("CInitSELECT");
            if(!noEn){
                connectNodes(enCond, node, 2, EDGE_TYPE_CTRL, true);
            }else{
                auto ConstONE = addNode("CONST", node->BB());
                ConstONE->setConstVal(1);
                connectNodes(ConstONE, node, 2, EDGE_TYPE_CTRL);
            }
            if(!noInit){
                connectNodes(initCond, node, 3, EDGE_TYPE_CTRL, true);
            }else{
                auto ConstONE = addNode("CONST", node->BB());
                ConstONE->setConstVal(0);
                connectNodes(ConstONE, node, 3, EDGE_TYPE_CTRL);
            }
        }else{
            if(noEn && noInit){
                node->setCustomInstruction("ACC");
            }else if(noInit){
                node->setCustomInstruction("CACC");
                connectNodes(enCond, node, 1, EDGE_TYPE_CTRL, true);
            }else{
                node->setCustomInstruction("CIACC");
                if(!noEn){
                    connectNodes(enCond, node, 1, EDGE_TYPE_CTRL, true);
                }else{
                    auto ConstONE = addNode("CONST", node->BB());
                    ConstONE->setConstVal(1);
                    connectNodes(ConstONE, node, 1, EDGE_TYPE_CTRL);
                }
                connectNodes(initCond, node, 2, EDGE_TYPE_CTRL, true);
            }
        }
        //set as the MAX_CYCLE according to Hardware
        node->setAccPattern(MAX_CYCLE, 1, 1);
    }
    //Add task-level control ports to memory access nodes 
    for(auto loadNode:_loadList){
        // if(loadNode->customInstruction() == "CLOAD"){
        if(loadNode->getInputPort(1)!=NULL){
            loadNode->setCustomInstruction("TCLOAD");
            connectNodes(endExit, loadNode, 2, EDGE_TYPE_TEND, true);
        }
        else{//when task control is needed, memory accesses do not have patterns
            loadNode->setCustomInstruction("TLOAD");
            connectNodes(endExit, loadNode, 1, EDGE_TYPE_TEND, true);
        }
    }
    for(auto storeNode:_storeList){
        // if(storeNode->customInstruction() == "CSTORE"){
        if(storeNode->getInputPort(2)!=NULL){
            storeNode->setCustomInstruction("TCSTORE");
            connectNodes(endExit, storeNode, 3, EDGE_TYPE_TEND, true);
        }
        else{//when task control is needed, memory accesses do not have patterns
            storeNode->setCustomInstruction("TSTORE");
            connectNodes(endExit, storeNode, 2, EDGE_TYPE_TEND, true);
        }
    }

    printDOT(_name + "_after_addTaskCTRL.dot");

    auto toRemove = _loop2exitNodesMap;
    for(auto elem : toRemove){
        auto node = elem.second;
        auto cond = node->inputNodes()[0];
        auto outputs = node->outputNodes();
        for(auto outNode:outputs){
            int idx = outNode->getInputIdx(node);
            bool isbackEdge = outNode->isInputBackEdge(node);
            delEdge(edge(node, outNode));
            connectNodes(cond, outNode, idx, EDGE_TYPE_CTRL, isbackEdge);
        }
        delNode(node);
    }
}

