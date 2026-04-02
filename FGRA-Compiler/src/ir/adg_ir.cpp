
#include "ir/adg_ir.h"


ADGIR::ADGIR(std::string filename)
{
    std::ifstream ifs(filename);
    if(!ifs){
        std::cout << "Cannnot open ADG file: " << filename << std::endl;
        exit(1);
    }
    json adgJson;
    ifs >> adgJson;
    _adg = parseADG(adgJson);
    _adg->setmaxLUTinput(_maxLUTinput);
}

ADGIR::~ADGIR()
{
    if(_adg){
        delete _adg;
    }
}


// parse ADG json object
ADG* ADGIR::parseADG(json& adgJson){
    // std::cout << "Parse ADG..." << std::endl;
    ADG* adg = new ADG();
    // adg->addBitWidth(adgJson["data_width"].get<int>());
    // adg->addBitWidth(1);
    // if(adgJson.contains("num_input")){
    //     adg->setNumInputs(adgJson["num_input"].get<int>());
    // }
    // if(adgJson.contains("num_output")){
    //     adg->setNumOutputs(adgJson["num_output"].get<int>());
    // } 
    if(adgJson.contains("cfg_spad_data_width")){ // @yuan: data-spad and cfg-spad have the same data-width
        adg->setCfgSpadDataWidth(adgJson["cfg_spad_data_width"].get<int>());
    }
    if(adgJson.contains("cfg_data_width")){
        adg->setCfgDataWidth(adgJson["cfg_data_width"].get<int>());
        adg->setCfgAddrWidth(adgJson["cfg_addr_width"].get<int>()); // they are together
        adg->setCfgBlkOffset(adgJson["cfg_blk_offset"].get<int>());
    }
    if(adgJson.contains("num_MIMD_GPE")){
        adg->setNumMIMDGpeNodes(adgJson["num_MIMD_GPE"].get<int>());
    }
    if(adgJson.contains("iob_to_spad_banks")){
        std::map<int, std::vector<int>> spadBankToIobs; // the IOBs connnected to each scratchpad bank, <bank-index, <iobs>>
        for(auto& elem : adgJson["iob_to_spad_banks"].items()){
            int iobId = std::stoi(elem.key());
            std::vector<int> banks;
            for(auto& bank : elem.value()){
                banks.push_back(bank.get<int>());
                spadBankToIobs[bank].push_back(iobId);
            }
            // std::cout << "set iob -> bank iobid: " << iobId << std::endl;
            adg->setIobToSpadBanks(iobId, banks);
        }
        for(auto &elem : spadBankToIobs){
            std::sort(elem.second.begin(), elem.second.end());
            adg->setSpadBankToIobs(elem.first, elem.second);
        }
    }
    if(adgJson.contains("iob_ag_nest_levels")){
        adg->setIobAgNestLevels(adgJson["iob_ag_nest_levels"].get<int>());
    }else{
        adg->setIobAgNestLevels(1);//@yuan: for test
    }
    if(adgJson.contains("iob_spad_bank_size")){
        adg->setIobSpadBankSize(adgJson["iob_spad_bank_size"].get<int>());
    }
    if(adgJson.contains("cfg_spad_size")){
        adg->setCfgSpadSize(adgJson["cfg_spad_size"].get<int>());
    }
    std::map<int, std::pair<ADGNode*, bool>> modules; // // <moduleId, <ADGNode*, used>>
    for(auto& nodeJson : adgJson["sub_modules"]){
        ADGNode* node = parseADGNode(nodeJson);
        modules[node->id()] = std::make_pair(node, false);
    }
    for(auto& nodeJson : adgJson["instances"]){
        ADGNode* node = parseADGNode(nodeJson, modules);
        int nodeId = nodeJson["id"].get<int>();
        if(node){ // not store sub-module of "This" type
            adg->addNode(nodeId, node);
        }else{ // "This" sub-module
            adg->setId(nodeId);
        }
    }
    parseADGEdges(adg, adgJson["connections"]);
    postProcess(adg);  
    return adg; 
}


// parse ADGNode from sub-modules json object 
ADGNode* ADGIR::parseADGNode(json& nodeJson){
    // std::cout << "Parse ADG node" << std::endl;
    std::string type = nodeJson["type"].get<std::string>();
    // if(type == "This"){
    //     return nullptr;
    // }
    int nodeId = nodeJson["id"].get<int>();
    // std::cout << "Parse ADG node type:" << type << std::endl;
    ADGNode* adg_node;
    if(type == "GPE" || type == "GIB" || type == "IOB"){ // || type == "IB" || type == "OB"){
        auto& attrs = nodeJson["attributes"];
        int bitWidth = attrs["data_width"].get<int>();
        // std::cout << "Parse sub-ADG..." << std::endl;        
        if(type == "GPE" || type == "IOB"){
            FUNode *fu_node;
            if(type == "GPE"){
                GPENode* node = new GPENode(nodeId);
                int num_input_lut = 0;
                if(attrs.contains("num_input_lut")){
                    num_input_lut = attrs["num_input_lut"].get<int>();
                    node->setNumInputLUT(num_input_lut);
                    node->sethasLUT(true);
                    if(num_input_lut > _maxLUTinput){
                        _maxLUTinput = num_input_lut;
                    }
                    // std::cout << "inside num_input_lut: " << num_input_lut<< std::endl;
                }else{
                    node->sethasLUT(false);
                }
                if(attrs.contains("gpe_mode")){
                    if(attrs["gpe_mode"].get<int>() == 1){//@yuan_xcore: there are 3 modes for a GPE: 0: normal; 1: MIMD; 2: XCore
                        node->setMIMD(true);
                    }
                }
                if(attrs.contains("xcore_cfg_id_fg_flag")){//@yuan_xcore: we need to creat the configuration map here
                    auto& iocCfgId = attrs["xcore_cfg_id_fg_flag"];
                    node->cfgIdMap["n"] = iocCfgId["n"].get<int>();
                    node->cfgIdMap["fp_flag"] = iocCfgId["fp_flag"].get<int>();
                    node->cfgIdMap["bias_in"] = iocCfgId["bias_in"].get<int>();
                    node->cfgIdMap["opc"] = iocCfgId["opc"].get<int>();
                    for(int i = 0; i < 5; i++){
                        std::string breakIdx = "break_points_" + std::to_string(i);
                        node->cfgIdMap[breakIdx] = iocCfgId[breakIdx].get<int>();
                    }
                    for(int i = 0; i < 6; i++){
                        std::string biasIdx = "constant_bias_" + std::to_string(i);
                        node->cfgIdMap[biasIdx] = iocCfgId[biasIdx].get<int>();
                    }
                    for(int i = 0; i < 6; i++){
                        std::string ksIdx = "K_in_" + std::to_string(i);
                        // std::cout << "ksIdx: " << ksIdx << std::endl;
                        node->cfgIdMap[ksIdx] = iocCfgId[ksIdx].get<int>();
                    }
                    for(int i = 0; i < 6; i++){
                        for(int j = 0; j < 5; j++){
                            std::string logcIdx = "logc_" + std::to_string(i) + "_" + std::to_string(j);
                            node->cfgIdMap[logcIdx] = iocCfgId[logcIdx].get<int>();
                        }
                    }
                }
                if(attrs.contains("alu_eu_cfg_id_fg_flag")){
                    std::map <int, std::string> CGEusTemp;
                    std::map <int, std::string> FGEusTemp;
                    for(auto& elem : attrs["alu_eu_cfg_id_fg_flag"].items()){
                        std::string euName = elem.key();
                        auto& info = elem.value();
                        int euCfgId = info[0].get<int>();
                        int hasFgOut = info[1].get<int>();
                        node->cfgIdMap[euName] = euCfgId;
                        if(euName == "outMuxCG" || euName == "outMuxFG") continue;
                        CGEusTemp[euCfgId] = euName;
                        if(hasFgOut) FGEusTemp[euCfgId] = euName;
                        EuCfgDataLoc cfg;
                        cfg.imuxFGCfgWIdth = info[2].get<int>();
                        cfg.imuxCGCfgWIdth = info[3].get<int>();
                        cfg.CoreCfgWIdth = info[4].get<int>();
                        node->addEuConfigInfo(euName, cfg);
                        // std::cout << "EU name: " << euName << " cfgId: " << euCfgId << " hasFgOut: " << hasFgOut << std::endl;
                    }
                    int offset = 0;
                    for(auto& elem : CGEusTemp){//@hw_yuan: we sort the Eus here, based on the CfgId
                        node->addCGEusId(elem.second, offset);
                        offset ++ ;
                    }
                    offset = 0;
                    for(auto& elem : FGEusTemp){//@hw_yuan: we sort the Eus here, based on the CfgId
                        node->addFGEusId(elem.second, offset);
                        offset ++ ;
                    }
                }
                
                for(auto& op : attrs["operations"]){
                    node->addOperation(op.get<std::string>());
                }
                if(attrs.contains("affine_ctrl_reg_cfg_id")){
                    auto& iocCfgId = attrs["affine_ctrl_reg_cfg_id"];
                    node->cfgIdMap["InitVal"] = iocCfgId["InitVal"].get<int>();
                    node->cfgIdMap["Cycles"] = iocCfgId["Cycles"].get<int>();
                    node->cfgIdMap["WI"] = iocCfgId["WI"].get<int>();
                    node->cfgIdMap["Latency"] = iocCfgId["Latency"].get<int>();
                    node->cfgIdMap["Repeats"] = iocCfgId["Repeats"].get<int>();
                    node->cfgIdMap["SkipFirst"] = iocCfgId["SkipFirst"].get<int>();
                    node->cfgIdMap["delayEn"] = iocCfgId["delayEn"].get<int>();
                    int NumNonLUTFG = attrs["num_operand_fg"].get<int>() - num_input_lut;
                    // std::cout << "num_operand_fg: " << attrs["num_operand_fg"].get<int>() << " num_input_lut: " << num_input_lut << std::endl;
                    // node->setNumConACCINput(1, NumNonLUTFG);
                }
                fu_node = node;
            }else{
                IOBNode* node  = new IOBNode(nodeId);
                auto& iocCfgId = attrs["io_controller_cfg_id"];
                node->cfgIdMap["BaseAddr"] = iocCfgId["BaseAddr"].get<int>();
                // node->cfgIdMap["Stride"] = iocCfgId["Stride"].get<int>();
                // node->cfgIdMap["Cycles"] = iocCfgId["Cycles"].get<int>();
                node->cfgIdMap["II"] = iocCfgId["II"].get<int>();
                node->cfgIdMap["Latency"] = iocCfgId["Latency"].get<int>();
                node->cfgIdMap["IsLoad"] = iocCfgId["IsLoad"].get<int>();
                node->cfgIdMap["UseFGIn"] = iocCfgId["UseFGIn"].get<int>();
                node->cfgIdMap["UseFGOut"] = iocCfgId["UseFGOut"].get<int>();
                int agNestLevels = attrs["ag_nest_levels"].get<int>();
                for(int i = 0; i < agNestLevels; i++){
                    std::string strideName = "Stride" + std::to_string(i);
                    node->cfgIdMap[strideName] = iocCfgId[strideName].get<int>();
                    std::string cyclesName = "Cycles" + std::to_string(i);
                    node->cfgIdMap[cyclesName] = iocCfgId[cyclesName].get<int>();
                }
                // node->iocCfgIdMap["UseAddr"] = iocCfgId["UseAddr"].get<int>();
                if(iocCfgId.contains("startBankIdx")){
                    node->cfgIdMap["startBankIdx"] = iocCfgId["startBankIdx"].get<int>();
                }
                if(iocCfgId.contains("logN")){
                    node->cfgIdMap["logN"] = iocCfgId["logN"].get<int>();
                }
                if(iocCfgId.contains("logB")){
                    node->cfgIdMap["logB"] = iocCfgId["logB"].get<int>();
                }
                if(iocCfgId.contains("parMode")){
                    node->cfgIdMap["parMode"] = iocCfgId["parMode"].get<int>();
                }
                if(iocCfgId.contains("UseAddr")){
                    node->cfgIdMap["UseAddr"] = iocCfgId["UseAddr"].get<int>();
                }
                if(iocCfgId.contains("useTask")){
                    node->cfgIdMap["useTask"] = iocCfgId["useTask"].get<int>();
                }
                int iobMode = attrs["iob_mode"].get<int>();
                // std::cout << "IOB MODE: " << iobMode << std::endl;
                //std::string modeName = _iobModeNames[iobMode];
                if(iobMode == 1){ // IOFIFO_MODE
                    node->addOperation("INPUT");
                    node->addOperation("OUTPUT");
                }else if(iobMode == 2){ // SRAM_MODE
                    node->addOperation("INPUT");
                    node->addOperation("OUTPUT");
                    node->addOperation("LOAD");
                    node->addOperation("STORE");
                }else if(iobMode == 3){ // TASK_EXIT_MODE
                    node->addOperation("INPUT");
                    node->addOperation("OUTPUT");
                    node->addOperation("LOAD");
                    node->addOperation("STORE");
                    node->addOperation("TLOAD");
                    node->addOperation("TSTORE");
                    // std::cout << "add tstore op!!!" << std::endl;
                }
                int fgOperand = 0;
                if(attrs.contains("num_operand_fg")){
                    fgOperand = attrs["num_operand_fg"].get<int>();
                }
                if(fgOperand == 2 && iobMode == 3){
                    node->addOperation("CINPUT");
                    node->addOperation("COUTPUT");
                    node->addOperation("CLOAD");
                    node->addOperation("CSTORE");
                    node->addOperation("TCLOAD");
                    node->addOperation("TCSTORE");
                }else if(fgOperand == 1 && iobMode == 2){
                    node->addOperation("CINPUT");
                    node->addOperation("COUTPUT");
                    node->addOperation("CLOAD");
                    node->addOperation("CSTORE");
                }else if(fgOperand == 1 && iobMode == 1){
                    node->addOperation("CINPUT");
                    node->addOperation("COUTPUT");
                }
                // std::cout << "fgOperand: " << fgOperand << std::endl;
                fu_node = node;
            }
            // fu_node->setNumInputs(bitWidth, attrs["num_input_cg"].get<int>());
            // fu_node->setNumInputs(1, attrs["num_input_fg"].get<int>());
            // fu_node->setNumOutputs(bitWidth, attrs["num_output_cg"].get<int>());
            // fu_node->setNumOutputs(1, attrs["num_output_fg"].get<int>());
            fu_node->setMaxDelay(bitWidth, attrs["max_delay_cg"].get<int>());
            fu_node->setMaxDelay(1, attrs["max_delay_fg"].get<int>());
            fu_node->setNumOperands(bitWidth, attrs["num_operand_cg"].get<int>());
            fu_node->setNumOperands(1, attrs["num_operand_fg"].get<int>());  // ALU + LUT fine grain operand                       
            adg_node = fu_node;
            adg_node->addBitWidth(bitWidth);
            if(attrs["num_input_fg"].get<int>() > 0 || attrs["num_output_fg"].get<int>() > 0){
                adg_node->addBitWidth(1);
            }
        }else{
            GIBNode* node  = new GIBNode(nodeId);
            // node->setNumInputs(bitWidth, attrs["num_input"].get<int>());
            // node->setNumOutputs(bitWidth, attrs["num_output"].get<int>());
            adg_node = node;
            adg_node->addBitWidth(bitWidth);
        }
        // }else if(type == "IB"){
        //     IOBNode* node  = new IOBNode(nodeId);
        //     adg_node = node;
        // }else if(type == "OB"){
        //     IOBNode* node  = new IOBNode(nodeId);
        //     adg_node = node;
        // }     
        adg_node->setCfgBlkIdx(attrs["cfg_blk_index"].get<int>());
        ADG* subADG = parseADG(attrs); // parse sub-adg
        adg_node->setSubADG(subADG);
        subADG->setBitWidths(adg_node->bitWidths());
        // subADG->setNumInputs(adg_node->numInputs());
        // subADG->setNumOutputs(adg_node->numOutputs());
        if(attrs.count("configuration")){
            for(auto& elem : attrs["configuration"].items()){
                int subModuleId = std::stoi(elem.key());
                auto& info = elem.value();
                CfgDataLoc cfg;
                cfg.high = info[1].get<int>();
                cfg.low = info[2].get<int>();
                adg_node->addConfigInfo(subModuleId, cfg);
            }
        }
    }else{ // common components: ALU, LUT, Muxn, RF, DelayPipe, Const
        adg_node = new ADGNode(nodeId);
    } 
    // std::cout << "type: " << type << std::endl;
    adg_node->setType(type);
    return adg_node;
}


// parse ADGNode from instances json object, 
// modules<moduleId, <ADGNode*, used>>,  
ADGNode* ADGIR::parseADGNode(json& nodeJson, std::map<int, std::pair<ADGNode*, bool>>& modules){
    std::string type = nodeJson["type"].get<std::string>();
    if(type == "This"){
        return nullptr;
    }
    int nodeId = nodeJson["id"].get<int>();
    int moduleId = nodeJson["module_id"].get<int>();
    ADGNode* adg_node;
    ADGNode* module = modules[moduleId].first;
    bool renewNode = modules[moduleId].second; // used, need to re-new ADGNode
    // if(type == "LNS")
    //     std::cout << "ADG Node TYPE: " << type << std::endl;
    if(type == "GPE"|| type == "IOB" || type == "CGGIB" || type == "FGGIB"){ // || type == "IB" || type == "OB"){                
        if(renewNode){ // re-new ADGNode
            if(type == "GPE"){
                GPENode* node = new GPENode(nodeId);
                *node = *(dynamic_cast<GPENode*>(module));
                //std::cout << " one GPE has lut? : " << node->hasLUT() << std::endl;
                adg_node = node;            
            }else if(type == "IOB"){
                IOBNode* node  = new IOBNode(nodeId);
                *node = *(dynamic_cast<IOBNode*>(module));
                adg_node = node;
            }else{
                GIBNode* node  = new GIBNode(nodeId);
                *node = *(dynamic_cast<GIBNode*>(module));
                adg_node = node;
            }
            // }else if(type == "IB"){
            //     IOBNode* node  = new IOBNode(nodeId);
            //     *node = *(dynamic_cast<IOBNode*>(module));
            //     adg_node = node;
            // }else if(type == "OB"){
            //     IOBNode* node  = new IOBNode(nodeId);
            //     *node = *(dynamic_cast<IOBNode*>(module));
            //     adg_node = node;
            // }
            ADG* subADG = new ADG(); 
            *subADG = *(adg_node->subADG()); // COPY Sub-ADG
            adg_node->setSubADG(subADG);
        }else{ // reuse the ADGNode in modules
            adg_node = module;
            modules[moduleId].second = true;
        }
        int bitWidth = 1;
        for(auto bits : adg_node->bitWidths()){
            if(bits > 1){
                bitWidth = bits;
                break;
            }
        }
        if(type == "GPE"){
            GPENode *gpe_node = dynamic_cast<GPENode*>(adg_node);
            gpe_node->setMaxDelay(bitWidth, nodeJson["max_delay_cg"].get<int>());
            gpe_node->setMaxDelay(1, nodeJson["max_delay_fg"].get<int>());
        }else if(type == "IOB"){
            IOBNode *iob_node = dynamic_cast<IOBNode*>(adg_node);
            iob_node->setIndex(nodeJson["iob_index"].get<int>());
            iob_node->setMaxDelay(bitWidth, nodeJson["max_delay_cg"].get<int>());
            iob_node->setMaxDelay(1, nodeJson["max_delay_fg"].get<int>());
        }else{
            GIBNode *gib_node = dynamic_cast<GIBNode*>(adg_node);
            gib_node->setTrackReged(nodeJson["track_reged"].get<bool>());
            // int gibBitWidth = nodeJson["data_width"].get<int>();
            // if(gibBitWidth != bitWidth){
            //     gib_node->replaceBitWidth(bitWidth, gibBitWidth);
            // }
        }
        adg_node->setCfgBlkIdx(nodeJson["cfg_blk_index"].get<int>());  
        adg_node->setX(nodeJson["x"].get<int>());     
        adg_node->setY(nodeJson["y"].get<int>());        
    }else{ // common components: ALU, Muxn, RF, DelayPipe, Const
        if(renewNode){ // re-new ADGNode
            adg_node = new ADGNode(nodeId);
            *adg_node = *module;  
        }else{ // reuse the ADGNode in modules
            adg_node = module;
            modules[moduleId].second = true;
        }      
    } 
    adg_node->setId(nodeId);
    adg_node->setName(type+std::to_string(nodeId));
    // adg_node->setType(type);
    return adg_node;
}


// parse ADGEdge json object
void ADGIR::parseADGEdges(ADG* adg, json& edgeJson){
    // std::cout << "Parse ADG Edge" << std::endl;
    for(auto& elem : edgeJson.items()){
        int edgeId = std::stoi(elem.key());
        auto& edge = elem.value();
        int srcId = edge[0].get<int>();
        // std::string srcType = edge[1].get<std::string>();
        int srcPort = edge[2].get<int>();
        int dstId = edge[3].get<int>();
        // std::string dstType = edge[4].get<std::string>();
        int dstPort = edge[5].get<int>();
        int bits = edge[6].get<int>();
        ADGEdge* adg_edge = new ADGEdge(srcId, dstId);
        adg_edge->setId(edgeId);
        adg_edge->setSrcId(srcId);
        adg_edge->setDstId(dstId);
        adg_edge->setSrcPortIdx(srcPort);
        adg_edge->setDstPortIdx(dstPort);
        adg_edge->setBitWidth(bits);
        adg->addEdge(edgeId, adg_edge);
    }
}


// analyze the connections among the internal sub-modules for GPENode
// fill _operandInputs 
void ADGIR::analyzeIntraConnect(GPENode* node){
    ADG* subAdg = node->subADG();   
    // std::cout << "node id: " << node->id() << " name: " << node->name() << " is MIMD: " << node->isMIMD() <<" operation: ";
    // for(auto& elem : node->operations()){
    //     std::cout << " " << elem ;
    // }
    // std::cout << std::endl;
    for(auto allBitInputs : subAdg->inputs()){
        int bits = allBitInputs.first;
        // std::cout << "bits: " << bits << std::endl;
        for(auto& elem : allBitInputs.second){
            // std::cout << "input index: " << elem.first << std::endl;
            for(auto& input : elem.second){
                // std::cout << "subnode id: " << input.first <<std::endl;
                ADGNode* subNode = subAdg->node(input.first);   
                // std::cout << "begin subnode type: " << subNode->type() << std::endl;
                int opeIdx = input.second; // operand index
                // if (bits != 1)  std::cout << "begin subnode type: " << subNode->type() << " opeIdx: " << opeIdx <<std::endl;  
                while (subNode->type() != "ALU" && subNode->type() != "LUT" && subNode->type() != "DMR" && subNode->type() != "LNS"){//@yuan_xcore: the LNS unit is the terminal
                    if(subNode->outputs(bits).size() == 1){ // only one output
                        opeIdx = 0;
                    }
                    auto out = subNode->output(bits, opeIdx).begin(); 
                    // std::cout << "subNode->output(bits, opeIdx) size: " << subNode->output(bits, opeIdx).size() << " bits: "<< bits<< std::endl;
                    subNode = subAdg->node(out->first);
                    opeIdx = out->second;    
                    // if (bits != 1)  std::cout << "subnode type: " << subNode->type() << " opeIdx: " << opeIdx <<std::endl;     

                }
                if(bits > 1){
                    // opeIdx is ALU operand index now
                    node->addOperandInput(bits, opeIdx, elem.first);
                }else if(subNode->type() == "ALU"){ // ALU fg-input is operand 0
                    node->addOperandInput(1, 0, elem.first);
                }else if(subNode->type() == "DMR"){// opeIdx means the operand of DMR
                    // std::cout << "opeidx: " << opeIdx << std::endl;
                    if(!(node->opCapable("SEL") || node->opCapable("SEXT") || node->opCapable("ZEXT"))){
                        opeIdx -= 1;
                    }
                    // std::cout << " DMR opeIdx: " << opeIdx << " input index: " << elem.first << std::endl;
                    node->addConDMRInput(1, opeIdx, elem.first);
                }else{
                    int idx = node->getOperandIdxLUT(opeIdx);
                    // std::cout << "idx: " << idx << std::endl;
                    node->addOperandInput(1, idx, elem.first);
                }   
            }                    
        }
    }
    // exit(0);
}


// analyze the connections among the internal sub-modules for IOBNode
void ADGIR::analyzeIntraConnect(IOBNode* node){
    ADG* subAdg = node->subADG();   
    for(auto allBitInputs : subAdg->inputs()){
        int bits = allBitInputs.first;
        for(auto& elem : allBitInputs.second){
            for(auto& input : elem.second){
                ADGNode* subNode = subAdg->node(input.first);
                int opeIdx = input.second; // operand index
                while (subNode->type() != "IOController"){
                    if(subNode->outputs(bits).size() == 1){ // only one output
                        opeIdx = 0;
                    }
                    auto out = subNode->output(bits, opeIdx).begin(); // submodule should only have one output in GPE
                    subNode = subAdg->node(out->first);
                    opeIdx = out->second;
                }
                // std::cout << "bits: " << bits << " opeIdx: " << opeIdx << " inputIdx: " << elem.first << std::endl;
                node->addOperandInput(bits, opeIdx, elem.first);  
            }      
        }
    }
    // std::cout << "One IOB: " << node->id() << "~~~~~~~~~~~~~~~~~" << std::endl;
}
// void ADGIR::analyzeIntraConnect(IOBNode* node){
//     ADG* subAdg = node->subADG();
//     for(auto& ielem : subAdg->inputs()){
//         int inPort = ielem.first;
//         int outPort;
//         for(auto& subNode : ielem.second){
//             if(subNode.first == subAdg->id()){ // input directly connected to output
//                 outPort = subNode.second;
//             } else {
//                 ADGNode* subNodePtr = subAdg->node(subNode.first);
//                 outPort = subNodePtr->output(0).begin()->second; // only one layer of Muxn
//             }           
//             node->addIn2outs(inPort, outPort);
//             node->addOut2ins(outPort, inPort);
//         }
//     }
// }


// analyze the connections among the internal sub-modules for GIBNode
// fill _out2ins, _in2outs 
void ADGIR::analyzeIntraConnect(GIBNode* node){
    ADG* subAdg = node->subADG();
    int bits = *(node->bitWidths().begin()); // single grain
    for(auto& ielem : subAdg->inputs(bits)){
        int inPort = ielem.first;
        for(auto& subNode : ielem.second){
            int outPort;
            if(subNode.first == subAdg->id()){ // input directly connected to output
                outPort = subNode.second;
            } else {
                ADGNode* subNodePtr = subAdg->node(subNode.first);
                outPort = subNodePtr->output(bits, 0).begin()->second; // only one layer of Muxn
            }
            node->addIn2outs(inPort, outPort);
            node->addOut2ins(outPort, inPort);
        }
    }
}


// analyze if there are registers in the output ports of GIB
// fill _outReged
void ADGIR::analyzeOutReg(ADG* adg, GIBNode* node){
    int bits = *(node->bitWidths().begin()); // single grain
    for(auto& elem : node->outputs(bits)){
        int id = elem.second.begin()->first; // GIB output port only connected to one node
        if(node->trackReged() && (adg->node(id)->type() == "GIB")){ // this edge is track
            node->setOutReged(elem.first, true);
        }else{
            node->setOutReged(elem.first, false);
        }
    }
}


// post-process the ADG nodes
void ADGIR::postProcess(ADG* adg){
    int numGpeNodes = 0;
    int numIobNodes = 0;
    // std::cout << "begin to postProcess!" << std::endl;
    for(auto& node : adg->nodes()){
        auto nodePtr = node.second;
        if(nodePtr->type() == "GPE"){
            numGpeNodes++;
            // if(dynamic_cast<GPENode*>(nodePtr)->hasLUT()){
            //     numGpeNodes++;
            // }
            // std::cout << "begin to postProcess of GPE's internal connection!" << std::endl;
            analyzeIntraConnect(dynamic_cast<GPENode*>(nodePtr));
        } else if(nodePtr->type() == "IOB"){
            numIobNodes++;
            IOBNode* iob = dynamic_cast<IOBNode*>(nodePtr);
            adg->setIobIdxToId(iob->index(), nodePtr->id());
            analyzeIntraConnect(iob);
        } else if(nodePtr->type() == "GIB"){
            analyzeIntraConnect(dynamic_cast<GIBNode*>(nodePtr));
            analyzeOutReg(adg, dynamic_cast<GIBNode*>(nodePtr));        
        }
    }
    adg->setNumGpeNodes(numGpeNodes);
    adg->setNumIobNodes(numIobNodes);
}