
#include "graph/graph.h"

Graph::Graph(){}

Graph::~Graph()
{
    // for(auto& elem : _nodes){
    //     delete elem.second;
    // }
    // for(auto& elem : _edges){
    //     delete elem.second;
    // }
}


int Graph::numInputs(int bits){ 
    int num = 0;
    if(_inputs.count(bits)){
        num = _inputs[bits].size();
    }
    return num; 
}


int Graph::numOutputs(int bits){ 
    int num = 0;
    if(_outputs.count(bits)){
        num = _outputs[bits].size();
    }
    return num; 
}


void Graph::setInputName(int bits, int index, std::string name){
    _inputNames[bits][index] = name;
}

std::string Graph::inputName(int bits, int index){
    if(_inputNames.count(bits)){
        if(_inputNames[bits].count(index)){
            return _inputNames[bits][index];
        }        
    }
    return "";
}

void Graph::setOutputName(int bits, int index, std::string name){
    _outputNames[bits][index] = name;
}

std::string Graph::outputName(int bits, int index){
    if(_outputNames.count(bits)){
        if(_outputNames[bits].count(index)){
            return _outputNames[bits][index];
        }        
    }
    return "";
}


const std::map<int, std::pair<int, int>>& Graph::outputs(int bits){ 
    assert(_outputs.count(bits));
    return _outputs[bits]; 
}

const std::map<int, std::set<std::pair<int, int>>>& Graph::inputs(int bits){ 
    assert(_inputs.count(bits));
    return _inputs[bits]; 
}


// return <node-id, node-port-idx>
std::pair<int, int> Graph::output(int bits, int index){
    if(_outputs.count(bits)){
        if(_outputs[bits].count(index)){
            return _outputs[bits][index];
        }        
    }
    return {}; // return empty set
}

// return set<node-id, node-port-idx>
std::set<std::pair<int, int>> Graph::input(int bits, int index){
    if(_inputs.count(bits)){
        if(_inputs[bits].count(index)){
            return _inputs[bits][index];
        }        
    }
    return {}; // return empty set
}

// add output, _output_used
void Graph::addOutput(int bits, int index, std::pair<int, int> node){
    _outputs[bits][index] = node;
}

// add input, _input_used
void Graph::addInput(int bits, int index, std::pair<int, int> node){
    _inputs[bits][index].emplace(node);
}

// delete output
void Graph::delOutput(int bits, int index){
    if(_outputs.count(bits)){
        _outputs[bits].erase(index);
    }
}

// delete input
void Graph::delInput(int bits, int index, std::pair<int, int> node){
    if(_inputs.count(bits)){
        if(_inputs[bits].count(index)){
            _inputs[bits][index].erase(node);
        }
    }
}

// delete input
void Graph::delInput(int bits, int index){
    if(_inputs.count(bits)){
        _inputs[bits].erase(index);
    }
}



const std::map<int, int>& Graph::outputEdges(int bits){
    assert(_outputEdges.count(bits));
    return _outputEdges[bits];
}


const std::map<int, std::set<int>>& Graph::inputEdges(int bits){
    assert(_inputEdges.count(bits));
    return _inputEdges[bits];
}

// return edge id
int Graph::outputEdge(int bits, int index){
    if(_outputEdges.count(bits)){
        if(_outputEdges[bits].count(index)){
            return _outputEdges[bits][index];
        }
    }
    return -1; 
}

// return set<edge-id>
std::set<int> Graph::inputEdge(int bits, int index){
    if(_inputEdges.count(bits)){
        if(_inputEdges[bits].count(index)){
            return _inputEdges[bits][index];
        }
    }
    return {}; // return empty set
}

// add output edge
void Graph::addOutputEdge(int bits, int index, int edgeId){
    _outputEdges[bits][index] = edgeId;
}

// add input edge
void Graph::addInputEdge(int bits, int index, int edgeId){
    _inputEdges[bits][index].emplace(edgeId);
}

// delete output edge
void Graph::delOutputEdge(int bits, int index){
    if(_outputEdges.count(bits)){
        _outputEdges[bits].erase(index);
    }
}

// delete input edge
void Graph::delInputEdge(int bits, int index, int edgeId){
    if(_inputEdges.count(bits)){
        if(_inputEdges[bits].count(index)){
            _inputEdges[bits][index].erase(edgeId);
        }
    }
}

// delete input edge
void Graph::delInputEdge(int bits, int index){
    if(_inputEdges.count(bits)){
        _inputEdges[bits].erase(index);
    }
}


// GraphNode* Graph::node(int id){
//     if(_nodes.count(id)){
//         return _nodes[id];
//     } else {
//         return nullptr;
//     }  
// }


// GraphEdge* Graph::edge(int id){
//     if(_edges.count(id)){
//         return _edges[id];
//     } else {
//         return nullptr;
//     }  
// }


// void Graph::addNode(GraphNode* node){
//     int id = node->id();
//     _nodes[id] = node;
// }


// void Graph::addEdge(GraphEdge* edge){
//     int id = edge->id();
//     _edges[id] = edge;
//     int srcId = edge->srcId();
//     int dstId = edge->dstId();
//     int srcPort = edge->srcPortIdx();
//     int dstPort = edge->dstPortIdx();
//     int bits = edge->bitWidth();
//     if(srcId == _id){ // source is input port
//         addInput(bits, srcPort, std::make_pair(dstId, dstPort));
//         if(_recordNodeEdge) {
//             addInputEdge(bits, srcPort, id);
//         }
//     } else {
//         GraphNode* src = node(srcId);
//         assert(src);
//         src->addOutput(bits, srcPort, std::make_pair(dstId, dstPort));
//         if(_recordNodeEdge) {
//             src->addOutputEdge(bits, srcPort, id);
//         }
//     }
//     if(dstId == _id){ // destination is output port
//         addOutput(bits, dstPort, std::make_pair(srcId, srcPort));
//         if(_recordNodeEdge) {
//             addOutputEdge(bits, dstPort, id);
//         }
//     } else{        
//         GraphNode* dst = node(dstId);
//         assert(dst);
//         dst->addInput(bits, dstPort, std::make_pair(srcId, srcPort));
//         if(_recordNodeEdge) {
//             dst->addInputEdge(bits, dstPort, id);
//         }
//     }
// }


// void Graph::delNode(int id){
//     GraphNode* dfgNode = node(id);
//     if(_recordNodeEdge) {
//         for(auto bits : dfgNode->bitWidths()){
//             for(auto& elem : dfgNode->inputEdges(bits)){
//                 delEdge(elem.second);
//             }
//             for(auto& elem : dfgNode->outputEdges(bits)){
//                 for(auto eid : elem.second){
//                     delEdge(eid);
//                 }        
//             }
//         }
//     }
//     _nodes.erase(id);
//     delete dfgNode;
// }


// void Graph::delEdge(int id){
//     GraphEdge* e = edge(id);
//     int srcId = e->srcId();
//     int dstId = e->dstId();
//     int srcPortIdx = e->srcPortIdx();
//     int dstPortIdx = e->dstPortIdx();
//     int bits = e->bitWidth();
//     if(srcId == _id){
//         if(_recordNodeEdge) {
//             delInputEdge(bits, srcPortIdx, id);
//         }
//         delInput(bits, srcPortIdx, std::make_pair(dstId, dstPortIdx));
//     }else{
//         GraphNode* srcNode = node(srcId);   
//         if(_recordNodeEdge) {    
//             srcNode->delOutputEdge(bits, srcPortIdx, id);
//         }
//         srcNode->delOutput(bits, srcPortIdx, std::make_pair(dstId, dstPortIdx));
//     }
//     if(dstId == _id){
//         if(_recordNodeEdge) {
//             delOutputEdge(bits, dstPortIdx);
//         }
//         delOutput(bits, dstPortIdx);
//     }else{
//         GraphNode* dstNode = node(dstId);
//         if(_recordNodeEdge) {
//             dstNode->delInputEdge(bits, dstPortIdx);
//         }
//         dstNode->delInput(bits, dstPortIdx);
//     }
//     _edges.erase(id);
//     delete e;
// }


// // ====== operators >>>>>>>>>>
// // Graph copy
// Graph& Graph::operator=(const Graph& that){
//     if(this == &that) return *this;
//     this->_id = that._id;
//     this->_bitWidths = that._bitWidths;
//     this->_inputNames = that._inputNames;
//     this->_outputNames = that._outputNames;
//     this->_inputs = that._inputs;
//     this->_outputs = that._outputs;
//     this->_inputEdges = that._inputEdges;
//     this->_outputEdges = that._outputEdges;
//     for(auto& elem : that._nodes){
//         int id = elem.first;
//         GraphNode* node = new GraphNode();
//         *node = *(elem.second);
//         this->_nodes[id] = node;
//     }
//     // this->_edges = that._edges;
//     for(auto& elem : that._edges){
//         int id = elem.first;
//         GraphEdge* edge = new GraphEdge();
//         *edge = *(elem.second);
//         this->_edges[id] = edge;
//     }
//     return *this;
// }



void Graph::printGraph(){
    std::cout << "======================================================================\n";
    std::cout << "Graph(id): " << _id << std::endl;
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
    for(auto& elem : _inputNames){
        std::cout << "\ninputNames(" << elem.first << "): ";
        for(auto& port_name : elem.second){
            std::cout << "(" << port_name.first << ", " << port_name.second << ") ";
        }        
    }
    for(auto& elem : _outputNames){
        std::cout << "\noutputNames(" << elem.first << "): ";
        for(auto& port_name : elem.second){
            std::cout << "(" << port_name.first << ", " << port_name.second << ") ";
        }        
    }
    std::cout << std::endl;
    for(auto& bits : _inputs){
        std::cout << "Inputs(" << bits.first << "):\n";
        for(auto &elem : bits.second){
            std::cout << elem.first << ": ";
            auto& s = elem.second;
            for(auto it = s.begin(); it != s.end(); it++)
                std::cout << "(" << it->first << ", " << it->second << ") ";
            std::cout << std::endl;
        }
    }
    for(auto& bits : _outputs){
        std::cout << "Outputs(" << bits.first << "):\n";
        for(auto &elem : bits.second){
            std::cout << elem.first << ": (" << elem.second.first << ", " << elem.second.second << ")\n";
        }
    }
    for(auto& bits : _inputEdges){
        std::cout << "InputEdges(" << bits.first << "):\n";
        for(auto &elem : bits.second){
            std::cout << elem.first << ": ";
            auto& s = elem.second;
            for(auto it = s.begin(); it != s.end(); it++)
                std::cout << *it;
            std::cout << std::endl;
        }
    }
    for(auto& bits : _outputEdges){
        std::cout << "OutputEdges(" << bits.first << "):\n";
        for(auto &elem : bits.second){
            std::cout << elem.first << ": " << elem.second << "\n";
        }
    }
    // for(auto& elem : _nodes){
    //     elem.second->printGraphNode();
    // }
}



