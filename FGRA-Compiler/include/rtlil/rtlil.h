#ifndef __RTLIL_H__
#define __RTLIL_H__

#include <iostream>
#include <fstream>
#include <algorithm>
#include "dfg/dfg.h"

class Wire
{
private:
    int _id;
    int _srcPortIdx; // source node I/O port index
    int _dstPortIdx; // destination node I/O port index
    int _srcId;   // source node ID
    int _dstId;   // destination node ID
    int _width;  // the width of the edge
    bool _isBackEdge = false;
    int _logicLat = 0; // due to multport add a logic lat //mulrport   
    int _iterDist = 0; // iteration distance for loop-carried dependence   
    int _edgeType = 0; // @yuan: the type of edge, default is data dependency edge
public:
    Wire(){}
    Wire(int edgeId){ _id = edgeId; }
    Wire(int srcId, int dstId) : _srcId(srcId), _dstId(dstId) {}
    ~Wire(){}
    int id(){ return _id; }
    void setId(int id){ _id = id; }
    int srcPortIdx(){ return _srcPortIdx; }
    void setSrcPortIdx(int srcPortIdx){ _srcPortIdx = srcPortIdx; }
    int dstPortIdx(){ return _dstPortIdx; }
    void setDstPortIdx(int dstPortIdx){ _dstPortIdx = dstPortIdx; }
    int srcId(){ return _srcId; }
    void setSrcId(int srcId){ _srcId = srcId; }
    int dstId(){ return _dstId; }
    void setDstId(int dstId){ _dstId = dstId; }
    bool isBackEdge(){ return _isBackEdge; }
    void setBackEdge(bool back){ _isBackEdge = back; }
    void setlogicLat(int logicLat){ _logicLat = logicLat; }
    int logicLat(){ return _logicLat; }
    // void setBackCtrlEdge(bool ctrlback){ _backCtrledge = ctrlback; }
    void setIterDist(int dist){ _iterDist = dist; }
    int iterDist(){ return _iterDist; }
    void setType(int type){ _edgeType = type; }
    int Type(){ return _edgeType; }
    void setWire(int srcId, int dstId){
        _srcId = srcId;
        _dstId = dstId;
    }
    void setWire(int srcId, int srcPort, int dstId, int dstPort){
        _srcId = srcId;
        _srcPortIdx = srcPort;
        _dstId = dstId;
        _dstPortIdx = dstPort;
    }
    void setWidth(int width){ _width  = width; }
    int Width(){ return _width; }
};

class Cell
{
private:
    int _id;
    std::string _name;
    std::string _operation;
    std::set<int> _bitWidths;
    int64_t _value;
    std::map<int, int> _inputWires; // <input-index, edge-id>
    std::map<int, std::set<int>> _outputWires; // <output-index, set<edge-id>>
    std::map<std::string, std::string> _parameters;
    std::set<std::string> _dependencyCell; //@yuan: used by store nodes, record the input nodes that the store node dependent on <cells name>
public:
    Cell(){}
    ~Cell(){}
    int id(){ return _id; }
    void setId(int id){ _id = id; }
    std::string name(){ return _name; }
    void setName(std::string name){ _name = name; }
    std::string operation(){ return _operation; }
    // set operation, latency, commutative according to operation name
    void setOperation(std::string operation){ _operation = operation; }
    // set the width of the cell
    const std::set<int>& bitWidths(){ return _bitWidths; }
    void setBitWidths(std::set<int> bits){ _bitWidths = bits; }
    void addBitWidth(int bitWidth){ _bitWidths.insert(bitWidth); }
    void delBitWidth(int bitWidth){ _bitWidths.erase(bitWidth); }

    // set the dependent nodes
    const std::set<std::string>& dependencyCells() { return _dependencyCell; }
    void addDependency(std::string name){ _dependencyCell.insert(name); }
    void delDependency(std::string name){ _dependencyCell.erase(name); }

    void setValue(int64_t value){ _value = value; }
    int64_t Value(){ return _value; }
    const std::map<int, int>& inputWires(){ return _inputWires; }
    const std::map<int, std::set<int>>& outputWires(){ return _outputWires; }
    int inputWire(int index); // return edge id
    std::set<int> outputWire(int index); // return set<edge-id>
    void addInputWire(int index, int wireId);  // add input edge
    void addOutputWire(int index, int wireId); // add output edge
    void delInputWire(int index); // delete input edge
    void delOutputWire(int index, int wireId); // delete output edge

    // set the parameters, such as offset, cycle, etc.
    void setParameters(std::string name, std::string value){_parameters[name] = value;};
    const std::map<std::string, std::string>& Parameters(){return _parameters;}
    // does the node is iob node?
    bool isIOCell();
};

class RTLIL 
{
 private:
    DFG* _dfg;
    std::string _filename;
    std::string _dirname; // file directory name 
    std::string _design_name; // design name 
    std::map<int, int> _inputWire; // the wire that connects to input nodes, <index, width>
    std::map<int, int> _outputWire; // the wire that output nodes connect to, <index, width>
    std::map<int, Cell*> _cells;   // <node-id, node>
    std::map<int, Wire*> _wires;   // <edge-id, edge>


 public:
    RTLIL(std::string filename, std::string dirname, std::string design_name): _filename(filename), _dirname(dirname), _design_name(design_name){}
    RTLIL(){}
    ~RTLIL(){}
    void dumpRtlil(std::string dirname, std::string design_name);//generate RTLIL file 
    void setInWire(int index, int width);
    void delInWire(int index);
    const std::map<int, int>& inWires(){ return _inputWire; }
    void setOutWire(int index, int width);
    void delOutWire(int index);
    const std::map<int, int>& outWires(){ return _outputWire; }
    void addCell(Cell* cell);
    void addWire(Wire* wire);
    void delCell(int id);
    void delWire(int id);
    Cell* getCell(int id);
    Wire* getWire(int id);
    const std::map<int, Cell*>& cells(){ return _cells; }
    const std::map<int, Wire*>& wires(){ return _wires; }
    void runYosys(bool YosysQuiet, std::string dirname, bool Viz, int lut);//running Yosys
};


#endif