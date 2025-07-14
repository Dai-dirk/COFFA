
 #include "mapper/configuration.h"

 //@yuan: find the colsest power-of-2 value of memory offset, and return their difference
int closestPowerOfTwoDifference(int offset){
    if(offset == 0){
        return 0;
    }
    int highestBitPosition = 0;
    int absOffset = abs(offset);
    // std::cout << "power of 2 offset: " << absOffset << std::endl;
    while (absOffset > 1) {
        absOffset >>= 1;
        highestBitPosition++;
    }
    //@yuan: if offset < 0, highestBitPosition should added 1
    if(offset < 0){
        highestBitPosition++;
    }

    // the colosest power-of-w value of offset
    int closestPowerOfTwo = 1 << highestBitPosition;

    int difference = offset - closestPowerOfTwo;

    return difference;

 }

 //@yuan: find the bias 
 int biasNumInOneTrans(int offset, int busWidthinByte){
    return offset % busWidthinByte;
 }

void addCfgData(std::map<int, CfgData> &cfg, const CfgDataLoc &loc, uint32_t data){
    CfgData loc_data(loc.high - loc.low + 1, data);
    cfg[loc.low] = loc_data;
}

void addCfgData(std::map<int, CfgData> &cfg, const CfgDataLoc &loc, uint64_t data){
    int len = loc.high - loc.low + 1;
    CfgData loc_data(len);
    uint64_t val = data & (((uint64_t)1 << len) - 1);
    // std::cout << "len: " << len << " data: " << data << " val: " << val << std::endl;
    while(len > 0){
        loc_data.data.push_back(uint32_t(val&0xffffffff));
        len -= 32;
        val = val>> 32;
        // std::cout << "len: " << len << " val: " << val << std::endl;
    }
    cfg[loc.low] = loc_data;
}

void addCfgData(std::map<int, CfgData> &cfg, const CfgDataLoc &loc, const std::vector<uint32_t> &data){
    CfgData loc_data(loc.high - loc.low + 1, data);
    cfg[loc.low] = loc_data;
}

// get config data for GPE, return<LSB-location, CfgData>
std::map<int, CfgData> Configuration::getGpeCfgData(GPENode* node){
    int adgNodeId = node->id();
    ADG* subAdg = node->subADG();
    auto& adgNodeAttr = _mapping->adgNodeAttr(adgNodeId);
    if(adgNodeAttr.dfgNodes.empty()){
        return {};
    }
    std::map<int, CfgData> cfg;
    int cg_width = _mapping->getDFG()->CGWidth();
    std::map<int, std::map<int, std::map<int, int>>> delayUsed; // <RDU-id, delayused>
    //@yuan: GPE can simultaneously map ALU and LUT nodes
    for(auto& mappedNode : adgNodeAttr.dfgNodes){
        DFGNode* dfgNode = mappedNode.second;
        if(dfgNode == nullptr){
            continue;
        }
        // std::cout << "dfgNode: " << dfgNode->name() << " operatopn: " << dfgNode->operation() << " ISISEL: " << dfgNode->initSelection()<< " GPE NodeId: " << node->id()<<std::endl;
        int type = _mapping->mappedFUType(dfgNode);
        int lutId = -1;
        int cg_rduId; // coarse-grain RDU-id
        int fg_rduId; // fine-grain RDU-id
        std::map<int, std::set<int>> usedOperands; // <fu-type, used-op>
        if( type == TYPE_ALU){
            int opc = Operations::OPC(dfgNode->operation());
            // std::cout << "opc: " << opc << std::endl;
            int aluId = -1;
            std::map<int, int> usedRdu; // <width, RDU-id>
            // for(auto& elem : dfgNode->bitWidths()){
            //     if(elem != 1){
            //         cg_width = elem;
            //         break;
            //     }
            // }
            for(auto& elem : dfgNode->inputEdges()){
                int bitWidth = elem.first;// for each bitwidth
                for(auto& edge : elem.second){ // for each edge
                    int eid = edge.second;
                    DFGEdge* e = _mapping->getDFG()->edge(eid);
                    DFGNode* src = _mapping->getDFG()->node(e->srcId()); 
                    DFGNode* dst = _mapping->getDFG()->node(e->dstId()); 
                    auto& edgeAttr = _mapping->dfgEdgeAttr(eid);
                    int delay = edgeAttr.delay; // delay cycles
                    int inputIdx = edgeAttr.edgeLinks.rbegin()->srcPort; // last edgeLInk, dst port
                    if(inputIdx == -1){// using the internal link in GPE
                        std::cout << "using GPE internal links~~~" << std::endl;
                        int dstPort = e->dstPortIdx();
                        int Operand = 0;
                        if(dfgNode->accumulative() || dfgNode->operation() == "CISEL"){
                            int opereandNumCG = dfgNode->numInputs(cg_width);
                            Operand = node->getOperandIdxFg(opereandNumCG, dstPort);
                            inputIdx = *node->ConDMRInputs(bitWidth, Operand).begin();
                        }else if (dfgNode->operation() == "SEXT" || dfgNode->operation() == "ZEXT"){// @yuan: for sext/zext node, its operand is 0
                            inputIdx = *node->operandInputs(bitWidth, Operand).begin();
                        }else{
                            Operand = dstPort - node->numOperands(cg_width);
                            inputIdx = *node->operandInputs(bitWidth, Operand).begin();
                        }
                        // std::cout << "accumulate inputs: ";
                        // for(auto& t :node->ConACCInputs(bitWidth, Operand)){
                        //     std::cout << t << " " ;
                        // }
                        // std::cout << std::endl;
                        auto muxPair = subAdg->input(bitWidth, inputIdx).begin(); // one input only connected to one Mux, @yuan: for sub-adg, the input() returns the relationship between inport and the sub-module
                        int muxId = muxPair->first;
                        auto mux = subAdg->node(muxId);
                        for(auto& sm2mux : mux->inputs(bitWidth)){
                            int id = sm2mux.second.first;
                            if(id == subAdg->id()) continue;
                            if(subAdg->node(id)->type() == "RF4LUT"){ // filled with zero
                                addCfgData(cfg, node->configInfo(muxId), (uint32_t)sm2mux.first);
                                // std::cout << "muxId: " <<muxId<<" muxCfgData: " <<(uint32_t)sm2mux.first<< std::endl;
                                break;
                            }
                        }
                        auto rduPair = mux->output(bitWidth, 0).begin();
                        cg_rduId = rduPair->first;  
                        auto rdu = subAdg->node(cg_rduId);   
                        int rduPort = 0;
                        bool findALU = false;
                        for(auto rduOut : rdu->outputs(bitWidth)){
                            for(auto u : rduOut.second){
                                if(subAdg->node(u.first)->type() == "ALU"){
                                    //aluId = u.first;
                                    rduPort = rduOut.first;
                                    findALU = true;
                                    break;
                                }
                            }
                            if(findALU) break;
                        }
                        usedRdu[bitWidth] = cg_rduId;
                        delayUsed[bitWidth][cg_rduId][rduPort] = edgeAttr.delay; // delay cycles used by this port   
                        // if(dfgNode->accumulative()){
                        //     auto aluPair = rdu->output(bitWidth, rduPort).begin();  // RDU has the same input/output index
                        //     aluId = aluPair->first;
                        // }
                        // int oprand = rduPort;
                        // if(bitWidth == 1){
                        //     oprand = rduPort + node->numOperands(cg_width);
                        // }
                        usedOperands[type].emplace(dstPort); // operand index
                        // std::cout << "aluId: " <<aluId<<" dstPort: " << dstPort << " Operand: " << Operand <<" cg_rduId: " << cg_rduId << " rduPort: " << rduPort<< std::endl;
                        // std::cout << "edge from: " << src->name() << " to: " << dst->name() << " width: " <<  bitWidth <<" inputIdx: "<<inputIdx<< std::endl; 
                        continue;
                    }
                    // std::cout << "edge from: " << src->name() << " to: " << dst->name() << " width: " <<  bitWidth <<" inputIdx: "<<inputIdx<< std::endl; 
                    auto muxPair = subAdg->input(bitWidth, inputIdx).begin(); // one input only connected to one Mux, @yuan: for sub-adg, the input() returns the relationship between inport and the sub-module
                    int muxId = muxPair->first;
                    int muxCfgData = muxPair->second;
                    // std::cout << "muxid: " << muxId << " muxCfgData: " << muxCfgData << std::endl;
                    auto mux = subAdg->node(muxId);
                    auto rduPair = mux->output(bitWidth, 0).begin();
                    cg_rduId = rduPair->first; 
                    int rduPort = rduPair->second;
                    usedRdu[bitWidth] = cg_rduId;
                    auto rdu = subAdg->node(cg_rduId);   
                    delayUsed[bitWidth][cg_rduId][rduPort] = edgeAttr.delay; // delay cycles used by this port
                    if(!(dfgNode->accumulative() && bitWidth == 1)){
                        auto aluPair = rdu->output(bitWidth, rduPort).begin();  // RDU has the same input/output index
                        aluId = aluPair->first;
                    }
                    // std::cout << "aluid: " << aluId << " cg_width: " << cg_width << std::endl;
                    int oprand = e->dstPortIdx();
                    // if(bitWidth == 1){
                    //     oprand = rduPort + node->numOperands(cg_width);
                    // }
                    
                    // std::cout << "cg_rduId: " <<cg_rduId<<" rduPort: " << rduPort << " operand: " << oprand<< " num_op: " <<node->numOperands(cg_width)<< " rduPort: " << rduPort<<std::endl;
                    // exit(0);
                    if(bitWidth != 1) usedOperands[type].emplace(rduPort); // operand index
                    addCfgData(cfg, node->configInfo(muxId), (uint32_t)muxCfgData);
                    // std::cout << "AFTER config muxid: " << muxId << " muxCfgData: " << muxCfgData << std::endl;
                }
            }
            // std::cout << "~~~~~~~~~~~~~~ "  << aluId << std::endl;
            if(aluId == -1){// in case that some node has no input
                auto muxPair = subAdg->input(cg_width, 0).begin(); // one input only connected to one Mux
                int muxId = muxPair->first;
                auto mux = subAdg->node(muxId);
                cg_rduId = mux->output(cg_width, 0).begin()->first; 
                usedRdu[cg_width] = cg_rduId; // used for the node only has const input
                auto rdu = subAdg->node(cg_rduId);   
                auto aluPair = rdu->output(cg_width, 0).begin();   
                aluId = aluPair->first;  
            }
            // std::cout << "============== "  << aluId << std::endl;
            addCfgData(cfg, node->configInfo(aluId), (uint32_t)opc);
            // std::cout << "Before config const~~~~ " << std::endl;
            // Constant for ALU
            for(auto& elem : dfgNode->bitWidths()){
                if(dfgNode->hasImm(elem)){
                    if(elem == 1){ 
                        // std::cout << "ALU 1-bit constant~~~" << std::endl;
                        int bit_value = dfgNode->imm(elem).second;
                        assert(bit_value <= 1);
                        // @yuan: for 1-bit const, its operand is certain, but start from 0
                        int opId = dfgNode->imm(elem).first;
                        int inputIdx = 0;
                        // int muxId = 0;
                        // std::cout << "opId: " << opId << std::endl;
                        if(dfgNode->accumulative() || dfgNode->operation() == "CISEL"){
                            int opereandNumCG = dfgNode->numInputs(cg_width);
                            opId = node->getOperandIdxFg(opereandNumCG, opId);
                            inputIdx = *node->ConDMRInputs(elem, opId).begin();
                        }else{
                            opId = opId - node->numOperands(cg_width);
                            inputIdx = *node->operandInputs(elem, opId).begin();
                        }  
                        auto muxPair = subAdg->input(1, inputIdx).begin(); // one input only connected to one Mux   
                        int muxId = muxPair->first;                   
                        // int rduId; 
                        // if(usedRdu.count(elem)){ // in case that the node only has one 1-bit const
                        //     rduId = usedRdu[elem]; 
                        // }else{
                        //     auto muxPair = subAdg->input(1, 0).begin(); // one input only connected to one Mux
                        //     int muxId = muxPair->first;
                        //     auto mux = subAdg->node(muxId);
                        //     rduId = mux->output(1, 0).begin()->first; 
                        // }
                        // auto rdu = subAdg->node(rduId); // used default delay 
                        // int muxId = rdu->input(elem, opId).first;
                        // std::cout << "muxId: " << muxId << " opId: " << opId<< std::endl;
                        for(auto& elem_i : subAdg->node(muxId)->inputs(elem)){
                            int id = elem_i.second.first;
                            if(id == subAdg->id()) continue;
                            if(subAdg->node(id)->type() == "Const1" && bit_value == 1){
                                addCfgData(cfg, node->configInfo(muxId), (uint32_t)elem_i.first);
                                // std::cout << "muxId: " << muxId << " MUXdata: " << (uint32_t)elem_i.first << std::endl;
                            }else if(subAdg->node(id)->type() == "Const0" && bit_value == 0){
                                addCfgData(cfg, node->configInfo(muxId), (uint32_t)elem_i.first);
                            }
                        }
                    }else{
                        // std::cout << " width: "<< elem <<" num_input: "<< dfgNode->numInputs(elem) << std::endl;
                        // find unused operand
                        int i = 0;
                        for(; i < dfgNode->numInputs(elem); i++){
                            if(!usedOperands[type].count(i)){ 
                                break;
                            }
                        }
                        assert(i < dfgNode->numInputs(elem));
                        int rduId = usedRdu[elem];
                        // std::cout << "coarse grained i: " << i << " rduId: " << rduId  << std::endl;
                        auto rdu = subAdg->node(rduId); // used default delay 
                        int muxId = rdu->input(elem, i).first;
                        for(auto& elem_i : subAdg->node(muxId)->inputs(elem)){
                            int id = elem_i.second.first;
                            if(id == subAdg->id()) continue;
                            if(subAdg->node(id)->type() == "Const"){
                            addCfgData(cfg, node->configInfo(muxId), (uint32_t)elem_i.first);
                            // std::cout << " muxId: " << muxId << std::endl;
                            // std::cout << " id: " << id << std::endl;
                            // std::cout << " const: " << (int)dfgNode->imm(elem).second << std::endl;
                            // std::cout << " uint const: " << (uint64_t)dfgNode->imm(elem).second << std::endl;
                            //TODO: check when the const is negetive number(20230327, checked)
                            addCfgData(cfg, node->configInfo(id), (uint64_t)(dfgNode->imm(elem).second));
                            break;
                            }
                        }  
                    }
                }
            }
            //@yuan: for ACC/CACC/CIACC nodes, which can only be ALU type 
            // accumulative node
            if(dfgNode->accumulative()){
                //initial value
                int initValId = node->cfgIdMap["InitVal"];
                addCfgData(cfg, node->configInfo(initValId), (uint64_t)(dfgNode->initVal()));
                // write interval
                int II =  _mapping->II();
                int WI = dfgNode->interval() * II;
                int wiId = node->cfgIdMap["WI"];
                addCfgData(cfg, node->configInfo(wiId), (uint32_t)WI);
                // node latency
                auto& dfgNodeAttr = _mapping->dfgNodeAttr(dfgNode->id());
                int latency = dfgNodeAttr.lat - dfgNode->opLatency(); 
                int latencyId = node->cfgIdMap["Latency"];
                addCfgData(cfg, node->configInfo(latencyId), (uint32_t)latency);
                // write cycles
                int cycles = dfgNode->cycles();
                int cyclesId = node->cfgIdMap["Cycles"];
                addCfgData(cfg, node->configInfo(cyclesId), (uint32_t)cycles);
                //wirte repeats number
                int repeats = dfgNode->repeats();
                int repeatsId = node->cfgIdMap["Repeats"];
                addCfgData(cfg, node->configInfo(repeatsId), (uint32_t)repeats);
                //if skip first accumulative num
                bool skipfisrt = !dfgNode->isAccFirst();
                int skipfisrtId = node->cfgIdMap["SkipFirst"];
                addCfgData(cfg, node->configInfo(skipfisrtId), (uint32_t)skipfisrt);
                //@yuan: for the enable signal edge, we shoudl judge if it need delay one cycle (if the edge is back-edge)
                bool backEdgeEn = false;
                if(dfgNode->numInputs(1) > 0){ // only for CACC/CIACC/CDIACC
                    for(auto elem : dfgNode->inputEdges(1)){ // just check the fine-grain edges
                        auto edge = _mapping->getDFG()->edge(elem.second);
                        int dstPort = edge->dstPortIdx() - dfgNode->numInputs(cg_width);
                        if(dstPort == 0){
                            backEdgeEn =  edge->isBackEdge();
                            break;
                        }
                    }
                }
                bool delayEn = skipfisrt && !backEdgeEn;
                int delayenId = node->cfgIdMap["delayEn"];
                addCfgData(cfg, node->configInfo(delayenId), (uint32_t)delayEn);
                // std::cout << "backEdgeEn: " << backEdgeEn << " delayenId: " << delayenId << " delayEn: " << delayEn << std::endl;
                if(dfgNode->has2ndInit()){
                    // find unused operand
                    // int i = 0;
                    // for(; i < dfgNode->numInputs(cg_width); i++){
                    //     if(!usedOperands[type].count(i)){ 
                    //         break;
                    //     }
                    // }
                    int opIdx = dfgNode->InitIdx();
                    // std::cout << "opIdx: " << opIdx << std::endl;
                    assert(opIdx < dfgNode->numInputs(cg_width));
                    int rduId = usedRdu[cg_width];
                    auto rdu = subAdg->node(rduId); // used default delay 
                    int muxId = rdu->input(cg_width, opIdx).first;
                    for(auto& elem_i : subAdg->node(muxId)->inputs(cg_width)){
                        int id = elem_i.second.first;
                        if(id == subAdg->id()) continue;
                        if(subAdg->node(id)->type() == "Second_Const"){
                            addCfgData(cfg, node->configInfo(muxId), (uint32_t)elem_i.first);
                            // std::cout << " id: " << id << std::endl;
                            // std::cout << " const: " << (int)dfgNode->get2ndInit() << std::endl;
                            addCfgData(cfg, node->configInfo(id), (uint64_t)(dfgNode->get2ndInit()));
                            break;
                        }
                    }
                }
            }else if(dfgNode->initSelection()){// @yuan: for init-selection nodes, which can also be ALU Type only
                // write interval
                int II =  _mapping->II();
                int WI = dfgNode->interval() * II;
                int wiId = node->cfgIdMap["WI"];
                addCfgData(cfg, node->configInfo(wiId), (uint32_t)WI);
                // std::cout << "isel WI: " << WI << std::endl;
                // node latency
                auto& dfgNodeAttr = _mapping->dfgNodeAttr(dfgNode->id());
                int latency = dfgNodeAttr.lat - dfgNode->opLatency(); 
                int latencyId = node->cfgIdMap["Latency"];
                addCfgData(cfg, node->configInfo(latencyId), (uint32_t)latency);
                // std::cout << "isel latency: " << latency << std::endl;
                // write cycles
                int cycles = dfgNode->cycles();
                int cyclesId = node->cfgIdMap["Cycles"];
                addCfgData(cfg, node->configInfo(cyclesId), (uint32_t)cycles);
                // std::cout << "isel cycles: " << cycles << std::endl;
                //wirte repeats number
                int repeats = dfgNode->repeats();
                int repeatsId = node->cfgIdMap["Repeats"];
                addCfgData(cfg, node->configInfo(repeatsId), (uint32_t)repeats);
                // std::cout << "isel cycles: " << cycles << std::endl;
                // //if skip first accumulative num
                // bool skipfisrt = !dfgNode->isAccFirst();
                // int skipfisrtId = node->cfgIdMap["SkipFirst"];
                // addCfgData(cfg, node->configInfo(skipfisrtId), (uint32_t)skipfisrt);
            }
        }
        else if(type == TYPE_LUT){
            // std::cout << "dfgNode: " << dfgNode->name() << " operatopn: " << dfgNode->operation() << " GPE NodeId: " << node->id()<<std::endl;
            for(auto& elem : dfgNode->inputEdges()){
                int bitWidth = elem.first;// for each bitwidth, is 1-bit
                for(auto& edge : elem.second){ // for each edge
                    int eid = edge.second;
                    DFGEdge* e = _mapping->getDFG()->edge(eid);
                    DFGNode* src = _mapping->getDFG()->node(e->srcId()); 
                    DFGNode* dst = _mapping->getDFG()->node(e->dstId()); 
                    auto& edgeAttr = _mapping->dfgEdgeAttr(eid);
                    int delay = edgeAttr.delay; // delay cycles
                    int inputIdx = edgeAttr.edgeLinks.rbegin()->srcPort; // last edgeLInk, dst port
                    // std::cout << "edge from: " << src->name() << " to: " << dst->name() << " width: " <<  bitWidth <<" inputIdx: "<<inputIdx<< std::endl;
                    if(inputIdx == -1){
                        int dstPort = e->dstPortIdx();
                        int Operand = node->getOperandIdxLUT(dstPort);
                        inputIdx = *node->operandInputs(bitWidth, Operand).begin();
                        auto muxPair = subAdg->input(bitWidth, inputIdx).begin(); // one input only connected to one Mux, @yuan: for sub-adg, the input() returns the relationship between inport and the sub-module
                        int muxId = muxPair->first;
                        auto mux = subAdg->node(muxId);
                        for(auto& sm2mux : mux->inputs(bitWidth)){
                            int id = sm2mux.second.first;
                            if(id == subAdg->id()) continue;
                            if(subAdg->node(id)->type() == "RF4ALU"){ // filled with zero
                                addCfgData(cfg, node->configInfo(muxId), (uint32_t)sm2mux.first);
                                // std::cout << "muxId: " <<muxId<<" muxCfgData: " <<(uint32_t)sm2mux.first<< std::endl;
                                break;
                            }
                        }
                        auto rduPair = mux->output(bitWidth, 0).begin();
                        fg_rduId = rduPair->first;  
                        int rduPort = rduPair->second;
                        auto rdu = subAdg->node(fg_rduId);   
                        delayUsed[bitWidth][fg_rduId][rduPort] = edgeAttr.delay; // delay cycles used by this port   
                        auto lutPair = rdu->output(bitWidth, rduPort).begin();  // RDU has the same input/output index
                        lutId = lutPair->first;
                        usedOperands[type].emplace(Operand); // operand index
                        // std::cout << "dstPort: " << dstPort << " Operand: " << Operand << " delay: "<<edgeAttr.delay<< std::endl;
                        continue;
                    } //exit(0);
                    auto muxPair = subAdg->input(bitWidth, inputIdx).begin(); // one input only connected to one Mux, @yuan: for sub-adg, the input() returns the relationship between inport and the sub-module
                    int muxId = muxPair->first;
                    int muxCfgData = muxPair->second;
                    auto mux = subAdg->node(muxId);
                    auto rduPair = mux->output(bitWidth, 0).begin();
                    fg_rduId = rduPair->first;  
                    int rduPort = rduPair->second;
                    auto rdu = subAdg->node(fg_rduId);   
                    delayUsed[bitWidth][fg_rduId][rduPort] = edgeAttr.delay; // delay cycles used by this port
                    auto lutPair = rdu->output(bitWidth, rduPort).begin();  // RDU has the same input/output index
                    lutId = lutPair->first;
                    int oprand = rduPort - node->numOperands(1) + node->numInputLUT();
                    // std::cout << "lutid: " <<lutId<<" fg_rduId: " <<fg_rduId<<"rduPort: " << rduPort << " operand: " << oprand<< " DELAY: " << edgeAttr.delay <<std::endl;
                    // std::cout << "muxId: " <<muxId<<" muxCfgData: " <<muxCfgData<< std::endl;
                    usedOperands[type].emplace(oprand); // operand index
                    addCfgData(cfg, node->configInfo(muxId), (uint32_t)muxCfgData);
                }
            }
            uint32_t lutCfgData = 0;
            if(lutId == -1){// in case that some node has no input, the output of lut is 0
                auto muxPair = subAdg->input(1, 0).begin(); // one input only connected to one Mux
                int muxId = muxPair->first;
                auto mux = subAdg->node(muxId);
                fg_rduId = mux->output(1, 0).begin()->first; 
                auto rdu = subAdg->node(fg_rduId);
                auto lutpair = rdu->output(1, 0).begin();
                lutId = lutpair->first;
                lutCfgData = 0;
                // addCfgData(cfg, node->configInfo(lutId), lutCfg);
            }else if(dfgNode->LUTsize() < node->numInputLUT()){// @yuan: should consider the width of GPE's lut > dfg lut node
                //@yuan: lut node occupies the GPE's lut from the lowest bit 
                int dfg_lut_size = dfgNode->LUTsize();
                int num = node->numInputLUT() - dfg_lut_size; // the number of GPE's lut to be filled
                for(int i = 0; i < num; i++){
                    int operand = dfg_lut_size + i;
                    operand = node->getOperandIdxLUT(operand);
                    // std::cout << "operand: " << operand << " fill with 0" << std::endl;
                    auto rdu = subAdg->node(fg_rduId); // used default delay 
                    int muxId = rdu->input(1, operand).first;
                    // std::cout << "muxId: " << muxId << std::endl;
                    for(auto& elem_i : subAdg->node(muxId)->inputs(1)){
                        int id = elem_i.second.first;
                        if(id == subAdg->id()) continue;
                        if(subAdg->node(id)->type() == "Const0"){ // filled with zero
                            addCfgData(cfg, node->configInfo(muxId), (uint32_t)elem_i.first);
                            break;
                        }
                    }
                }
            }
            std::string LutCfg = dfgNode->LUTconfig();
            lutCfgData = std::stoul(LutCfg.substr(), nullptr, 2);
            addCfgData(cfg, node->configInfo(lutId), lutCfgData);
            // std::cout << "lutId: " <<lutId<<" LUT config: " << LutCfg << " uint: "<<lutCfgData <<" lut size: " << node->numInputLUT()<<std::endl;
            if(dfgNode->hasImm(1)){
                int bit_value = dfgNode->imm(1).second;
                assert(bit_value <= 1);
                // @yuan: for 1-bit const, its operand is certain
                int opId = dfgNode->imm(1).first;
                int operand = node->getOperandIdxLUT(opId);
                auto rdu = subAdg->node(fg_rduId); // used default delay 
                int muxId = rdu->input(1, operand).first;
                // std::cout << "lut imm muxId: " << muxId <<" opId: "<<operand<< std::endl;
                for(auto& elem_i : subAdg->node(muxId)->inputs(1)){
                    int id = elem_i.second.first;
                    if(id == subAdg->id()) continue;
                    if(subAdg->node(id)->type() == "Const1" && bit_value == 1){
                        addCfgData(cfg, node->configInfo(muxId), (uint32_t)elem_i.first);
                    }else if(subAdg->node(id)->type() == "Const0" && bit_value == 0){
                        addCfgData(cfg, node->configInfo(muxId), (uint32_t)elem_i.first);
                    }
                }
            } 
        }
    }
    
    //@yuan: we deal with the RDU configuration finally
    // std::cout << "config RDU for GPE~~~" << std::endl;
    for(auto& eachwidth : delayUsed){
        int bitWidth = eachwidth.first;
        for(auto& elem : eachwidth.second ){
            int rduId = elem.first;
            CfgDataLoc rduCfgLoc = node->configInfo(rduId);
            uint64_t delayCfg = 0;
            int eachDelayWidth = (rduCfgLoc.high - rduCfgLoc.low + 1) / node->numOperands(bitWidth);
            for(auto& used : elem.second){
                // if(bitWidth==1) std::cout << "eachDelayWidth: "<<eachDelayWidth<<" port: "<<used.first<<" used delay: " << used.second <<" eachDelayWidth * used.first" <<eachDelayWidth * used.first<< std::endl;
                // if(bitWidth==1)std::cout << "delayCfg_init: " << delayCfg << std::endl;
                uint64_t used_delay = (uint64_t)used.second << (eachDelayWidth * used.first);
                delayCfg |= used_delay;
                // if(bitWidth==1)std::cout << "delayCfg: " << delayCfg << std::endl;
            }
            // if(bitWidth==1) std::cout << "delayCfg: " << delayCfg << std::endl;
            addCfgData(cfg, rduCfgLoc, (uint64_t)delayCfg);
        }
    }
    // std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    return cfg;
}


// get config data for GIB, return<LSB-location, CfgData>
std::map<int, CfgData> Configuration::getGibCfgData(GIBNode* node){
    int adgNodeId = node->id();
    int bitWidth = *node->bitWidths().begin(); // one GIB has only one width
    ADG* subAdg = node->subADG();
    auto& adgNodeAttr = _mapping->adgNodeAttr(adgNodeId);
    auto& passEdges = adgNodeAttr.dfgEdgePass;
    if(passEdges.empty()){
        return {};
    }
    // std::cout << "GIB Node: " << node->name() << " has edge num: " << passEdges.size() << std::endl;
    std::map<int, CfgData> cfg;
    for(auto& elem : passEdges){
        int muxId = subAdg->output(bitWidth, elem.dstPort).first; // one output connected to one mux
        if(muxId == subAdg->id()){ // actually connected to input port
            continue;
        }
        DFGEdge* e = elem.edge;
        // std::cout << "edge from: " << _mapping->getDFG()->node(e->srcId())->name() << " to " << _mapping->getDFG()->node(e->dstId())->name() << " srcport: " << elem.srcPort << " dstport: " << elem.dstPort << std::endl;
        auto mux = subAdg->node(muxId);        
        // find srcPort
        for(auto in : mux->inputs()){
            if(in.first != bitWidth) continue;
            for(auto in_width : in.second){
                if(in_width.second.second == elem.srcPort){ 
                    // std::cout << "muxid: " << muxId << " muxdata: " << (uint32_t)in_width.first << std::endl;
                    addCfgData(cfg, node->configInfo(muxId), (uint32_t)in_width.first);
                    // CfgDataLoc muxCfgLoc = node->configInfo(muxId);
                    // CfgData muxCfg(muxCfgLoc.high - muxCfgLoc.low + 1, (uint32_t)in.first);
                    // cfg[muxCfgLoc.low] = muxCfg;
                    break;
                }
            }
        }
    // std::cout << "one edge" << std::endl;
    }
    // std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    return cfg;
}

// get config data for IOB, return<LSB-location, CfgData>
std::map<int, CfgData> Configuration::getIobCfgData(IOBNode* node){
    int type = TYPE_IOC; // IOB only has one type
    // std::cout << "IOBNode: " << node->name() << std::endl;
    if(!_mapping->isMapped(node, type)){
        return {};
    }
    int adgNodeId = node->id();
    ADG* subAdg = node->subADG();
    auto& adgNodeAttr = _mapping->adgNodeAttr(adgNodeId);
    auto mappedDFGNodes = adgNodeAttr.dfgNodes;
    DFGNode* dfgNode = mappedDFGNodes[type];
    // for(auto& elem : adgNodeAttr.dfgNodes){
    //     if(elem.first == type){
    //         dfgNode = elem.second;
    //     }
    // }
    DFGIONode* dfgIONode = dynamic_cast<DFGIONode*>(dfgNode);
    // if(dfgIONode == NULL){
    //     std::cout << "cast fail " << std::endl;
    // }
    auto& dfgNodeAttr = _mapping->dfgNodeAttr(dfgNode->id());
    std::map<int, CfgData> cfg;
    // int ioctrlId = subAdg->output(0).first; // IOB has only one output connected to IOController
    // CfgDataLoc ioctrlCfgLoc = node->configInfo(ioctrlId);
    // int ioctrlCfgLen = ioctrlCfgLoc.high - ioctrlCfgLoc.low + 1;
    // CfgData ioctrlCfg(ioctrlCfgLen);
    // while(ioctrlCfgLen > 0){ // config data of IOController is set by the host
    //     ioctrlCfg.data.push_back((uint32_t)0);
    //     ioctrlCfgLen -= 32;
    // }
    // cfg[ioctrlCfgLoc.low] = ioctrlCfg;
    int isLoad = 0;
    auto dfg = _mapping->getDFG();
    bool isUsedAsOB = dfg->getOutNodes().count(dfgNode->id());
    if(isUsedAsOB){ // IOB used as OB
        isLoad = 0;     
    }else{ // IOB used as IB
        isLoad = 1;
    }
    int II = _mapping->II();
    int latency = dfgNodeAttr.lat - dfgNode->opLatency(); // substract load/store latency
    int bitWidth = _mapping->getDFG()->CGWidth();
    // for(auto& elem : dfgNode->bitWidths()){
    //     if(elem != 1){
    //         bitWidth = elem;
    //         break;
    //     }
    // }
    // std::cout << "IOBNode: " << node->name() << " DFGNode: " << dfgNode->name() << std::endl;
    int dataBytes = bitWidth/ 8;
    // int baseAddr = _dfgIoSpadAddrs[dfgNode->id()];
    int baseAddr = _dfgIoSpadAddrs[dfgNode->id()]; 
    // std::cout << "baseAddr: " << baseAddr << std::endl;
    int offset = dfgIONode->reducedMemOffset() / dataBytes;
    // std::cout << "offset: " << offset << std::endl;
    int memoffset = dfgIONode->memOffset() / dataBytes;
    // std::cout << "memoffset: " << memoffset << std::endl;
    if(memoffset < 0){
        offset += memoffset;
    }
    auto agd = _mapping->getADG();
    int systemBus_Width_inByte = agd->cfgSpadDataWidth() / 8;
    // std::cout << "systemBus_Width: " << systemBus_Width_inByte << std::endl;
    // int Padding_Difference = closestPowerOfTwoDifference(dfgIONode->memOffset());
    int Padding_Difference = biasNumInOneTrans(dfgIONode->memOffset(), systemBus_Width_inByte);
    // std::cout << "Padding_Difference: " << Padding_Difference << std::endl;
    if(Padding_Difference > 1){
        // int numPadding = systemBus_Width_inByte / Padding_Difference;
        int numPadding = Padding_Difference / dataBytes;
        // std::cout << "numPadding: " << numPadding << std::endl;
        offset += numPadding;
    }
    // std::cout << "offset_c: " << offset << std::endl;
    // int offset = 32 / dataBytes;//@yuan: for test
    int baseAddrId = node->cfgIdMap["BaseAddr"];    
    // // CfgDataLoc baseAddrCfgLoc = node->configInfo(baseAddrId);
    // // int baseAddrCfgLen = baseAddrCfgLoc.high - baseAddrCfgLoc.low + 1;
    // // CfgData baseAddrCfg(baseAddrCfgLen);    
    // // baseAddrCfg.data.push_back((uint32_t)(baseAddr+offset)); 
    // // cfg[baseAddrCfgLoc.low] = baseAddrCfg;
    addCfgData(cfg, node->configInfo(baseAddrId), (uint32_t)(baseAddr+offset));
    // std::cout << "baseAddr: " << baseAddr <<" offset: "<<offset<< std::endl;
    // std::cout << "Base Addr cfg size: " << cfg.size() << std::endl;
    int dfgNestedLevels = dfgIONode->getNestedLevels();
    //int iobNestedLevels = 10;//@yuan: for test
    int iobNestedLevels = _mapping->getADG()->iobAgNestLevels();
    // std::cout << "dfgNestedLevels: " << dfgNestedLevels << " iobNestedLevels: " << iobNestedLevels << std::endl;
    assert(dfgNestedLevels <= iobNestedLevels);
    auto& pattern = dfgIONode->pattern();
    for(int i = 0; i < iobNestedLevels; i++){
        int stride = 0;
        int cycles = 0;
        if(i < dfgNestedLevels){
            stride = pattern[i].first;
            assert(stride % dataBytes == 0);
            stride = stride / dataBytes;
            cycles = pattern[i].second;
        }
        // std::cout << "pattern: " << i << " = " << pattern[i].first << " stride: " << stride << std::endl;
        // std::string strideName = "Stride"; // + std::to_string(i);
        std::string strideName = "Stride" + std::to_string(i);
        int strideId = node->cfgIdMap[strideName];
        CfgDataLoc strideCfgLoc = node->configInfo(strideId);
        int strideCfgLen = strideCfgLoc.high - strideCfgLoc.low + 1;
        // std::cout << "strideCfgLen: " << strideCfgLen << std::endl;
        uint32_t strideAlign = stride & ((1 << strideCfgLen) - 1);
        // std::cout << "(1 << strideCfgLen) - 1: " << (1 << strideCfgLen) - 1 << std::endl;
        // std::cout << "strideId: " << strideId << " strideAlign: " << strideAlign << std::endl;
        // CfgData strideCfg(strideCfgLen);        
        // strideCfg.data.push_back(strideAlign); 
        // cfg[strideCfgLoc.low] = strideCfg;
        addCfgData(cfg, strideCfgLoc, (uint32_t)strideAlign);
    // std::cout << "strideId cfg size: " << cfg.size() << std::endl;
        // std::string cyclesName = "Cycles"; // + std::to_string(i);
        std::string cyclesName = "Cycles" + std::to_string(i);
        int cyclesId = node->cfgIdMap[cyclesName];
        addCfgData(cfg, node->configInfo(cyclesId), (uint32_t)cycles);
        // std::string test = "Cycles"  + std::to_string(i);
        // int testId = node->cfgIdMap[test];
        // std::cout << "cyclesId: " << cyclesId << " cycles: " << cycles << std::endl;
        // CfgDataLoc cyclesCfgLoc = node->configInfo(cyclesId);
        // int cyclesCfgLen = cyclesCfgLoc.high - cyclesCfgLoc.low + 1;
        // CfgData cyclesCfg(cyclesCfgLen);
        // cyclesCfg.data.push_back((uint32_t)cycles); 
        // cfg[cyclesCfgLoc.low] = cyclesCfg;
    }
    int iiId = node->cfgIdMap["II"];
    addCfgData(cfg, node->configInfo(iiId), (uint32_t)II);
    // std::cout << "iiId: " << iiId << " ii: " << II << std::endl;
    // CfgDataLoc iiCfgLoc = node->configInfo(iiId);
    // int iiCfgLen = iiCfgLoc.high - iiCfgLoc.low + 1;
    // CfgData iiCfg(iiCfgLen);
    // iiCfg.data.push_back((uint32_t)II); 
    // cfg[iiCfgLoc.low] = iiCfg;
    int latencyId = node->cfgIdMap["Latency"];
    addCfgData(cfg, node->configInfo(latencyId), (uint32_t)latency);
    // std::cout << "latencyId: " << latencyId << " latency: " << latency << std::endl;
    // CfgDataLoc latencyCfgLoc = node->configInfo(latencyId);
    // int latencyCfgLen = latencyCfgLoc.high - latencyCfgLoc.low + 1;
    // CfgData latencyCfg(latencyCfgLen);
    // latencyCfg.data.push_back((uint32_t)latency); 
    // cfg[latencyCfgLoc.low] = latencyCfg;
    int isLoadId = node->cfgIdMap["IsLoad"];
    addCfgData(cfg, node->configInfo(isLoadId), (uint32_t)isLoad);
    // std::cout << "isLoadId: " << isLoadId << " isLoad: " << isLoad << std::endl;
    // CfgDataLoc isStoreCfgLoc = node->configInfo(isStoreId);
    // int isStoreCfgLen = isStoreCfgLoc.high - isStoreCfgLoc.low + 1;
    // CfgData isStoreCfg(isStoreCfgLen);
    // isStoreCfg.data.push_back((uint32_t)isStore); 
    // cfg[isStoreCfgLoc.low] = isStoreCfg;
    int N = 1;
    int B = 1;
    int startBankIdx = 0;
    int parMode = 0;
    // std::cout << "dfgIONode->MultiportType(): " << dfgIONode->MultiportType() << std::endl;
    if(dfgIONode->NumMultiportBank() > 1 || dfgIONode->MultiportType() > 0){
        auto StartBanks = partitionedStartBank();
        std::string memName = dfgIONode->memRefName();
        if(StartBanks.count(memName)){
            startBankIdx = StartBanks[memName];
            // std::cout << "~~~~" << std::endl;
        } 
        // parMode = dfgIONode->PartitionMode();
        N = dfgIONode->NumMultiportBank();
        B = dfgIONode->MultiportBankSize();
        // std::cout << "startBankIdx: " << startBankIdx << " N: " << N << " B: " << B << std::endl;
    }
    if(node->cfgIdMap.count("startBankIdx")){
        int startBankId = node->cfgIdMap["startBankIdx"];
        addCfgData(cfg, node->configInfo(startBankId), (uint32_t)startBankIdx);
    }
    if(node->cfgIdMap.count("logN")){
        int logNId = node->cfgIdMap["logN"];
        int logN = log2(N);
        addCfgData(cfg, node->configInfo(logNId), (uint32_t)logN);
        // std::cout <<"N: "<<N<< " logN: " << logN << std::endl;
    }
    if(node->cfgIdMap.count("logB")){
        int logBId = node->cfgIdMap["logB"];
        int logB = log2(B);
        addCfgData(cfg, node->configInfo(logBId), (uint32_t)logB);
        // std::cout << "B: "<<B<<" logB: " << logB << std::endl;
    }
    // if(node->cfgIdMap.count("parMode")){
    //     int parModeId = node->cfgIdMap["parMode"];
    //     addCfgData(cfg, node->configInfo(parModeId), (uint32_t)parMode);
    //     // std::cout << "parMode: " << parMode << std::endl;
    // }
    auto op = dfgNode->operation();
    if(node->cfgIdMap.count("UseAddr")){        
        int useAddr = op == "LOAD" || op == "STORE" || op == "CLOAD" || op == "CSTORE"|| op == "TLOAD" || op == "TSTORE"|| op == "TCLOAD" || op == "TCSTORE";
        int useAddrId = node->cfgIdMap["UseAddr"];
        addCfgData(cfg, node->configInfo(useAddrId), (uint32_t)useAddr);
        // std::cout << "useAddrId: " << useAddrId << " useAddr: " << useAddr << std::endl;
        // CfgDataLoc useAddrCfgLoc = node->configInfo(useAddrId);
        // int useAddrCfgLen = useAddrCfgLoc.high - useAddrCfgLoc.low + 1;
        // CfgData useAddrCfg(useAddrCfgLen);
        // useAddrCfg.data.push_back((uint32_t)useAddr); 
        // cfg[useAddrCfgLoc.low] = useAddrCfg;
    }
    if(node->cfgIdMap.count("UseFGIn")){        
        int useFGIn = op == "CLOAD" || op == "CSTORE" || op == "CINPUT" || op == "COUTPUT"|| op == "TCLOAD" || op == "TCSTORE";
        int useFGInId = node->cfgIdMap["UseFGIn"];
        addCfgData(cfg, node->configInfo(useFGInId), (uint32_t)useFGIn);
        // std::cout << "op: " << op << " useFGInId: " << useFGInId << " useFGIn: " << useFGIn << std::endl;
        // CfgDataLoc useEnCfgLoc = node->configInfo(useEnId);
        // int useEnCfgLen = useEnCfgLoc.high - useEnCfgLoc.low + 1;
        // CfgData useEnCfg(useEnCfgLen);
        // useEnCfg.data.push_back((uint32_t)useEn); 
        // cfg[useEnCfgLoc.low] = useEnCfg;
    }
    //@yuan: when the memory dependency edge exists, use fine-grain out
    if(node->cfgIdMap.count("UseFGOut")){
        int useFGOutId = node->cfgIdMap["UseFGOut"];
        int useFGOut = 0;
        if(op == "OUTPUT"|| op == "STORE" || op == "CSTORE" || op == "COUTPUT"|| op == "TCSTORE"){
            const std::map<int, std::set<int>>& result = dfgIONode->outputEdges(1);
            if(!result.empty()){
                useFGOut = 1;
            }
        }
        // std::cout << "useFGOut: " << useFGOut << std::endl;
        addCfgData(cfg, node->configInfo(useFGOutId), (uint32_t)useFGOut);
    } 
    if(node->cfgIdMap.count("useTask")){
        int useTaskId = node->cfgIdMap["useTask"];
        int useTask = op == "TLOAD" || op == "TSTORE"|| op == "TCLOAD" || op == "TCSTORE";
        // std::cout << "useTask: " << useTask << std::endl;
        addCfgData(cfg, node->configInfo(useTaskId), (uint32_t)useTask);
    }
    bool haveDontTouchFGIn = true;
    if(op != "INPUT"){ // only INPUT node donot use Mux     
        int rduId;
        std::map<int, std::map<int, std::map<int, int>>> delayUsed; // <RDU-id, delayused>
        for(auto& elem : dfgNode->inputEdges()){
            int width = elem.first;
            for(auto& edge : elem.second){
                int eid = edge.second;
                auto e = dfg->edge(eid);
                if(e->isMemEdge()) continue;
                if(width == 1) haveDontTouchFGIn = false;
                // std::cout << "eid: " << eid << ", " << dfg->node(e->srcId())->name() << " -> " << dfg->node(e->dstId())->name() << std::endl;
                auto& edgeAttr = _mapping->dfgEdgeAttr(eid);
                int delay = edgeAttr.delay; // delay cycles
                int inputIdx = edgeAttr.edgeLinks.rbegin()->srcPort; // last edgeLInk, dst port
                // std::cout << "inputIdx: " << inputIdx << std::endl;
                auto muxPair = subAdg->input(width, inputIdx).begin(); // one input only connected to one Mux
                int muxId = muxPair->first;
                int muxCfgData = muxPair->second;
                // std::cout << "muxId: " << muxId << " muxCfgData: " << muxCfgData << std::endl;
                auto mux = subAdg->node(muxId);
                addCfgData(cfg, node->configInfo(muxId), (uint32_t)muxCfgData);
                // CfgDataLoc muxCfgLoc = node->configInfo(muxId);
                // CfgData muxCfg((muxCfgLoc.high - muxCfgLoc.low + 1), (uint32_t)muxCfgData);
                // cfg[muxCfgLoc.low] = muxCfg;
                auto rduPair = mux->output(width, 0).begin();
                rduId = rduPair->first; 
                int rduPort = rduPair->second;
                delayUsed[width][rduId][rduPort] = edgeAttr.delay; // delay cycles used by this port
                auto rdu = subAdg->node(rduId);            
                // if(rdu->type() != "RDU"){
                //     delayUsed[rduPort] = edgeAttr.delay; // delay cycles used by this port
                //     addCfgData(cfg, node->configInfo(rduId), (uint32_t)delay);
                //     // CfgDataLoc rduCfgLoc = node->configInfo(rduId);
                //     // CfgData rduCfg(rduCfgLoc.high - rduCfgLoc.low + 1, (uint32_t)delay);
                //     // cfg[rduCfgLoc.low] = rduCfg;      
                // } 
            }        
        }  
        // RDU
        //@yuan: we deal with the RDU configuration finally
        for(auto& eachwidth : delayUsed){
            int bitWidth = eachwidth.first;
            for(auto& elem : eachwidth.second ){
                int rduId = elem.first;
                CfgDataLoc rduCfgLoc = node->configInfo(rduId);
                uint32_t delayCfg = 0;
                int eachDelayWidth = (rduCfgLoc.high - rduCfgLoc.low + 1) / node->numOperands(bitWidth);
                for(auto& used : elem.second){
                    delayCfg |= used.second << (eachDelayWidth * used.first);
                }
                // std::cout << "rduId: " << rduId << " delayCfg: " << delayCfg << std::endl;
                addCfgData(cfg, rduCfgLoc, (uint32_t)delayCfg);
            }
        }
    }
    if(node->cfgIdMap.count("UseFGIn")){        
        int useFGIn = ((op == "CLOAD" || op == "CSTORE" || op == "CINPUT" || op == "COUTPUT"|| op == "TCLOAD" || op == "TCSTORE") && !haveDontTouchFGIn);
        int useFGInId = node->cfgIdMap["UseFGIn"];
        addCfgData(cfg, node->configInfo(useFGInId), (uint32_t)useFGIn);
        // std::cout << "useFGInId: " << useFGInId << " useFGIn: " << useFGIn << std::endl;
        // CfgDataLoc useEnCfgLoc = node->configInfo(useEnId);
        // int useEnCfgLen = useEnCfgLoc.high - useEnCfgLoc.low + 1;
        // CfgData useEnCfg(useEnCfgLen);
        // useEnCfg.data.push_back((uint32_t)useEn); 
        // cfg[useEnCfgLoc.low] = useEnCfg;
    }
    //@yuan: when the memory dependency edge exists, use fine-grain out
    if(node->cfgIdMap.count("UseFGOut")){
        int useFGOutId = node->cfgIdMap["UseFGOut"];
        int useFGOut = 0;
        if(op == "OUTPUT"|| op == "STORE" || op == "CSTORE" || op == "COUTPUT"|| op == "TCSTORE"){
            const std::map<int, std::set<int>>& result = dfgIONode->outputEdges(1);
            if(!result.empty()){
                useFGOut = 1;
            }
        }
        // std::cout << "useFGOut: " << useFGOut << std::endl;
        addCfgData(cfg, node->configInfo(useFGOutId), (uint32_t)useFGOut);
    } 
    // std::cout << "cfg size: " << cfg.size() << std::endl;
    // std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
    return cfg;
}


// // get config data for IOB, return<LSB-location, CfgData>
// std::map<int, CfgData> Configuration::getIobCfgData(IOBNode* node){
//     int type = TYPE_IOC; // IOB only has one type
//     if(!_mapping->isMapped(node, type)){
//         return {};
//     }
//     int adgNodeId = node->id();
//     ADG* subAdg = node->subADG();
//     auto& adgNodeAttr = _mapping->adgNodeAttr(adgNodeId);
//     DFGNode* dfgNode = nullptr;
//     for(auto& elem : adgNodeAttr.dfgNodes){
//         if(elem.first == type){
//             dfgNode = elem.second;
//         }
//     }
//     auto& dfgNodeAttr = _mapping->dfgNodeAttr(dfgNode->id());
//     DFGIONode* dfgIONode = dynamic_cast<DFGIONode*>(dfgNode);
//     std::map<int, CfgData> cfg;
//     // int ioctrlId = subAdg->output(0).first; // IOB has only one output connected to IOController
//     // CfgDataLoc ioctrlCfgLoc = node->configInfo(ioctrlId);
//     // int ioctrlCfgLen = ioctrlCfgLoc.high - ioctrlCfgLoc.low + 1;
//     // CfgData ioctrlCfg(ioctrlCfgLen);
//     // while(ioctrlCfgLen > 0){ // config data of IOController is set by the host
//     //     ioctrlCfg.data.push_back((uint32_t)0);
//     //     ioctrlCfgLen -= 32;
//     // }
//     // cfg[ioctrlCfgLoc.low] = ioctrlCfg;
//     int isStore = 0;
//     auto dfg = _mapping->getDFG();
//     bool isUsedAsOB = dfg->getOutNodes().count(dfgNode->id());
//     if(isUsedAsOB){ // IOB used as OB
//         isStore = 1;     
//     }else{ // IOB used as IB
//         isStore = 0;
//     }
//     int II = 1;
//     int latency = dfgNodeAttr.lat - dfgNode->opLatency(); // substract load/store latency
//     int bitWidth = 0;
//     for(auto& elem : dfgNode->bitWidths()){
//         if(elem != 1){
//             bitWidth = elem;
//             break;
//         }
//     }
//     int dataBytes = bitWidth/ 8;
//     // int baseAddr = _dfgIoSpadAddrs[dfgNode->id()];
//     int baseAddr = 0; //@yuan: for test
//     int offset = dfgIONode->reducedMemOffset() / dataBytes;
//     // int offset = 32 / dataBytes;//@yuan: for test
//     int baseAddrId = node->cfgIdMap["BaseAddr"];    
//     // // CfgDataLoc baseAddrCfgLoc = node->configInfo(baseAddrId);
//     // // int baseAddrCfgLen = baseAddrCfgLoc.high - baseAddrCfgLoc.low + 1;
//     // // CfgData baseAddrCfg(baseAddrCfgLen);    
//     // // baseAddrCfg.data.push_back((uint32_t)(baseAddr+offset)); 
//     // // cfg[baseAddrCfgLoc.low] = baseAddrCfg;
//     addCfgData(cfg, node->configInfo(baseAddrId), (uint32_t)(baseAddr+offset));
//     int dfgNestedLevels = dfgIONode->getNestedLevels();
//     //int iobNestedLevels = 10;//@yuan: for test
//     int iobNestedLevels = _mapping->getADG()->iobAgNestLevels();
//     assert(dfgNestedLevels <= iobNestedLevels);
//     auto& pattern = dfgIONode->pattern();
//     for(int i = 0; i < iobNestedLevels; i++){
//         int stride = 0;
//         int cycles = 0;
//         if(i < dfgNestedLevels){
//             stride = pattern[i].first;
//             assert(stride % dataBytes == 0);
//             stride = stride / dataBytes;
//             cycles = pattern[i].second;
//         }
//         std::string strideName = "Stride" + std::to_string(i);
//         int strideId = node->cfgIdMap[strideName];
//         CfgDataLoc strideCfgLoc = node->configInfo(strideId);
//         int strideCfgLen = strideCfgLoc.high - strideCfgLoc.low + 1;
//         uint32_t strideAlign = stride & ((1 << strideCfgLen) - 1);
//         // CfgData strideCfg(strideCfgLen);        
//         // strideCfg.data.push_back(strideAlign); 
//         // cfg[strideCfgLoc.low] = strideCfg;
//         addCfgData(cfg, strideCfgLoc, (uint32_t)strideAlign);
//         std::string cyclesName = "Cycles" + std::to_string(i);
//         int cyclesId = node->cfgIdMap[cyclesName];
//         addCfgData(cfg, node->configInfo(cyclesId), (uint32_t)cycles);
//         // CfgDataLoc cyclesCfgLoc = node->configInfo(cyclesId);
//         // int cyclesCfgLen = cyclesCfgLoc.high - cyclesCfgLoc.low + 1;
//         // CfgData cyclesCfg(cyclesCfgLen);
//         // cyclesCfg.data.push_back((uint32_t)cycles); 
//         // cfg[cyclesCfgLoc.low] = cyclesCfg;
//     }
//     int iiId = node->cfgIdMap["II"];
//     addCfgData(cfg, node->configInfo(iiId), (uint32_t)II);
//     // CfgDataLoc iiCfgLoc = node->configInfo(iiId);
//     // int iiCfgLen = iiCfgLoc.high - iiCfgLoc.low + 1;
//     // CfgData iiCfg(iiCfgLen);
//     // iiCfg.data.push_back((uint32_t)II); 
//     // cfg[iiCfgLoc.low] = iiCfg;
//     int latencyId = node->cfgIdMap["Latency"];
//     addCfgData(cfg, node->configInfo(latencyId), (uint32_t)latency);
//     // CfgDataLoc latencyCfgLoc = node->configInfo(latencyId);
//     // int latencyCfgLen = latencyCfgLoc.high - latencyCfgLoc.low + 1;
//     // CfgData latencyCfg(latencyCfgLen);
//     // latencyCfg.data.push_back((uint32_t)latency); 
//     // cfg[latencyCfgLoc.low] = latencyCfg;
//     int isStoreId = node->cfgIdMap["IsStore"];
//     addCfgData(cfg, node->configInfo(isStoreId), (uint32_t)isStore);
//     // CfgDataLoc isStoreCfgLoc = node->configInfo(isStoreId);
//     // int isStoreCfgLen = isStoreCfgLoc.high - isStoreCfgLoc.low + 1;
//     // CfgData isStoreCfg(isStoreCfgLen);
//     // isStoreCfg.data.push_back((uint32_t)isStore); 
//     // cfg[isStoreCfgLoc.low] = isStoreCfg;
//     auto op = dfgNode->operation();
//     if(node->cfgIdMap.count("UseAddr")){        
//         int useAddr = op == "LOAD" || op == "STORE" || op == "CLOAD" || op == "CSTORE";
//         int useAddrId = node->cfgIdMap["UseAddr"];
//         addCfgData(cfg, node->configInfo(useAddrId), (uint32_t)useAddr);
//         // CfgDataLoc useAddrCfgLoc = node->configInfo(useAddrId);
//         // int useAddrCfgLen = useAddrCfgLoc.high - useAddrCfgLoc.low + 1;
//         // CfgData useAddrCfg(useAddrCfgLen);
//         // useAddrCfg.data.push_back((uint32_t)useAddr); 
//         // cfg[useAddrCfgLoc.low] = useAddrCfg;
//     }
//     if(node->cfgIdMap.count("UseEn")){        
//         int useEn = op == "CLOAD" || op == "CSTORE";
//         int useEnId = node->cfgIdMap["UseEn"];
//         addCfgData(cfg, node->configInfo(useEnId), (uint32_t)useEn);
//         // CfgDataLoc useEnCfgLoc = node->configInfo(useEnId);
//         // int useEnCfgLen = useEnCfgLoc.high - useEnCfgLoc.low + 1;
//         // CfgData useEnCfg(useEnCfgLen);
//         // useEnCfg.data.push_back((uint32_t)useEn); 
//         // cfg[useEnCfgLoc.low] = useEnCfg;
//     }
//     if(op != "INPUT"){ // only INPUT node donot use Mux     
//         int rduId;
//         std::map<int, std::map<int, std::map<int, int>>> delayUsed; // <RDU-id, delayused>
//         for(auto& elem : dfgNode->inputEdges()){
//             int width = elem.first;
//             for(auto& edge : elem.second){
//                 int eid = edge.second;
//                 // auto edge = dfg->edge(eid);
//                 // std::cout << "eid: " << eid << ", " << dfg->node(edge->srcId())->name() << " -> " << dfg->node(edge->dstId())->name() << std::endl;
//                 auto& edgeAttr = _mapping->dfgEdgeAttr(eid);
//                 int delay = edgeAttr.delay; // delay cycles
//                 int inputIdx = edgeAttr.edgeLinks.rbegin()->srcPort; // last edgeLInk, dst port
//                 auto muxPair = subAdg->input(width, inputIdx).begin(); // one input only connected to one Mux
//                 int muxId = muxPair->first;
//                 int muxCfgData = muxPair->second;
//                 auto mux = subAdg->node(muxId);
//                 addCfgData(cfg, node->configInfo(muxId), (uint32_t)muxCfgData);
//                 // CfgDataLoc muxCfgLoc = node->configInfo(muxId);
//                 // CfgData muxCfg((muxCfgLoc.high - muxCfgLoc.low + 1), (uint32_t)muxCfgData);
//                 // cfg[muxCfgLoc.low] = muxCfg;
//                 auto rduPair = mux->output(width, 0).begin();
//                 rduId = rduPair->first; 
//                 int rduPort = rduPair->second;
//                 delayUsed[width][rduId][rduPort] = edgeAttr.delay; // delay cycles used by this port
//                 auto rdu = subAdg->node(rduId);            
//                 // if(rdu->type() != "RDU"){
//                 //     delayUsed[rduPort] = edgeAttr.delay; // delay cycles used by this port
//                 //     addCfgData(cfg, node->configInfo(rduId), (uint32_t)delay);
//                 //     // CfgDataLoc rduCfgLoc = node->configInfo(rduId);
//                 //     // CfgData rduCfg(rduCfgLoc.high - rduCfgLoc.low + 1, (uint32_t)delay);
//                 //     // cfg[rduCfgLoc.low] = rduCfg;      
//                 // } 
//             }        
//         }  
//         // RDU
//         //@yuan: we deal with the RDU configuration finally
//         for(auto& eachwidth : delayUsed){
//             int bitWidth = eachwidth.first;
//             for(auto& elem : eachwidth.second ){
//                 int rduId = elem.first;
//                 CfgDataLoc rduCfgLoc = node->configInfo(rduId);
//                 uint32_t delayCfg = 0;
//                 int eachDelayWidth = (rduCfgLoc.high - rduCfgLoc.low + 1) / node->numOperands(bitWidth);
//                 for(auto& used : elem.second){
//                     delayCfg |= used.second << (eachDelayWidth * used.first);
//                 }
//                 addCfgData(cfg, rduCfgLoc, (uint32_t)delayCfg);
//             }
//         }
//     }
//     return cfg;
// }


// get config data for ADG node
void Configuration::getNodeCfgData(ADGNode* node, std::vector<CfgDataPacket>& cfg){
    std::map<int, CfgData> cfgMap;
    if(node->type() == "GPE"){
        cfgMap = getGpeCfgData(dynamic_cast<GPENode*>(node));
    }else if(node->type() == "GIB"){
        cfgMap = getGibCfgData(dynamic_cast<GIBNode*>(node));
    }else if(node->type() == "IOB"){
        cfgMap = getIobCfgData(dynamic_cast<IOBNode*>(node));
    }
    //else if(node->type() == "IOB"){
    //     cfgMap = getIobCfgData(node);
    // // }else if(node->type() == "GIB" || node->type() == "OB"){
    // //     cfgMap = getGibIobCfgData(node);
    // // }else if(node->type() == "IB" && node->numInputs() > 1){ // only if input number > 1, there is config data
    // //     cfgMap = getGibIobCfgData(node);
    // }
    if(cfgMap.empty()){
        return;
    }
    ADG* adg = _mapping->getADG();
    int cfgDataWidth = adg->cfgDataWidth();
    int totalLen = cfgMap.rbegin()->first + cfgMap.rbegin()->second.len;
    int num = (totalLen+31)/32;
    std::vector<uint32_t> cfgDataVec(num, 0);
    std::set<uint32_t> addrs;
    for(auto& elem : cfgMap){ // std::map auto-sort keys
        int lsb = elem.first;
        int len = elem.second.len;
        auto& data = elem.second.data;
        // cache valid address
        uint32_t targetAddr = lsb/cfgDataWidth;
        int addrNum = (len + (lsb%cfgDataWidth) + cfgDataWidth - 1)/cfgDataWidth;
        for(int i = 0; i < addrNum; i++){
            addrs.emplace(targetAddr+i);
        } 
        // cache data from 0 to MSB   
        int targetIdx = lsb/32;
        int offset = lsb%32;
        uint64_t tmpData = data[0];
        int dataIdx = 0;
        int dataLenLeft = 32;
        // @yuan: for the case that some configdata is less than 32-bit(uint32_t), and the configdata is negative number
        uint32_t reducedData = 0;
        for(int i = 0; i < len; i++){
            reducedData <<= 1;
            reducedData |= 1;
        }
        tmpData &= reducedData;
        // std::cout << "low: " << lsb << " high: " << lsb + len - 1 << " data: " << data[0] << " offset: " << offset <<" len: "<<len<< " reducedData: "<<reducedData<<std::endl;
        while(len > 0){
            if(len <= 32 - offset){
                len = 0;
                cfgDataVec[targetIdx] |= (tmpData << offset);
            }else{                          
                dataLenLeft -= 32 - offset; 
                // std::cout << "dataLenLeft: " << dataLenLeft << " data size: " << data.size()<< std::endl;
                cfgDataVec[targetIdx] |= (tmpData << offset);                
                targetIdx++;
                dataIdx++;
                tmpData >>= 32 - offset;
                if(dataIdx < data.size()){
                    // std::cout << "dataIdx: " << dataIdx << " data: " << data[1]<< std::endl;
                    tmpData |= data[dataIdx] << dataLenLeft;
                    dataLenLeft += 32;
                }
                len -= 32 - offset;
                offset = 0;
            }
        }
    }
    // construct CfgDataPacket
    int cfgBlkOffset = adg->cfgBlkOffset();
    int cfgBlkIdx = node->cfgBlkIdx();
    // int x = node->x();
    uint32_t highAddr = uint32_t(cfgBlkIdx << cfgBlkOffset);
    int n;
    int mask;
    if(cfgDataWidth >= 32){
        assert(cfgDataWidth%32 == 0);
        n = cfgDataWidth/32;
    }else{
        assert(32%cfgDataWidth == 0);
        n = 32/cfgDataWidth;
        mask = (1 << cfgDataWidth) - 1;
    }
    for(auto addr : addrs){
        CfgDataPacket cdp(highAddr|addr);
        if(cfgDataWidth >= 32){
            int size = cfgDataVec.size();
            for(int i = 0; i < n; i++){
                int idx = addr*n+i;
                uint32_t data = (idx < size)? cfgDataVec[idx] : 0;
                cdp.data.push_back(data);
            }
        }else{
            uint32_t data = (cfgDataVec[addr/n] >> ((addr%n)*cfgDataWidth)) & mask;
            cdp.data.push_back(data);
        }
        cfg.push_back(cdp);
    }
}


// get config data for ADG
void Configuration::getCfgData(std::vector<CfgDataPacket>& cfg){
    cfg.clear();
    for(auto& elem : _mapping->getADG()->nodes()){
        getNodeCfgData(elem.second, cfg);
    }
}


// // dump config data
void Configuration::dumpCfgData(std::ostream& os){
    std::vector<CfgDataPacket> cfg;
    getCfgData(cfg);
    ADG* adg = _mapping->getADG();
    int cfgAddrWidth = adg->cfgAddrWidth();
    int cfgDataWidth = adg->cfgDataWidth();
    int addrWidthHex = (cfgAddrWidth+3)/4;
    int dataWidthHex = std::min(cfgDataWidth/4, 8);
    os << std::hex;
    for(auto& cdp : cfg){
        os << std::setw(addrWidthHex) << std::setfill('0') << (cdp.addr) << " ";
        for(int i = cdp.data.size() - 1; i >= 0; i--){
            os << std::setw(dataWidthHex) << std::setfill('0') << cdp.data[i];
        }
        os << std::endl;
    }
    os << std::dec;
}
