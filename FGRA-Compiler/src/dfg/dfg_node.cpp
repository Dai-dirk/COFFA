
#include "dfg/dfg_node.h"

// ===================================================
//   DFGNode functions
// ===================================================

// set operation, latency, commutative according to operation name
void DFGNode::setOperation(std::string operation){ 
    if(!Operations::opCapable(operation) && operation != "LUT" && operation.substr(0, 2) != "MY" && operation != "ARGIN"){//@hw_yuan_op
        std::cout << operation << " is not supported!" << std::endl;
        exit(1);
    }
    // TODO: add bitwidth check
    _operation = operation; 
    std::cout << "operation: " << operation << std::endl;
    //@yuan_xcore: XCore has different latencies for different operations, hence its latency is set outside
    if(operation.substr(0, 2) != "MY")
        setOpLatency(Operations::latency(operation));
    setCommutative(Operations::isCommutative(operation));
    setAccumulative(Operations::isAccumulative(operation));
    if(operation == "ISEL" || operation == "CISEL"){
        setInitSelection(true);
    }
}


int DFGNode::numInputs(int bits){ 
    int num = 0;
    if(_inputs.count(bits)){
        num = _inputs[bits].size();
    }
    if(hasImm(bits)){
        num += 1;
    }
    if(has2ndImm() && bits != 1){
        num += 1;
    }
    return num; 
}


int DFGNode::numOutputs(int bits){ 
    int num = 0;
    if(_outputs.count(bits)){
        num = _outputs[bits].size();
    }
    return num; 
}


void DFGNode::print(){
    printGraphNode();
    std::cout << "operation: " << _operation << ", opLatency: " << _opLatency << std::endl;
    std::cout << "commutative: " << _commutative << std::endl;
    std::cout << "imm: ";
    for(auto elem : _bitWidths){
        if(hasImm(elem)){
            std::cout << "(" << elem << ", " << imm(elem).first << ", " << imm(elem).second << ") ";
        }        
    } 
    std::cout << std::endl;
}

void DFGNode::addInternalAttribute(std::string opName, std::string srcName, int srcPort, int operand, int width){
    internalAttr newConnection;
    newConnection.srcName = srcName;
    newConnection.srcPort = srcPort;
    // newConnection.edgeId = edgeId;
    newConnection.width =width;
    _internalAttrMap[opName][operand]= newConnection;
}

const std::map<int, internalAttr>& DFGNode::internalAttrMap(std::string opName){
    assert(_internalAttrMap.count(opName));
    return _internalAttrMap[opName];
}

void DFGNode::setMultiOps(std::vector<std::string> ops){
    for(auto elem : ops){
        _multiOps.push_back(elem);
    }
}

void DFGNode::clearImm(int bits){
    if(_imm.count(bits)){
        _imm.erase(bits);
    }
}

// ===================================================
//   DFGIONode functions
// ===================================================

void DFGIONode::print(){
    print();
    std::cout << "memRefName: " << _memRefName << std::endl;
    std::cout << "pattern: ";
    for(auto &elem : _pattern){
        std::cout << elem.first << " " << elem.second << " ";
    }
    std::cout << std::endl;    
}


std::string DFGIONode::memRefName(){
    if(_isOverSize && _multiportType == 0){
        std::string memName = _memRefName + name();
        return memName;
    }else{
        return _memRefName;
    }
}



//@yuan_fp
bool DFGIONode::isSmallPrecision(){
    if(_precision == -1){
        return false;
    }else{
        int bitwidth = *bitWidths().begin();
        if(bitwidth > _precision){
            return true;
        }else{
            return false;
        }
    }
}