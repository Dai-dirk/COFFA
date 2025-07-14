
#include "dfg/dfg_node.h"

// ===================================================
//   DFGNode functions
// ===================================================

// set operation, latency, commutative according to operation name
void DFGNode::setOperation(std::string operation){ 
    if(!Operations::opCapable(operation) && operation != "LUT"){
        std::cout << operation << " is not supported!" << std::endl;
        exit(1);
    }
    // TODO: add bitwidth check
    _operation = operation; 
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
    if(has2ndInit() && bits != 1){
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