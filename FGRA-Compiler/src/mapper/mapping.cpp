
#include "mapper/mapping.h"

 //@yuan: find the colsest power-of-2 value of memory offset, and return their difference
int closestPowerOfTwo(int num){
    if(num == 0){
        return 0;
    }
    int highestBitPosition = 0;
    int absnum = abs(num);
    // std::cout << "power of 2 offset: " << absOffset << std::endl;
    while (absnum > 1) {
        absnum >>= 1;
        highestBitPosition++;
    }
    // the colosest power-of-w value of offset
    int closestPowerOfTwo = 1 << highestBitPosition;

    return closestPowerOfTwo;

 }
 //@yuan: find the colsest power-of-2 value of memory offset, and return their difference
int closestPowerOfTwoUpper(int num){
    if(num == 0){
        return 0;
    }
    int highestBitPosition = 0;
    int absnum = abs(num);
    // std::cout << "power of 2 offset: " << absOffset << std::endl;
    while (absnum > 1) {
        absnum >>= 1;
        highestBitPosition++;
    }
    // the colosest power-of-w value of offset
    int closestPowerOfTwo = 1 << highestBitPosition;

    if(closestPowerOfTwo == num){
        return closestPowerOfTwo;
    }

    return closestPowerOfTwo << 1;

 }
// reset mapping intermediate result and status
void Mapping::reset(){
    _dfgNodeAttr.clear();
    _dfgEdgeAttr.clear();
    // _dfgInputAttr.clear();
    // _dfgOutputAttr.clear();
    _adgNodeAttr.clear();
    // _adgLinkAttr.clear();
    _vioDfgEdges.clear();
    // _totalViolation = 0;
    _numNodeMapped = 0;
}


// if this input port of this ADG node is used
bool Mapping::isAdgNodeInPortUsed(int nodeId, int bits, int portIdx){
    if(_adgNodeAttr.count(nodeId)){
        auto& status = _adgNodeAttr[nodeId].inPortUsed;
        if(status.count(bits)){                        
            if(status[bits].count(portIdx)){
                return status[bits][portIdx];
            }
        }
    }
    return false;
}


// if this output port of this ADG node is used
bool Mapping::isAdgNodeOutPortUsed(int nodeId, int bits, int portIdx){
    if(_adgNodeAttr.count(nodeId)){
        auto& status = _adgNodeAttr[nodeId].outPortUsed;
        if(status.count(bits)){                        
            if(status[bits].count(portIdx)){
                return status[bits][portIdx];
            }
        }
    }
    return false;
}


// if the DFG node is already mapped
bool Mapping::isMapped(DFGNode* dfgNode){
    if(_dfgNodeAttr.count(dfgNode->id())){
        return _dfgNodeAttr[dfgNode->id()].adgNode != nullptr;
    }
    return false;
    
}

// if the ADG node is already mapped
bool Mapping::isMapped(ADGNode* adgNode, int type){
    int id = adgNode->id();
    if(_adgNodeAttr.count(id)){
        auto& dfgNodes = _adgNodeAttr[id].dfgNodes;
        if(dfgNodes.count(type)){
            return dfgNodes[type] != nullptr;
        }
    }
    return false;
}


// if the IOB node is available 
bool Mapping::isIOBFree(ADGNode* adgNode){
    // assert(adgNode->type() == "IOB");
    return !isMapped(adgNode, TYPE_IOC);
}


// // if the IOB node is already used as IB 
// bool Mapping::isUsedAsIB(ADGNode* adgNode){
//     // assert(adgNode->type() == "IOB");
//     int id = adgNode->id();
//     if(_adgNodeAttr.count(id)){
//         for(auto& elem : _adgNodeAttr[id].inPortUsed){
//             if(elem.second){ // used as IB
//                 return true;
//             }
//         }
//     }
//     return false;
// }

// // if the IOB node is already used as OB 
// bool Mapping::isUsedAsOB(ADGNode* adgNode){
//     // assert(adgNode->type() == "IOB");
//     int id = adgNode->id();
//     if(_adgNodeAttr.count(id)){
//         for(auto& elem : _adgNodeAttr[id].outPortUsed){
//             if(elem.second){ // used as OB
//                 return true;
//             }
//         }
//     }
//     return false;
// }

// // if the IB node is available 
// bool Mapping::isIBAvail(ADGNode* adgNode){
//     assert(adgNode->type() == "IB");
//     int id = adgNode->id();
//     if(_adgNodeAttr.count(id)){
//         auto& inPortUsed = _adgNodeAttr[id].inPortUsed;
//         for(int i = 0; i < adgNode->numInputs(); i++){
//             if(!inPortUsed.count(i)){
//                 return true;
//             }else if(!inPortUsed[i]){
//                 return true;
//             }
//         }
//         return false;
//     }
//     return true;
// }

// // if the OB node is available 
// bool Mapping::isOBAvail(ADGNode* adgNode){
//     assert(adgNode->type() == "OB");
//     int id = adgNode->id();
//     if(_adgNodeAttr.count(id)){
//         auto& outPortUsed = _adgNodeAttr[id].outPortUsed;
//         for(int i = 0; i < adgNode->numOutputs(); i++){
//             if(!outPortUsed.count(i) || !outPortUsed[i]){
//                 return true;
//             }
//         }
//         return false;
//     }
//     return true;
// }

// // if the DFG input port is mapped
// bool Mapping::isDfgInputMapped(int idx){
//     if(_dfgInputAttr.count(idx) && !_dfgInputAttr[idx].routedEdgeIds.empty()){
//         return true;
//     }
//     return false;
// }

// // if the DFG output port is mapped
// bool Mapping::isDfgOutputMapped(int idx){
//     if(_dfgOutputAttr.count(idx) && !_dfgOutputAttr[idx].routedEdgeIds.empty()){
//         return true;
//     }
//     return false;
// }

// occupied ADG node of this DFG node
ADGNode* Mapping::mappedNode(DFGNode* dfgNode){
    int dfgNodeId = dfgNode->id();
    if(_dfgNodeAttr.count(dfgNodeId)){
        return _dfgNodeAttr[dfgNodeId].adgNode;
    }else{
        return nullptr;
    } 
}


// mapped DFG node of this ADG node
DFGNode* Mapping::mappedNode(ADGNode* adgNode, int type){
    int adgNodeId = adgNode->id();
    if(_adgNodeAttr.count(adgNodeId)){
        auto& dfgNodes = _adgNodeAttr[adgNodeId].dfgNodes;
        if(dfgNodes.count(type)){
            return dfgNodes[type];
        }      
    }
    return nullptr;
}


// mapped DFG nodes of this ADG node
std::vector<DFGNode*> Mapping::mappedNode(ADGNode* adgNode){
    std::vector<DFGNode*> res;
    int adgNodeId = adgNode->id();
    if(_adgNodeAttr.count(adgNodeId)){
        auto& dfgNodes = _adgNodeAttr[adgNodeId].dfgNodes;
        for(auto& elem : dfgNodes){
            if(elem.second != nullptr){
                res.push_back(elem.second);
            }
        }     
    }
    return res;
}


// get mapped FU type of the DFG Node
int Mapping::mappedFUType(DFGNode* dfgNode){
    std::string op = dfgNode->operation();
    if(op == "INPUT" || op == "OUTPUT" || op == "LOAD" || op == "STORE" || op == "CINPUT" || op == "COUTPUT" || op == "CLOAD" || op == "CSTORE"
    || op == "TLOAD" || op == "TSTORE" || op == "TCLOAD" || op == "TCSTORE"){
        return TYPE_IOC;
    }else if(op == "LUT"){
        return TYPE_LUT;
    }else{
        return TYPE_ALU;
    }
}

// just map DFG node to ADG node without routing the input/output edges
bool Mapping::mapDfgNodeNoRoute(DFGNode* dfgNode, ADGNode* adgNode){
    int dfgNodeId = dfgNode->id();
    // if(_dfgNodeAttr.count(dfgNodeId)){
    //     if(_dfgNodeAttr[dfgNodeId].adgNode == adgNode){
    //         return true;
    //     } else if(_dfgNodeAttr[dfgNodeId].adgNode != nullptr){
    //         return false;
    //     }
    // } 
    _dfgNodeAttr[dfgNodeId].adgNode = adgNode;
    // constant are only processed in configuration stage since it does not affect mapping
    // one operand is immediate and the operands are not commutative
    // if(dfgNode->hasImm() && !dfgNode->commutative()){ // assign constant port
    //     GPENode* gpeNode = dynamic_cast<GPENode*>(adgNode);
    //     for(auto inPort : gpeNode->operandInputs(dfgNode->immIdx())){
    //         if(adgAttr.inPortUsed.count(inPort) && adgAttr.inPortUsed[inPort] == true){ // already used
    //             return false;
    //         }
    //         adgAttr.inPortUsed[inPort] = true; // set all the input ports connetced to this operand as used
    //     }
    // }
    int type = mappedFUType(dfgNode);
    _adgNodeAttr[adgNode->id()].dfgNodes[type] = dfgNode;
    // adgNode->addUsage(dfgNodeId);
    _numNodeMapped++;
    if(_dfg->isMultiportIoNode(dfgNodeId)){
        DFGIONode *ionode = dynamic_cast<DFGIONode*>(dfgNode);
        auto name = ionode->memRefName();
        _multportIobs[name].insert(adgNode->id());
    }
    return true;
}


// map DFG node to ADG node and route the input/output edges connected to already-mapped DFG nodes
// map the connected input/output nodes alongside

bool Mapping::mapDfgNode(DFGNode* dfgNode, ADGNode* targetAdgNode){
    bool succeed = true;
    // if(_dfg->isIONode(dfgNode->id())){ // pre-map IO node to prevent duplicate mapping during mapping the connected IO nodes
    //     succeed = mapDfgNodeNoRoute(dfgNode, targetAdgNode);
    // }
    // assert(succeed);
    std::vector<DFGEdge*> routedEdges;
    // std::vector<DFGNode*> ioNodes; // newly mapped IO nodes
    // std::vector<DFGEdge*> inEdges;  // edges connected to DFG input node
    // std::vector<DFGEdge*> outEdges; // edges connected to DFG output node
    // route the src edges whose src nodes have been mapped or connected to DFG input node
    for(auto& elem : dfgNode->inputEdges()){
        for(auto& edgePair : elem.second){
            DFGEdge* edge = _dfg->edge(edgePair.second);
            if(edge->isMemEdge()) continue; // memory dependent edge doesn't need to route
            int inNodeId = edge->srcId();
            DFGNode* inNode = _dfg->node(inNodeId);
            ADGNode* adgNode = mappedNode(inNode);
            bool routed = false;
            if(adgNode){
                if((mappedFUType(dfgNode) != mappedFUType(inNode)) && (adgNode == targetAdgNode)){
                    succeed = routeDfgEdge(edge, targetAdgNode); // route edge between targetAdgNode and adgNode
                    spdlog::info("Using internal link between ALU and LUT of ADG Node{0}", adgNode->name());
                    // std::cout << "Using GPE internal link for: " << inNode->name() << " to " << dfgNode->name() << std::endl;
                }else{
                    succeed = routeDfgEdge(edge, adgNode, targetAdgNode); // route edge between targetAdgNode and adgNode  
                }     
                routed = true;          
        // }else if(_dfg->isIONode(inNodeId)){ // connected to DFG input node which has not been placed
        //     succeed = routeDfgEdge(edge, targetAdgNode, true); // route edge between targetAdgNode and ADG IOB
        //     ioNodes.push_back(inNode); // cache newly mapped IO nodes
        //     // inEdges.push_back(edge);
        //     routed = true; 
            }
            if(!succeed){
                break;
            }else if(routed){
                routedEdges.push_back(edge); // cache the routed edge
            }
        }   
        if(!succeed) break; 
    }
    if(succeed){
        // route the dst edge whose dst nodes have been mapped or connected to DFG output port
        for(auto& elem : dfgNode->outputEdges()){
            for(auto& bitEdges : elem.second){
                for(auto& outEdgeId : bitEdges.second){
                    DFGEdge* edge = _dfg->edge(outEdgeId);
                    if(edge->isMemEdge()) continue; // memory dependent edge doesn't need to route
                    int outNodeId = edge->dstId();
                    DFGNode* outNode = _dfg->node(outNodeId);
                    ADGNode* adgNode = mappedNode(outNode);
                    bool routed = false;
                    if(adgNode){
                        if((mappedFUType(dfgNode) != mappedFUType(outNode)) && (adgNode == targetAdgNode)){
                            spdlog::info("Using internal link between ALU and LUT of ADG Node{0}", adgNode->name());
                            succeed = routeDfgEdge(edge, targetAdgNode); // route edge between targetAdgNode and adgNode
                            // std::cout << "Using GPE internal link for: " << dfgNode->name() << " to " << outNode->name() << std::endl;
                        }else{
                            succeed = routeDfgEdge(edge, targetAdgNode, adgNode); // route edge between targetAdgNode and adgNode  
                        }
                        routed = true;                      
                    // }else if(_dfg->isIONode(outNodeId)){ // connected to DFG output node which has not been placed
                    //     succeed = routeDfgEdge(edge, targetAdgNode, false); // route edge between targetAdgNode and ADG IOB
                    //     ioNodes.push_back(outNode); // cache newly mapped IO nodes
                    //     // outEdges.push_back(edge);
                    //     routed = true; 
                    }
                    if(!succeed){
                        break;
                    }else if(routed){
                        routedEdges.push_back(edge); // cache the routed edge
                    }
                } 
                if(!succeed) break;  
                }
                if(!succeed) break; 
            }
        }
    if(!succeed){
        for(auto re : routedEdges){ // unroute all the routed edges
            unrouteDfgEdge(re);
        }
        return false;
    }
    // if(succeed && !_dfg->isIONode(dfgNode->id())){
    // map the DFG node to this targetAdgNode
    bool res = mapDfgNodeNoRoute(dfgNode, targetAdgNode);
    assert(res);
    // }
    
    return succeed;
}


/*bool Mapping::mapDfgNode(DFGNode* dfgNode, ADGNode* targetAdgNode){
    bool succeed = true;
    std::vector<DFGEdge*> routedEdges;
    std::vector<DFGEdge*> inEdges;  // edges connected to DFG input node
    std::vector<DFGEdge*> outEdges; // edges connected to DFG output node
    // route the src edges whose src nodes have been mapped or connected to DFG input node
    for(auto& elem : dfgNode->inputEdges()){ 
        for(auto& edgePair : elem.second){// for each bitwidth, there may have multiple edges.
            DFGEdge* edge = _dfg->edge(edgePair.second);
            DFGNode* inNode = _dfg->node(edge->srcId());
            ADGNode* adgNode = mappedNode(inNode);
            bool routed = false;
            if(adgNode){
                succeed = routeDfgEdge(edge, adgNode, targetAdgNode); // route edge between targetAdgNode and the mapped adgNode       
                routed = true;          
            }else if(_dfg->isIONode(edge->srcId())){ // connected to DFG input node which has not been placed
                succeed = routeDfgEdge(edge, targetAdgNode, true); // route edge between targetAdgNode and ADG IOB
                inEdges.push_back(edge);
                routed = true; 
            }
            if(!succeed){
                break;
            }else if(routed){
                routedEdges.push_back(edge); // cache the routed edge
            }
        }
        if(!succeed) break;
    }
    if(succeed){
        // route the dst edge whose dst nodes have been mapped or connected to DFG output port
        for(auto& elem : dfgNode->outputEdges()){
            for(auto& bitEdges : elem.second){
                for(int outEdgeId : bitEdges.second){
                    DFGEdge* edge = _dfg->edge(outEdgeId);
                    DFGNode* outNode = _dfg->node(edge->dstId());
                    ADGNode* adgNode = mappedNode(outNode);
                    bool routed = false;
                    if(adgNode){
                        succeed = routeDfgEdge(edge, targetAdgNode, adgNode); // route edge between targetAdgNode and adgNode  
                        routed = true;                      
                    }else if(_dfg->isIONode(edge->dstId())){ // connected to DFG output node which has not been placed
                        succeed = routeDfgEdge(edge, targetAdgNode, false); // route edge between targetAdgNode and ADG IOB
                        outEdges.push_back(edge);
                        routed = true; 
                    }
                    if(!succeed){
                        break;
                    }else if(routed){
                        routedEdges.push_back(edge); // cache the routed edge
                    }
                } 
                if(!succeed) break;  
            }  
            if(!succeed) break;   
        }
    }
    if(!succeed){
        for(auto re : routedEdges){ // unroute all the routed edges
            unrouteDfgEdge(re);
        }
        return false;
    }
    // map the DFG node to this targetAdgNode
    bool res = mapDfgNodeNoRoute(dfgNode, targetAdgNode);
    spdlog::debug("Map DFG node {0} to ADG node {1}", dfgNode->name(), targetAdgNode->name()); 
    // map DFG input and output nodes
    //@yuan: the input and output nodes are not bonded with the ADGNode while routing
    for(auto& edge : inEdges){
        DFGNode* inDfgNode = _dfg->node(edge->srcId());
        auto inLinkAttr = dfgEdgeAttr(edge->id()).edgeLinks.begin(); // the link across the IOB
        ADGNode* inAdgNode = inLinkAttr->adgNode;
        res &= mapDfgNodeNoRoute(inDfgNode, inAdgNode);
        spdlog::debug("Map DFG node {0} to ADG node {1}", inDfgNode->name(), inAdgNode->name()); 
    }
    for(auto& edge : outEdges){
        DFGNode* outDfgNode = _dfg->node(edge->dstId());
        auto outLinkAttr = dfgEdgeAttr(edge->id()).edgeLinks.rbegin(); // the link across the IOB
        ADGNode* outAdgNode = outLinkAttr->adgNode;
        res &= mapDfgNodeNoRoute(outDfgNode, outAdgNode);
        spdlog::debug("Map DFG node {0} to ADG node {1}", outDfgNode->name(), outAdgNode->name()); 
    }
    assert(res);
    return true;
}*/


// unmap DFG node without unrouting the input/output edges
void Mapping::unmapDfgNodeNoUnroute(DFGNode* dfgNode){
    ADGNode* adgNode = mappedNode(dfgNode);
    if(adgNode == nullptr) return;
    int adgNodeId = adgNode->id();
    int type = mappedFUType(dfgNode);
    _adgNodeAttr[adgNodeId].dfgNodes.erase(type);
    if(_adgNodeAttr[adgNodeId].dfgNodes.empty()){
        _adgNodeAttr.erase(adgNodeId);
    }    
    _dfgNodeAttr.erase(dfgNode->id());
    _numNodeMapped--;
    spdlog::debug("Unmap DFG node {0} from ADG node {1}", dfgNode->name(), adgNode->name()); 
    if(_dfg->isMultiportIoNode(dfgNode->id())){
        DFGIONode *ionode = dynamic_cast<DFGIONode*>(dfgNode);
        auto name = ionode->memRefName();
        _multportIobs[name].erase(adgNode->id());
    }
}

// unmap DFG node, unroute its input/output edges and unmap the to-be-free input/output nodes if any
void Mapping::unmapDfgNode(DFGNode* dfgNode){
    std::vector<DFGNode*> unmapInNodes;
    // unroute the input edges of this node
    for(auto& elem : dfgNode->inputEdges()){ // bit-width, input-idx, edge-id
        for(auto& edges : elem.second){      // input-idx, edge-id
            DFGEdge* edge = _dfg->edge(edges.second);
            unrouteDfgEdge(edge);  
            /*int inNodeId = edge->srcId();      
            DFGNode* inNode = _dfg->node(inNodeId);    
            if(_dfg->isIONode(inNodeId) && isMapped(inNode)){ // connected to mapped DFG input node
                unmapInNodes.push_back(inNode);
            }*/
        }
    }
    // unroute the output edges of this node
    for(auto& elem : dfgNode->outputEdges()){ // bit-width, output-idx, edge-ids
        for(auto& edges : elem.second){       // output-idx, edge-ids
            for(auto& outEdgeId : edges.second){   // edge-id
                DFGEdge* edge = _dfg->edge(outEdgeId);
                unrouteDfgEdge(edge);  
                /*int outNodeId = edge->dstId();          
                if(_dfg->isIONode(outNodeId)){ // connected to DFG output node
                    unmapDfgNodeNoUnroute(_dfg->node(outNodeId));
                }*/
            }
        }
    }
    // unmap this DFG node
    unmapDfgNodeNoUnroute(dfgNode);
    // unmap the connected input nodes
    /*for(auto& inNode : unmapInNodes){
        bool mapped = false;
        for(auto& elem : inNode->outputs()){
            for(auto& bitOuts : elem.second){
                for(auto& out : bitOuts.second){
                    if(isMapped(_dfg->node(out.first))){ // there are still routed edges connected to the dfg input node
                        mapped = true;
                        break;
                    }
                }
                if(mapped) break;
            }
            if(mapped) break;
        }
        if(!mapped){
            unmapDfgNodeNoUnroute(inNode);
        }
    }*/
}


// // if the ADG link is already routed
// bool Mapping::isRouted(ADGLink* link){
//     return !_adgLinkAttr[link->id()].dfgEdges.empty();
// }

// routed DFG edge links, including inter-node and intra-node links
const std::vector<EdgeLinkAttr>& Mapping::routedEdgeLinks(DFGEdge* edge){
    assert(_dfgEdgeAttr.count(edge->id()));
    return _dfgEdgeAttr[edge->id()].edgeLinks;
}

// /* route DFG edge to ADG link
// *  one link can route multiple edges, but they should have the same srcId and srcPortIdx 
// */
// bool Mapping::routeDfgEdge(DFGEdge* edge, ADGLink* link){
//     if(_adgLinkAttr.count(link->id())){
//         for(auto& linkEdge : _adgLinkAttr[link->id()].dfgEdges){
//             if(linkEdge->srcId() != edge->srcId() || linkEdge->srcPortIdx() != edge->srcPortIdx()){
//                 return false; 
//             }
//         }
//     }
//     _adgLinkAttr[link->id()].dfgEdges.push_back(edge);
//     EdgeLinkAttr attr;
//     attr.isLink = true;
//     attr.edgeLink.adgLink = link;
//     _dfgEdgeAttr[edge->id()].edgeLinks.push_back(attr);
//     return true;
// }


// route DFG edge to passthrough ADG node, only GIB 
// one internal link can passthrough multiple edges, but they should have the same srcId and srcPortIdx
// isTry: just try to route, not change the routing status
bool Mapping::routeDfgEdgePass(DFGEdge* edge, ADGNode* passNode, int srcPort, int dstPort, bool isTry){
    assert(passNode->type() == "GIB");
    GIBNode* gibNode = dynamic_cast<GIBNode*>(passNode);
    int passNodeId = passNode->id();
    int routeSrcPort = srcPort;
    int routeDstPort = dstPort;
    int bits = edge->bitWidth();
    bool hasSameSrcEdge = false;
    if(_adgNodeAttr.count(passNodeId)){
        auto& passNodeAttr = _adgNodeAttr[passNodeId];
        // check if conflict with current routed edges
        for(auto& edgeLink : passNodeAttr.dfgEdgePass){
            auto passEdge = edgeLink.edge;
            if((edgeLink.srcPort == srcPort || edgeLink.dstPort == dstPort) &&  // occupy the same port
               (passEdge->srcId() != edge->srcId() || passEdge->srcPortIdx() != edge->srcPortIdx())){
                return false; 
            } else if(edgeLink.srcPort == srcPort || edgeLink.dstPort == dstPort){ // try to route same-source edge to same internal link
                routeSrcPort = edgeLink.srcPort; // default route link
                routeDstPort = edgeLink.dstPort;
                hasSameSrcEdge = true;
            } 
        }
        if(srcPort >= 0 && dstPort >= 0){ // manually assign srcPort and dstPort
            if(!gibNode->isInOutConnected(srcPort, dstPort)){
                return false;
            } else if(isAdgNodeOutPortUsed(passNodeId, bits, dstPort) && (!hasSameSrcEdge || (routeSrcPort != srcPort) || (routeDstPort != dstPort))){
                return false; // if dstPort used, must have same-source edge with same srcPort and dstPort
            } else if(isAdgNodeInPortUsed(passNodeId, bits, srcPort) && (!hasSameSrcEdge || (routeSrcPort != srcPort))){
                return false; // if srcPort used, must have same-source edge with same srcPort
            }   
            routeSrcPort = srcPort;
            routeDstPort = dstPort;         
        } else if(srcPort >= 0){ // auto-assign dstPort           
            if(isAdgNodeInPortUsed(passNodeId, bits, srcPort) && (!hasSameSrcEdge || (routeSrcPort != srcPort))){ 
                return false; // no same-source edge or have different srcPort
            }
            if(!isAdgNodeInPortUsed(passNodeId, bits, srcPort)){                    
                bool flag = false;
                for(auto port : gibNode->in2outs(srcPort)){
                    if(isAdgNodeOutPortUsed(passNodeId, bits, port)){ // already used
                        continue;
                    }
                    // find one available port
                    routeSrcPort = srcPort;
                    routeDstPort = port;
                    flag = true;
                    break;
                }
                if(!flag){ // cannot find one available port
                    return false;
                }
            }           
            // if have same-source edge and same srcPort, select the same dstPort            
        } else if(dstPort >= 0){ // auto-assign srcPort            
            if(isAdgNodeOutPortUsed(passNodeId, bits, dstPort) && (!hasSameSrcEdge || (routeDstPort != dstPort))){
                return false;
            }
            if(!isAdgNodeOutPortUsed(passNodeId, bits, dstPort)){ // no same-source edge or have different dstPort
                bool flag = false;
                for(auto port : gibNode->out2ins(dstPort)){
                    if(isAdgNodeInPortUsed(passNodeId, bits, port)){ // already used
                        continue;
                    }
                    // find one available port
                    routeSrcPort = port;
                    routeDstPort = dstPort;
                    flag = true;
                    break;                    
                }
                if(!flag){ // cannot find one available port
                    return false;
                }                
            }            
            // if have same-source edge with same dstPort, select the same srcPort and dstPort          
        } else { // auto-assign srcPort and dstPort
            if(!hasSameSrcEdge){
                bool outflag = false;
                for(auto& elem : passNode->outputs(bits)){
                    int outPort = elem.first;
                    if(isAdgNodeOutPortUsed(passNodeId, bits, outPort)){ // already used
                        continue;
                    }
                    bool inflag = false;
                    for(auto inPort : gibNode->out2ins(outPort)){
                        if(isAdgNodeInPortUsed(passNodeId, bits, inPort)){ // already used
                            continue;
                        }
                        // find one available inport
                        routeSrcPort = inPort;
                        routeDstPort = outPort;
                        inflag = true;
                        break;                        
                    }
                    if(inflag){ // find one available port
                        outflag = true;
                        break;
                    }
                }
                if(!outflag){ // cannot find one available port
                    return false;
                }
            }
        }
    } else { // _adgNodeAttr.count(passNodeId) = 0; this passNode has not been used
        bool outflag = false;
        for(auto& elem : passNode->outputs(bits)){
            int outPort = elem.first;
            auto inPorts = gibNode->out2ins(outPort);
            if(!inPorts.empty()){ // find one available inport
                routeSrcPort = *(inPorts.begin());
                routeDstPort = outPort;
                outflag = true;
                break;
            }
        }
        if(!outflag){ // cannot find one available port
            return false;
        }
    }

    if(!isTry){
        EdgeLinkAttr edgeLinkAttr;    
        edgeLinkAttr.srcPort = routeSrcPort;
        edgeLinkAttr.dstPort = routeDstPort;
        edgeLinkAttr.adgNode = passNode;
        _dfgEdgeAttr[edge->id()].edgeLinks.push_back(edgeLinkAttr);
        DfgEdgePassAttr edgePassAttr;
        edgePassAttr.edge = edge;
        edgePassAttr.srcPort = routeSrcPort;
        edgePassAttr.dstPort = routeDstPort;
        _adgNodeAttr[passNodeId].inPortUsed[bits][routeSrcPort] = true;
        _adgNodeAttr[passNodeId].outPortUsed[bits][routeDstPort] = true;
        _adgNodeAttr[passNodeId].dfgEdgePass.push_back(edgePassAttr);        
    }
    return true;
}


// route DFG edge between srcNode and dstNode/OB
// find a routable path from srcNode to dstNode/OB by BFS
// dstNode = nullptr: arbitrary OB
// dstPortRange: the input port index range of the dstNode
bool Mapping::routeDfgEdgeFromSrc(DFGEdge* edge, ADGNode* srcNode, ADGNode* dstNode, const std::set<int>& dstPortRange){
    struct VisitNodeInfo{
        // int inPortIdx;  // input port index of this node 
        int srcNodeId;  // src node ID
        int srcInPortIdx; // input port index of src node
        int srcOutPortIdx; // output port index of src node
    };
    // cache visited node information, <<node-id, inport-index>, VisitNodeInfo>
    std::map<std::pair<int, int>, VisitNodeInfo> visitNodes; 
    // cache visited nodes, <node, inport-index>
    std::queue<std::pair<ADGNode*, int>> nodeQue; 
    // Breadth first search for possible routing path
    // assign the index of the output port of the srcNode
    int srcNodePortIdx = edge->srcPortIdx();
    int srcNodeId = edge->srcId();
    int bits = edge->bitWidth();
    std::string srcNodeOp = _dfg->node(srcNodeId)->operation();
    //@yuan: for the GPE has alu and lut fine-grain outputs, we should obtain the correct srcportIdx here
    if(srcNodeOp == "LUT"){
        srcNodePortIdx = srcNode->outputs(1).size() - 1;
    }
    // int dstNodePortIdx;
    // int mappedAdgOutPort;
    std::pair<int, int> finalDstNode; // <node-id, inport-index>
    bool success = false;
    nodeQue.push(std::make_pair(srcNode, -1));
    while(!nodeQue.empty()){
        auto queHead = nodeQue.front();
        ADGNode* adgNode = queHead.first;
        int inPortIdx = queHead.second;
        nodeQue.pop();
        VisitNodeInfo info;
        std::vector<int> outPortIdxs;
        if(inPortIdx == -1){ // srcNode
            outPortIdxs.push_back(srcNodePortIdx);
        }else{ // intermediate node, can only be GIB
            for(int outPortIdx : dynamic_cast<GIBNode*>(adgNode)->in2outs(inPortIdx)){
                if(!routeDfgEdgePass(edge, adgNode, inPortIdx, outPortIdx, true)){ // try to find an internal link to route the edge
                    continue;
                }
                outPortIdxs.push_back(outPortIdx); // collect all available outPort 
            }   
        } 
        // search this layer of nodes
        for(int outPortIdx : outPortIdxs){
            for(auto& elem : adgNode->output(bits, outPortIdx)){
                int nextNodeId = elem.first;
                int nextSrcPort = elem.second;
                auto nextId = std::make_pair(nextNodeId, nextSrcPort);
                ADGNode* nextNode = _adg->node(nextNodeId);
                auto nextNodeType = nextNode->type();                
                if((dstNode != nullptr) && (dstNode->id() == nextNodeId) && dstPortRange.count(nextSrcPort)){ // get to the dstNode                    
                    success = true;
                    finalDstNode = nextId;
                } else if((dstNode != nullptr) && // route to dstNode
                          ((dstNode->id() == nextNodeId) || 
                           (nextNodeType == "GPE") || // not use GPE node to route
                           (nextNodeType == "IOB") || // cannot route to the dstNode through IOB    
                        //   (nextNodeType == "OB") || // cannot route to the dstNode through IOB                        
                           visitNodes.count(nextId))){ // the <node-id, inport-index> already visited                              
                    continue;
                }else if((dstNode == nullptr) && nextNodeType == "IOB" && !isMapped(nextNode, TYPE_IOC)){ // get to free IOB
                    success = true;
                    finalDstNode = nextId;                         
                } else if((dstNode == nullptr) && 
                          ((nextNodeType == "IOB") || // already used
                           (nextNodeType == "GPE") || // not use GPE node to route
                           visitNodes.count(nextId))){ // the <node-id, inport-index> already visited                              
                    continue;
                } else if(isAdgNodeInPortUsed(nextNodeId, bits, nextSrcPort)){ // the input port is already used
                    // if has the same srcId and srcPortIdx, nextId can still be used 
                    auto& nextNodeAttr = _adgNodeAttr[nextNodeId];                        
                    bool conflict = false;
                    for(auto& edgeLink : nextNodeAttr.dfgEdgePass){
                        auto passEdge = edgeLink.edge;
                        if(bits == passEdge->bitWidth() && edgeLink.srcPort == nextSrcPort && (passEdge->srcId() != srcNodeId || passEdge->srcPortIdx() != srcNodePortIdx)){
                            conflict = true; // the edge with different srcId or srcPortIdx occupied nextSrcPort 
                            break; 
                        }
                    }
                    if(conflict) continue;
                }             
                nodeQue.push(std::make_pair(nextNode, nextSrcPort)); // cache in queue
                info.srcNodeId = adgNode->id(); //  current node ID
                info.srcInPortIdx = inPortIdx;  // input port index of src node
                info.srcOutPortIdx = outPortIdx;   //  output port index of current node 
                visitNodes[nextId] = info;
                if(success){
                    break;
                }  
            }
            if(success){
                break;
            }
        }
        if(success){
            break;
        }
    }
    if(!success){
        return false;
    }
    // route the found path
    auto routeNode = finalDstNode;
    auto& edgeLinks = _dfgEdgeAttr[edge->id()].edgeLinks;
    int dstPort = -1;
    while (true){
        int nodeId = routeNode.first;
        int srcPort = routeNode.second;
        // keep the DFG edge routing status
        EdgeLinkAttr edgeAttr;
        edgeAttr.srcPort = srcPort;
        edgeAttr.dstPort = dstPort;
        edgeAttr.adgNode = _adg->node(nodeId);        
        edgeLinks.push_back(edgeAttr);
        // keep the ADG Node routing status
        ADGNodeAttr& nodeAttr = _adgNodeAttr[nodeId];
        if(routeNode == finalDstNode){ // dstNode
            //std::cout << "Set ADG node " << routeNode.first << " inport " << srcPort << " used\n";
            nodeAttr.inPortUsed[bits][srcPort] = true;  // only change the input port status
        } else if(nodeId == srcNode->id()){ // srcNode
            nodeAttr.outPortUsed[bits][dstPort] = true; // only change the output port status
            break; // get to the srcNode
        } else{ // intermediate routing nodes or OB
            nodeAttr.outPortUsed[bits][dstPort] = true; 
            nodeAttr.inPortUsed[bits][srcPort] = true;
            DfgEdgePassAttr passAttr;
            passAttr.edge = edge;
            passAttr.srcPort = srcPort;
            passAttr.dstPort = dstPort;
            nodeAttr.dfgEdgePass.push_back(passAttr);  
        }  
        dstPort = visitNodes[routeNode].srcOutPortIdx;
        routeNode = std::make_pair(visitNodes[routeNode].srcNodeId, visitNodes[routeNode].srcInPortIdx);
    }
    // reverse the passthrough nodes from srcNode to dstNode
    std::reverse(edgeLinks.begin(), edgeLinks.end());
    // // map DFG output port
    // if(dstNode == nullptr){
    //     auto& attr = _dfgOutputAttr[edge->dstPortIdx()];
    //     // attr.adgIOPort = mappedAdgOutPort;
    //     attr.iobId = finalDstNode.first;
    //     attr.routedEdgeIds.emplace(edge->id());
    // }
    return true;
}


// find a routable path from dstNode to srcNode/IB by BFS
// srcNode = nullptr: arbitrary IB
bool Mapping::routeDfgEdgeFromDst(DFGEdge* edge, ADGNode* srcNode, ADGNode* dstNode, const std::set<int>& dstPortRange){
    struct VisitNodeInfo{
        int dstNodeId;     // dst node ID
        int dstInPortIdx;  // input port index of dst node
        int dstOutPortIdx; // output port index of dst node
    };
    // cache visited node information, <<node-id, outport-index>, VisitNodeInfo>
    std::map<std::pair<int, int>, VisitNodeInfo> visitNodes; 
    // cache visited nodes, <node, outport-index>
    std::queue<std::pair<ADGNode*, int>> nodeQue; 
    // Breadth first search for possible routing path
    // assign the index of the output port of the srcNode
    int srcNodeOutPortIdx = edge->srcPortIdx();
    // std::cout << "1-bit output: " << srcNode->outputs().size() << std::endl;
    int srcNodeId = edge->srcId();
    int bits = edge->bitWidth();
    std::string srcNodeOp = _dfg->node(srcNodeId)->operation();
    //@yuan: for the GPE has alu and lut fine-grain outputs, we should obtain the correct srcportIdx here
    if(srcNodeOp == "LUT"){
        srcNodeOutPortIdx = srcNode->outputs(1).size() - 1;
    }
    // std::cout << "src op: " << srcNodeOp << " srcPortidx: " << srcNodeOutPortIdx << std::endl;
    // int srcNodeInPortIdx = -1;
    // int mappedAdgInPort;
    // bool isThisInputMapped = false; // if this DFG input port is already mapped to a IOB
    // int mappedIobId; // the mapped IOB ID
    // if(srcNode == nullptr && isDfgInputMapped(srcNodeOutPortIdx)){
    //     isThisInputMapped = true;
    //     mappedIobId = _dfgInputAttr[srcNodeOutPortIdx].iobId;
    // } 
    std::pair<int, int> finalSrcNode; // <node-id, outport-index>
    bool success = false;
    DFGEdge* sameSrcEdge = nullptr; // the already-routed edge with the same srcId and srcPortIdx
    nodeQue.push(std::make_pair(dstNode, -1));
    while(!nodeQue.empty()){
        auto queHead = nodeQue.front();
        ADGNode* adgNode = queHead.first;
        int outPortIdx = queHead.second;
        nodeQue.pop();        
        std::vector<int> inPortIdxs;
        if(outPortIdx == -1){ // dstNode
            inPortIdxs.assign(dstPortRange.begin(), dstPortRange.end());
        }else{ // intermediate node, can only be GIB
            for(int inPortIdx : dynamic_cast<GIBNode*>(adgNode)->out2ins(outPortIdx)){
                if(!routeDfgEdgePass(edge, adgNode, inPortIdx, outPortIdx, true)){ // try to find an internal link to route the edge
                    continue;
                }
                inPortIdxs.push_back(inPortIdx); // collect all available inPort 
            }   
        } 
        // search this layer of nodes
        for(int inPortIdx : inPortIdxs){
            auto elem = adgNode->input(bits, inPortIdx);// @yuan: one input port can only connect to one GIB
            int nextNodeId = elem.first;
            int nextDstPort = elem.second;
            auto nextId = std::make_pair(nextNodeId, nextDstPort);
            ADGNode* nextNode = _adg->node(nextNodeId);
            auto nextNodeType = nextNode->type();                           
            if((srcNode != nullptr) && (srcNode->id() == nextNodeId) && srcNodeOutPortIdx == nextDstPort){ // get to the srcNode                    
                success = true;
                finalSrcNode = nextId;
            } else if((srcNode != nullptr) && // route to srcNode
                      ((srcNode->id() == nextNodeId) || 
                       (nextNodeType == "GPE") || // not use GPE node to route
                       (nextNodeType == "IOB") || // cannot route to the srcNode through IOB
                    //   (nextNodeType == "IB") || // cannot route to the srcNode through IOB
                       visitNodes.count(nextId))){ // the <node-id, outport-index> already visited                          
                continue;
            }else if((srcNode == nullptr) && nextNodeType == "IOB" && !isMapped(nextNode, TYPE_IOC) && \
            dynamic_cast<IOBNode*>(nextNode)->opCapable(srcNodeOp)){ // get to free IOB
                success = true;
                finalSrcNode = nextId;                   
            } else if((srcNode == nullptr) && // route to arbitrary IOB
                      ((nextNodeType == "IOB") || // already used 
                       (nextNodeType == "GPE") || // not use GPE node to route
                       visitNodes.count(nextId))){ // the <node-id, outport-index> already visited                          
                continue;
            } else if(isAdgNodeOutPortUsed(nextNodeId, bits, nextDstPort)){ // the output port is already used
                // if has the same srcId and srcPortIdx, nextId can still be used 
                auto& nextNodeAttr = _adgNodeAttr[nextNodeId];                        
                bool conflict = false;
                for(auto& edgeLink : nextNodeAttr.dfgEdgePass){
                    auto passEdge = edgeLink.edge;
                    if(bits == passEdge->bitWidth() && edgeLink.dstPort == nextDstPort && (passEdge->srcId() != srcNodeId || passEdge->srcPortIdx() != srcNodeOutPortIdx)){
                        conflict = true; // the edge with different srcId or srcPortIdx occupied nextDstPort 
                        break; 
                    }else if(bits == passEdge->bitWidth() && edgeLink.dstPort == nextDstPort){
                        sameSrcEdge = passEdge; // the edge with the same srcId and srcPortIdx
                        finalSrcNode = nextId;
                        success = true; // REUSE part of the routing path of the edge
                        break;
                    }
                }
                if(conflict) continue;
            }                 
            nodeQue.push(std::make_pair(nextNode, nextDstPort)); // cache in queue
            VisitNodeInfo info;
            info.dstNodeId = adgNode->id(); //  current node ID
            info.dstInPortIdx = inPortIdx;  // input port index of src node
            info.dstOutPortIdx = outPortIdx;   //  output port index of current node 
            visitNodes[nextId] = info;
            if(success){
                break;
            }  
        }
        if(success){
            break;
        } 
    }
    if(!success){
        return false;
    }
    // route the found path
    auto routeNode = finalSrcNode;
    int srcPort = -1; // (srcNode != nullptr)? -1 : srcNodeInPortIdx;
    auto& edgeLinks = _dfgEdgeAttr[edge->id()].edgeLinks;
    // reuse part of the routing path of the edge
    if(sameSrcEdge != nullptr){
        auto& sameSrcEdgeLinks = _dfgEdgeAttr[sameSrcEdge->id()].edgeLinks;
        for(auto& link : sameSrcEdgeLinks){ // search from the source node
            // keep the DFG edge routing status   
            edgeLinks.push_back(link);
            // keep the ADG Node routing status
            ADGNodeAttr& nodeAttr = _adgNodeAttr[link.adgNode->id()];
            DfgEdgePassAttr passAttr;
            passAttr.edge = edge;
            passAttr.srcPort = link.srcPort;
            passAttr.dstPort = link.dstPort;
            nodeAttr.dfgEdgePass.push_back(passAttr);
            if(link.adgNode->id() == finalSrcNode.first && link.dstPort == finalSrcNode.second){
                break;
            }
        }
        srcPort = visitNodes[finalSrcNode].dstInPortIdx;
        routeNode = std::make_pair(visitNodes[finalSrcNode].dstNodeId, visitNodes[finalSrcNode].dstOutPortIdx);
    }    
    while (true){
        int nodeId = routeNode.first;
        int dstPort = routeNode.second;
        // keep the DFG edge routing status
        EdgeLinkAttr edgeAttr;
        edgeAttr.srcPort = srcPort;
        edgeAttr.dstPort = dstPort;
        edgeAttr.adgNode = _adg->node(nodeId);        
        edgeLinks.push_back(edgeAttr);
        // keep the ADG Node routing status
        ADGNodeAttr& nodeAttr = _adgNodeAttr[nodeId];
        if(routeNode == finalSrcNode){ // srcNode
            nodeAttr.outPortUsed[bits][dstPort] = true; // only change the output port status 
        } else if(nodeId == dstNode->id()){ // dstNode
            //std::cout << "Set ADG node " << nodeId << " inport " << srcPort << " used\n";
            nodeAttr.inPortUsed[bits][srcPort] = true;  // only change the input port status 
            break; // get to the dstNode
        } else{
            nodeAttr.inPortUsed[bits][srcPort] = true; 
            nodeAttr.outPortUsed[bits][dstPort] = true;
            DfgEdgePassAttr passAttr;
            passAttr.edge = edge;
            passAttr.srcPort = srcPort;
            passAttr.dstPort = dstPort;
            nodeAttr.dfgEdgePass.push_back(passAttr);
        }        
        srcPort = visitNodes[routeNode].dstInPortIdx;
        routeNode = std::make_pair(visitNodes[routeNode].dstNodeId, visitNodes[routeNode].dstOutPortIdx);
    }
    // // map DFG input port
    // if(srcNode == nullptr){
    //     auto& attr = _dfgInputAttr[edge->srcPortIdx()];
    //     // attr.adgIOPort = mappedAdgInPort;
    //     attr.iobId = finalSrcNode.first;
    //     attr.routedEdgeIds.emplace(edge->id());
    // }
    //@yuan: for the test, delete when the code finishing
    /*if(_dfg->node(edge->dstId())->operation() == "LUT"){
        std::cout << "srcNodeOutPortIdx: " << srcNodeOutPortIdx << "srcNodeId: " << srcNodeId << " bits: " << bits << std::endl;
        std::cout << "dstnode: " << dstNode->id() << " dstNode type: " << dstNode->type()<< std::endl;
        std::cout << "finalSrcNode id: " << finalSrcNode.first << std::endl;
        exit(0);
    }*/
    return true;
}

// @yuan: find a routable path from LUT to ALU
/*bool Mapping::routeDfgEdgeFromLUT(DFGEdge* edge, ADGNode* adgNode, DFGNode* unmapdfgNode){
    //int dstPortidx = edge->dstPortIdx();
    ///auto srcNode = _dfg->node(edge->srcId());
    auto& edgeLinks = _dfgEdgeAttr[edge->id()].edgeLinks;
    EdgeLinkAttr edgeAttr;
    edgeAttr.srcPort = -1;
    edgeAttr.dstPort = -1;
    edgeAttr.adgNode = adgNode;     
    edgeLinks.push_back(edgeAttr);

}

// @yuan: find a routable path from ALU to LUT
bool Mapping::routeDfgEdgeFromALU(DFGEdge* edge, ADGNode* adgNode, DFGNode* unmapdfgNode){

}*/


// @yuan: route inside the GPE
bool Mapping::routeDfgEdgeInsideGPE(DFGEdge* edge, ADGNode* targetNode){
    auto& edgeLinks = _dfgEdgeAttr[edge->id()].edgeLinks;
    // int edgeDstPort = edge->dstPortIdx();
    // int bits = edge->bitWidth();
    // DFGNode* dstDfgNode = _dfg->node(edge->dstId());
    // int type = mappedFUType(dstDfgNode);
    // if(type == TYPE_LUT){ // operand index of LUT in GPE may not start from 0; ALU and IOC do.
    //     edgeDstPort = dynamic_cast<GPENode*>(targetNode)->getOperandIdxLUT(edgeDstPort);
    // }else if( type == TYPE_ALU && bits == 1){// for the fine-grain input of alu, its index start from 0
    //     int cg_width = 32;
    //     for(auto& width : dstDfgNode->bitWidths()){
    //         if(width != 1){
    //             cg_width = width;
    //             break;
    //         }
    //     }
    //     edgeDstPort = edgeDstPort - dynamic_cast<GPENode*>(targetNode)->numOperands(cg_width);
    // }
    // FUNode* targetFUNode = dynamic_cast<FUNode*>(targetNode);
    // // @yuan: random selet one port that connnet to this operand   
    // auto& inPorts = targetFUNode->operandInputs(bits, edgeDstPort);
    // int srcport;
    // for(auto& inport : inPorts){
    //     srcport = inport;
    //     break;
    // }
    EdgeLinkAttr edgeAttr;
    edgeAttr.srcPort = -1;
    edgeAttr.dstPort = -1;
    edgeAttr.adgNode = targetNode;     
    edgeLinks.push_back(edgeAttr);
    return true;
}

// find the available input ports in the dstNode to route edge
// assume route the edge outside the dstNode, rather than inside.
std::set<int> Mapping::availDstPorts(DFGEdge* edge, ADGNode* dstNode){
    DFGNode* dstDfgNode = _dfg->node(edge->dstId());
    int bits = edge->bitWidth();
    // std::cout << " edge: " << _dfg->node(edge->srcId())->name() << " to " << _dfg->node(edge->dstId())->name() <<" width: " << bits<< std::endl;
    int edgeDstPort = edge->dstPortIdx();
    int opereandNum = dstDfgNode->numInputs(bits);
    // if(_dfg->isIONode(edge->dstId()) && bits == 1){
    //     DFGIONode* iobNode = dynamic_cast<DFGIONode*>(dstDfgNode);
    //     if(!iobNode->dependencyNodes().empty()){
    //         opereandNum -= iobNode->dependencyNodes().size();
    //     }        
    // }
    // std::cout << "edgeDstPort: " << edgeDstPort << " opereandNum: " << opereandNum <<" width: "<<bits<< " is commutative: "<< dstDfgNode->commutative()<<std::endl;
    int type = mappedFUType(dstDfgNode);
    FUNode* dstFUNode = dynamic_cast<FUNode*>(dstNode);
    int dstNodeId = dstNode->id();
    // std::cout << "Target ADGNODEId: " << dstNodeId << std::endl;
    std::set<int> dstPortRange; // the input port index range of the dstNode
    std::vector<int> opIdxs; // operand indexes
    int cg_width = _dfg->CGWidth();
    if(dstDfgNode->isPacked()){
        if(dstDfgNode->hasImm(bits)){//@hw_yuan: for now, the packed node has constant cannot support port exchange
            opIdxs.push_back(edgeDstPort);
        }else{
            std::string earlestOp = dstDfgNode->multiOps().front();
            int numOperandEarlestOp = Operations::numOperands(earlestOp);
            if(edgeDstPort >= numOperandEarlestOp){// the input edge doesn't belong to earlest operation
                //maybe we need to check the secondary operation
                int opIdx = 1;
                std::string secondaryOp;
                for(auto op : dstDfgNode->multiOps()){
                    if(opIdx == 2){
                        secondaryOp = op;
                        break;
                    }
                    opIdx++;
                }
                auto interConnection = dstDfgNode->internalAttrMap();
                bool isIndependent = true;
                for(auto& elem : interConnection){
                    if(elem.first != _dfg->getEuType(secondaryOp)) continue;
                    for(auto eachAttr : elem.second){
                        if(eachAttr.second.srcName == _dfg->getEuType(earlestOp)){
                            isIndependent = false;
                            break;
                        }
                    }
                }
                if(Operations::isCommutative(secondaryOp) && isIndependent){// it means that first and second ops are independent
                    int numOperandSecondOp = Operations::numOperands(secondaryOp);
                    if(edgeDstPort >= numOperandEarlestOp + numOperandSecondOp){
                       opIdxs.push_back(edgeDstPort); 
                    }else{
                        for(int opIdx = numOperandEarlestOp; opIdx < numOperandEarlestOp + numOperandSecondOp; opIdx++){
                            opIdxs.push_back(opIdx);
                        }
                    }
                }else{
                    opIdxs.push_back(edgeDstPort);
                }

            }else{
                if(Operations::isCommutative(earlestOp)){
                    for(int opIdx = 0; opIdx < numOperandEarlestOp; opIdx++){
                        opIdxs.push_back(opIdx);
                    }
                }else{
                    opIdxs.push_back(edgeDstPort);
                }
            }
        }
    }else{
        if(dstDfgNode->commutative()){ // operands are commutative, use all the operand indexes
            for(int opIdx = 0; opIdx < opereandNum; opIdx++){
                opIdxs.push_back(opIdx);
            }
        } else{ // operands are not commutative, use the edgeDstPort as the operand index
            opIdxs.push_back(edgeDstPort);
        }
    }
    //@yuan: for the test, delete when the code finishing
    /*if(type == TYPE_LUT){
        std::cout << " edge: " << _dfg->node(edge->srcId())->name() << " to " << _dfg->node(edge->dstId())->name() << std::endl;
        for(int opIdx : opIdxs){
            std::cout << " dstDfgNode: " << dstDfgNode->name() << " opidx: "<< opIdx << std::endl;
        }
            exit(0);
    }*/
    // select all the ports connected to available operand
    for(int opIdx : opIdxs){
        if(type == TYPE_LUT){ // operand index of LUT in GPE may not start from 0; ALU and IOC do.
            opIdx = dynamic_cast<GPENode*>(dstNode)->getOperandIdxLUT(opIdx);
        }else if( type == TYPE_ALU && bits == 1 ){// for the fine-grain input of alu, its index start from 0
            // int cg_width = 32;
            // for(auto& width : dstDfgNode->bitWidths()){
            //     if(width != 1){
            //         cg_width = width;
            //         break;
            //     }
            // }
            if(dstDfgNode->accumulative() || dstDfgNode->operation() == "CISEL"){
                int opereandNumCG = dstDfgNode->numInputs(cg_width);
                opIdx = dynamic_cast<GPENode*>(dstNode)->getOperandIdxFg(opereandNumCG, opIdx);
                // std::cout << "opereandNumCG: " << opereandNumCG << std::endl;
            }else if(dstDfgNode->operation() != "SEXT" && dstDfgNode->operation() != "ZEXT"){
                int numOpCG = dynamic_cast<GPENode*>(dstNode)->numOperands(cg_width);
                opIdx = opIdx - numOpCG;
                if(!dstDfgNode->isPacked() && dynamic_cast<GPENode*>(dstNode)->isMIMD()){//@hw_yuan: MIMD-ALU has 4 corase-grained inputs
                    opIdx += 2;
                }
            }

            // std::cout << "dstDfgNode: " << dstDfgNode->name() << " width port: " << bits << " opidx:" << opIdx << std::endl;
        }else if (type == TYPE_IOC && bits == 1){//@yuan: for the fine-grain input of IOB, its index start from 0
            // int cg_width = 32;
            // for(auto& width : dstDfgNode->bitWidths()){
            //     if(width != 1){
            //         cg_width = width;
            //         break;
            //     }
            // }
            int opereandNumCG = dstDfgNode->numInputs(cg_width);
            opIdx = opIdx - opereandNumCG;
            auto dstOp = dstDfgNode->operation();
            if(dstOp == "TLOAD" || dstOp == "TSTORE"){
                opIdx = dynamic_cast<IOBNode*>(dstNode)->getOperandIdxIOB(opIdx);
            }
            // std::cout << "dstDfgNode: " << dstDfgNode->name() << " width port: " << bits << " opidx:" << opIdx <<" opereandNumCG: "<<opereandNumCG<< std::endl;
        }
        // std::cout << "dstDfgNode: " << dstDfgNode->name() << " width port: " << bits << " opidx:" << opIdx << std::endl;
        bool operandUsed = false; // if this operand is used
        std::set<int> inPorts;
        if((dstDfgNode->accumulative()||dstDfgNode->operation() == "CISEL") && bits == 1){
            inPorts = dynamic_cast<GPENode*>(dstNode)->ConDMRInputs(bits, opIdx);
        }else{
            inPorts = dstFUNode->operandInputs(bits, opIdx);
        }
        for(int inPort : inPorts){ // input ports connected to the operand with index of opIdx
            if(isAdgNodeInPortUsed(dstNodeId, bits, inPort)){
                operandUsed = true;
                break;
            }
        }
        if(!operandUsed){ 
            for(int inPort : inPorts){ // all the ports are available
                dstPortRange.emplace(inPort);
            }                
        }
    }
    // if(type == TYPE_LUT && dstDfgNode->LUTsize() == 2){
    // std::cout << "Current ADG node " << dstNode->id() << " available inputs: ";
    // for(auto &port : dstPortRange){
    //     std::cout << port << " ";
    // }
    // std::cout << std::endl;
    // }
    // std::cout << "dstPortRange size: " << dstPortRange.size() << std::endl;
    return dstPortRange;
}


// route DFG edge between srcNode and dstNode
// find a routable path from srcNode to dstNode by BFS
bool Mapping::routeDfgEdge(DFGEdge* edge, ADGNode* srcNode, ADGNode* dstNode){
    
    //@yuan: for the test, delete when the code finishing
    //  if(_dfg->node(edge->dstId())->operation() == "LUT"){
    //  std::cout << "Route edge(" << _dfg->node(edge->srcId())->name() << " -> " << _dfg->node(edge->dstId())->name() 
    //            << "), dst ADG node " << dstNode->id() ;
    //  std::cout << std::endl;
    //     // exit(0);
    //  }
    std::set<int> dstPortRange = availDstPorts(edge, dstNode); // the input port index range of the dstNode
    if(dstPortRange.empty()){ // no available input port in the dstNode
        return false;
    }
    //  std::cout << std::endl;
    //@yuan: for the test, delete when the code finishing
    //  if(_dfg->node(edge->dstId())->operation() == "LUT"){
    //  std::cout << "Route edge(" << _dfg->node(edge->srcId())->name() << " -> " << _dfg->node(edge->dstId())->name() 
    //            << "), dst ADG node " << dstNode->id() << " available inputs: ";
    //  for(auto &port : dstPortRange){
    //      std::cout << port << " ";
    //  }
    //  std::cout << std::endl;
        //exit(0);
    //  }
    //  std::cout << "Route edge(" << _dfg->node(edge->srcId())->name() << " -> " << _dfg->node(edge->dstId())->name() 
    //            << "), dst ADG node " << dstNode->id() << " available inputs: ";
    //  for(auto &port : dstPortRange){
    //      std::cout << port << " ";
    //  }
    //  std::cout << std::endl;
    return routeDfgEdgeFromDst(edge, srcNode, dstNode, dstPortRange);
    // if(!routeDfgEdgeFromDst(edge, srcNode, dstNode, dstPortRange)){
    //     return routeDfgEdgeFromSrc(edge, srcNode, dstNode, dstPortRange);
    // }
    // return true;
}

//@yuan: route DFG edge by using the internal link of the GPE, i.e. route between ALU and LUT
bool Mapping::routeDfgEdge(DFGEdge* edge, ADGNode* adgNode){
    /*if(mappedFUType(unmapNode) == TYPE_ALU){
        return routeDfgEdgeFromLUT(edge, adgNode, unmapNode);
    }else if(mappedFUType(unmapNode) == TYPE_LUT){
        return routeDfgEdgeFromALU(edge, adgNode, unmapNode);
    }else{
        return false;
    }*/
    return routeDfgEdgeInsideGPE(edge, adgNode);
}

// route DFG edge between adgNode and IOB
// is2Input: whether connected to IB or OB
bool Mapping::routeDfgEdge(DFGEdge* edge, ADGNode* adgNode, bool is2Input){
    std::set<int> dstPortRange; // the input port index range of the dstNode
    if(!is2Input){ // route to OB
        return routeDfgEdgeFromSrc(edge, adgNode, nullptr, dstPortRange);       
    } 
    dstPortRange = availDstPorts(edge, adgNode); // the input port index range of the dstNode
    if(dstPortRange.empty()){ // no available input port in the dstNode
        return false;
    }
    // std::cout << "Route edge(input -> " << _dfg->node(edge->dstId())->name() 
    //           << "), dst ADG node " << adgNode->id() << " available inputs: ";
    // for(auto &port : dstPortRange){
    //     std::cout << port << " ";
    // }
    // std::cout << std::endl;
    return routeDfgEdgeFromDst(edge, nullptr, adgNode, dstPortRange);
}


// unroute DFG edge
void Mapping::unrouteDfgEdge(DFGEdge* edge){
    int eid = edge->id();
    if(!_dfgEdgeAttr.count(eid)) return;
    int bits = edge->bitWidth();
    auto& edgeAttr = _dfgEdgeAttr[eid]; 
    spdlog::debug("Unroute DFG edge {0} ({1} -> {2}) ", eid, _dfg->node(edge->srcId())->name(), _dfg->node(edge->dstId())->name());   
    // std::cout << "unroute DFG edge " << eid << " (" << _dfg->edge(eid)->srcId() << ", " << _dfg->edge(eid)->dstId() << ")\n";
    // remove this edge from all the routed ADG links
    if(edgeAttr.edgeLinks.size() != 1)
        for(auto& edgeLink : edgeAttr.edgeLinks){
            auto node = edgeLink.adgNode;
            if(!_adgNodeAttr.count(node->id())) continue; 
            auto& nodeAttr = _adgNodeAttr[node->id()];
                   
            auto& nodeEdges = nodeAttr.dfgEdgePass;
            auto iter = std::remove_if(nodeEdges.begin(), nodeEdges.end(), [&](DfgEdgePassAttr& x){ return (x.edge == edge); });
            nodeEdges.erase(iter, nodeEdges.end());
            // if(node->type() != "IOB"){
            bool setInPortUnused = (edgeLink.srcPort != -1);
            bool setOutPortUnused = (edgeLink.dstPort != -1);
            for(auto& nodeEdge : nodeEdges){ // the srcPort/dstPort may be used by other edges
                bool isSameBits = bits == nodeEdge.edge->bitWidth();
                if(setInPortUnused && isSameBits && (nodeEdge.srcPort == edgeLink.srcPort)){
                    setInPortUnused = false; // do not set unused
                }
                if(setOutPortUnused && isSameBits && (nodeEdge.dstPort == edgeLink.dstPort)){
                    setOutPortUnused = false;
                }
                if(!setInPortUnused && !setOutPortUnused){
                    break;
                }
            }
            if(setInPortUnused){
                nodeAttr.inPortUsed[bits][edgeLink.srcPort] = false;
                // if(node->type() == "GPE"){
                //     std::cout << "Set ADG node " << node->id() << " inport " << edgeLink.srcPort << " unused\n";
                // }
            }
            if(setOutPortUnused){
                nodeAttr.outPortUsed[bits][edgeLink.dstPort] = false;
            }           
    }
    _dfgEdgeAttr.erase(eid);
    // // unmap DFG IO
    // if(edge->srcId() == _dfg->id()){ // connected to DFG input port
    //     int idx = edge->srcPortIdx();
    //     _dfgInputAttr[idx].routedEdgeIds.erase(eid);
    // }
    // if(edge->dstId() == _dfg->id()){ // connected to DFG output port
    //     int idx = edge->dstPortIdx();
    //     _dfgOutputAttr[idx].routedEdgeIds.erase(eid);
    // }
}


// if succeed to map all DFG nodes
bool Mapping::success(){
    return _dfg->nodes().size() == _numNodeMapped;
}


// total/max edge length (link number)
void Mapping::getEdgeLen(int& totalLen, int& maxLen){
    int total = 0;
    int maxVal = 0;
    for(auto& elem : _dfgEdgeAttr){
        int num = elem.second.edgeLinks.size();
        total += num;
        maxVal = std::max(maxVal, num);
    }
    totalLen = total;
    maxLen = maxVal;
}

// number of total mapped ADG nodes
int Mapping::getMappedAdgNodeNum(){
    return _adgNodeAttr.size();
}

// deprecated
// // assign DFG IO to ADG IO according to mapping result
// // post-processing after mapping
// void Mapping::assignDfgIO(){
//     // assign input ports
//     for(auto& elem : _dfg->inputEdges()){
//         // IB has only one input port, connected to ADG input port
//         // all the edges with the same src routing to the same IB
//         int eid = *(elem.second.begin());
//         // auto ib = _dfgEdgeAttr[eid].edgeLinks.begin()->adgNode; // ADG IB node
//         // int inport = ib->input(0).second; 
//         auto ibLinkAttr = _dfgEdgeAttr[eid].edgeLinks.begin();
//         auto ib = ibLinkAttr->adgNode; // ADG IB node
//         auto srcPort = ibLinkAttr->srcPort; // ADG IB source port
//         int inport = ib->input(srcPort).second;
//         _dfgInputAttr[elem.first].adgIOPort = inport;
//     }
//     // assign output ports
//     for(auto& elem : _dfg->outputEdges()){
//         // OB has only one output port, connected to ADG output port
//         // auto ob = _dfgEdgeAttr[elem.second].edgeLinks.rbegin()->adgNode; // ADG OB node
//         // int outport = ob->output(0).begin()->second;
//         auto obLinkAttr = _dfgEdgeAttr[elem.second].edgeLinks.rbegin();
//         auto ob = obLinkAttr->adgNode; // ADG OB node
//         auto dstPort = obLinkAttr->dstPort; // ADG OB dest port
//         int outport = ob->output(dstPort).begin()->second;
//         _dfgOutputAttr[elem.first].adgIOPort = outport;
//     }
// }



// ==== tming schedule >>>>>>>>>>>>>
// // reset the timing bounds of each DFG node
// void Mapping::resetBound(){
//     // int INF = 0x3fffffff;
//     for(auto& elem : _dfg->nodes()){
//         auto& attr = _dfgNodeAttr[elem.second->id()];
//         attr.minLat = 0;
//         attr.maxLat = 0;
//     }
// }


// get currently available delay cycles in the FU node according to the mapped DFG node
/*int Mapping::getAvailDelay(FUNode* fuNode, DFGNode* dfgNode, int bitwidth){
    if(fuNode->numOperands(bitwidth) < 2){
        return 0; // no delay unit
    }
    int maxDelay = fuNode->maxDelay(bitwidth);
    int numOp = dfgNode->inputs(bitwidth).size();
    if(numOp == 0) return 0;
    int availDelay;
    if(_fuDelayAttr.count(fuNode->id())){
        if(_fuDelayAttr[fuNode->id()].count(bitwidth)){
            auto& attr = _fuDelayAttr[fuNode->id()][bitwidth];
            int leftOps = numOp - attr.delayUsed.size();
            availDelay = (maxDelay - attr.totalDelayUsed) / std::max(leftOps, 1); // left delay cycles divided by the left operands
            std::cout << " leftOps: " << leftOps << " delay used: " << attr.totalDelayUsed << std::endl; 
        }else{
            availDelay = maxDelay / numOp;
        }
    }else{
        availDelay = maxDelay / numOp;
    }
    std::cout << "dfg node: " << dfgNode->name() <<" width:"<< bitwidth << " maxdelay: " << maxDelay<<" numOp: " << numOp << " avaidelay: " << availDelay << std::endl;
    return availDelay;
}*/
int Mapping::getAvailDelay(FUNode* fuNode, DFGNode* dfgNode, int bitwidth){
    // std::cout << "funode type: " << fuNode->type() << " 1-bit op num: " << fuNode->numOperands(1) << " 1-bit delay: " << fuNode->maxDelay(1)<< std::endl;
    // if(fuNode->numOperands(bitwidth) < 2 && fuNode->type() != "IOB"){//@hw_yuan: 1 operand still has delay unit
    //     return 0; // no delay unit
    // }
    int maxDelay = fuNode->maxDelay(bitwidth);
    int numOp = dfgNode->inputs(bitwidth).size();
    // for(auto & elem : dfgNode->inputEdges(bitwidth)){// @yuan: for the memory dependency edge, it doesn't need use RDU
    //     if(_dfg->edge(elem.second)->isMemBackEdge()){
    //         numOp--;
    //     }
    // }
    // if(_dfg->isIONode(dfgNode->id()) && bitwidth == 1){ // @yuan: memory-dependent edge doesn't use RDU
    //     DFGIONode* iobNode = dynamic_cast<DFGIONode*>(dfgNode);
    //     if(!iobNode->dependencyNodes().empty()){
    //         numOp -= iobNode->dependencyNodes().size();
    //     }
    // }
    if(numOp == 0) return 0;
    // int gpeOp = fuNode->numOperands(bitwidth);
    int dfgOp = dfgNode->numInputs(bitwidth);
    int availDelay;
    //@hw_yuan: we reserve the additional delay resource for packed DFGNode
    if(dfgNode->isPacked()){
        for(auto& inEdge : dfgNode->inputEdges(bitwidth)){
            DFGEdge* e = _dfg->edge(inEdge.second);
            maxDelay -= e->addDelay();
        }
    }
    // std::cout << "old maxdelay: " << fuNode->maxDelay(bitwidth) << " new delay: " << maxDelay << std::endl;
    if(_fuDelayAttr.count(fuNode->id())){
        if(_fuDelayAttr[fuNode->id()].count(bitwidth)){
            auto& attr = _fuDelayAttr[fuNode->id()][bitwidth];
            int leftOps = dfgOp - attr.delayUsed.size();
            availDelay = (maxDelay - attr.totalDelayUsed) / std::max(leftOps, 1); // left delay cycles divided by the left operands
            //std::cout << " leftOps: " << leftOps << " delay used: " << attr.totalDelayUsed << std::endl; 
        }else{
            availDelay = maxDelay / numOp;
        }
    }else{
        availDelay = maxDelay / numOp;
    }
    //std::cout << "dfg node: " << dfgNode->name() <<" width:"<< bitwidth << " maxdelay: " << maxDelay<<" numOp: " << numOp << " avaidelay: " << availDelay << std::endl;
    return availDelay;
}


// calculate the routing latency of each edge, not inlcuding the delay pipe
void Mapping::calEdgeRouteLat(){
    for(auto& elem : _dfgEdgeAttr){
        auto& attr = elem.second;
        int lat = 0;
        for(auto& linkAttr : attr.edgeLinks){
            if(linkAttr.adgNode->type() == "GIB"){ // edge only pass-through GIB nodes
                bool reged = dynamic_cast<GIBNode*>(linkAttr.adgNode)->outReged(linkAttr.dstPort); // output port reged
                lat += reged;
            }
        }
        attr.lat = lat;
        // attr.latNoDelay = lat;
    }
}


// calculate the DFG node latency bounds not considering the Delay components, including 
// min latency of the output ports
// ID of the DFG node with the max latency
void Mapping::latencyBound(){
    int maxLatDfg = 0; // max latency of DFG
    int maxLatNodeId;  // ID of the DFG node with the max latency
    // calculate the LOWER bounds in topological order
    // int inputLat = _adg->loadLatency(); // IOB input latency
    // auto dfgInNodes = _dfg->getInNodes(); // In nodes: INPUT node, LOAD node without input
    // std::cout << "_dfg->topoNodes() size: " << _dfg->topoNodes().size()<< std::endl;
    // available latency%II for multiport IO nodes, 0~(II-1)
    std::map<std::string, std::map<int, std::set<int>>> multiportAvailLatModII;
    std::map<std::string, std::map<int, std::set<int>>> multiportAvailLat;
    // std::map<std::string, std::set<int>> multiportAvailLatModII;
    std::set<int> nums;
    for(int i = 0; i < _II; i++){ // @yuan: available time slots
        nums.insert(i);
    }
    // std::cout << "_II: " << _II << std::endl;
    for(auto &elem : _dfg->multiportIOs()){ // @yuan: for every memmory(same ref_name), set their initial available time slots
        // auto multiportBr = _dfg->multiportIObr();
        // for(int br = 0; br <= multiportBr[elem.first]; br++){
        auto multiportStep = _dfg->multiportIOSteps();
        for(int step = 0; step < multiportStep[elem.first].size(); step++){
            // std::cout << "array: " << elem.first << " branch: " << br << std::endl;
            multiportAvailLatModII[elem.first][step] = nums;
        }        
    }
    for(auto nodeId : _dfg->topoNodes()){
        DFGNode *node = _dfg->node(nodeId);
        // std::cout << "node name: " << node->name() << std::endl;
        int maxLat = 0; 
        int nodeOpLat = _dfg->node(nodeId)->opLatency(); // operation latency
        for(auto& elem : node->inputEdges()){
            int bits = elem.first;
            for(auto& edges : elem.second){
                int eid = edges.second;
                DFGEdge* edge = _dfg->edge(eid);// @yuan: don't touch the edge that is the end of loop
                edge->setDontTouch(false);
                if(edge->isBackEdge() || edge->isMemEdge()){
                    continue;
                }
                int routeLat = _dfgEdgeAttr[eid].lat + edge->logicLat() * _II; // latNoDelay;
                int srcNodeId = _dfg->edge(eid)->srcId();
                // int srcNodeLat = inputLat; // DFG input node min latency = IOB input latency
                // if(!dfgInNodes.count(srcNodeId)){ // not connected to DFG input node
                int srcNodeLat = _dfgNodeAttr[srcNodeId].lat;                
                // }
                int inPortLat = srcNodeLat + routeLat; 
                maxLat = std::max(maxLat, inPortLat);
            }
        }
        // for(auto& elem : node->inputs()){
        //     int srcNodeId = elem.second.first;
        //     int srcNodeLat = 0;
        //     if(srcNodeId != _dfg->id()){ // not connected to DFG input port
        //         srcNodeLat = _dfgNodeAttr[srcNodeId].lat;                
        //     }
        //     int inPortLat = srcNodeLat; // edge no latency
        //     maxLat = std::max(maxLat, inPortLat);
        // }
        int lat = maxLat + nodeOpLat;// @yuan: maxLat is the input edge has latest arrival
        // adjust the latency of the multiport IO node to avoid spad access conflict
        if(_dfg->isMultiportIoNode(nodeId) ){
            if(dynamic_cast<DFGIONode*>(node)->MultiportType() > 0){ 
                std::string name = dynamic_cast<DFGIONode*>(node)->memRefName();
                // int nodeBr = dynamic_cast<DFGIONode*>(node)->Branch();
                int nodeStep = dynamic_cast<DFGIONode*>(node)->ControlStep(); //@yuan: return the control step
                int tune_times = 1;
                // int extra_lat = 0; //@yuan: extra latency for the backedge from output to input
                std::string op = node->operation();
                // std::cout << "array: "<<name<< " io node name: " << dynamic_cast<DFGIONode*>(node)->name() << " op: "<< op <<" step: " << nodeStep<< " tune times: " << tune_times << " lat: " << lat << " multiport size: " << multiportAvailLatModII[name][nodeStep].size()<< std::endl;
                if(op == "OUTPUT"|| op == "STORE" || op == "CSTORE" || op == "COUTPUT"|| op == "TCSTORE"){
                    for(auto& out : node->outputEdges(1)){
                        for(auto& outEdgeId : out.second){
                            DFGEdge* edge = _dfg->edge(outEdgeId);
                            // extra_lat = std::max(extra_lat, _dfgEdgeAttr[outEdgeId].lat + edge->logicLat() * _II);
                        }
                    }
                }
                int enable_latency = _dfg->getOutNodes().count(nodeId) ? 0 : node->opLatency(); //@yuan: we need to ensure the the enable signal of load and store can be aligned
                for(; tune_times <= _II; tune_times++){//@yuan: tuning time over II will back to the start since "% II"
                    if(!isTimeAvai(node, lat - enable_latency)){//@yuan: first, the latency can not satisfy the dependency
                        lat++;
                        tune_times++;
                        // std::cout << "!!!" << std::endl;
                        continue;
                    }
                    for(auto& elem: multiportAvailLatModII[name]){//@yuan: if the latency satisfy the dependency, we should make sure it can not be occupied by other stpes
                        // std::cout << "elem.first: " << elem.first << std::endl;
                        if(elem.first == nodeStep) continue;//@yuan: the time slot of current step can not overlap with the slot in other steps, while the nodes in the same step means that they don't occur any conflict
                        // for(auto test: elem.second){
                        //     std::cout << test << " " ; 
                        // }
                        if(!elem.second.count((lat - enable_latency)%_II)){
                            lat++;
                            tune_times++;
                            break;
                        }
                    }
                }
                // while((!multiportAvailLatModII[name][nodeBr].count((lat - node->opLatency())%_II) || !isTimeAvai(node, lat - node->opLatency()))){ // @yuan: find the available time slot, i.e. the time slot does not be erased
                //     lat++;
                //     tune_times++;
                //     if(tune_times > _II) break;
                //     // std::cout << "lat: " << lat << std::endl;
                // }
                if(multiportAvailLatModII[name][nodeStep].count((lat - enable_latency)%_II))
                    multiportAvailLatModII[name][nodeStep].erase((lat - enable_latency)%_II); // @yuan: once the available time slot is occupied by one IOB in this spad, erase it
                // std::cout << "io node name: " << dynamic_cast<DFGIONode*>(node)->name() << " step: " << nodeStep<< " tune times: " << tune_times <<" after tuning lat: " << lat << " multiport size: " << multiportAvailLatModII[name][nodeStep].size()<< std::endl;
            }
        }else if(_dfg->isIONode(nodeId)){
            int enable_latency = _dfg->getOutNodes().count(nodeId) ? 0 : node->opLatency(); //@yuan: we need to ensure the the enable signal of load and store can be aligned
            while((!isTimeAvai(node, lat - enable_latency))){ // @yuan: find the available time slot
                lat++;
            }
        }
        lat = TaskLat(node, lat);// @yuan: avoid the output value before start affects corectness
        _dfgNodeAttr[nodeId].lat = lat;
        if(lat >= maxLatDfg){
            maxLatDfg = lat;
            maxLatNodeId = nodeId;
        }      
        // std::cout << "node name: " << node->name() << " lat:"<<lat<<std::endl;

    }
    _maxLat = maxLatDfg;
    _maxLatNodeId = maxLatNodeId;
    // std::cout << "maxlatnode: " << _dfg->node(_maxLatNodeId)->name() << " lat: " << _maxLat << std::endl;
}

// // schedule the latency of each DFG node based on the mapping status
// // DFG node latency: output port latency
// // DFG edge latency: latency from the output port of src node to the FU Input port of dst Node, including DelayPipe 
void Mapping::latencySchedule(){
    _fuDelayAttr.clear();
    // available latency%II for multiport IO nodes, 0~(II-1)
    std::map<std::string, std::map<int, std::set<int>>> multiportAvailLatModII;
    // std::map<std::string, std::set<int>> multiportAvailLatModII;
    std::set<int> nums;
    for(int i = 0; i < _II; i++){
        nums.insert(i);
    }
    for(auto &elem : _dfg->multiportIOs()){
        // auto multiportBr = _dfg->multiportIObr();
        auto multiportStep = _dfg->multiportIOSteps();
        for(int step = 0; step < multiportStep[elem.first].size(); step++){
            // std::cout << "array: " << elem.first << " step: " << step << std::endl;
            multiportAvailLatModII[elem.first][step] = nums;
        }
    }
    std::set<int> scheduledNodeIds;  
    // std::vector<int> unscheduledNodes = _dfg->topoNodes();
    // std::reverse(unscheduledNodes.begin(), unscheduledNodes.end()); // in reversed topological order
    auto& topoNodes = _dfg->topoNodes();
    std::list<int> unscheduledNodes(topoNodes.rbegin(), topoNodes.rend());
    // calculate the routing latency of each edge, not inlcuding the RDU
    calEdgeRouteLat();
    // calculate the DFG node latency bounds, finding the max-latency path 
    latencyBound();
    // schedule the DFG nodes in the max-latency path
    DFGNode* dfgNode = _dfg->node(_maxLatNodeId);
    scheduledNodeIds.emplace(dfgNode->id());
    unscheduledNodes.remove(dfgNode->id());
    int cg_width = _dfg->CGWidth(); 
    // auto iterEnd = std::remove(unscheduledNodes.begin(), unscheduledNodes.end(), dfgNode->id());
    // std::cout << " topo size: " << _dfg->topoNodes().size() << " unsche size: " << unscheduledNodes.size() << " scheduledNodeIds size: " << scheduledNodeIds.size()<< std::endl;
    while(dfgNode){ // until getting to the input port
        int nodeId = dfgNode->id();     
        // std::cout << "critical path node name: "<< dfgNode->name()<<" id: " << nodeId  << std::endl;
        // use the latency lower bound as the target latency
        int inPortLat = _dfgNodeAttr[nodeId].lat - _dfg->node(nodeId)->opLatency(); // input port latency
        FUNode* fuNode = dynamic_cast<FUNode*>(_dfgNodeAttr[nodeId].adgNode); // mapped FU node (GPE/IOB)
        DFGNode* srcNode = nullptr;
        int inPort = 0;
        int bitwidth = 32;
        bool pathGet = false;
        int delayRequired = 0;
        int maxArriveLat = -1;
        bool postrm = true;
        for(auto& elem : dfgNode->inputEdges()){
            bitwidth = elem.first;
            for(auto& edge : elem.second){
                int eid = edge.second;
                DFGEdge* e = _dfg->edge(eid);// @yuan: don't touch the edge that is the end of loop
                if(e->isBackEdge() || e->isMemEdge()){
                    continue;
                }
                int routeLat = _dfgEdgeAttr[eid].lat  + e->logicLat() * _II; // latNoDelay + logiclat;
                int srcNodeId = _dfg->edge(eid)->srcId();
                int srcNodeLat = _dfgNodeAttr[srcNodeId].lat;         
                int arriveLat = srcNodeLat + routeLat;            
                if(inPortLat == arriveLat){ 
                    delayRequired = 0;
                    scheduledNodeIds.emplace(srcNodeId); // latency fixed
                    // std::cout << " src node: " << _dfg->node(srcNodeId)->name() << " latency fixed"  << " srcNodeLat: " << srcNodeLat << " routeLat: " << routeLat<< std::endl;
                    srcNode = _dfg->node(srcNodeId);
                    unscheduledNodes.remove(srcNodeId);
                    inPort = edge.first;
                    // iterEnd = std::remove(unscheduledNodes.begin(), iterEnd, srcNodeId);
                    // unscheduledNodes.eremoverase(std::remove(unscheduledNodes.begin(), unscheduledNodes.end(), srcNode), unscheduledNodes.end());
                    postrm = false;
                    pathGet = true;
                    break; // only find one path
                }else if(arriveLat > maxArriveLat){
                    maxArriveLat = arriveLat;
                    delayRequired = inPortLat - arriveLat;
                    srcNode = _dfg->node(srcNodeId);
                    inPort = elem.first;
                } 
            }
            if(pathGet) break;
        }    
        // should consider the fine-grain input index of alu and lut
        int type = mappedFUType(dfgNode);
        if(type == TYPE_LUT){ // operand index of LUT in GPE may not start from 0; ALU and IOC do.
            inPort = dynamic_cast<GPENode*>(fuNode)->getOperandIdxLUT(inPort);
        }else if( type == TYPE_ALU && bitwidth == 1){// for the fine-grain input of alu, its index start from 0
            // int cg_width = 32;
            // for(auto& width : dfgNode->bitWidths()){
            //     if(width != 1){
            //         cg_width = width;
            //         break;
            //     }
            // }
            if(dfgNode->accumulative()){
                int opereandNumCG = dfgNode->numInputs(cg_width);
                inPort = dynamic_cast<GPENode*>(fuNode)->getOperandIdxFg(opereandNumCG, inPort);
            }else if(dfgNode->operation() != "SEXT" && dfgNode->operation() != "ZEXT"){
                inPort = inPort - dynamic_cast<GPENode*>(fuNode)->numOperands(cg_width);    
                if(!dfgNode->isPacked() && dynamic_cast<GPENode*>(fuNode)->isMIMD()){//@hw_yuan: MIMD-ALU has 4 caose-grained inputs
                    inPort += 2;
                }   
            }
        }else if (type == TYPE_IOC && bitwidth == 1){//@yuan: for the fine-grain input of IOB, its index start from 0
            // int cg_width = 32;
            // for(auto& width : dfgNode->bitWidths()){
            //     if(width != 1){
            //         cg_width = width;
            //         break;
            //     }
            // }
            int opereandNumCG = dfgNode->numInputs(cg_width);
            inPort = inPort - opereandNumCG;
            auto dstOp = dfgNode->operation();
            if(dstOp == "TLOAD" || dstOp == "TSTORE"){
                inPort = dynamic_cast<IOBNode*>(fuNode)->getOperandIdxIOB(inPort);
            }

        }
        // if the dfgnode has both coarse and fine grain inputs, the min latency should consider each grain RDU
        int minAvailDelay;
        if(dfgNode->inputEdges().empty()){ //@yuan: maybe there are some situations: the top node of critical path is input/load, it doesn't have input edges, which will cause an error in getAvailDelay()
            minAvailDelay = 0;
            for(auto& width : dfgNode->bitWidths() ){
                minAvailDelay = std::max(fuNode->maxDelay(width), minAvailDelay);
            }    
        }else{
            minAvailDelay = INT32_MAX;
            for(auto& width : dfgNode->bitWidths() ){
                if(!dfgNode->inputEdges().count(width)) continue;
                minAvailDelay = std::min(getAvailDelay(fuNode, dfgNode, width), minAvailDelay);
            }
        }    
        /*if(dfgNode->bitWidths().size() > 1){
            for(auto& width : dfgNode->bitWidths()){
                minAvailDelay = std::min(getAvailDelay(fuNode, dfgNode, width), minAvailDelay);
            }
        }else{
            int bitwidth = *(dfgNode->bitWidths().begin());
            minAvailDelay = getAvailDelay(fuNode, dfgNode, bitwidth);
        }*/
        _fuDelayAttr[fuNode->id()][bitwidth].delayUsed[inPort] = std::min(delayRequired, minAvailDelay);
        _dfgNodeAttr[nodeId].maxLat = inPortLat; // input port max latency 
        _dfgNodeAttr[nodeId].minLat = std::max(inPortLat - minAvailDelay, 0); // input port min latency >= IOB Input latency
        // std::cout << "critical path node name: "<< dfgNode->name()<<" id: " << nodeId  << " Lat: " << _dfgNodeAttr[nodeId].lat << " maxLat: " << inPortLat << " minLat: "<<std::max(inPortLat - minAvailDelay, 0)<< std::endl;   
        dfgNode = srcNode;
        if(postrm && srcNode != nullptr){
            scheduledNodeIds.emplace(srcNode->id()); // latency fixed
            unscheduledNodes.remove(srcNode->id());
            // iterEnd = std::remove(unscheduledNodes.begin(), iterEnd, srcNode->id());
        }
        
    }
    // unscheduledNodes.erase(iterEnd, unscheduledNodes.end());
    // std::cout << " schedule critical path done!!  unscheduledNodes size: " << unscheduledNodes.size() << " scheduledNodeIds size: " <<scheduledNodeIds.size()<< std::endl;
    // schedule the DFG nodes not in the max-latency path
    while(!unscheduledNodes.empty()){
        // std::cout << "unscheduledNodes size: " << unscheduledNodes.size() << std::endl;
        for(auto iter = unscheduledNodes.begin(); iter != unscheduledNodes.end();){           
            int nodeId = *iter;
            dfgNode = _dfg->node(nodeId);
            // std::cout << "unschedule node: " << dfgNode->name() << " id: " << nodeId << std::endl;
            // std::cout << "id: " << nodeId << std::endl;
            int maxLat = 0x3fffffff;
            int minLat = 0;
            int targetLat = _dfgNodeAttr[nodeId].lat; 
            bool fail = false; // fail to schedule this DFG node
            bool updated = false; // maxLat/minLat UPDATED
            for(auto& elem : dfgNode->outputEdges()){
                int bitwidth = elem.first;
                for(auto& outsPerPort : elem.second){
                    for(auto& eid : outsPerPort.second){
                        DFGEdge* e = _dfg->edge(eid);
                        if(e->isMemEdge()){ // don't touch memory-dependent edge
                            continue;
                        }
                        int dstNodeId = e->dstId();
                        //@yuan: don't touch the back-edges with long iteration distance, only for Memory Node
                        if(bitwidth == 1 && e->isBackEdge()){
                            int dstNodeLat = _dfgNodeAttr[dstNodeId].lat;
                            int edgeIterDist = e->iterDist();
                            if((targetLat > dstNodeLat)&&(floor(1.5* (targetLat - dstNodeLat)) <  edgeIterDist)){
                            // if((targetLat > dstNodeLat)&&(((targetLat - dstNodeLat)/edgeIterDist) < _II)){// it means the current II can satisfy the dependency
                                continue;
                            }
                        }
                        // if(e->isMemBackEdge()){// not need to make sync for memory-dependent back-edge // @yuan: maybe it should be synchronized
                        //     continue;
                        // }
                        int routeLat = _dfgEdgeAttr[eid].lat + e->logicLat() * _II; // latNoDelay;
                        // if(dstNodeId == _dfg->id()){ // connected to DFG output port
                        //     continue;
                        // }
                        // std::cout << "output node: " << _dfg->node(dstNodeId)->name() << std::endl;
                        if(scheduledNodeIds.count(dstNodeId)){ // already scheduled  
                            if(e->isBackEdge()){ 
                                routeLat -= _II * e->iterDist(); // latency due to iteration distance
                                if(_dfgNodeAttr[dstNodeId].maxLat - routeLat < targetLat){ // need to increase II
                                    continue;
                                }
                            }                      
                            maxLat = std::min(maxLat, _dfgNodeAttr[dstNodeId].maxLat - routeLat);
                            minLat = std::max(minLat, _dfgNodeAttr[dstNodeId].minLat - routeLat);
                            updated = true;
                        }else{
                            if(e->isBackEdge()) {
                                updated = false;
                                continue;
                            }
                            fail = true;
                            break;
                        }
                    }
                    if(fail) break;
                }    
                if(fail) break;
            }
            if(fail){
                iter++;
                continue;
            } 
            // if all its output ports are connected to DFG output ports, keep original latency
            // otherwise, update the latency
            // int targetLat = _dfgNodeAttr[nodeId].lat; 
            // std::cout << " initial lat: " << targetLat << std::endl;
            //@yuan: the initial target latency is caculated by node-op-latency and routed-latency
            if(updated){
                targetLat = std::max(targetLat, std::min(maxLat, minLat));
                //_dfgNodeAttr[nodeId].lat = targetLat;
            }   
            // adjust the latency of the multiport IO node to avoid spad access conflict
            if(_dfg->isMultiportIoNode(nodeId)){
                // if(dynamic_cast<DFGIONode*>(dfgNode)->MultiportType() != 2){
                //     std::string name = dynamic_cast<DFGIONode*>(dfgNode)->memRefName();
                //     int nodeBr = dynamic_cast<DFGIONode*>(dfgNode)->Branch();
                //     int tune_times = 1;
                //     while((!multiportAvailLatModII[name][nodeBr].count((targetLat - dfgNode->opLatency())%_II)) || !isTimeAvai(dfgNode, targetLat - dfgNode->opLatency())){
                //         targetLat--;
                //         tune_times++;
                //         // std::cout << "targetLat: " << targetLat << std::endl;
                //         if(tune_times > _II) break;
                //     }
                //     multiportAvailLatModII[name][nodeBr].erase((targetLat - dfgNode->opLatency())%_II);
                // }
                if(dynamic_cast<DFGIONode*>(dfgNode)->MultiportType() > 0){ 
                    std::string name = dynamic_cast<DFGIONode*>(dfgNode)->memRefName();
                    // int nodeBr = dynamic_cast<DFGIONode*>(node)->Branch();
                    int nodeStep = dynamic_cast<DFGIONode*>(dfgNode)->ControlStep(); //@yuan: return the control step
                    int tune_times = 1;
                    int enable_latency = _dfg->getOutNodes().count(nodeId) ? 0 : dfgNode->opLatency(); //@yuan: we need to ensure the the enable signal of load and store can be aligned
                    for(; tune_times <= _II; tune_times++){//@yuan: tuning time over II will back to the start since "% II"
                        if(!isTimeAvai(dfgNode, targetLat - enable_latency)){//@yuan: first, the latency can not satisfy the dependency
                            targetLat--;
                            tune_times++;
                            continue;
                        }
                        for(auto& elem: multiportAvailLatModII[name]){//@yuan: if the latency satisfy the dependency, we should make sure it can not be occupied by other stpes
                            if(elem.first == nodeStep) continue;//@yuan: the time slot of this step can not overlap with the slot in other steps
                            if(!elem.second.count((targetLat - enable_latency)%_II)){
                                targetLat--;
                                tune_times++;
                                break;
                            }
                        }
                    }
                    multiportAvailLatModII[name][nodeStep].erase((targetLat - enable_latency)%_II);
                }
            }else if(_dfg->isIONode(nodeId)){
                int enable_latency = _dfg->getOutNodes().count(nodeId) ? 0 : dfgNode->opLatency(); //@yuan: we need to ensure the the enable signal of load and store can be aligned
                while((!isTimeAvai(dfgNode, targetLat - enable_latency))){ // @yuan: find the available time slot
                    targetLat--;
                }
            }  
            _dfgNodeAttr[nodeId].lat = targetLat;         
            // update             
            int maxInportLat = targetLat - _dfg->node(nodeId)->opLatency(); // input port max latency   
            _dfgNodeAttr[nodeId].maxLat = maxInportLat;   
            scheduledNodeIds.emplace(nodeId); // latency fixed
            iter = unscheduledNodes.erase(iter);
        // std::cout << "unscheduledNodes size: " << unscheduledNodes.size() << std::endl;
            // update the delay status and minLat of the this node if its srcNode already scheduled
            FUNode* fuNode = dynamic_cast<FUNode*>(_dfgNodeAttr[nodeId].adgNode); // mapped FU node (GPE/IOB)
            int minAvailDelay = INT_MAX;
            // std::cout << " after update!!" << std::endl;
            for(auto& elem : dfgNode->inputEdges()){
                int bitwidth = elem.first;
                for(auto& edge : elem.second){
                    int eid = edge.second;
                    DFGEdge* e = _dfg->edge(eid);// 
                    if(e->isMemEdge()){
                        continue;
                    }
                    //@yuan: don't touch the back-edges with long iteration distance
                    if(bitwidth == 1 && e->isBackEdge()){
                        int dstNodeLat = _dfgNodeAttr[e->dstId()].lat;
                        int srcNodeLat = _dfgNodeAttr[e->srcId()].lat;
                        int edgeIterDist = e->iterDist();
                        // std::cout << "input edge iterdist: " << edgeIterDist << " srcNode: "<<_dfg->node(e->srcId())->name()<<" srclat: " << srcNodeLat << " dstNode:" << dfgNode->name()<< " dstlat: " << dstNodeLat << std::endl;
                        if((srcNodeLat > dstNodeLat)&&(floor(1.5* (srcNodeLat - dstNodeLat)) <  edgeIterDist)){
                        // if((srcNodeLat > dstNodeLat)&&(((srcNodeLat - dstNodeLat) / edgeIterDist) < _II)){
                            continue;
                        }
                    }
                    int routeLat = _dfgEdgeAttr[eid].lat + e->logicLat() * _II; // latNoDelay;
                    if(e->isBackEdge()){
                        // if(e->isMemEdge()){ // not need to make sync for memory-dependent back-edge// @yuan: maybe it should be synchronized
                        //     continue;
                        // } 
                        routeLat -= _II * e->iterDist(); // latency due to iteration distance
                    }
                    int inPort = edge.first;               
                    int srcNodeId = _dfg->edge(eid)->srcId();
                    // std::cout << " unschedule node src node: " << _dfg->node(srcNodeId)->name() << std::endl; 
                    if(scheduledNodeIds.count(srcNodeId)){ // already scheduled   
                        // should consider the fine-grain input index of alu and lut
                        int type = mappedFUType(dfgNode);
                        if(type == TYPE_LUT){ // operand index of LUT in GPE may not start from 0; ALU and IOC do.
                            inPort = dynamic_cast<GPENode*>(fuNode)->getOperandIdxLUT(inPort);
                        }else if( type == TYPE_ALU && bitwidth == 1){// for the fine-grain input of alu, its index start from 0
                            // int cg_width = 32;
                            // for(auto& width : dfgNode->bitWidths()){
                            //     if(width != 1){
                            //         cg_width = width;
                            //         break;
                            //     }
                            // }
                            if(dfgNode->accumulative()){
                                int opereandNumCG = dfgNode->numInputs(cg_width);
                                inPort = dynamic_cast<GPENode*>(fuNode)->getOperandIdxFg(opereandNumCG, inPort);
                            }else if(dfgNode->operation() != "SEXT" && dfgNode->operation() != "ZEXT"){
                                inPort = inPort - dynamic_cast<GPENode*>(fuNode)->numOperands(cg_width);   
                                if(!dfgNode->isPacked() && dynamic_cast<GPENode*>(fuNode)->isMIMD()){//@hw_yuan: MIMD-ALU has 4 caose-grained inputs
                                    inPort += 2;
                                }   
                            }
                            // inPort = inPort - dynamic_cast<GPENode*>(fuNode)->numOperands(cg_width);
                        }else if (type == TYPE_IOC && bitwidth == 1){//@yuan: for the fine-grain input of IOB, its index start from 0
                            // int cg_width = 32;
                            // for(auto& width : dfgNode->bitWidths()){
                            //     if(width != 1){
                            //         cg_width = width;
                            //         break;
                            //     }
                            // }
                            int opereandNumCG = dfgNode->numInputs(cg_width);
                            inPort = inPort - opereandNumCG;
                            auto dstOp = dfgNode->operation();
                            if(dstOp == "TLOAD" || dstOp == "TSTORE"){
                                inPort = dynamic_cast<IOBNode*>(fuNode)->getOperandIdxIOB(inPort);
                            }
                        }
                        int srcNodeLat = _dfgNodeAttr[srcNodeId].lat;     
                        int delayRequired = std::max(maxInportLat - routeLat - srcNodeLat, 0);   
                        int delayUsed = std::min(delayRequired, getAvailDelay(fuNode, dfgNode, bitwidth));         
                        _fuDelayAttr[fuNode->id()][bitwidth].delayUsed[inPort] = delayUsed;
                        _fuDelayAttr[fuNode->id()][bitwidth].totalDelayUsed += delayUsed;
                        // std::cout << dfgNode->name() << " fuid: " << fuNode->id() << " port: " << inPort << " set delay: " << delayUsed <<" bitwidth: "<<bitwidth<< std::endl;
                    }
                }   
                minAvailDelay = std::min(getAvailDelay(fuNode, dfgNode, bitwidth), minAvailDelay);
            }
            _dfgNodeAttr[nodeId].minLat = std::max(maxInportLat - minAvailDelay, 0); // input port min latency         
            // update the delay status and minLat of the dstNodes
            for(auto& elem : dfgNode->outputEdges()){
                int bitwidth = elem.first;
                for(auto& outsPerPort : elem.second){
                    for(auto& eid : outsPerPort.second){
                        DFGEdge* e = _dfg->edge(eid);// @yuan: don't touch the edge that is the end of loop
                        if(e->isMemEdge()){
                            continue;
                        }
                        //@yuan: don't touch the back-edges with long iteration distance
                        if(bitwidth == 1 && e->isBackEdge()){
                            int dstNodeLat = _dfgNodeAttr[e->dstId()].lat;
                            int srcNodeLat = _dfgNodeAttr[e->srcId()].lat;
                            int edgeIterDist = e->iterDist();
                            if((srcNodeLat > dstNodeLat)&&(floor(1.5* (srcNodeLat - dstNodeLat)) <  edgeIterDist)){
                            // if((srcNodeLat > dstNodeLat)&&(((srcNodeLat - dstNodeLat) / edgeIterDist) < _II)){
                                continue;
                            }
                        }
                        int routeLat = _dfgEdgeAttr[eid].lat + e->logicLat() * _II; // latNoDelay;
                        if(e->isBackEdge()){
                            // if(e->isMemEdge()){ // not need to make sync for memory-dependent back-edge // @yuan: maybe it should be synchronized
                            //     continue;
                            // }
                            routeLat -= _II * e->iterDist(); // latency due to iteration distance
                        }
                        int dstNodeId = _dfg->edge(eid)->dstId();
                        // if(dstNodeId == _dfg->id()){ // connected to DFG output port
                        //     continue;
                        // }
                        DFGNode* dstNode = _dfg->node(dstNodeId);
                        int inPort = _dfg->edge(eid)->dstPortIdx();  
                        int delayRequired = std::max(_dfgNodeAttr[dstNodeId].maxLat - routeLat - targetLat, 0);      
                        // std::cout << " unschedule node dst node: " << dstNode->name() <<" delay_require: "<<delayRequired<< std::endl;                             
                        FUNode* dstFuNode = dynamic_cast<FUNode*>(_dfgNodeAttr[dstNodeId].adgNode); // mapped FU node
                        // should consider the fine-grain input index of alu and lut
                        int type = mappedFUType(dstNode);
                        if(type == TYPE_LUT){ // operand index of LUT in GPE may not start from 0; ALU and IOC do.
                            inPort = dynamic_cast<GPENode*>(dstFuNode)->getOperandIdxLUT(inPort);
                        }else if( type == TYPE_ALU && bitwidth == 1){// for the fine-grain input of alu, its index start from 0
                            // int cg_width = 32;
                            // for(auto& width : dstNode->bitWidths()){
                            //     if(width != 1){
                            //         cg_width = width;
                            //         break;
                            //     }
                            // }
                            if(dstNode->accumulative()){
                                int opereandNumCG = dstNode->numInputs(cg_width);
                                inPort = dynamic_cast<GPENode*>(dstFuNode)->getOperandIdxFg(opereandNumCG, inPort);
                            }else if(dstNode->operation() != "SEXT" && dstNode->operation() != "ZEXT"){
                                inPort = inPort - dynamic_cast<GPENode*>(dstFuNode)->numOperands(cg_width);   
                                if(!dstNode->isPacked() && dynamic_cast<GPENode*>(dstFuNode)->isMIMD()){//@hw_yuan: MIMD-ALU has 4 caose-grained inputs
                                    inPort += 2;
                                }  
                            }
                            // inPort = inPort - dynamic_cast<GPENode*>(dstFuNode)->numOperands(cg_width);
                        }else if (type == TYPE_IOC && bitwidth == 1){//@yuan: for the fine-grain input of IOB, its index start from 0
                            // int cg_width = 32;
                            // for(auto& width : dstNode->bitWidths()){
                            //     if(width != 1){
                            //         cg_width = width;
                            //         break;
                            //     }
                            // }
                            int opereandNumCG = dstNode->numInputs(cg_width);
                            inPort = inPort - opereandNumCG;
                            auto dstOp = dstNode->operation();
                            if(dstOp == "TLOAD" || dstOp == "TSTORE"){
                                inPort = dynamic_cast<IOBNode*>(dstFuNode)->getOperandIdxIOB(inPort);
                            }
                        }
                        int delayUsed = std::min(delayRequired, getAvailDelay(dstFuNode, dstNode, bitwidth));  
                        _fuDelayAttr[dstFuNode->id()][bitwidth].delayUsed[inPort] = delayUsed;
                        _fuDelayAttr[dstFuNode->id()][bitwidth].totalDelayUsed += delayUsed;
                        // std::cout << dstNode->name() << " fuid: " << dstFuNode->id() << " port: " << inPort << " set delay: " << delayUsed <<" bitwidth: "<<bitwidth<< std::endl;
                        int minDelay = INT32_MAX; 
                        for(auto& width : dstNode->bitWidths()){
                            if(!dstNode->inputEdges().count(width)) continue;
                            minDelay = std::min(getAvailDelay(dstFuNode, dstNode, width), minDelay);
                        }
                        _dfgNodeAttr[dstNodeId].minLat = std::max(_dfgNodeAttr[dstNodeId].maxLat - minDelay, 0); // input port min latency                     
                    }
                }    
            }
        }
    }
    // for(auto nodeId : _dfg->topoNodes()){
    //     DFGNode *node = _dfg->node(nodeId);
    //     //std::cout << " node: " << node->name() << " id: " << node->id() << std::endl;
    //     for(auto& elem : node->inputEdges()){
    //         //std::cout << " in edge bit : " << elem.first << " size: " << elem.second.size() << std::endl;
    //         for(auto& edge: elem.second){
    //             int eid = edge.second;
    //             int srcNodeId = _dfg->edge(eid)->srcId();
    //             DFGNode *srcnode = _dfg->node(srcNodeId);
    //             int routedelay = _dfgEdgeAttr[eid].lat;
    //             //std::cout << " edge from: " << srcnode->name() << " to: " << node->name() << " route delay: " << routedelay<< std::endl;
    //         }
    //     }
    //     // for(auto& elem : node->outputEdges()){
    //     //     std::cout << " out edge bit : " << elem.first << " size: " << elem.second.size() << std::endl;
            
    //     // }
    //     // std::cout << "node: " << node->name() << " maxlat: " <<  _dfgNodeAttr[nodeId].maxLat << " and minlat: " <<  _dfgNodeAttr[nodeId].minLat << std::endl;
    // }
    // std::cout << "begin calculating edge latency violation!!" << std::endl;
    // calculate the latency violation of each edge
    calEdgeLatVio();
    // std::cout << "Finish calculating edge latency violation!!" << std::endl;
}


// calculate the latency violation of each edge
void Mapping::calEdgeLatVio(){
    int dfgSumVio = 0; // total edge latency violation
    int dfgMaxVio = 0; // max edge latency violation
    _vioDfgEdges.clear(); // DFG edges with latency violation
    int cg_width = _dfg->CGWidth();
    // std::cout << "before calculate edge violation toponodes size: " << _dfg->topoNodes().size() << std::endl; 
    for(auto nodeId : _dfg->topoNodes()){
        DFGNode *node = _dfg->node(nodeId);
        // int minLat = _dfgNodeAttr[nodeId].minLat; // min latency of the input ports
        int maxLat = _dfgNodeAttr[nodeId].maxLat; // max latency of the input ports =  latency - operation_latency
        int Nodelat = _dfgNodeAttr[nodeId].lat;
        FUNode* fuNode = dynamic_cast<FUNode*>(_dfgNodeAttr[nodeId].adgNode);
        // std::cout << "calEdge node: " << node->name() << " maxLat: " << maxLat<< " GPE id: "<< fuNode->id()<<std::endl;
        int fuNodeId = fuNode->id();
        for(auto& elem : node->inputEdges()){
            int bitwidth = elem.first;
            auto& fuDelayAttr = _fuDelayAttr[fuNodeId][bitwidth];
            auto& delayUsed = fuDelayAttr.delayUsed;
            for(auto& edge : elem.second){
                int eid = edge.second;
                DFGEdge* e = _dfg->edge(eid);// @yuan: don't touch the edge that is the end of loop
                if(e->isMemEdge()){
                    continue;
                }
                //@yuan: don't touch the back-edges with long iteration distance, only for memory node
                if(e->isBackEdge()){
                    int dstNodeLat = _dfgNodeAttr[e->dstId()].lat;
                    int srcNodeLat = _dfgNodeAttr[e->srcId()].lat;
                    int edgeIterDist = e->iterDist();
                    if((srcNodeLat > dstNodeLat) && e->bitWidth() == 1 && floor(1.5* (srcNodeLat - dstNodeLat)) <  edgeIterDist){
                    // if((srcNodeLat > dstNodeLat) && e->bitWidth() == 1 && ((srcNodeLat - dstNodeLat) / edgeIterDist < _II) ){
                        e->setDontTouch(true);
                        continue;
                    }
                }
                int routeLat = _dfgEdgeAttr[eid].lat + e->logicLat() * _II; // latNoDelay;
                if(e->isBackEdge()){
                    // if(e->isMemEdge()){ // not need to make sync for memory-dependent back-edge // @yuan: maybe it should be synchronized
                    //     _dfgEdgeAttr[eid].vio = 0;
                    //     _dfgEdgeAttr[eid].delay = 0;
                    //     continue;
                    // } 
                    routeLat -= _II * e->iterDist(); // latency due to iteration distance
                }
                int srcNodeId = _dfg->edge(eid)->srcId();
                // int srcNodeLat;
                // if(srcNodeId != _dfg->id()){ // not connected to DFG input port
                int srcNodeLat = _dfgNodeAttr[srcNodeId].lat;       
                if(srcNodeLat == Nodelat){//@yuan: fix the bug that srclatency = dstlatency
                    // std::cout << node->name() << " src lat: " << srcNodeLat << " self lat: "<< Nodelat<<std::endl;
                    _dfgEdgeAttr[eid].vio = 0;
                    _vioDfgEdges.push_back(eid);
                    dfgSumVio += 1;
                    _totalViolation = dfgSumVio;
                    return;
                }                 
                // } else{ // connected to DFG input port
                //     srcNodeLat = _dfgInputAttr[_dfg->edge(eid)->srcPortIdx()].lat;  
                // }
                // _dfgEdgeAttr[eid].lat = maxLat - srcNodeLat; // including RDU latency   
                //@yuan: for the test, delete when the code finishing
                DFGNode *srcnode = _dfg->node(srcNodeId);
                // std::cout << " edge from: " << srcnode->name() << " to: " << node->name() << " port: " << edge.first <<" width: " << bitwidth << std::endl;
                // std::cout << "maxLat: " <<maxLat<<" srcNodeLat: " << srcNodeLat << " routeLat: " << routeLat << std::endl;
                //@yuan
                int requiredDelay = maxLat - srcNodeLat - routeLat; // RDU latency  
                // std::cout << "requiredDelay: " << requiredDelay << std::endl;
                if(requiredDelay < 0){
                    _dfgEdgeAttr[eid].vio = requiredDelay;
                    _vioDfgEdges.push_back(eid);
                    dfgSumVio += std::abs(requiredDelay);
                    _totalViolation = dfgSumVio;
                    // std::cout << "return~~~~~~~~~~~~~~~~~~" << std::endl;
                    return; // need to increase II
                } 
                // //@yuan: for the test, delete when the code finishing
                // DFGNode *srcnode = _dfg->node(srcNodeId);
                // std::cout << " edge from: " << srcnode->name() << " to: " << node->name() << " port: " << edge.first << std::endl;
                // std::cout << "srcNodeLat: " << srcNodeLat << " routeLat: " << routeLat << std::endl;
                //@yuan
                _dfgEdgeAttr[eid].delay = requiredDelay;    
                int assignedDelay; 
                int inport = edge.first;
                // should consider the fine-grain input index of alu and lut
                int type = mappedFUType(node);
                if(type == TYPE_LUT){ // operand index of LUT in GPE may not start from 0; ALU and IOC do.
                    // std::cout << "lut~~~~~~~~~~~~~"<<std::endl;
                    inport = dynamic_cast<GPENode*>(fuNode)->getOperandIdxLUT(inport);
                }else if( type == TYPE_ALU && bitwidth == 1){// for the fine-grain input of alu, its index start from 0
                    // int cg_width = 32;
                    // for(auto& width : node->bitWidths()){
                    //     if(width != 1){
                    //         cg_width = width;
                    //         break;
                    //     }
                    // }
                    if(node->accumulative()){
                        int opereandNumCG = node->numInputs(cg_width);
                        inport = dynamic_cast<GPENode*>(fuNode)->getOperandIdxFg(opereandNumCG, inport);
                    }else if(node->operation() != "SEXT" && node->operation() != "ZEXT"){
                        inport = inport - dynamic_cast<GPENode*>(fuNode)->numOperands(cg_width);   
                        if(!node->isPacked() && dynamic_cast<GPENode*>(fuNode)->isMIMD()){//@hw_yuan: MIMD-ALU has 4 caose-grained inputs
                            inport += 2;
                        } 
                    }
                    // inport = inport - dynamic_cast<GPENode*>(fuNode)->numOperands(cg_width);
                }else if (type == TYPE_IOC && bitwidth == 1){//@yuan: for the fine-grain input of IOB, its index start from 0
                    // int cg_width = 32;
                    // for(auto& width : node->bitWidths()){
                    //     if(width != 1){
                    //         cg_width = width;
                    //         break;
                    //     }
                    // }
                    int opereandNumCG = node->numInputs(cg_width);
                    inport = inport - opereandNumCG;
                    auto dstOp = node->operation();
                    if(dstOp == "TLOAD" || dstOp == "TSTORE"){
                        inport = dynamic_cast<IOBNode*>(fuNode)->getOperandIdxIOB(inport);
                    }
                }
                if(!delayUsed.count(inport)){ // not assign delay for this port
                    assignedDelay = std::min(getAvailDelay(fuNode, node, bitwidth), requiredDelay);
                    // std::cout << " not~ assignedDelay: " << assignedDelay << " inport: "<<inport<<std::endl;
                    delayUsed[inport] = assignedDelay;
                    fuDelayAttr.totalDelayUsed += assignedDelay;
                    _dfgNodeAttr[nodeId].minLat = std::max(_dfgNodeAttr[nodeId].maxLat - getAvailDelay(fuNode, node, bitwidth), 0); // input port min latency 
                }else{
                    assignedDelay = delayUsed[inport];
                    // std::cout << " assignedDelay: " << assignedDelay << " inport: "<<inport<<std::endl;
                }        
                // std::cout << " assignedDelay: " << assignedDelay << " requiredDelay: "<<requiredDelay <<std::endl; 
                assert(assignedDelay <= requiredDelay && assignedDelay >= 0); 
                if(assignedDelay < requiredDelay){ // need to add pass node to compensate the latency gap
                    int vio = requiredDelay - assignedDelay;
                    _dfgEdgeAttr[eid].vio = vio;
                    _vioDfgEdges.push_back(eid);
                    dfgSumVio += vio;
                    dfgMaxVio = std::max(dfgMaxVio, vio);
                } else {
                    _dfgEdgeAttr[eid].vio = 0;
                }
            }  
        }      
    }
    _totalViolation = dfgSumVio;
    _maxViolation = dfgMaxVio;

}

// // schedule the latency of each DFG node based on the mapping status
// // DFG node latency: output port latency
// // DFG edge latency: latency from the output port of src node to the FU Input port of dst Node, including DelayPipe 
// void Mapping::latencySchedule(){
//     std::set<int> scheduledNodeIds;  
//     std::vector<DFGNode*> unscheduledNodes = _dfg->topoNodes();
//     std::reverse(unscheduledNodes.begin(), unscheduledNodes.end()); // in reversed topological order
//     // calculate the routing latency of each edge, not inlcuding the delay pipe
//     calEdgeRouteLat();
//     // calculate the DFG node latency bounds, finding the max-latency path 
//     latencyBound();
//     // schedule the DFG nodes in the max-latency path
//     DFGNode* dfgNode = _dfg->node(_maxLatNodeId);
//     scheduledNodeIds.emplace(dfgNode->id());
//     auto iterEnd = std::remove(unscheduledNodes.begin(), unscheduledNodes.end(), dfgNode);
//     // unscheduledNodes.erase(std::remove(unscheduledNodes.begin(), unscheduledNodes.end(), dfgNode), unscheduledNodes.end());
//     int inputLat = _adg->loadLatency(); // IOB input latency
//     while(dfgNode){ // until getting to the input port
//         int nodeId = dfgNode->id();     
//         // std::cout << "id: " << nodeId << std::endl;   
//         // use the latency lower bound as the target latency
//         int inPortLat = _dfgNodeAttr[nodeId].lat - _dfg->node(nodeId)->opLatency(); // input port latency
//         GPENode* gpeNode = dynamic_cast<GPENode*>(_dfgNodeAttr[nodeId].adgNode); // mapped GPE node
//         _dfgNodeAttr[nodeId].maxLat = inPortLat; // input port max latency 
//         _dfgNodeAttr[nodeId].minLat = std::max(inPortLat - gpeNode->maxDelay(), inputLat); // input port min latency >= IOB Input latency 
//         DFGNode* srcNode = nullptr;
//         for(auto& elem : dfgNode->inputEdges()){
//             int eid = elem.second;
//             int routeLat = _dfgEdgeAttr[eid].lat; // latNoDelay;
//             int srcNodeId = _dfg->edge(eid)->srcId();
//             if(srcNodeId == _dfg->id()){ // connected to DFG input port
//                 continue;               
//             }
//             int srcNodeLat = _dfgNodeAttr[srcNodeId].lat;             
//             if(inPortLat == srcNodeLat + routeLat){ 
//                 scheduledNodeIds.emplace(srcNodeId); // latency fixed
//                 srcNode = _dfg->node(srcNodeId);
//                 iterEnd = std::remove(unscheduledNodes.begin(), iterEnd, srcNode);
//                 // unscheduledNodes.erase(std::remove(unscheduledNodes.begin(), unscheduledNodes.end(), srcNode), unscheduledNodes.end());
//                 break; // only find one path
//             } 
//         }
//         // for(auto& elem : dfgNode->inputs()){
//         //     int srcNodeId = elem.second.first;
//         //     if(srcNodeId == _dfg->id()){ // connected to DFG input port
//         //         continue;               
//         //     }
//         //     int srcNodeLat = _dfgNodeAttr[srcNodeId].lat;             
//         //     if(inPortLat == srcNodeLat){ // edge no latency
//         //         scheduledNodeIds.emplace(srcNodeId); // latency fixed
//         //         srcNode = _dfg->node(srcNodeId);
//         //         iterEnd = std::remove(unscheduledNodes.begin(), iterEnd, srcNode);
//         //         // unscheduledNodes.erase(std::remove(unscheduledNodes.begin(), unscheduledNodes.end(), srcNode), unscheduledNodes.end());
//         //         break; // only find one path
//         //     } 
//         // }
//         dfgNode = srcNode;
//     }
//     unscheduledNodes.erase(iterEnd, unscheduledNodes.end());
//     // schedule the DFG nodes not in the max-latency path
//     while(!unscheduledNodes.empty()){
//         for(auto iter = unscheduledNodes.begin(); iter != unscheduledNodes.end();){
//             dfgNode = *iter;
//             int nodeId = dfgNode->id();
//             // std::cout << "id: " << nodeId << std::endl;
//             int maxLat = 0x3fffffff;
//             int minLat = inputLat;
//             bool fail = false; // fail to schedule this DFG node
//             bool updated = false; // maxLat/minLat UPDATED
//             // schedule DFG node only if all its output nodes are already scheduled 
//             // for(auto& outsPerPort : dfgNode->outputs()){
//             //     for(auto& elem : outsPerPort.second){
//             //         int dstNodeId = elem.first;
//             //         if(dstNodeId == _dfg->id()){ // connected to DFG output port
//             //             continue;
//             //         }
//             //         if(scheduledNodeIds.count(dstNodeId)){ // already scheduled                     
//             //             maxLat = std::min(maxLat, _dfgNodeAttr[dstNodeId].maxLat);
//             //             minLat = std::max(minLat, _dfgNodeAttr[dstNodeId].minLat);
//             //             updated = true;
//             //         }else{
//             //             fail = true;
//             //             break;
//             //         }
//             //     }
//             //     if(fail) break;
//             // }
//             for(auto& outsPerPort : dfgNode->outputEdges()){
//                 for(auto& eid : outsPerPort.second){
//                     int routeLat = _dfgEdgeAttr[eid].lat; // latNoDelay;
//                     int dstNodeId = _dfg->edge(eid)->dstId();
//                     if(dstNodeId == _dfg->id()){ // connected to DFG output port
//                         continue;
//                     }
//                     if(scheduledNodeIds.count(dstNodeId)){ // already scheduled                     
//                         maxLat = std::min(maxLat, _dfgNodeAttr[dstNodeId].maxLat - routeLat);
//                         minLat = std::max(minLat, _dfgNodeAttr[dstNodeId].minLat - routeLat);
//                         updated = true;
//                     }else{
//                         fail = true;
//                         break;
//                     }
//                 }
//                 if(fail) break;
//             }
//             if(fail){
//                 iter++;
//                 continue;
//             } 
//             // if all its output ports are connected to DFG output ports, keep original latency
//             // otherwise, update the latency
//             int targetLat = _dfgNodeAttr[nodeId].lat; 
//             if(updated){
//                 targetLat = std::max(targetLat, std::min(maxLat, minLat));
//                 _dfgNodeAttr[nodeId].lat = targetLat;
//             }            
//             GPENode* gpeNode = dynamic_cast<GPENode*>(_dfgNodeAttr[nodeId].adgNode); // mapped GPE node
//             _dfgNodeAttr[nodeId].maxLat = targetLat - _dfg->node(nodeId)->opLatency(); // input port max latency 
//             _dfgNodeAttr[nodeId].minLat = std::max(_dfgNodeAttr[nodeId].maxLat - gpeNode->maxDelay(), inputLat); // input port min latency 
//             scheduledNodeIds.emplace(nodeId); // latency fixed
//             iter = unscheduledNodes.erase(iter);
//         }
//     }
//     // calculate the latency of DFG IO
//     calIOLat();
//     // calculate the latency violation of each edge
//     calEdgeLatVio();
// }


// // calculate the latency of DFG IO
// void Mapping::calIOLat(){
//     // DFG input port latency
//     int inputLat = _adg->loadLatency(); // IOB input latency
//     for(auto& insPerPort : _dfg->inputEdges()){
//         int minLat = inputLat;
//         int maxLat = 0x3fffffff;
//         for(auto& eid : insPerPort.second){
//             int routeLat = _dfgEdgeAttr[eid].lat; // latNoDelay;
//             int dstNodeId = _dfg->edge(eid)->dstId();
//             if(dstNodeId == _dfg->id()){ // connected to DFG output port
//                 continue;
//             }
//             minLat = std::max(minLat, _dfgNodeAttr[dstNodeId].minLat - routeLat);
//             maxLat = std::min(maxLat, _dfgNodeAttr[dstNodeId].maxLat - routeLat);
//         }
//         int targetLat = std::min(maxLat, minLat);
//         _dfgInputAttr[insPerPort.first].lat = targetLat;
//     }
//     int maxLat = 0;
//     // DFG output port latency
//     for(auto& elem : _dfg->outputEdges()){
//         int eid = elem.second;
//         int routeLat = _dfgEdgeAttr[eid].lat; // latNoDelay;
//         int srcNodeId = _dfg->edge(eid)->srcId();
//         int srcNodeLat;
//         if(srcNodeId == _dfg->id()){ // connected to DFG input port
//             srcNodeLat = _dfgInputAttr[_dfg->edge(eid)->srcPortIdx()].lat;               
//         } else{
//             srcNodeLat = _dfgNodeAttr[srcNodeId].lat;
//         }
//         int targetLat = srcNodeLat + routeLat;
//         _dfgOutputAttr[elem.first].lat = targetLat;
//         maxLat = std::max(maxLat, targetLat);
//     }
//     _maxLat = maxLat;
// }


// // calculate the latency violation of each edge
// void Mapping::calEdgeLatVio(){
//     int dfgSumVio = 0; // total edge latency violation
//     int dfgMaxVio = 0; // max edge latency violation
//     _vioDfgEdges.clear(); // DFG edges with latency violation
//     for(DFGNode* node : _dfg->topoNodes()){        
//         int nodeId = node->id();
//         int minLat = _dfgNodeAttr[nodeId].minLat; // min latency of the input ports
//         int maxLat = _dfgNodeAttr[nodeId].maxLat; // max latency of the input ports, =  latency - operation_latency
//         for(auto& elem : node->inputEdges()){
//             int eid = elem.second;
//             int routeLat = _dfgEdgeAttr[eid].lat; // latNoDelay;
//             int srcNodeId = _dfg->edge(eid)->srcId();
//             int srcNodeLat;
//             if(srcNodeId != _dfg->id()){ // not connected to DFG input port
//                 srcNodeLat = _dfgNodeAttr[srcNodeId].lat;                
//             } else{ // connected to DFG input port
//                 srcNodeLat = _dfgInputAttr[_dfg->edge(eid)->srcPortIdx()].lat;  
//             }
//             _dfgEdgeAttr[eid].lat = maxLat - srcNodeLat; // including delay pipe latency    
//             _dfgEdgeAttr[eid].delay = maxLat - srcNodeLat - routeLat; // delay pipe latency    
//             int inPortLat = srcNodeLat + routeLat;       
//             if(inPortLat < minLat){ // need to add pass node to compensate the latency gap
//                 int vio = minLat - inPortLat;
//                 _dfgEdgeAttr[eid].vio = vio;
//                 _vioDfgEdges.push_back(eid);
//                 dfgSumVio += vio;
//                 dfgMaxVio = std::max(dfgMaxVio, vio);
//             } else {
//                 _dfgEdgeAttr[eid].vio = 0;
//             }
//         }    
//     }
//     _totalViolation = dfgSumVio;
//     _maxViolation = dfgMaxVio;
// }


// // insert pass-through DFG nodes into a copy of current DFG
// @yuan: for now, only insert pass-through nodes on the coarse-grain edges;
// as to fine-grained edges, we insert an and-gate (transfer to lut-2)
void Mapping::insertPassDfgNodes(DFG* newDfg, bool CGOnly){
    *newDfg = *_dfg;
    int maxNodeId = newDfg->nodes().rbegin()->first; // std::map auto sort the key
    int maxEdgeId = newDfg->edges().rbegin()->first; 
    // std::cout << "maxEdgeId: " << maxEdgeId << std::endl;
    int CGWidth = newDfg->CGWidth();
    // int maxDelay = 1;
    /*for(auto& elem : _dfgNodeAttr){
        auto adgNode = elem.second.adgNode;
        if(adgNode){
            maxDelay = dynamic_cast<GPENode*>(adgNode)->maxDelay() + 1;
            break;
        }
    }
    int maxInsertNodesPerEdge = 2;*/
    // std::cout << "_vioDfgEdges size: " << _vioDfgEdges.size() << std::endl;dfg->nodes().size() - dfg->ioNodes().size() - dfg->lutNodes().size()
    int numGpe = _adg->numGpeNodes() - _adg->numMIMDGpeNodes();
    int maxInsertNUm = numGpe - (newDfg->nodes().size() - newDfg->ioNodes().size() - newDfg->lutNodes().size() - newDfg->numPackedNode());
    // std::cout << "maxInsertNUm: " << maxInsertNUm << std::endl;
    if(maxInsertNUm <= 0) return;
    for(int eid : _vioDfgEdges){ // DFG edges with latency violation
        DFGEdge* e = newDfg->edge(eid);
        if(e->isrealBackEdge()) continue; // @yuan: maybe it doesn't need to insert pass node at back-edge
        int width = e->bitWidth();
        int vio = _dfgEdgeAttr[eid].vio; // maybe add multiple nodes according to vio   
        if(vio <= 0 ) {continue; 
        // std::cout << "skipp ~~~~~~~~~~~~~~~~" << std::endl;
        }// @yuan: maybe it doesn't need to insert pass-node at minus violation edges
        int num = 1; //std::min(maxInsertNodesPerEdge, std::max(1, vio/maxDelay));  
        int srcId = e->srcId();
        int dstId = e->dstId();
        int srcPortIdx = e->srcPortIdx();
        int dstPortIdx = e->dstPortIdx();
        bool isBackedge = e->isBackEdge();
        int iterDist = e->iterDist();
        int additionDelay = e->addDelay();
        // newDfg->deleteBackEdgeLoop(eid);
        newDfg->delEdge(eid);
        int lastId = srcId;
        int lastPort = srcPortIdx;
        if(width == 1 && !CGOnly){
            for(int i = 0; i < num; i++){
                DFGNode* newNode = new DFGNode();
                newNode->setId(++maxNodeId);
                newNode->setName("pass_fg_"+std::to_string(maxNodeId));
                newNode->setOperation("LUT");
                newNode->setLUTsize(2); //lut-2
                std::string and_gate_truth = "1000" ;
                newNode->setLUTconfig(and_gate_truth);
                newNode->addBitWidth(width);  
                newDfg->addNode(newNode); 
                newDfg->addLUTNode(newNode->id()); 
                DFGEdge* e1 = new DFGEdge(lastId, maxNodeId);
                e1->setId(++maxEdgeId); 
                e1->setSrcPortIdx(lastPort);
                e1->setDstPortIdx(0);
                e1->setBitWidth(width);
                newDfg->addEdge(e1);
                // and-gate another input is const_1
                newNode->setImm(width, std::make_pair(1, 1));
                lastId = maxNodeId;
                lastPort = 0;
            }
            DFGEdge* e2 = new DFGEdge(maxNodeId, dstId);
            e2->setId(++maxEdgeId); 
            e2->setSrcPortIdx(0);
            e2->setDstPortIdx(dstPortIdx);  
            e2->setBitWidth(width);  
            if(isBackedge){
                e2->setBackEdge(true);
                e2->setIterDist(iterDist);
                e2->setrealBackEdge(false);
                
            }    
            newDfg->addEdge(e2);
            // std::cout << "vio edge id: " << eid << " width: "<<width<<" src: "<< newDfg->node(srcId)->name() <<" dstid: "<<newDfg->node(dstId)->name()<< std::endl;
            continue;
        }else if(width == 1 && CGOnly){
            for(int i = 0; i < num; i++){
                DFGNode* newNode = new DFGNode();
                newNode->setId(++maxNodeId);
                newNode->setName("pass_cf_"+std::to_string(maxNodeId));
                newNode->setOperation("PASS_CF");
                newNode->addBitWidth(32);  
                newDfg->addNode(newNode); 
                DFGEdge* e1 = new DFGEdge(lastId, maxNodeId);
                e1->setId(++maxEdgeId); 
                e1->setSrcPortIdx(lastPort);
                e1->setDstPortIdx(0);
                e1->setBitWidth(32);
                newDfg->addEdge(e1);
                lastId = maxNodeId;
                lastPort = 0;
            }
            DFGEdge* e2 = new DFGEdge(maxNodeId, dstId);
            e2->setId(++maxEdgeId); 
            e2->setSrcPortIdx(0);
            e2->setDstPortIdx(dstPortIdx);  
            e2->setBitWidth(width);  
            if(isBackedge){
                e2->setBackEdge(true);
                e2->setIterDist(iterDist);
            }    
            newDfg->addEdge(e2);
            // std::cout << "vio edge id: " << eid << " width: "<<width<<" src: "<< newDfg->node(srcId)->name() <<" dstid: "<<newDfg->node(dstId)->name()<< std::endl;
            continue;
        }
        int logicLat = e->logicLat() * _II;// form src to dst logic lat //@multport 
        for(int i = 0; i < num; i++){
            DFGNode* newNode = new DFGNode();
            newNode->setId(++maxNodeId);
            newNode->setName("pass"+std::to_string(maxNodeId));
            newNode->setOperation("PASS");
            newNode->addBitWidth(width);
            newDfg->addNode(newNode);
            FUNode* fuNode = dynamic_cast<FUNode*>(_dfgNodeAttr[srcId].adgNode); 
            int maxDelay = fuNode->maxDelay(CGWidth);
            int logicLatSlice =  std::min(maxDelay, logicLat);
            logicLat -= logicLatSlice;
            DFGEdge* e1 = new DFGEdge(lastId, maxNodeId);
            e1->setId(++maxEdgeId); 
            e1->setSrcPortIdx(lastPort);
            e1->setDstPortIdx(0);
            e1->setlogicLat(logicLatSlice);
            e1->setBitWidth(width);
            newDfg->addEdge(e1);
            lastId = maxNodeId;
            lastPort = 0;
        }           
        DFGEdge* e2 = new DFGEdge(maxNodeId, dstId);
        e2->setId(++maxEdgeId); 
        e2->setSrcPortIdx(0);
        e2->setDstPortIdx(dstPortIdx);  
        e2->setBitWidth(width);  
        e2->setlogicLat(logicLat);  
        e2->setAddDelay(additionDelay);  
        if(isBackedge){
            e2->setBackEdge(true);
            e2->setIterDist(iterDist);
        }       
        newDfg->addEdge(e2);
        maxInsertNUm -= 1;
        if(maxInsertNUm <= 0) return;
        // std::cout << "vio edge id: " << eid << " width: "<<width<<" src: "<< newDfg->node(srcId)->name() <<" dstid: "<<newDfg->node(dstId)->name()<< std::endl;
    }
}

//@yuan: get the II of the DFG
int Mapping::getII(){
    int II = 1;
    for(auto& edge: _dfg->edges()){
        //@yuan: II = latency(start node) + edge_latency - latency(end node) + operation_latency(end node)
        if(edge.second->isBackEdge()){
            int startId = edge.second->srcId();
            int endId = edge.second->dstId();
            DFGNode* endNode = _dfg->node(endId);
            int startlat = dfgNodeAttr(startId).lat;
            int endlat = dfgNodeAttr(endId).lat;
            int oplat = endNode->opLatency();
            int edgelat = dfgEdgeAttr(edge.first).lat;
            II = std::max((startlat + edgelat - endlat + oplat), II);
        }
    }
    return II;
}


// EVALUATE II
int Mapping::evaluateII(){
    int II = 1;
    // std::cout << "backEdgeLoops size: " << _dfg->backEdgeLoops().size() << std::endl;
    for(auto& elem: _dfg->backEdgeLoops()){ 
        // std::cout << "edge id: " << elem.first << std::endl;
        DFGEdge *edge = _dfg->edge(elem.first);
        // std::cout << "edge id: " << elem.first << " edge.id(): " << edge->id() <<std::endl;
        int srcId = edge->srcId();
        int dstId = edge->dstId();
        int dstInportLat = dfgNodeAttr(dstId).lat - _dfg->node(dstId)->opLatency();
        int srclat = dfgNodeAttr(srcId).lat;  // @yuan: each dfg node latency will be fixed before schedule RDU, but will change after schedule RDU, so here will have a certain value
        int routeLat = _dfgEdgeAttr[elem.first].lat + edge->logicLat() * _II; // latNoDelay
        int iterDist = std::max(edge->iterDist(), 1); // >= 1
        int newII = (srclat + routeLat - dstInportLat + iterDist -1) / iterDist;//向上取整
        II = std::max(newII, II);
    }
    auto& _backEdgeLoops = _dfg->backEdgeLoops();
    // std::cout << "backEdges size: " << _dfg->backEdges().size() << std::endl;
    for(auto& elem: _dfg->backEdges()){
        if(_backEdgeLoops.count(elem)){
            continue;
        }
        DFGEdge *edge = _dfg->edge(elem);
        int srcId = edge->srcId();
        int dstId = edge->dstId();
        int dstInportLat = dfgNodeAttr(dstId).lat - _dfg->node(dstId)->opLatency();
        int srclat = dfgNodeAttr(srcId).lat;
        int routeLat = _dfgEdgeAttr[elem].lat + edge->logicLat() * _II; // latNoDelay
        int iterDist = std::max(edge->iterDist(), 1); // >= 1
        int newII = (srclat + routeLat - dstInportLat + iterDist -1) / iterDist;//向上取整
        II = std::max(newII, II);

    }
    return std::max(II, _II);
}


void Mapping::clearADGNodeUsage(){
    auto adg = getADG();
    for(auto& elem : adg->nodes()){
        auto node = elem.second;
        node->clearUsage();
    }
}


int Mapping::getADGNodeHisUsage(int ADGNodeId, int DFGNodeId){
    auto adg = getADG();
    auto adgNode = adg->node(ADGNodeId);
    return adgNode->getUsage(DFGNodeId);
}

bool Mapping::isTimeAvai(DFGNode* node, int time){
    DFGIONode* iobNode = dynamic_cast<DFGIONode*>(node);
    // std::cout << "node: " << node->name() << " time: " << time << std::endl;
    if(!iobNode->dependencyNodes().empty()){
        for(int id : iobNode->dependencyNodes()){
            DFGIONode* srcNode = dynamic_cast<DFGIONode*>(_dfg->node(id));
            int srcOplat = _dfg->getOutNodes().count(id) ? 0 : srcNode->opLatency();//@yuan: we need to make the enable signals of load and store be aligned
            int srcLat = _dfgNodeAttr[id].lat;
            if(srcLat == 0) return true; // src node has not been schduled
            int srcSlot = (srcLat - srcOplat);
            // std::cout << "srcname: " << srcNode->name() << " time: " << srcSlot << std::endl;
            if(time% _II <= srcSlot% _II){
                return false;
            }
        }
        return true;
    }else{
        return true;
    }

}


int Mapping::hasVioMemDenpendency(int vio){
    // int vio = 0;
    // std::map<std::string, std::set<int>> multiportAvailLatModII;
    std::map<std::string, std::map<int, std::set<int>>> multiportAvailLatModII;
    std::set<int> nums;
    for(int i = 0; i < _II; i++){ // @yuan: available time slots
        nums.insert(i);
    }
    
    for(auto &elem : _dfg->multiportIOs()){ // @yuan: for every memmory(same ref_name), set their initial available time slots
        // auto multiportBr = _dfg->multiportIObr();
        // for(int br = 0; br <= multiportBr[elem.first]; br++){
        auto multiportStep = _dfg->multiportIOSteps();
        for(int step = 0; step < multiportStep[elem.first].size(); step++){
            // std::cout << "array: " << elem.first << " branch: " << step << std::endl;
            multiportAvailLatModII[elem.first][step] = nums;
        }      
    }
    for(auto &elem : _dfg->ioNodes()){
        DFGIONode* iobNode = dynamic_cast<DFGIONode*>(_dfg->node(elem));
        if(!iobNode->dependencyNodes().empty()){
            int dstLat = _dfgNodeAttr[elem].lat;
            int dstOpLat = _dfg->getOutNodes().count(elem) ? 0 : iobNode->opLatency();//@yuan: enable signals alignment
            for(int id : iobNode->dependencyNodes()){
                DFGIONode* srcNode = dynamic_cast<DFGIONode*>(_dfg->node(id));
                int srcOplat = _dfg->getOutNodes().count(id) ? 0 : srcNode->opLatency();
                int srcLat = _dfgNodeAttr[id].lat;
                if((dstLat - dstOpLat)% _II <= (srcLat - srcOplat)% _II){
                    return 1;
                }
            }
        }
        // if(_dfg->isMultiportIoNode(elem) && iobNode->MultiportType() > 1){
        if(_dfg->isMultiportIoNode(elem)){
            std::string name = iobNode->memRefName();
            int Lat = _dfgNodeAttr[elem].lat;
            int OpLat = _dfg->getOutNodes().count(elem) ? 0 : iobNode->opLatency();//@yuan: enable signals alignment
            int nodeStep = iobNode->ControlStep();
            // std::cout << "name: " << name <<" IOB Name: "<<iobNode->name() << " Lat: " << Lat << "nodeStep: " << nodeStep << std::endl;
            for(auto& step: multiportAvailLatModII[name]){//@yuan: if the latency satisfy the dependency, we should make sure it can not be occupied by other stpes
                if(step.first == nodeStep) continue;//@yuan: the time slot of this step can not overlap with the slot in other steps
                if(!step.second.count((Lat - OpLat)%_II)){
                    if(_dfg->getOutNodes().count(elem) && vio == 0){//@yuan: we need to add a pass node before the input-node by setting the edge as the violation edge
                        for(auto edge : iobNode->outputEdges(1)){
                            for(auto eid : edge.second){
                                auto Edge = _dfg->edge(eid);
                                DFGIONode* dstIobNode = dynamic_cast<DFGIONode*>(_dfg->node(Edge->dstId()));
                                int dstSlot = _dfgNodeAttr[Edge->dstId()].lat - dstIobNode->opLatency();
                                if(((dstSlot)%_II) == ((Lat - OpLat)%_II)){
                                    _vioDfgEdges.push_back(eid);
                                }
                            }
                        }
                    }
                    // if()
                    return 1;
                }
            }
            if(multiportAvailLatModII[name][nodeStep].count((Lat - OpLat)%_II)){
                multiportAvailLatModII[name][nodeStep].erase((Lat - OpLat)%_II);
            }
        }
    }
    return 0;
}

int Mapping::TaskLat(DFGNode* node, int time){
    for(auto & elem :node->outputs()){
        for(auto & e : elem.second)
        for(auto & idx : e.second){
            if(_dfg->node(idx.first)->operation() == "TCSTORE" || _dfg->node(idx.first)->operation() == "TSTORE"){
                if(time % _II == 0){
                    return time + 1;
                }
            }
        }

    }
    return time;
}

//@yuan: alfter PnR and Synchronization, we can try to make best use of the memory banks for non-multiport memory access
//@yuan: tune the number of used bank for non-multiport memory access, enable making best use of the memory bank

int Mapping::updateNonMultiPortBank(bool isreset){
    std::vector<int> iobNodeSet;
    for(auto elem : _dfg->ioNodes()){
        DFGIONode* iobDfg = dynamic_cast<DFGIONode*>(_dfg->node(elem));
        if(_dfg->isMultiportIoNode(iobDfg)) continue;
        iobNodeSet.push_back(elem);
        iobDfg->setNumMultiportBank(1);//@yuan: every time we finish PnR, we need to reset the N and B for non-multiport nodes
        iobDfg->setMultiportBankSize(1);
    }
    if(isreset) return 1;
    std::sort(iobNodeSet.begin(), iobNodeSet.end(), [this](int a, int b){
        DFGIONode* iobDfgNodeA = dynamic_cast<DFGIONode*>(_dfg->node(a));
        DFGIONode* iobDfgNodeB = dynamic_cast<DFGIONode*>(_dfg->node(b));
        return iobDfgNodeA->memSize() > iobDfgNodeB->memSize();
    });
    std::set<std::string> mappedNonMultiPortArray;
    int DateinByte = _dfg->CGWidth() / 8;
    int Bmax = _adg->iobSpadBankSize() / DateinByte;
    for(auto elem : iobNodeSet){
        DFGIONode* iobDfg = dynamic_cast<DFGIONode*>(_dfg->node(elem));
        if(iobDfg->memSize() / DateinByte <= Bmax) continue; //@yuan: the data of the array can be stored in one bank, no need to partition
        // if(_dfg->isMultiportIoNode(iobDfg)) continue;
        // std::cout << "io node id: " << elem << " name: " << _dfg->node(elem)->name() << " size: " << iobDfg->memSize() / DateinByte << std::endl;
        IOBNode* mappedIobNode = dynamic_cast<IOBNode*>(dfgNodeAttr(elem).adgNode);
        const std::vector<int>& iobGrp = _adg->getIobGrp(mappedIobNode->index()); // @yuan: get the IOB that is connected to the same bank as the current adgNode
        int subUsedBankNum = 0;
        int NonMultiPortNum = 1;
        std::set<std::string> mappedMultiPortArray;
        for(auto ioINdex : iobGrp){
            if(ioINdex == mappedIobNode->index()) continue;
            int iobId = _adg->getIobIdFromIdx(ioINdex);
            ADGNode* iobNode = _adg->node(iobId);
            if(!isIOBFree(iobNode)){
                auto iobAttr = adgNodeAttr(iobId);
                auto mappedDfgNode = iobAttr.dfgNodes;
                for(auto node : mappedDfgNode){
                    DFGIONode* mappedIobDfg = dynamic_cast<DFGIONode*>(node.second);
                    if(_dfg->isMultiportIoNode(mappedIobDfg) && !mappedMultiPortArray.count(mappedIobDfg->memRefName())){
                        BankingSolution curSolution = _dfg->getCurrBankingSolution(mappedIobDfg->memRefName());
                        subUsedBankNum += curSolution.N;
                        mappedMultiPortArray.emplace(mappedIobDfg->memRefName());
                    }else if(!_dfg->isMultiportIoNode(mappedIobDfg)&& mappedNonMultiPortArray.count(mappedIobDfg->memRefName())){
                        subUsedBankNum += mappedIobDfg->NumMultiportBank();
                    }else if(!_dfg->isMultiportIoNode(mappedIobDfg)&& !mappedNonMultiPortArray.count(mappedIobDfg->memRefName())){
                        NonMultiPortNum ++;
                    }
                }

            }
        }
        int freeBankNum = iobGrp.size() - subUsedBankNum;
        int dataWidthinByte = _dfg->CGWidth() / 8;
        //@yuan: the free banks are used in average 
        if(NonMultiPortNum == 1){
            int maxN = closestPowerOfTwo(freeBankNum);
            int memSize = iobDfg->memSize() / dataWidthinByte;
            int eachBankSize = (memSize + maxN - 1) / maxN;
            int maxB = closestPowerOfTwoUpper(eachBankSize);
            if(maxB > Bmax){
                spdlog::warn("No feasible partition for non-multiport memory access!");
                std::cout << "[Fusion Error] The array: " << iobDfg->memRefName() << " is too big for SPM!" << std::endl;
                exit(0);
                return 0;
            }
            iobDfg->setNumMultiportBank(maxN);
            iobDfg->setMultiportBankSize(maxB);
            spdlog::info("Partition the data of array {0} into {1} banks with {2} elements each!", iobDfg->memRefName(), maxN, maxB);
            std::cout << "[Fusion] Partition the data of array " << iobDfg->memRefName() << " into " << maxN << " banks with " << maxB <<" elements each!"<< std::endl;
        }else{
            int upperN = (freeBankNum + NonMultiPortNum - 1) / NonMultiPortNum;
            int maxN = closestPowerOfTwo(upperN);
            int memSize = iobDfg->memSize()/ dataWidthinByte;
            int eachBankSize = (memSize + maxN - 1) / maxN;
            int maxB = closestPowerOfTwoUpper(eachBankSize);
            if(maxB > Bmax){
                spdlog::warn("No feasible partition for non-multiport memory access!");
                std::cout << "[Fusion Error] The array: " << iobDfg->memRefName() << " is too big for SPM!" << std::endl;
                exit(0);
                return 0;
            }
            iobDfg->setNumMultiportBank(maxN);
            iobDfg->setMultiportBankSize(maxB);
            spdlog::info("Partition the data of array {0} into {1} banks!", iobDfg->memRefName(), maxN);
            std::cout << "[Fusion] Partition the data of array " << iobDfg->memRefName() << " into " << maxN << " banks with " << maxB <<" elements each!"<< std::endl;
        }
        mappedNonMultiPortArray.emplace(iobDfg->memRefName());
    }
    return 1;
}