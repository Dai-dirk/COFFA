#ifndef __GRAPH_NODE_H__
#define __GRAPH_NODE_H__

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <assert.h>


class GraphNode
{
protected:
    int _id;
    std::string _name;
    std::string _type;

    std::set<int> _bitWidths;  
    // <bit-width, <input-index, <node-id, node-port-idx>>>
    std::map<int, std::map<int, std::pair<int, int>>> _inputs;
    // <bit-width, <output-index, set<node-id, node-port-idx>>>>
    std::map<int, std::map<int, std::set<std::pair<int, int>>>> _outputs;
    // <bit-width, <input-index, edge-id>>
    std::map<int, std::map<int, int>> _inputEdges; 
    // <bit-width, <output-index, set<edge-id>>>
    std::map<int, std::map<int, std::set<int>>> _outputEdges; 
public:
    GraphNode(){}
    ~GraphNode(){}
    int id(){ return _id; }
    void setId(int id){ _id = id; }
    std::string name();
    void setName(std::string name){ _name = name; }
    std::string type(){ return _type; }
    void setType(std::string type){ _type = type; }
    
    const std::set<int>& bitWidths(){ return _bitWidths; }
    void setBitWidths(std::set<int> bits){ _bitWidths = bits; }
    void addBitWidth(int bitWidth){ _bitWidths.insert(bitWidth); }
    void delBitWidth(int bitWidth){ _bitWidths.erase(bitWidth); }

    virtual int numInputs(int bits);
    virtual int numOutputs(int bits);
    const std::map<int, std::map<int, std::pair<int, int>>>& inputs(){ return _inputs; }
    const std::map<int, std::pair<int, int>>& inputs(int bits);
    const std::map<int, std::map<int, std::set<std::pair<int, int>>>>& outputs(){ return _outputs; } 
    const std::map<int, std::set<std::pair<int, int>>>& outputs(int bits);
    std::pair<int, int> input(int bits, int index);                // return <node-id, node-port-idx>
    std::set<std::pair<int, int>> output(int bits, int index);     // return set<node-id, node-port-idx>
    void addInput(int bits, int index, std::pair<int, int> node);
    void addOutput(int bits, int index, std::pair<int, int> node);
    void delInput(int bits, int index);  // delete input
    void delOutput(int bits, int index, std::pair<int, int> node); // delete output
    void delOutput(int bits, int index); // delete output

    const std::map<int, std::map<int, int>>& inputEdges(){ return _inputEdges; }
    const std::map<int, std::map<int, std::set<int>>>& outputEdges(){ return _outputEdges; }
    const std::map<int, int>& inputEdges(int bits);
    const std::map<int, std::set<int>>& outputEdges(int bits);
    int inputEdge(int bits, int index); // return edge id
    std::set<int> outputEdge(int bits, int index); // return set<edge-id>
    void addInputEdge(int bits, int index, int edgeId);  // add input edge
    void addOutputEdge(int bits, int index, int edgeId); // add output edge
    void delInputEdge(int bits, int index); // delete input edge
    void delOutputEdge(int bits, int index, int edgeId); // delete output edge
    void delOutputEdge(int bits, int index); // delete output edge
    
    virtual void printGraphNode();
};





#endif