#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "graph/graph_node.h"
#include "graph/graph_edge.h"


class Graph
{
private:
    // bool _recordNodeEdge = false; // record the input/output edges of each node and the graph
protected:
    int _id; // "This"(INPUT/OUTPUT) ID
    // std::map<int, GraphNode*> _nodes;   // <node-id, node>
    // std::map<int, GraphEdge*> _edges;   // <edge-id, edge>

    std::set<int> _bitWidths;  
    // std::map<int, int> _numInputs; // <bit-width, input-num>
    // std::map<int, int> _numOutputs; // <bit-width,output-num>
    std::map<int, std::map<int, std::string>> _inputNames;  // <bit-width, <input-index, input-port-name>>
    std::map<int, std::map<int, std::string>> _outputNames; // <bit-width, <input-index, input-port-name>>
    // <bit-width, <output-index, <node-id, node-port-idx>>>
    std::map<int, std::map<int, std::pair<int, int>>> _outputs;
    // <bit-width, <input-index, set<node-id, node-port-idx>>>>
    std::map<int, std::map<int, std::set<std::pair<int, int>>>> _inputs;
    // <bit-width, <input-index, set<edge-id>>>
    std::map<int, std::map<int, std::set<int>>> _inputEdges; 
    // <bit-width, <output-index, edge-id>>
    std::map<int, std::map<int, int>> _outputEdges; 

public:
    Graph();
    ~Graph();
    int id(){ return _id; }
    void setId(int id){ _id = id; }
    // bool recordNodeEdge(){ return _recordNodeEdge; }
    // void setRecordNodeEdge(bool record){ _recordNodeEdge = record; }

    const std::set<int>& bitWidths(){ return _bitWidths; }
    void setBitWidths(std::set<int> bits){ _bitWidths = bits; }
    void addBitWidth(int bitWidth){ _bitWidths.insert(bitWidth); }
    void delBitWidth(int bitWidth){ _bitWidths.erase(bitWidth); }

    int numInputs(int bits);
    int numOutputs(int bits);
    void setInputName(int bits, int index, std::string name);
    std::string inputName(int bits, int index);
    void setOutputName(int bits, int index, std::string name);
    std::string outputName(int bits, int index);

    const std::map<int, std::map<int, std::set<std::pair<int, int>>>>& inputs(){ return _inputs; };
    const std::map<int, std::map<int, std::pair<int, int>>>& outputs(){ return _outputs; };
    const std::map<int, std::set<std::pair<int, int>>>& inputs(int bits);
    const std::map<int, std::pair<int, int>>& outputs(int bits);
    std::set<std::pair<int, int>> input(int bits, int index); // return <node-id, node-port-idx>
    std::pair<int, int> output(int bits, int index); // return set<node-id, node-port-idx>
    void addInput(int bits, int index, std::pair<int, int> node); // add input
    void addOutput(int bits, int index, std::pair<int, int> node); // add output
    void delInput(int bits, int index, std::pair<int, int> node);  // delete input
    void delInput(int bits, int index);  // delete input
    void delOutput(int bits, int index); // delete output

    const std::map<int, std::map<int, int>>& outputEdges(){ return _outputEdges; }
    const std::map<int, std::map<int, std::set<int>>>& inputEdges(){ return _inputEdges; }
    const std::map<int, int>& outputEdges(int bits);
    const std::map<int, std::set<int>>& inputEdges(int bits);
    int outputEdge(int bits, int index); // return edge id
    std::set<int> inputEdge(int bits, int index); // return set<edge-id>
    void addOutputEdge(int bits, int index, int edgeId);  // add output edge
    void addInputEdge(int bits, int index, int edgeId); // add input edge
    void delOutputEdge(int bits, int index); // delete output edge
    void delInputEdge(int bits, int index, int edgeId); // delete input edge
    void delInputEdge(int bits, int index); // delete input edge

    // const std::map<int, GraphNode*>& nodes(){ return _nodes; }
    // const std::map<int, GraphEdge*>& edges(){ return _edges; }
    // GraphNode* node(int id);
    // GraphEdge* edge(int id);
    // void addNode(GraphNode* node);
    // void addEdge(GraphEdge* edge);
    // void delNode(int id);
    // void delEdge(int id);


    // ====== operators >>>>>>>>>>
    // Graph copy
    // Graph& operator=(const Graph& that);

    void printGraph();

};




#endif