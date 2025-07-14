
#include "graph/graph_node.h"

// ===================================================
//   GraphNode functions
// ===================================================

std::string GraphNode::name(){ 
    if(!_name.empty()){
        return _name; 
    }else{
        return _type + std::to_string(_id);
    }
}


int GraphNode::numInputs(int bits){ 
    int num = 0;
    if(_inputs.count(bits)){
        num = _inputs[bits].size();
    }
    return num; 
}


int GraphNode::numOutputs(int bits){ 
    int num = 0;
    if(_outputs.count(bits)){
        num = _outputs[bits].size();
    }
    return num; 
}


const std::map<int, std::pair<int, int>>& GraphNode::inputs(int bits){ 
    assert(_inputs.count(bits));
    return _inputs[bits]; 
}

const std::map<int, std::set<std::pair<int, int>>>& GraphNode::outputs(int bits){ 
    assert(_outputs.count(bits));
    return _outputs[bits]; 
}


// return <node-id, node-port-idx>
std::pair<int, int> GraphNode::input(int bits, int index){
    if(_inputs.count(bits)){
        if(_inputs[bits].count(index)){
            return _inputs[bits][index];
        }        
    }
    return {}; // return empty set
}

// return set<node-id, node-port-idx>
std::set<std::pair<int, int>> GraphNode::output(int bits, int index){
    if(_outputs.count(bits)){
        if(_outputs[bits].count(index)){
            return _outputs[bits][index];
        }        
    }
    return {}; // return empty set
}

// add input
void GraphNode::addInput(int bits, int index, std::pair<int, int> node){
    _inputs[bits][index] = node;
}

// add output
void GraphNode::addOutput(int bits, int index, std::pair<int, int> node){
    _outputs[bits][index].emplace(node);
}

// delete input
void GraphNode::delInput(int bits, int index){
    if(_inputs.count(bits)){
        _inputs[bits].erase(index);
    }
}

// delete output
void GraphNode::delOutput(int bits, int index, std::pair<int, int> node){
    if(_outputs.count(bits)){
        if(_outputs[bits].count(index)){
            _outputs[bits][index].erase(node);
        }
    }
}

// delete output
void GraphNode::delOutput(int bits, int index){
    if(_outputs.count(bits)){
        _outputs[bits].erase(index);
    }
}


const std::map<int, int>& GraphNode::inputEdges(int bits){
    assert(_inputEdges.count(bits));
    return _inputEdges[bits];
}


const std::map<int, std::set<int>>& GraphNode::outputEdges(int bits){
    // assert(_outputEdges.count(bits));
    static std::map<int, std::set<int>> emptyMap;
    if(!_outputEdges.count(bits)){
        return emptyMap;
    }else{
        return _outputEdges[bits];
    }
}

// return edge id
int GraphNode::inputEdge(int bits, int index){
    if(_inputEdges.count(bits)){
        if(_inputEdges[bits].count(index)){
            return _inputEdges[bits][index];
        }
    }
    return -1; 
}

// return set<edge-id>
std::set<int> GraphNode::outputEdge(int bits, int index){
    if(_outputEdges.count(bits)){
        if(_outputEdges[bits].count(index)){
            return _outputEdges[bits][index];
        }
    }
    return {}; // return empty set
}

// add input edge
void GraphNode::addInputEdge(int bits, int index, int edgeId){
    _inputEdges[bits][index] = edgeId;
}

// add output edge
void GraphNode::addOutputEdge(int bits, int index, int edgeId){
    _outputEdges[bits][index].emplace(edgeId);
}

// delete input edge
void GraphNode::delInputEdge(int bits, int index){
    if(_inputEdges.count(bits)){
        _inputEdges[bits].erase(index);
    }
}

// delete output edge
void GraphNode::delOutputEdge(int bits, int index, int edgeId){
    if(_outputEdges.count(bits)){
        if(_outputEdges[bits].count(index)){
            _outputEdges[bits][index].erase(edgeId);
        }
    }
}

// delete output edge
void GraphNode::delOutputEdge(int bits, int index){
    if(_outputEdges.count(bits)){
        _outputEdges[bits].erase(index);
    }
}


void GraphNode::printGraphNode(){
    std::cout << "=====================================\n";
    std::cout << "id: " << _id << std::endl;
    std::cout << "type: " << _type << std::endl;
    std::cout << "name: " << _name << std::endl;
    std::cout << "bitWidths: ";
    for(auto& elem : _bitWidths){
        std::cout << elem << " ";
    }
    std::cout << "\nnumInputs: ";
    for(auto elem : _bitWidths){
        std::cout << "(" << elem << ", " << numInputs(elem) << ") ";
    } 
    std::cout << "\nnumOutputs: ";
    for(auto elem : _bitWidths){
        std::cout << "(" << elem << ", " << numOutputs(elem) << ") ";
    } 
    std::cout << std::endl;
    for(auto& bits : _inputs){
        std::cout << "Inputs(" << bits.first << "):\n";
        for(auto &elem : bits.second){
            std::cout << elem.first << ": (" << elem.second.first << ", " << elem.second.second << ")\n";
        }
    }
    for(auto& bits : _outputs){
        std::cout << "Outputs(" << bits.first << "):\n";
        for(auto &elem : bits.second){
            std::cout << elem.first << ": ";
            auto& s = elem.second;
            for(auto it = s.begin(); it != s.end(); it++)
                std::cout << "(" << it->first << ", " << it->second << ") ";
            std::cout << std::endl;
        }
    }
}