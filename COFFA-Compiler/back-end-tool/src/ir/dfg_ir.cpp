
#include "ir/dfg_ir.h"


DFGIR::DFGIR(std::string filename)
{
    _dfg = parseDFG(filename);
}

DFGIR::~DFGIR()
{
    if(_dfg){
        delete _dfg;
    }
}

// int DFGIR::numInputs(int bits){ 
//     if(_numInputs.count(bits)){
//         return _numInputs[bits]; 
//     }
//     return 0;
// }
// void DFGIR::setNumInputs(int bits, int numInputs){ 
//     _numInputs[bits] = numInputs; 
// }

// int DFGIR::numOutputs(int bits){ 
//     if(_numOutputs.count(bits)){
//         return _numOutputs[bits]; 
//     }
//     return 0;
// }    

// void DFGIR::setNumOutputs(int bits, int numOutputs){ 
//     _numOutputs[bits] = numOutputs; 
// }


// get node ID according to name
int DFGIR::nodeId(std::string name){
    if(_nodeName2id.count(name)){
        return _nodeName2id[name];
    } else{
        return -1;
    }
}


void DFGIR::setNodeId(std::string name, int id){
    _nodeName2id[name] = id;
}

// // get input index according to name
// std::pair<int, int> DFGIR::inputIdx(std::string name){
//     if(_inputName2idx.count(name)){
//         return _inputName2idx[name];
//     } else{
//         return {};
//     }
// }


// void DFGIR::setInputIdx(std::string name, std::pair<int, int> idx){
//     _inputName2idx[name] = idx;
// }

// // get output index according to name
// std::pair<int, int> DFGIR::outputIdx(std::string name){
//     if(_outputName2idx.count(name)){
//         return _outputName2idx[name];
//     } else{
//         return {};
//     }
// }

// void DFGIR::setOutputIdx(std::string name, std::pair<int, int> idx){
//     _outputName2idx[name] = idx;
// }

// get constant value according to name
std::pair<int, int64_t> DFGIR::constValue(std::string name){
    if(_name2Consts.count(name)){
        return _name2Consts[name];
    } else{
        return {};
    }
}

bool DFGIR::isConst(std::string name){
    return _name2Consts.count(name);
}


void DFGIR::setConst(std::string name, std::pair<int, int64_t> value){
    _name2Consts[name] = value;
}


// // get input index according to id
// std::pair<int, int> DFGIR::inputIdx(int id){
//     if(_inputId2idx.count(id)){
//         return _inputId2idx[id];
//     } else{
//         return {};
//     }
// }


// void DFGIR::setInputIdx(int id, std::pair<int, int> idx){
//     _inputId2idx[id] = idx;
// }

// // get output index according to id
// std::pair<int, int> DFGIR::outputIdx(int id){
//     if(_outputId2idx.count(id)){
//         return _outputId2idx[id];
//     } else{
//         return {};
//     }
// }

// void DFGIR::setOutputIdx(int id, std::pair<int, int> idx){
//     _outputId2idx[id] = idx;
// }

// get constant value according to id
int64_t DFGIR::constValue(int id){
    if(_id2Consts.count(id)){
        return _id2Consts[id];
    } else{
        return {};
    }
}

bool DFGIR::isConst(int id){
    return _id2Consts.count(id);
}


void DFGIR::setConst(int id, int64_t value){
    _id2Consts[id] = value;
}

// deprecated!!!
// DFG* DFGIR::parseDFGDot(std::string filename){
//     std::ifstream ifs(filename);
//     if(!ifs){
//         std::cout << "Cannot open DFG file: " << filename << std::endl;
//         exit(1);
//     }
//     DFG* dfg = new DFG();
//     dfg->setId(0); // DFG id = 0, node id = 1,...,n
//     std::string line;
//     int edgeIdx = 0;
//     std::stringstream edge_stream;
//     while (getline(ifs, line))
//     {
//         remove(line.begin(), line.end(), ' ');
//         if(line.empty() || line.substr(0, 2) == "//"){
//             continue;
//         }
//         int idx0 = line.find("[");
//         int idx1 = line.find("=");
//         int idx2 = line.find("]");
//         int idx3 = line.find("->");
//         if (idx0 != std::string::npos && idx3 == std::string::npos) { //nodes
//             std::string nodeName = line.substr(0, idx0);
//             std::string opName = line.substr(idx1 + 1, idx2-idx1-1);
//             std::transform(opName.begin(), opName.end(), opName.begin(), toupper);
//             int id = _nodeName2id.size() + 1;
//             setNodeId(nodeName, id);
//             if(opName == "INPUT"){
//                 int idx = _inputName2idx.size();
//                 setInputIdx(nodeName, idx);
//                 dfg->setInputName(idx, nodeName);
//             } else if(opName == "OUTPUT"){
//                 int idx = _outputName2idx.size();
//                 setOutputIdx(nodeName, idx);
//                 dfg->setOutputName(idx, nodeName);
//             } else if(opName == "CONST"){
//                 setConst(nodeName, 0); // ???????????????? PARSE CONST VALUE
//             } else{
//                 DFGNode* dfg_node = new DFGNode();
//                 dfg_node->setId(id);
//                 dfg_node->setName(nodeName);
//                 dfg_node->setOperation(opName);
//                 dfg->addNode(dfg_node);
//             }
//         }
//     }
//     // rescan the file to parse the edges, making sure all the nodes are already parsed
//     ifs.clear();
//     ifs.seekg(0);
//     while (getline(ifs, line))
//     {
//         remove(line.begin(), line.end(), ' ');
//         if(line.empty() || line.substr(0, 2) == "//"){
//             continue;
//         }
//         int idx0 = line.find("[");
//         int idx1 = line.find("=");
//         int idx2 = line.find("]");
//         int idx3 = line.find("->");
//         if (idx0 != std::string::npos && idx3 != std::string::npos) { // edges            
//             std::string srcName = line.substr(0, idx3);
//             std::string dstName = line.substr(idx3+2, idx0-idx3-2);
//             int dstPort = std::stoi(line.substr(idx1+1, idx2-idx1-1));
//             int srcPort = 0; // default one output for each node
//             if(isConst(srcName)){ // merge const node into the node connected to it
//                 DFGNode* node = dfg->node(nodeId(dstName));
//                 node->setImm(constValue(srcName));
//                 node->setImmIdx(dstPort);
//             } else{
//                 DFGEdge* edge = new DFGEdge(edgeIdx++);
//                 if(inputIdx(srcName) >= 0){ // INPUT                        
//                     edge->setEdge(0, inputIdx(srcName), nodeId(dstName), dstPort);                    
//                 } else if(outputIdx(dstName) >= 0){ // output
//                     edge->setEdge(nodeId(srcName), srcPort, 0, outputIdx(dstName));
//                 } else{
//                     edge->setEdge(nodeId(srcName), srcPort, nodeId(dstName), dstPort);
//                 }
//                 dfg->addEdge(edge);
//             }         
//         }
//     }
//     return dfg;
// }

// split a string and convert to integer array
std::vector<int> strSplit2Int(const std::string &str, char split){
    std::vector<int> res;
    std::string new_str = str + split;
    size_t pos = new_str.find(split);
    while(pos != new_str.npos){        
        int value = std::stoi(new_str.substr(0, pos));
        res.push_back(value);
        new_str = new_str.substr(pos+1, new_str.size());
        pos = new_str.find(split);
    }
    return res;
}

// split a string and convert to uint64_t array
std::vector<uint64_t> strSplit2Uint64(const std::string &str, char split){
    std::vector<uint64_t> res;
    std::string new_str = str + split;
    size_t pos = new_str.find(split);
    while(pos != new_str.npos){        
        int value = std::stoull(new_str.substr(0, pos));
        res.push_back(value);
        new_str = new_str.substr(pos+1, new_str.size());
        pos = new_str.find(split);
    }
    return res;
}


// Json file transformed from dot file using graphviz
DFG* DFGIR::parseDFGJson(std::string filename){
    std::ifstream ifs(filename);
    if(!ifs){
        std::cout << "Cannot open DFG file: " << filename << std::endl;
        exit(1);
    }
    json dfgJson;
    ifs >> dfgJson;
    _rtlil = new RTLIL();
    DFG* dfg = new DFG();
    dfg->setId(0); // DFG id = 0, node id = 1,...,n
    dfg->setFineGrained(false);
    int bitWidth = 32; // TMP for test
    int64_t value = 0;
    // parse nodes
    for(auto& nodeJson : dfgJson["objects"]){
        std::string nodeName = nodeJson["name"].get<std::string>();
        std::string opName = nodeJson["opcode"].get<std::string>();
        std::transform(opName.begin(), opName.end(), opName.begin(), toupper);
        int id = nodeJson["_gvid"].get<int>() + 1; // start from 1
        //build RTLIL Cells
        Cell* cell = new Cell();
        cell->setId(id);
        cell->setName(nodeName);
        cell->setOperation(opName);
        // std::cout << "dfgnode name: " << nodeName << std::endl;
        if(opName == "CONST"){
            if(nodeJson.contains("value")){
                value = std::stoull(nodeJson["value"].get<std::string>());
            }
            // std::cout << " const value: " << value << std::endl;
            setConst(id, value); 
        } else{
            DFGNode* dfg_node;
            if(opName == "INPUT" || opName == "OUTPUT" || opName == "LOAD" || opName == "STORE" || opName == "CLOAD" || opName == "CSTORE"|| opName == "CINPUT" || opName == "COUTPUT" 
            || opName == "TLOAD" || opName == "TSTORE" || opName == "TCLOAD" || opName == "TCSTORE"){
                dfg->addIONode(id);
                DFGIONode* dfg_io_node = new DFGIONode();
                if(nodeJson.contains("ref_name")){
                    dfg_io_node->setMemRefName(nodeJson["ref_name"].get<std::string>());
                    // std::cout << "refname: " << nodeJson["ref_name"].get<std::string>() << std::endl;
                    cell->setParameters("ref_name", nodeJson["ref_name"].get<std::string>());
                }
                if(nodeJson.contains("multiport")){//@multport Padding
                    int multiport = std::stoi(nodeJson["multiport"].get<std::string>());
                    dfg_io_node->setMultiport(multiport>=1);
                    dfg_io_node->setMultiportType(multiport);
                    cell->setParameters("multiport", std::to_string(multiport));
                }else{
                    dfg_io_node->setMultiport(false);
                    dfg_io_node->setMultiportType(0);
                    cell->setParameters("multiport", "0");
                }
                // if(nodeJson.contains("N")){
                //     int N = std::stoi(nodeJson["N"].get<std::string>());
                //     assert(N && !(N & (N-1))); //@yuan: N must be power of 2
                //     dfg_io_node->setNumMultiportBank(N);
                //     cell->setParameters("N", std::to_string(N));
                // }
                // if(nodeJson.contains("B")){
                //     int B = std::stoi(nodeJson["B"].get<std::string>());
                //     assert(B && !(B & (B-1)));//@yuan: B must be power of 2
                //     dfg_io_node->setMultiportBankSize(B);
                //     cell->setParameters("B", std::to_string(B));
                // }
                // if(nodeJson.contains("controlstep")){
                //     int step = std::stoi(nodeJson["controlstep"].get<std::string>());
                //     dfg_io_node->setControlStep(step);
                //     cell->setParameters("controlstep", std::to_string(step));
                // }else{
                //     dfg_io_node->setControlStep(0);
                //     cell->setParameters("controlstep", "0");
                // }
                if(nodeJson.contains("branch")){
                    int branch = std::stoi(nodeJson["branch"].get<std::string>());
                    dfg_io_node->setBranch(branch);
                    cell->setParameters("branch", nodeJson["branch"].get<std::string>());
                }else{
                    dfg_io_node->setBranch(0);
                }
                if(nodeJson.contains("offset")){
                    //int offset = std::stoi(nodeJson["offset"].get<std::string>());
                    std::string patStr = nodeJson["offset"].get<std::string>();
                    patStr.erase(std::remove(patStr.begin(), patStr.end(), ' '), patStr.end());
                    cell->setParameters("offset", patStr);
                    std::vector<int> values = strSplit2Int(patStr, ',');
                    dfg_io_node->setMemOffset(values[0]);
                    if(values.size() > 1){
                        dfg_io_node->setReducedMemOffset(values[1]);
                    }else{
                        dfg_io_node->setReducedMemOffset(0);
                    }
                }else{
                    dfg_io_node->setMemOffset(0);
                    dfg_io_node->setReducedMemOffset(0);
                }
                if(nodeJson.contains("size")){
                    cell->setParameters("size", nodeJson["size"].get<std::string>());
                    int size = std::stoi(nodeJson["size"].get<std::string>());
                    dfg_io_node->setMemSize(size);
                }
                if(nodeJson.contains("pattern")){
                    std::string patStr = nodeJson["pattern"].get<std::string>();
                    patStr.erase(std::remove(patStr.begin(), patStr.end(), ' '), patStr.end());
                    cell->setParameters("pattern", patStr);
                    std::vector<int> values = strSplit2Int(patStr, ',');
                    for(int i = 0; i < values.size(); i += 2){
                        dfg_io_node->addPatternLevel(values[i], values[i+1]);
                    }
                }
                if(nodeJson.contains("cycles")){
                    cell->setParameters("cycles", nodeJson["cycles"].get<std::string>());
                    int cycles = std::stoi(nodeJson["cycles"].get<std::string>());
                    dfg_io_node->addPatternLevel(0, cycles);
                }
                dfg_node = dfg_io_node;
            }else{
                dfg_node = new DFGNode();
                if(nodeJson.contains("acc_params")){
                    std::string patStr = nodeJson["acc_params"].get<std::string>();
                    patStr.erase(std::remove(patStr.begin(), patStr.end(), ' '), patStr.end());
                    // std::cout << "acc_params: " << patStr << std::endl;
                    cell->setParameters("acc_params", patStr);
                    cell->setParameters("acc_first", nodeJson["acc_first"].get<std::string>());
                    std::vector<uint64_t> values = strSplit2Uint64(patStr, ',');
                    assert(values.size() >= 3);
                    // std::cout << "acc_params~: " << values[0] << ", " << values[1]  << std::endl;
                    dfg_node->setInitVal(values[0]);
                    dfg_node->setCycles((int)values[1]);
                    dfg_node->setInterval((int)values[2]);
                    dfg_node->setRepeats((int)values[3]);
                    int first = std::stoi(nodeJson["acc_first"].get<std::string>());
                    if(first){
                        dfg_node->setIsAccFirst(true);
                    }else{
                        dfg_node->setIsAccFirst(false);
                    }
                    if(nodeJson.contains("second_initVal")){
                        uint64_t initVal = std::stoi(nodeJson["second_initVal"].get<std::string>());
                        cell->setParameters("second_initVal", nodeJson["second_initVal"].get<std::string>());
                        dfg_node->set2ndInit(initVal);
                        dfg_node->setInitIdx(1); // @yuan: for the second initial value, its operand idx is fixed to 1
                    }
                }else if(nodeJson.contains("isel_params")){
                    std::string patStr = nodeJson["isel_params"].get<std::string>();
                    patStr.erase(std::remove(patStr.begin(), patStr.end(), ' '), patStr.end());
                    cell->setParameters("isel_params", patStr);
                    std::vector<uint64_t> values = strSplit2Uint64(patStr, ',');
                    // std::cout << "values size: " << values.size() << std::endl;
                    assert(values.size() >= 3);
                    dfg_node->setCycles((int)values[0]);
                    dfg_node->setInterval((int)values[1]);
                    dfg_node->setRepeats((int)values[2]);
                }

            }
            //DFGNode* dfg_node = new DFGNode();
            dfg_node->setId(id);
            //dfg_node->addBitWidth(bitWidth);
            dfg_node->setName(nodeName);
            dfg_node->setOperation(opName);
            dfg->addNode(dfg_node);
            
            // if(std::find(this->addsub.begin(), this->addsub.end(), opName) != this->addsub.end()){
			//     this->optypecount.numaddsub +=1;
		    // }else if(std::find(this->logic.begin(), this->logic.end(), opName) != this->logic.end()){
			//     this->optypecount.numlogic +=1;
		    // }else if(std::find(this->multiplier.begin(), this->multiplier.end(), opName) != this->multiplier.end()){
			//     this->optypecount.nummul +=1;
		    // }else if(std::find(this->comp.begin(), this->comp.end(), opName) != this->comp.end()){
			//     this->optypecount.numcomp +=1;
		    // }else{
			// std::cout << "Operand not find"<< opName << std::endl;
		    // }
        }
        /*cell->setBitWidth(bitWidth);
        if(opName == "INPUT"){
            _rtlil->setInWire(id, bitWidth);
        }else if(opName == "OUTPUT"){
            _rtlil->setOutWire(id, bitWidth);
        }else */
        if(opName == "CONST"){
            cell->setValue(value);
        }
        _rtlil->addCell(cell);
    }
    // parse edges
    bool hasCGWidth = false;
    for(auto& edgeJson : dfgJson["edges"]){
        int srcId = edgeJson["tail"].get<int>() + 1;
        int dstId = edgeJson["head"].get<int>() + 1;
        int dstPort;
        int srcPort; // default one output for each node
        int bitWidth = 32;  // default width is 32
        int edgeId = edgeJson["_gvid"].get<int>();
        int iterdist;
        bool isBackEdge;
        bool isDynamicDist;
        int logicLat;//@multport
        int edgeType = 0; //@yuan: default is data dependence edge
        Cell* cell = _rtlil->getCell(dstId);
        // bool isCtrlBackEdge;
        if(edgeJson.contains("Width")){
            bitWidth = std::stoi(edgeJson["Width"].get<std::string>());
            if(bitWidth > 1 && !hasCGWidth){
                dfg->setCGWidth(bitWidth);
                hasCGWidth = true;
            }
        }
        if(edgeJson.contains("operand")){
            dstPort = std::stoi(edgeJson["operand"].get<std::string>());
        }else if(edgeJson.contains("headport")){
            auto str = edgeJson["headport"].get<std::string>(); // in0, in1...
            dstPort = std::stoi(str.substr(2, 1));
        // }else if(!_outputId2idx.count(dstId)){ // not annotate dst-port, not output port
        //     DFGNode* node = dfg->node(dstId);
        //     dstPort = node->numInputs(bitWidth);
        }
        if(edgeJson.contains("tailport")){
            auto str = edgeJson["tailport"].get<std::string>(); // out0, out1...
            srcPort = std::stoi(str.substr(3, 1));
        }else{ // default one output for each node
            srcPort = 0;
        }
        if(edgeJson.contains("logiclat")){
            logicLat = std::stoi(edgeJson["logiclat"].get<std::string>());
        }else{
            logicLat = 0;
        }
        if(edgeJson.contains("backedge")){
            isBackEdge = std::stoi(edgeJson["backedge"].get<std::string>()) >= 1;
            // isCtrlBackEdge = std::stoi(edgeJson["backedge"].get<std::string>()) == 2;
        }else{
            isBackEdge = false;
            // isCtrlBackEdge = false;
        }
        if(edgeJson.contains("iterdist")){
            iterdist = std::stoi(edgeJson["iterdist"].get<std::string>());
        }else{
            iterdist = 1;
        }
        if(edgeJson.contains("type")){
            edgeType = std::stoi(edgeJson["type"].get<std::string>());
        }
        //@yuan_ddp:
        if(edgeJson.contains("isdynamic")){
            if(std::stoi(edgeJson["isdynamic"].get<std::string>())){
                isDynamicDist = true;
            }else{
                isDynamicDist = false;
            }
        }else{
            isDynamicDist = false;
        }
        // std::cout << "dfg_ir isLoop: " << isLoop << std::endl;
        if(isConst(srcId)){ // merge const node into the node connected to it
            DFGNode* node = dfg->node(dstId);
            //(bitwidth, (node input port, const value))
            node->addBitWidth(bitWidth);
            node->setImm(bitWidth, std::make_pair(dstPort, constValue(srcId)));
            // std::cout <<node->name() <<" has const ~~~" << std::endl;
        } else{
            DFGEdge* edge = new DFGEdge(edgeId);
            // if(_inputId2idx.count(srcId)){ // INPUT                        
            //     edge->setEdge(inputIdx(srcId).first, 0, inputIdx(srcId).second, dstId, dstPort);                    
            // } else if(_outputId2idx.count(dstId)){ // output
            //     edge->setEdge(outputIdx(dstId).first, srcId, srcPort, 0, outputIdx(dstId).second);
            // } else{
            edge->setEdge(bitWidth, srcId, srcPort, dstId, dstPort);
            edge->setBackEdge(isBackEdge);
            edge->setIterDist(iterdist);
            edge->setlogicLat(logicLat);//@multport
            edge->setDynamicDist(isDynamicDist);//@yuan_ddp
            if(edgeType == 1){
                edge->setType(EDGE_TYPE_MEM);
                std::string srcNodeName = dfg->node(srcId)->name();
                DFGIONode* dstNode = dynamic_cast<DFGIONode*>(dfg->node(dstId));
                dstNode->addDependency(srcId);
                cell->addDependency(srcNodeName);
                // dfg->addEdge(edge);
                continue;
            }else if(edgeType == 2){
                edge->setType(EDGE_TYPE_CTRL);
            }else{
                edge->setType(EDGE_TYPE_DATA);
            }
            // }
            dfg->addEdge(edge);
        }  
        // record the width of the destination cell
        if(!dfg->hasFineGrained() && bitWidth == 1 && (cell->operation() == "AND" || cell->operation() == "EQ" || cell->operation() == "XOR" || cell->operation() == "OR" || cell->operation() == "NOT")){
            dfg->setFineGrained(true);
        }
        cell->addBitWidth(bitWidth);
        // record the width of the source cell
        cell = _rtlil->getCell(srcId);
        cell->addBitWidth(bitWidth);
        // buil RTLIL Wires
        Wire* wire = new Wire(edgeId);
        wire->setWire(srcId, srcPort, dstId, dstPort);
        wire->setWidth(bitWidth);
        wire->setIterDist(iterdist);
        wire->setlogicLat(logicLat);
        wire->setBackEdge(isBackEdge);
        wire->setType(edgeType);
        wire->setDynamicDist(isDynamicDist);//@yuan_ddp
        _rtlil->addWire(wire);       
    }
    // // add const operand for nodes with only one operand, should be avoid
    // for(auto &elem : dfg->nodes()){
    //     DFGNode* node = elem.second;
    //     if(!dfg->isIONode(elem.first) && node->numInputs() < 2){ // just for verification, not all cases
    //         node->setImm(bitWidth, std::make_pair(1, 0));
    //     }
    // }
    return dfg;
}


DFG* DFGIR::parseDFG(std::string filename, std::string format){
    // if(format == "dot"){
    //     return parseDFGDot(filename);
    // }else{
        return parseDFGJson(filename);
    // }
}