
#include "adg/adg_node.h"


// ===================================================
//   ADGNode functions
// ===================================================

// // replace bit-width, including _bitWidths, _numInputs, _numOutputs, _inputs, _outputs
// void ADGNode::replaceBitWidth(int ori_bits, int new_bits){
//     if(ori_bits == new_bits || !_bitWidths.count(ori_bits)){
//         return;
//     }
//     _bitWidths.insert(new_bits);
//     _numInputs.emplace(_numInputs[ori_bits]);
//     _numOutputs.emplace(_numOutputs[ori_bits]);
//     _inputs.emplace(_inputs[ori_bits]);
//     _outputs.emplace(_outputs[ori_bits]);
//     _bitWidths.erase(ori_bits);
//     _numInputs.erase(ori_bits);
//     _numOutputs.erase(ori_bits);
//     _inputs.erase(ori_bits);
//     _outputs.erase(ori_bits);
// }


// get config info for sub-module
const CfgDataLoc& ADGNode::configInfo(int id){
    assert(_configInfo.count(id));
    return _configInfo[id];
}


// add config info for sub-module
void ADGNode::addConfigInfo(int id, CfgDataLoc subModuleCfg){
    _configInfo[id] = subModuleCfg;
}




void ADGNode::printADGNode(){
    printGraphNode();
    std::cout << "cfgBlkIdx: " << _cfgBlkIdx << std::endl;
}


void ADGNode::print(){
    printADGNode();
}

//@yuan: add historical usage 
void ADGNode::addUsage(int dfgNodeId){
    if(_historyUsage.count(dfgNodeId)){
        _historyUsage[dfgNodeId] += 1;
    }else{
        _historyUsage[dfgNodeId] = 1;
    }
}

//@yuan: get the ADGNode historical usage of specific dfgNode
int ADGNode::getUsage(int dfgNodeId){
    if(_historyUsage.count(dfgNodeId)){
        return _historyUsage[dfgNodeId];
    }else{
        return 0;
    }
}


//@yuan: get the ADGNode historical usage of specific dfgNode
void ADGNode::clearUsage(){
    _historyUsage.clear();
}
// ===================================================
//   FUNode functions
// ===================================================

// add supported operation
void FUNode::addOperation(std::string op){
    _operations.emplace(op);
}

// delete supported operation
void FUNode::delOperation(std::string op){
    _operations.erase(op);
}

// check if the operation is supported
bool FUNode::opCapable(std::string op){
    return _operations.count(op);
}

int FUNode::numOperands(int bits){ 
    if(_numOperands.count(bits)){
        return _numOperands[bits]; 
    }
    return 0;
}

// set numOperand and resize _operandInputs 
void FUNode::setNumOperands(int bits, int numOperands){ 
    _numOperands[bits] = numOperands; 
    _operandInputs[bits].resize(numOperands);
}

int FUNode::maxDelay(int bits){ 
    if(_maxDelay.count(bits)){
        return _maxDelay[bits]; 
    }
    return 0;
}
void FUNode::setMaxDelay(int bits, int maxDelay){ 
    _maxDelay[bits] = maxDelay; 
}


// get input ports connected to this operand
const std::set<int>& FUNode::operandInputs(int bits, int opeIdx){
    // if(bits == 1) {
    //     std::cout << "_operandInputs[bits]: " << _operandInputs[bits].size() << " opeIdx: " << opeIdx << std::endl;
    //     for(auto elem : _operandInputs[bits]){
    //         for(auto e : elem){
    //             std::cout << "elem: " << e << std::endl;
    //         }
    //     }
    // }
    assert(_operandInputs.count(bits));
    assert(_operandInputs[bits].size() > opeIdx);
    return _operandInputs[bits][opeIdx]; 
}

// add input port connected to this operand
void FUNode::addOperandInput(int bits, int opeIdx, int inputIdx){
    assert(_operandInputs.count(bits));
    assert(_operandInputs[bits].size() > opeIdx);
    _operandInputs[bits][opeIdx].emplace(inputIdx);
    // if(inputs(bits).count(inputIdx)){
        
    // }
}


// delete input port connected to this operand
void FUNode::delOperandInput(int bits, int opeIdx, int inputIdx){
    assert(_operandInputs.count(bits));
    assert(_operandInputs[bits].size() > opeIdx);
    if(inputs(bits).count(inputIdx)){
        _operandInputs[bits][opeIdx].erase(inputIdx);
    }
}


// set input ports connected to this operand
void FUNode::setOperandInputs(int bits, int opeIdx, std::set<int> inputIdxs){
    assert(_operandInputs.count(bits));
    assert(_operandInputs[bits].size() > opeIdx);
    _operandInputs[bits][opeIdx] = inputIdxs;
}


// get which operand this input is connected
int FUNode::getOperandIdx(int bits, int inputIdx){
    auto &ins = _operandInputs[bits];
    int num = _numOperands[bits];
    for(int i = 0; i < num; i++){
        if(ins[i].count(inputIdx)){
            return i;
        }
    }
    return -1;
}



void FUNode::printFU(){
    printADGNode();
    std::cout << "operations: ";
    for(auto& elem : _operations){
        std::cout << elem << ", ";        
    }
    std::cout << "\nmaxDelay: ";
    for(auto& elem : _maxDelay){
        std::cout << "(" << elem.first << ", " << elem.second << ") ";
    }
    std::cout << "\nnumOperands: ";
    for(auto& elem : _numOperands){
        std::cout << "(" << elem.first << ", " << elem.second << ") ";
    }
    std::cout << std::endl;
    for(auto& bits : _operandInputs){
        std::cout << "operandInputs(" << bits.first << "):\n";
        int i = 0;
        for(auto &elem : bits.second){
            std::cout << i++ << ": ";
            for(auto in : elem){
                std::cout << in << " ";
            }
            std::cout << std::endl;
        }
    }
    
}

void FUNode::print(){
    printFU();
}


// ===================================================
//   GPENode functions
// ===================================================

// set numOperand and resize _operandInputs 
// void GPENode::setNumConACCINput(int bits, int numConACCInputs){ 
//     _ConACCInputs[bits].resize(numConACCInputs);
// }
// get the operand index in PE according to the operand index in LUT
int GPENode::getOperandIdxLUT(int idxLUT){
    if(_numOperands[1] > _numInputLUT){
        // std::cout << "_numOperands[1]: " << _numOperands[1] << " _numInputLUT: " << _numInputLUT << std::endl;
        return idxLUT + _numOperands[1] - _numInputLUT;
    }else{
        return idxLUT;
    }
}

// get input ports connected to this DMR control signal
const std::set<int>& GPENode::ConDMRInputs(int bits, int opeIdx){
    assert(_ConDMRInputs.count(bits));
    assert(_ConDMRInputs[bits].count(opeIdx));
    return _ConDMRInputs[bits][opeIdx]; 
}

// add input port connected to this DMR control signal
void GPENode::addConDMRInput(int bits, int opeIdx, int inputIdx){
    // assert(_ConACCInputs.count(bits));
    // assert(_ConACCInputs[bits].size() > opeIdx);
    if(inputs(bits).count(inputIdx)){
        _ConDMRInputs[bits][opeIdx].emplace(inputIdx);
    }
}


// delete input port connected to this DMR control signal
void GPENode::delConDMRInput(int bits, int opeIdx, int inputIdx){
    assert(_ConDMRInputs.count(bits));
    assert(_ConDMRInputs[bits].count(opeIdx));
    if(inputs(bits).count(inputIdx)){
        _ConDMRInputs[bits][opeIdx].erase(inputIdx);
    }
}


// set input ports connected to this DMR control signal
void GPENode::setConDMRInput(int bits, int opeIdx, std::set<int> inputIdxs){
    // assert(_ConACCInputs.count(bits));
    // assert(_ConACCInputs[bits].size() > opeIdx);
    _ConDMRInputs[bits][opeIdx] = inputIdxs;
}


// get which operand this DMR control signal is connected
int GPENode::getConDMRIdx(int bits, int inputIdx){
    assert(_ConDMRInputs.count(bits));
    auto &ins = _ConDMRInputs[bits];
    int num = ins.size();
    for(int i = 0; i < num; i++){
        if(ins[i].count(inputIdx)){
            return i;
        }
    }
    return -1;
}


// @yuan: get the operand index in PE for fine-grained signal excepted LUT
int GPENode::getOperandIdxFg(int numCghOp, int idxFg){
    int extraIdx = 0;
    if(opCapable("SEL") || opCapable("SEXT") || opCapable("ZEXT") || opCapable("ADC") || opCapable("ADD")){
        extraIdx = 1;
    }
    return idxFg - numCghOp + extraIdx;
}

void GPENode::addCGEusId(std::string Eu, int idx){
    if(!_CGEusIdx.count(Eu)){
        _CGEusIdx[Eu] = idx;
    }
}

void GPENode::addFGEusId(std::string Eu, int idx){
    if(!_FGEusIdx.count(Eu)){
        _FGEusIdx[Eu] = idx;
    }
}


int GPENode::FGEusId(std::string Eu) { 
    if(_FGEusIdx.count(Eu)){
        return _FGEusIdx[Eu];
    }else{
        return 0;
    }    
}

int GPENode::CGEusId(std::string Eu) { 
    if(_CGEusIdx.count(Eu)){
        return _CGEusIdx[Eu];
    }else{
        return 0;
    }    
}


void GPENode::print(){
    printFU();
    std::cout << "numInputLUT: " << _numInputLUT << std::endl;
}
//@hw_yuan:
// get config info for eu
const EuCfgDataLoc& GPENode::euConfigInfo(std::string name){
    assert(_euConfigInfo.count(name));
    return _euConfigInfo[name];
}


// add config info for eu
void GPENode::addEuConfigInfo(std::string name, EuCfgDataLoc euCfg){
    _euConfigInfo[name] = euCfg;
}

// ===================================================
//   IOBNode functions
// ===================================================

void IOBNode::print(){
    printFU();
    std::cout << "index: " << _index << std::endl;
    std::cout << "iocCfgIdMap: ";
    for(auto& elem : cfgIdMap){
        std::cout << elem.first << ", " << elem.second << "; ";      
    }
    std::cout << std::endl;
}

int IOBNode::getOperandIdxIOB(int idxIOB){
    if(opCapable("TCLOAD") && opCapable("TCSTORE")){ // for the TLOAD/TSTORE be mapped to IOB has both en and task_exit
        return idxIOB + 1;
    }else{
        return idxIOB;
    }
}



// ===================================================
//   GIBNode functions
// ===================================================

// if there are registers in the output port 
bool GIBNode::outReged(int idx){
    return _outReged[idx];
}

// if there are registers in the output port 
void GIBNode::setOutReged(int idx, bool reged){
    _outReged[idx] = reged;
}

// inputs connected to each output
std::set<int> GIBNode::out2ins(int outPort){
    if(_out2ins.count(outPort)){
        return _out2ins[outPort];
    } else {
        return {};
    }
}


// outputs connected to each input
std::set<int> GIBNode::in2outs(int inPort){
    if(_in2outs.count(inPort)){
        return _in2outs[inPort];
    } else {
        return {};
    }
}

// add input connected to the output
void GIBNode::addOut2ins(int outPort, int inPort){
    int bits = *(_bitWidths.begin()); // GIB has only one bitwidth
    if(_inputs[bits].count(inPort) && _outputs[bits].count(outPort)){
        _out2ins[outPort].emplace(inPort);
    }
}

// add output connected to the input
void GIBNode::addIn2outs(int inPort, int outPort){
    int bits = *(_bitWidths.begin()); // GIB has only one bitwidth
    if(_inputs[bits].count(inPort) && _outputs[bits].count(outPort)){
        _in2outs[inPort].emplace(outPort);
    }
}


// check if the input and output port are connected
bool GIBNode::isInOutConnected(int inPort, int outPort){
    if(_out2ins.count(outPort)){
        for(auto in : _out2ins[outPort]){
            if(in == inPort){
                return true;
            }
        }
        
    }
    return false;
}


void GIBNode::print(){
    printADGNode();
    std::cout << "trackReged: " << _trackReged << std::endl;
    std::cout << "outReged: " << std::endl;
    for(auto& elem : _outReged){
        std::cout << elem.first << ": " << elem.second << std::endl;
    }
    std::cout << "out2ins: " << std::endl;
    for(auto& elem : _out2ins){
        std::cout << elem.first << ": ";
        for(auto sec : elem.second)
            std::cout << sec << ", ";
        std::cout << std::endl;
    }
    std::cout << "in2outs: " << std::endl;
    for(auto& elem : _in2outs){
        std::cout << elem.first << ": ";
        for(auto sec : elem.second)
            std::cout << sec << ", ";
        std::cout << std::endl;
    }
}



// // ===================================================
// //   LSUNode functions
// // ===================================================

// // get input ports connected to this operand
// const std::set<int>& LSUNode::operandInputs(int opeIdx){
//     assert(_operandInputs.size() > opeIdx);
//     return _operandInputs[opeIdx]; 
// }

// // add input port connected to this operand
// void LSUNode::addOperandInputs(int opeIdx, int inputIdx){
//     assert(_operandInputs.size() > opeIdx);
//     if(inputs().count(inputIdx)){
//         _operandInputs[opeIdx].emplace(inputIdx);
//     }
// }


// // delete input port connected to this operand
// void LSUNode::delOperandInputs(int opeIdx, int inputIdx){
//     assert(_operandInputs.size() > opeIdx);
//     if(inputs().count(inputIdx)){
//         _operandInputs[opeIdx].erase(inputIdx);
//     }
// }


// // add input ports connected to this operand
// void LSUNode::addOperandInputs(int opeIdx, std::set<int> inputIdxs){
//     assert(_operandInputs.size() > opeIdx);
//     _operandInputs[opeIdx] = inputIdxs;
// }


// // get which operand this input is connected
// int LSUNode::getOperandIdx(int inputIdx){
//     for(int i = 0; i < 2; i++){
//         if(_operandInputs[i].count(inputIdx)){
//             return i;
//         }
//     }
//     return -1;
// }


// void LSUNode::print(){
//     printADGNode();
//     std::cout << "operandInputs: " << std::endl;
//     int i = 0;
//     for(auto& elem : _operandInputs){
//         std::cout << i++ << ": ";
//         for(auto in : elem){
//             std::cout << in << " ";
//         }
//         std::cout << std::endl;
//     }
// }