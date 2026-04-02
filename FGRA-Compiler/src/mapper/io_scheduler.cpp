#include "mapper/io_scheduler.h"

IOScheduler::IOScheduler(ADG *adg)
{
    _adg = adg;
    int banks = adg->numIobNodes();
    _cur_bank_status.assign(banks, {0, 0, 0});
    _old_bank_status.assign(banks, {0, 0, 0});
    _older_bank_status.assign(banks, {0, 0, 0});
    _old_cfg_status = {0, 0};
}

IOScheduler::~IOScheduler()
{
}
//@yuan TODO: task parallelism may crash the mapper, when multiport memory access occur, due to the selSatrt can not be 0 
void IOScheduler::ioSchedule(Mapping *mapping)
{
    // std::map<int, dfgIoInfo> res;
    _dfg_io_infos.clear();
    _ex_dep = 0;
    _iob_ens = 0;
    _dep_cost = 0;
    _allocatedMultiportBank.clear();
    int bankNum = _adg->numIobNodes();
    _cur_bank_status.assign(bankNum, {0, 0, 0});
    DFG* dfg = mapping->getDFG();
    auto outNodeIds = dfg->getOutNodes(); // OUTPUT/STORE nodes
    // LD_DEP_ST_LAST_SEC_TASK cost = 1
    long inNodeNum = dfg->ioNodes().size() - outNodeIds.size(); // LD_DEP_EX_LAST_TASK cost
    long inNodeNum_2 = inNodeNum * inNodeNum;   // EX_DEP_ST_LAST_TASK dep cost
    long inNodeNum_3 = inNodeNum_2 * inNodeNum; // LD_DEP_ST_LAST_TASK dep cost
    int sizeofBank = _adg->iobSpadBankSize();
    int dataByte = 4;
    for(auto &elem: _adg->bitWidths()){
        if(elem > 1){
            dataByte = elem / 8 ;
            break;
        }
    }
    int depthofBank = sizeofBank / dataByte; // accomodate total data number
    std::vector<int> sortedIoNodes;
    sortedIoNodes.assign(dfg->ioNodes().begin(), dfg->ioNodes().end());
    std::map<std::string, int> memNum;
    for(auto &elem : sortedIoNodes){
        auto IONode = dynamic_cast<DFGIONode*>(dfg->node(elem));
        auto name = IONode->memRefName();
        if(!memNum.count(name)){
            int N = IONode->NumMultiportBank();
            memNum[name] = N;
        }else{
            if(IONode->MultiportType() > 1) continue;
            int N = IONode->NumMultiportBank();
            memNum[name] += N;
        }
    }
    // sort by memRefName and memSize, small memory rank before big memory, than the memory with same size sorted by the name
    // std::sort(sortedIoNodes.begin(), sortedIoNodes.end(), [&](int ida, int idb){
    //     return dynamic_cast<DFGIONode*>(dfg->node(ida))->memRefName() < dynamic_cast<DFGIONode*>(dfg->node(idb))->memRefName();
    // });
    //@yuan: also sort by type, non-multiport rank before multiport, in case of multipot nodes cut off the banks 
    std::sort(sortedIoNodes.begin(), sortedIoNodes.end(), [&](int ida, int idb){
        auto nodea = dynamic_cast<DFGIONode*>(dfg->node(ida));
        auto nodeb = dynamic_cast<DFGIONode*>(dfg->node(idb));
        if(!dfg->isMultiportIoNode(ida) && dfg->isMultiportIoNode(idb)){
            return true;
        }else if(dfg->isMultiportIoNode(ida) && !dfg->isMultiportIoNode(idb)){
            return false;
        }else if(nodea->memSize() > nodeb->memSize()){
            return true;
        }else if(nodea->memSize() == nodeb->memSize()){
            auto namea = nodea->memRefName();
            auto nameb = nodeb->memRefName();
            if(memNum[namea] < memNum[nameb]){
                return true;
            }else if(memNum[namea] == memNum[nameb]){
                return namea < nameb;
            }
        }
        return false;
    });
    // sort by io type, input nodes rank before output nodes
    // std::stable_sort(sortedIoNodes.begin(), sortedIoNodes.end(), [&](int ida, int idb){
    //     return outNodeIds.count(ida) < outNodeIds.count(idb);
    // });
    std::map<std::string, int> memDep;
    auto multiportIOs = dfg->multiportIOs();
    std::map<std::string, std::pair<int, int>> allocatedMultiport; // <name, <Id, bank>>
    std::map<std::string, int> Mem2DFGNodeId;
    for(auto id : sortedIoNodes){
        std::cout << dfg->node(id)->name() << ": " << dynamic_cast<DFGIONode*>(dfg->node(id))->memRefName() << " multuport type: " << dynamic_cast<DFGIONode*>(dfg->node(id))->MultiportType() <<std::endl;
        dfgIoInfo ioInfo;
        auto dfgIONode =dynamic_cast<DFGIONode*>(dfg->node(id));
        bool isMultiport = false;
        int LSType = 1; // 1: load, 2: store, 3: load&store (multiport)
        auto memRefName = dfgIONode->memRefName();
        if(multiportIOs.count(memRefName)){
            isMultiport = true;
            if(multiportIOs[memRefName].first.size() > 0 && multiportIOs[memRefName].second.size() > 0){
                LSType = 3;
            }
        }
        if(!Mem2DFGNodeId.count(memRefName)){
            Mem2DFGNodeId[memRefName] = id;
        }
        bool isStore = outNodeIds.count(id);
        if(LSType != 3 && isStore){
            LSType = 2;
        }
        ioInfo.isStore = isStore;       
        int memSize = dfgIONode->memSize();
        int spadDataByte = _adg->cfgSpadDataWidth() / 8; // dual ports of cfg-spad have the same width  @yuan: just get the width of spad
        // std::cout << "before align memSize: " << memSize << " spadDataByte: " << spadDataByte << std::endl;
        memSize = (memSize + spadDataByte - 1) / spadDataByte * spadDataByte; // align to spadDataByte
        // std::cout << "after align memSize: " << memSize << std::endl;
        auto& attr =  mapping->dfgNodeAttr(id);
        int iobId = attr.adgNode->id();
        int iobIdx = dynamic_cast<IOBNode*>(_adg->node(iobId))->index();
        std::cout << "iobIdx: " << iobIdx << " _iob_ens: " << _iob_ens<< std::endl;
        _iob_ens |= ((uint64_t)1) << iobIdx;
        std::vector<int> banks = _adg->iobToSpadBanks(iobIdx); // spad banks connected to this IOB
        std::cout << "_iob_ens: " << _iob_ens << " bank size: " << banks.size()<< std::endl;
        std::sort(banks.begin(), banks.end());
        int minBank = *(std::min_element(banks.begin(), banks.end()));   
        int maxBank = *(std::max_element(banks.begin(), banks.end()));        
        std::vector<int> availBanks;
        if(_allocatedMultiportBank.count(memRefName)){
            if(dfgIONode->MultiportType() <= 1){//@yuan: for N = 1 AII > 1
                auto idbank = _allocatedMultiportBank[memRefName];
                availBanks.push_back(*idbank.begin());
            }else{
                for(int bank : _allocatedMultiportBank[memRefName]){
                    if(_cur_bank_status[bank].used == 0){
                        availBanks.push_back(bank);
                    }
                }
                if(availBanks.empty()){//@yuan: for N < number of IOB
                    auto idbank = _allocatedMultiportBank[memRefName];
                    availBanks.push_back(*idbank.begin());
                }
            }
        }else{
            for(int bank : banks){ // two IOs of the same DFG cannot access the same bank
                if(dfgIONode->MultiportType() <= 1 && dfgIONode->NumMultiportBank() == 1){ //@yuan: no partitioned memory
                    bool bankavalible = true;
                    // std::cout << "_allocatedMultiportBank size: " << _allocatedMultiportBank.size() << std::endl;
                    for(auto & elem : _allocatedMultiportBank){
                        if(elem.second.count(bank)){ //@yuan: current bank is occupied by multiport
                            bankavalible = false;
                            break;
                        }
                    }
                    if(_cur_bank_status[bank].used == 0 && bankavalible){
                        availBanks.push_back(bank);
                    }
                }else{//@yuan: for partitioned memory, the bank is available is not only depend on itself, but also the subsequent N-1 banks
                    bool bankavalible = true;
                    int N = dfgIONode->NumMultiportBank();
                    // std::cout << "_allocatedMultiportBank size: " << _allocatedMultiportBank.size() << std::endl;
                    if(_allocatedMultiportBank.size() == 0){
                        for(int i = 0; i < N; i++){
                            if(_cur_bank_status[bank + i].used != 0 || (bank + i) > maxBank){ //@yuan: current bank is occupied by multiport or other IOB
                                bankavalible = false;
                                break;
                            }
                        }
                    }
                    for(auto & elem : _allocatedMultiportBank){
                        for(int i = 0; i < N; i++){
                            if(elem.second.count(bank + i) || _cur_bank_status[bank + i].used != 0 || (bank + i) > maxBank){ //@yuan: current bank is occupied by multiport or other IOB
                                bankavalible = false;
                                break;
                            }
                        }
                        if(!bankavalible) break;
                    }
                    if(bankavalible){
                        for(int i = 0; i < N; i++){
                            availBanks.push_back(bank + i);
                            _allocatedMultiportBank[memRefName].emplace(bank + i);
                        }
                        _partitionedStartBank[memRefName] = bank - minBank;
                        break; //@yuan: found the banks
                    }

                }
            }
        }
        assert(!availBanks.empty());
        bool allocated = false;
        int selBank;
        int selStart = 0; //@yuan: for multiport memory accessment, the selStart must be 0
        std::vector<std::pair<int, int>> bankStatus; // <status, start-addr>
        std::cout << "availBanks size: " << availBanks.size() << " id: " ;
        for(int bank : availBanks){   
            std::cout << bank << " ";
        }
        std::cout << std::endl;
        // 0: both available; 1: old available, older not; 2: older available, old not; 3: both not
        for(int bank : availBanks){            
            int oldUsed = _old_bank_status[bank].used;
            int olderUsed = _older_bank_status[bank].used;
            int oldStart = _old_bank_status[bank].start;
            int olderStart = _older_bank_status[bank].start;
            int oldEnd = _old_bank_status[bank].end;
            int olderEnd = _older_bank_status[bank].end;
            int oldStatus = 0; // 0: free; 1: used but available; 2: not available
            int oldSelStart = 0;
            if(oldUsed == 0){               
                oldStatus = 0;
                oldSelStart = 0;
            }else if((oldUsed == 1 && (LSType == 1 || LSType == 3)) || (oldUsed >= 2 && LSType == 2)){ // the same operation
                if(memSize <= sizeofBank - oldEnd){
                    oldStatus = 1;
                    oldSelStart = oldEnd;
                }else if(memSize <= oldStart){
                    oldStatus = 1;
                    oldSelStart = 0;
                }else{
                    oldStatus = 2;
                }
            }else if(oldUsed == 1 && LSType == 2){ // old load, current store
                oldStatus = 0;
                oldSelStart = 0;
            }else{ // old store, current load; cannot access the same bank simultaneously
                oldStatus = 2;
            }

            int olderStatus = 0; // 0: free; 1: used but available; 2: not available
            int olderSelStart = 0;
            if(olderUsed >= 2 && (LSType == 1 || LSType == 3)){ 
                olderStatus = 2;                
            }else{
                olderStatus = 0;
                olderSelStart = 0;
            }

            int status = 0; // 0: both available; 1: old available, older not; 2: older available, old not; 3: both not            
            if(oldStatus < 2 && olderStatus == 0){
                status = 0;
                selStart = oldSelStart;              
            }else if(oldStatus < 2 && olderStatus == 2){
                status = 1;
                selStart = oldSelStart;
            }else if(oldStatus == 2 && olderStatus == 0){
                status = 2;
                selStart = olderSelStart;
            }else{ // (oldStatus == 2 && olderStatus == 2)
                status = 3;
                selStart = 0;
            }            
            if(status == 0){ // 0: both available
                selBank = bank;
                ioInfo.dep = 0;
                allocated = true;
                break;
            }
            if(dfgIONode->MultiportType() > 1 && selStart != 0){ //@yuan: multiport memory accessment requires selStart of each bank is 0
                continue;
            }
            bankStatus.push_back(std::make_pair(status, selStart));
        }            
        
        int availBankNum = bankStatus.size();
        if(!allocated && (availBankNum != availBanks.size())){
            std::cout << "[Error] Cannot find suitable bank for patitioned memory, please map again or optimize the memory partition!" << std::endl;
            exit(0);
        }
        if(!allocated){ // 1: old available, older not
            for(int i = 0; i < availBankNum; i++){
                if(bankStatus[i].first == 1){
                    selBank = availBanks[i];
                    selStart = bankStatus[i].second;
                    assert((LSType == 1 || LSType == 3) && _older_bank_status[selBank].used == 2);
                    if(!isStore){
                        ioInfo.dep = LD_DEP_ST_LAST_SEC_TASK;
                        if(!memDep.count(memRefName) || memDep[memRefName] < 1){ // IO nodes access to the same array have only one cost
                            memDep[memRefName] = 1;
                            _dep_cost += 1; 
                        }
                    }else{
                        ioInfo.dep = 0;
                    }           
                    allocated = true;
                    break;
                }
            }
        }
        // 2: older available, old not;
        if(!allocated && isStore){
            selBank = availBanks[0];
            selStart = bankStatus[0].second;
            ioInfo.dep = EX_DEP_ST_LAST_TASK;
            _ex_dep = EX_DEP_ST_LAST_TASK;
            allocated = true;
        }else if(!allocated){
            for(int i = 0; i < availBankNum; i++){
                selBank = availBanks[i];
                if(_old_bank_status[selBank].used == 1){                    
                    selStart = bankStatus[i].second;
                    ioInfo.dep = LD_DEP_EX_LAST_TASK;
                    if(!memDep.count(memRefName) || memDep[memRefName] < inNodeNum){ // IO nodes access to the same array have only one cost
                        memDep[memRefName] = inNodeNum;
                        _dep_cost += inNodeNum; 
                    }
                    // _dep_cost += inNodeNum;        
                    allocated = true;
                    break;
                }
            }
            if(!allocated){
                selBank = availBanks[0];
                selStart = bankStatus[0].second;
                ioInfo.dep = LD_DEP_ST_LAST_TASK;
                if(!memDep.count(memRefName) || memDep[memRefName] < inNodeNum_3){ // IO nodes access to the same array have only one cost
                    memDep[memRefName] = inNodeNum_3;
                    _dep_cost += inNodeNum_3; 
                }
                // _dep_cost += inNodeNum_3;
                allocated = true;
            }
        }
        assert(allocated); 
        ioInfo.addr = selBank * sizeofBank + selStart;
        int iobAdress = 0;
        if(dfgIONode->MultiportType() <= 1 && dfgIONode->NumMultiportBank() == 1){
            iobAdress = ((selBank - minBank) * sizeofBank + selStart) / dataByte; 
        }
        ioInfo.iobAddr = iobAdress;        
        _dfg_io_infos[id] = ioInfo;  
        dfgIONode->setbank(selBank);
        std::cout << "selbank: " << selBank << " selStart: " << selStart << " sizeofBank: " << sizeofBank<< std::endl;    
        std::cout << id << ": " << ioInfo.addr << std::endl;
        std::cout << "iobAddr: " << ": " << iobAdress  << "\n"<< std::endl;      
        // std::cout << id << ": " << ioInfo.addr << std::endl;
        _cur_bank_status[selBank].used = LSType;
        // _cur_bank_status[selBank].iob = iobIdx;
        _cur_bank_status[selBank].start = selStart;
        _cur_bank_status[selBank].end = selStart + memSize;     
        if((isMultiport) && !_allocatedMultiportBank.count(memRefName)){
            _allocatedMultiportBank[memRefName].emplace(selBank);
            // _allocatedMultiportBank[memRefName] = std::make_pair(iobId, selBank);
        }   
    }
    std::cout << "_allocatedMultiportBank size: " << _allocatedMultiportBank.size() << std::endl;
    for(auto & elem : _allocatedMultiportBank){//@yuan: we allocate the extra partitioned memory bank here
        if(elem.second.size() <= 1) continue;
        auto iobsforbank = _adg->spadBankToIobs(*elem.second.begin());
        int IONodeId = Mem2DFGNodeId[elem.first];
        std::cout << dfg->node(IONodeId)->name() << ": " << dynamic_cast<DFGIONode*>(dfg->node(IONodeId))->memRefName() << std::endl;
        // dfgIoInfo ioInfo;
        auto dfgIONode =dynamic_cast<DFGIONode*>(dfg->node(IONodeId));
        bool isMultiport = true;
        int LSType = 3; // @yuan: partitioned memory must be multiport memory accessment
        auto memRefName = dfgIONode->memRefName();
        bool isStore = outNodeIds.count(IONodeId);
        int memSize = dfgIONode->memSize();
        int spadDataByte = _adg->cfgSpadDataWidth() / 8; // dual ports of cfg-spad have the same width  @yuan: just get the width of spad
        // std::cout << "before align memSize: " << memSize << " spadDataByte: " << spadDataByte << std::endl;
        memSize = (memSize + spadDataByte - 1) / spadDataByte * spadDataByte; // align to spadDataByte
        // std::cout << "after align memSize: " << memSize << std::endl;    
        std::vector<int> availBanks;
        for(auto & unallocatedBankId : elem.second){ //@yuan: available banks are pre-defined
            if(_cur_bank_status[unallocatedBankId].used == 0){
                availBanks.push_back(unallocatedBankId);
            }
        }
        // std::cout << "availBanks size: " << availBanks.size() << std::endl;
        if(availBanks.empty()) continue; //@yuan: for N < number of IOB
        // assert(!availBanks.empty());
        // 0: both available; 1: old available, older not; 2: older available, old not; 3: both not
        for(int bank : availBanks){   
            bool allocated = false;
            int selBank;
            int selStart = 0; //@yuan: for multiport memory accessment, the selStart must be 0
            std::vector<std::pair<int, int>> bankStatus; // <status, start-addr>         
            int oldUsed = _old_bank_status[bank].used;
            int olderUsed = _older_bank_status[bank].used;
            int oldStart = _old_bank_status[bank].start;
            int olderStart = _older_bank_status[bank].start;
            int oldEnd = _old_bank_status[bank].end;
            int olderEnd = _older_bank_status[bank].end;
            int oldStatus = 0; // 0: free; 1: used but available; 2: not available
            int oldSelStart = 0;
            if(oldUsed == 0){               
                oldStatus = 0;
                oldSelStart = 0;
            }else if((oldUsed == 1 && (LSType == 1 || LSType == 3)) || (oldUsed >= 2 && LSType == 2)){ // the same operation
                if(memSize <= sizeofBank - oldEnd){
                    oldStatus = 1;
                    oldSelStart = oldEnd;
                }else if(memSize <= oldStart){
                    oldStatus = 1;
                    oldSelStart = 0;
                }else{
                    oldStatus = 2;
                }
            }else if(oldUsed == 1 && LSType == 2){ // old load, current store
                oldStatus = 0;
                oldSelStart = 0;
            }else{ // old store, current load; cannot access the same bank simultaneously
                oldStatus = 2;
            }

            int olderStatus = 0; // 0: free; 1: used but available; 2: not available
            int olderSelStart = 0;
            if(olderUsed >= 2 && (LSType == 1 || LSType == 3)){ 
                olderStatus = 2;                
            }else{
                olderStatus = 0;
                olderSelStart = 0;
            }

            int status = 0; // 0: both available; 1: old available, older not; 2: older available, old not; 3: both not            
            if(oldStatus < 2 && olderStatus == 0){
                status = 0;
                selStart = oldSelStart;              
            }else if(oldStatus < 2 && olderStatus == 2){
                status = 1;
                selStart = oldSelStart;
            }else if(oldStatus == 2 && olderStatus == 0){
                status = 2;
                selStart = olderSelStart;
            }else{ // (oldStatus == 2 && olderStatus == 2)
                status = 3;
                selStart = 0;
            }            
            if(status == 0){ // 0: both available
                selBank = bank;
                // ioInfo.dep = 0;
                allocated = true;
                // break;
            }
            if(dfgIONode->NumMultiportBank() > 1 && selStart != 0){ //@yuan: multiport memory accessment requires selStart of each bank is 0
                std::cout << "[Error] Cannot find suitable bank for patitioned memory, please map again or optimize the memory partition!" << std::endl;
                exit(0);
            }
            bankStatus.push_back(std::make_pair(status, selStart));
            if(!allocated){ 
                if(bankStatus[0].first == 1){
                    selBank = bank;
                    selStart = bankStatus[0].second;
                    assert((LSType == 1 || LSType == 3) && _older_bank_status[selBank].used == 2);
                    if(!isStore){
                        // ioInfo.dep = LD_DEP_ST_LAST_SEC_TASK;
                        if(!memDep.count(memRefName) || memDep[memRefName] < 1){ // IO nodes access to the same array have only one cost
                            memDep[memRefName] = 1;
                            _dep_cost += 1; 
                        }
                    }         
                    allocated = true;
                    // break;
                }
            }
            // 2: older available, old not;
            if(!allocated && isStore){
                selBank = bank;
                selStart = bankStatus[0].second;
                _ex_dep = EX_DEP_ST_LAST_TASK;
                allocated = true;
            }else if(!allocated){
                selBank = bank;
                if(_old_bank_status[selBank].used == 1){                    
                    selStart = bankStatus[0].second;
                    if(!memDep.count(memRefName) || memDep[memRefName] < inNodeNum){ // IO nodes access to the same array have only one cost
                        memDep[memRefName] = inNodeNum;
                        _dep_cost += inNodeNum; 
                    }
                    // _dep_cost += inNodeNum;        
                    allocated = true;
                    // break;
                }
                if(!allocated){
                    selBank = availBanks[0];
                    selStart = bankStatus[0].second;
                    if(!memDep.count(memRefName) || memDep[memRefName] < inNodeNum_3){ // IO nodes access to the same array have only one cost
                        memDep[memRefName] = inNodeNum_3;
                        _dep_cost += inNodeNum_3; 
                    }
                    // _dep_cost += inNodeNum_3;
                    allocated = true;
                }
            } 
            assert(allocated); 
            std::cout << "selbank: " << selBank << " selStart: " << selStart << " sizeofBank: " << sizeofBank<< std::endl;    
            std::cout << "iobAddr: " << ": " << ((selBank - *elem.second.begin()) * sizeofBank + selStart) / dataByte  << "\n"<< std::endl;  
            _cur_bank_status[selBank].used = LSType;
            // _cur_bank_status[selBank].iob = iobIdx;
            _cur_bank_status[selBank].start = selStart;
            _cur_bank_status[selBank].end = selStart + memSize;    
        }            

    }
    if(_ex_dep > 0){
        _dep_cost += inNodeNum_2;
    }
    // return res;
}


// get the config data and dependence
void IOScheduler::genCfgData(Mapping *mapping, std::ostream &os, std::ostream &os_bits)
{
    Configuration cfg(mapping);
    for(auto &elem : _dfg_io_infos){
        cfg.setDfgIoSpadAddr(elem.first, elem.second.iobAddr);
    }
    for(auto &elem: _partitionedStartBank){
        cfg.setpartitionedStartBank(elem.first, elem.second);
    }
    std::vector<CfgDataPacket> cfgData;
    cfg.getCfgData(cfgData);
    cfg.dumpCfgData(os_bits);
    int cfgSpadDataByte = _adg->cfgSpadDataWidth() / 8;
    int cfgAddrWidth = _adg->cfgAddrWidth();
    int cfgDataWidth = _adg->cfgDataWidth();
    int alignWidth = (cfgAddrWidth > 16) ? 32 : 16;
    assert(alignWidth >= cfgAddrWidth && cfgDataWidth >= alignWidth);
    int cfgNum = 0;
    for(auto& cdp : cfgData){
        cfgNum += cdp.data.size() * 32 / cfgDataWidth;
    }

    if(cfgAddrWidth > 16){
        os << "\tstatic unsigned int ";
    }else{
        os << "\tstatic unsigned short ";
    }
    os << "cin[" << cfgNum << "][" << (1 + cfgDataWidth / alignWidth) << "] __attribute__((aligned(" << cfgSpadDataByte << "))) = {\n";
    os << std::hex;
    int alignWidthHex = alignWidth/4;
    int arrayIdx = 0;
    std::map<int, int> arrayMap; //<the position of the configuration, the argIn Idx>
    for(auto& cdp : cfgData){
        int argInIdx = cdp.argInIdx;
        if(argInIdx >= 0){
            arrayMap[arrayIdx] = argInIdx;
        }
        os << "\t\t{";
        for(auto data : cdp.data){      
            if(alignWidth == 32){
                os << "0x" << std::setw(alignWidthHex) << std::setfill('0') << data << ", ";
            }else{
                os << "0x" << std::setw(alignWidthHex) << std::setfill('0') << (data & 0xffff) << ", ";
                os << "0x" << std::setw(alignWidthHex) << std::setfill('0') << (data >> 16) << ", ";
            }
            
        }
        os << "0x" << std::setw(alignWidthHex) << std::setfill('0') << (cdp.addr) << "},\n";
        arrayIdx++;
    }
    os << std::dec << "\t};\n\n";
    //@yuan_argIn: generate the code for replacing the configuration
    for(auto& elem : arrayMap){
        if(alignWidth == 32){
            os << "\tcin[" << elem.first << "][0] = argIn" << elem.second << ";\n";
        }else{
            os << "\tcin[" << elem.first << "][0] = argIn" << elem.second << "&0xffff;\n";
            os << "\tcin[" << elem.first << "][1] = (argIn" << elem.second << " >> 16)&0xffff;\n";
        }
    }
    _cfg_num = cfgNum;
    _cfg_len = cfgNum * (alignWidth + cfgDataWidth) / 8; // length of config_addr and config_data in bytes
    int cfgSpadSize = _adg->cfgSpadSize();
    int cfgBaseAddr;
    _ld_cfg_dep = 0;
    if(_cfg_len <= cfgSpadSize - _old_cfg_status.end){
        cfgBaseAddr = _old_cfg_status.end;
    }else if(_cfg_len <= _old_cfg_status.start){
        cfgBaseAddr = 0;
    }else{ // cfg data space overlap last cfg data space
        cfgBaseAddr = 0;
        _ld_cfg_dep = LD_DEP_EX_LAST_TASK;
    }
    _old_cfg_status.start = cfgBaseAddr;
    _old_cfg_status.end = cfgBaseAddr + (_cfg_len +  cfgSpadDataByte - 1) / cfgSpadDataByte * cfgSpadDataByte;
    _old_cfg_status.end = std::min(_old_cfg_status.end, cfgSpadSize);
}


// generate CGRA instructions
std::pair<std::vector<std::string>, std::vector<std::string>> IOScheduler::genInstructions(Mapping *mapping, std::ostream &os)
{
    DFG *dfg = mapping->getDFG();
    std::vector<std::string> ldArrayNames;
    std::vector<std::string> stArrayNames;
    int banks = _adg->numIobNodes();
    int sizeofBank = _adg->iobSpadBankSize();
    int cfgBaseAddrSpad = _old_cfg_status.start + banks * sizeofBank; // cfg spad on top of iob spad
    int cfgSpadDataByte = _adg->cfgSpadDataWidth() / 8;
    int cfgBaseAddrCtrl = _old_cfg_status.start / cfgSpadDataByte; // config base address the controller access
    os << "\tload_cfg((void*)cin, 0x" << std::hex << cfgBaseAddrSpad << std::dec << ", " 
       << _cfg_len << ", " << _task_id << ", " << _ld_cfg_dep << ");\n";
    std::map<int, DFGIONode*> dfgIoNodes;
    std::vector<std::pair<int, int>> ld_data_deps; // <id, dep>
    std::vector<int> st_ids; // store node ids  
    std::set<std::string> multiport_ld;
    std::set<std::string> multiport_st;
    std::map<std::string, int> arrayDeps; // store the max dependence type of an array 
    for(auto &elem : _dfg_io_infos){
        int id = elem.first;
        DFGIONode* dfgIONode = dynamic_cast<DFGIONode*>(dfg->node(id));
        dfgIoNodes[elem.first] = dfgIONode;
        auto name = dfgIONode->memRefName();
        bool isMultiport = dfg->isMultiportIoNode(dfgIONode);
        if(elem.second.isStore){
            if(!isMultiport || !multiport_st.count(name)){
                st_ids.push_back(id);
                if(isMultiport){ // only keep one
                    multiport_st.insert(name);
                }
            }            
        }else{
            if(!isMultiport || !multiport_ld.count(name)){
                int ori_dep = elem.second.dep;
                int sort_dep = 0;
                if(ori_dep == LD_DEP_ST_LAST_SEC_TASK){
                    sort_dep = 1;
                }else if(ori_dep == LD_DEP_EX_LAST_TASK){
                    sort_dep = 2;
                }else if(ori_dep == LD_DEP_ST_LAST_TASK){
                    sort_dep = 3;
                }
                ld_data_deps.push_back(std::make_pair(elem.first, sort_dep));
                if(isMultiport){ // only keep one
                    multiport_ld.insert(name);
                }
                std::string memRefName = dfgIONode->memRefName(); 
                if(!arrayDeps.count(memRefName)){
                    arrayDeps[memRefName] = sort_dep;
                }else if(arrayDeps[memRefName] < sort_dep){
                    arrayDeps[memRefName] = sort_dep;
                }
            }
        }        
    }
    

    // sort the load data commands according to array name to make same array access adjacent
    std::sort(ld_data_deps.begin(), ld_data_deps.end(), [&](std::pair<int, int> a, std::pair<int, int> b){
        std::string memRefNameA = dfgIoNodes[a.first]->memRefName(); 
        std::string memRefNameB = dfgIoNodes[b.first]->memRefName(); 
        // return memRefNameA < memRefNameB;
        return memRefNameA < memRefNameB || (memRefNameA == memRefNameB && a.second < b.second);
    });

    // sort the load data commands according to dependence type
    std::stable_sort(ld_data_deps.begin(), ld_data_deps.end(), [&](std::pair<int, int> a, std::pair<int, int> b){
        std::string memRefNameA = dfgIoNodes[a.first]->memRefName(); 
        std::string memRefNameB = dfgIoNodes[b.first]->memRefName(); 
        return (arrayDeps[memRefNameA] < arrayDeps[memRefNameB]);
        // return (a.second < b.second);
    });
    int i = 0;
    int ld_num = ld_data_deps.size();
    for(auto &elem : ld_data_deps){
        DFGIONode* dfgIONode = dfgIoNodes[elem.first];
        int dataLen = dfgIONode->memSize();
        std::string memRefName = dfgIONode->memRefName(); 
        std::string realMemName = dfgIONode->realMemRefName();
        int offset = dfgIONode->memOffset();   
        // std::cout << "io name: " << dfgIONode->name() << " offset: " << offset << std::endl;     
        // the next load command info
        int fused = 0; // fused with the next command
        int BankNum = dfgIONode->NumMultiportBank();
        int BankSize = dfgIONode->MultiportBankSize();
        auto& ioInfo = _dfg_io_infos[elem.first];
        if(BankNum > 1){//@yuan: for partitioned memory, generate multiple load command
            int logN = log2(BankNum);
            int logB = log2(BankSize);
            int j = 1;
            int numBank = _allocatedMultiportBank[memRefName].size();
            int sizeofBank = _adg->iobSpadBankSize();
            // std::cout << "lgN: " << logN << " logB: " << logB << " size: " <<  _allocatedMultiportBank[memRefName].size() << std::endl;
            for(auto &bank:  _allocatedMultiportBank[memRefName]){
                fused = j == numBank? 0 : 1;
                int bankAddr = bank * sizeofBank + _cur_bank_status[bank].start;
                // std::cout << "bank: " << bank << " bank size: " << sizeofBank << " start: " << _cur_bank_status[bank].start << std::endl;
                os << "\tload_data(din_addr[" << i << "], 0x" << std::hex << bankAddr << std::dec << ", " 
                << dataLen << ", " << fused << ", " << _task_id << ", " << ioInfo.dep << ", " << logN << ", " << logB << ");\n";
                j++;
            }
            if(offset > 0){
                realMemName = "(void*)" + realMemName + "+" + std::to_string(offset);
            }
            ldArrayNames.push_back(realMemName);
            i++;
            continue;
        }
        if(i < ld_num - 1){
            DFGIONode* dfgIONodeNext = dfgIoNodes[ld_data_deps[i+1].first];
            int dataLenNext = dfgIONodeNext->memSize();
            std::string memRefNameNext = dfgIONodeNext->realMemRefName(); 
            int offsetNext = dfgIONodeNext->memOffset();
            int BankNumNext = dfgIONodeNext->NumMultiportBank();
            int BankSizeNext = dfgIONodeNext->MultiportBankSize();
            if(realMemName == memRefNameNext && dataLen == dataLenNext && offset == offsetNext && BankNum == BankNumNext && BankSize == BankSizeNext){
                fused = 1;
            }
        }
        if(offset > 0){
            realMemName = "(void*)" + realMemName + "+" + std::to_string(offset);
        }
        ldArrayNames.push_back(realMemName);
        // void* remoteAddr = ioName2AddrLen[memRefName].first;        
        os << "\tload_data(din_addr[" << i << "], 0x" << std::hex << ioInfo.addr << std::dec << ", " 
           << dataLen << ", " << fused << ", " << _task_id << ", " << ioInfo.dep << ", " << 0 << ", " << 0 << ");\n";
        i++;
    }
    os << "\tconfig(0x" << std::hex << cfgBaseAddrCtrl << std::dec << ", " << _cfg_num << ", " << _task_id << ", " << 0 << ");\n";
    os << "\texecute(0x" << std::hex << _iob_ens << std::dec << ", " << _task_id << ", " << _ex_dep << ");\n";
    i = 0;
    for(auto id : st_ids){
        DFGIONode* dfgIONode = dfgIoNodes[id];
        int dataLen = dfgIONode->memSize();
        std::string memRefName = dfgIONode->memRefName();
        std::string realMemName = dfgIONode->realMemRefName();
        int offset = dfgIONode->memOffset();
        int fused = 0; // fused with the next command
        auto& ioInfo = _dfg_io_infos[id];
        int BankNum = dfgIONode->NumMultiportBank();
        int BankSize = dfgIONode->MultiportBankSize();
        if(BankNum > 1){//@yuan: for partitioned memory, generate multiple store command
            int logN = log2(BankNum);
            int logB = log2(BankSize);
            int j = 1;
            int numBank = _allocatedMultiportBank[memRefName].size();
            int sizeofBank = _adg->iobSpadBankSize();
            // std::cout << "lgN: " << logN << " logB: " << logB << " size: " <<  _allocatedMultiportBank[memRefName].size() << std::endl;
            for(auto &bank:  _allocatedMultiportBank[memRefName]){
                fused = j == numBank? 0 : 1;
                int bankAddr = bank * sizeofBank + _cur_bank_status[bank].start;
                os << "\tstore(dout_addr[" << i << "], 0x" << std::hex << bankAddr << std::dec << ", " 
                << dataLen << ", " << fused << ", "<< _task_id << ", " << 0 << ", " << logN << ", " << logB << ");\n";
                j++;
            }
            if(offset > 0){
                realMemName = "(void*)" + realMemName + "+" + std::to_string(offset);
            }
            stArrayNames.push_back(realMemName);
            i++;
            continue;
        }
        if(offset > 0){
            realMemName = "(void*)" + realMemName + "+" + std::to_string(offset);
        }
        stArrayNames.push_back(realMemName);
        // void* remoteAddr = ioName2AddrLen[memRefName].first;
        // int dataLen = ioName2AddrLen[memRefName].second;
        os << "\tstore(dout_addr[" << i << "], 0x" << std::hex << ioInfo.addr << std::dec << ", " 
           << dataLen << ", " << _task_id << ", " << 0 << ", "  << 0 << ", " << 0 << ", " << 0 << ");\n";
        i++;
    }
    return std::make_pair(ldArrayNames, stArrayNames);
}



// analyze dependence, allocate spad space, dump execution call function
void IOScheduler::execute(Mapping *mapping, std::ostream &os_func, std::ostream &os_call, std::ostream &os_bits)
{
    // dump execution call function head
    int numArgIn = mapping->getDFG()->numArgIn();
    if(numArgIn > 0){
        os_func << "void cgra_execute(void** din_addr, void** dout_addr, ";
        for(int i = 0; i < numArgIn - 1; i++){//TODO: adding support for 64/16 bit
            os_func << "uint32_t argIn" << i << ", ";
        }
        os_func << "uint32_t argIn" << (numArgIn - 1) << ")\n{\n" ;
    }else{
        os_func << "void cgra_execute(void** din_addr, void** dout_addr)\n{\n";
    }
    // analyze the dependence and allocate space in the spad
    ioSchedule(mapping);
    // update bank status
    int banks = _adg->numIobNodes();
    for (int i = 0; i < banks; i++) { 
        _older_bank_status[i] = _old_bank_status[i];
        _old_bank_status[i] = _cur_bank_status[i];        
    }
    /*
    Graphviz viz(mapping, "");
    // viz.drawDFG();
    // viz.drawADG();
    // viz.dumpDFGIO(); 
    viz.printDFGEdgePath();
    */

    // generate config data and dump to ostream
    genCfgData(mapping, os_func, os_bits);
    // generate CGRA execution instructions and dump
    auto arrayNames = genInstructions(mapping, os_func);
    os_func << "}\n";
    int ldNum = arrayNames.first.size();
    int stNum = arrayNames.second.size();
    if(ldNum > 0){
        os_call << "void* cgra_din_addr[" << ldNum << "] = {";
        for(int i = 0; i < ldNum-1; i++){
            os_call << arrayNames.first[i] << ", ";
        }
        os_call << arrayNames.first[ldNum-1] << "};\n";
    }
    if(stNum > 0){
        os_call << "void* cgra_dout_addr[" << stNum << "] = {";
        for(int i = 0; i < stNum-1; i++){
            os_call << arrayNames.second[i] << ", ";
        }
        os_call << arrayNames.second[stNum-1] << "};\n";
    }
    if(numArgIn > 0){
        os_call << "cgra_execute(cgra_din_addr, cgra_dout_addr, ";
        for(int i = 0; i < numArgIn - 1; i++){//TODO: adding support for 64/16 bit
            os_call << "uint32_t argIn" << i << ", ";
        }
        os_call << "uint32_t argIn" << (numArgIn - 1) << ");" ;

    }else{
        os_call << "cgra_execute(cgra_din_addr, cgra_dout_addr);\n";
    }
    _task_id++;
    // return arrayNames;
}


// @yuan: just for test, get the config data and dependence
void IOScheduler::genCfgDataOnly(Mapping *mapping, std::ostream &os)
{
    Configuration cfg(mapping);
    for(auto &elem : _dfg_io_infos){
        cfg.setDfgIoSpadAddr(elem.first, elem.second.iobAddr);
    }
    cfg.dumpCfgData(os);
    // std::vector<CfgDataPacket> cfgData;
    // cfg.getCfgData(cfgData);
    
}

//@yuan: just for test, generate the config-data for FGRA without SoC
void IOScheduler::execute(Mapping *mapping, std::ostream &os)
{
    ioSchedule(mapping);
    // generate config data and dump to ostream
    genCfgDataOnly(mapping, os);
}