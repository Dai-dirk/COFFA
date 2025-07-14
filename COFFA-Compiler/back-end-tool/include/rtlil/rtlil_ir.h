#ifndef __RTLIL_ir_H__
#define __RTLIL_IR_H__

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include "dfg/dfg.h"

class Wire_IR{//for the wire of RTLIL, one wire only can connect to one node's output port

private:
    std::string _name;
    int _width;  // the width of the edge
    std::map<std::string, std::set<int>> _dst; // source node message, <src node's name, set< port_index >>
    std::pair<std::string, int> _src; //destination node message, < dst node's name, port_index>
    int _dstPortIdx; // destination node I/O port index
    int _dstId;   // destination node ID


public:
    Wire_IR(){}
    ~Wire_IR(){}
    std::string name(){ return _name; }
    void setName(std::string name){ _name = name; }
    void setWidth(int width){ _width  = width; }
    int Width(){ return _width; }
    void setSrc(std::string cellName, int portIndex);// set the source of the wire 
    void setDst(std::string cellName, int portIndex);// set the destination of the wire
    void delDst(std::string cellName);
    void delDst(std::string cellName, int ndex); // delete the "index" port of destination cell
    const std::pair<std::string, int>& getSrc(){ return _src; }
    const std::map<std::string, std::set<int>>& getDst(){ return _dst; }
};



class Cell_IR{

private:
    std::string _name;
    std::string _operation;
    int _bitWidth = 0;
    int64_t _value;
    std::map<int, int> _inPortWidth;// <port-index, width>
    std::map<int, int> _outPortWidth;// <port-index, width>
    std::string _LUT;
    int _LUT_size;
    std::map<int, std::string> _inWire; // <port-index, wire-name>
    std::map<int, std::string> _outWire; // <port-index, wire-name>
    std::map<std::string, std::string> _parameters;
    std::set<std::string> _dependencyCell; //@yuan: used by store nodes, record the input nodes that the store node dependent on <cells name>


public:
    Cell_IR(){}
    ~Cell_IR(){}
    std::string name(){ return _name; }
    void setName(std::string name){ _name = name; }
    std::string operation(){ return _operation; }
    void setOperation(std::string operation){ _operation = operation; }
    int bitWidth(){ return _bitWidth; }
    void setBitWidth(int bitWidth){ _bitWidth = bitWidth; }
    void setValue(int64_t value){ _value = value; }
    int64_t Value(){ return _value; }
    void setLUT(std::string LUT){ _LUT = LUT; }
    std::string LUT(){ return _LUT; }
    void setLUTsize(int size){ _LUT_size = size; }
    int LUTsize(){ return _LUT_size; }
    void setInportWidth(int index, int width);
    int getInportWidth(int index);
    void setOutportWidth(int index, int width);
    int getOutportWidth(int index);
    const std::map<int, std::string>& inputWires(){ return _inWire; }
    const std::map<int, std::string>& outputWires(){ return _outWire; }
    std::string inputWire(int index); // return wire name
    std::string outputWire(int index); // return wire name
    void addInputWire(int index, std::string wireName);  // add input wire
    void addOutputWire(int index, std::string wireName); // add output wire
    void delInputWire(int index);  // add input wire
    void delOutputWire(int index); // add output wire

    // set the parameters, such as offset, cycle, etc.
    void setParameters(std::string name, std::string value){_parameters[name] = value;}
    const std::map<std::string, std::string>& Parameters(){return _parameters;}
    // set the dependent nodes
    const std::set<std::string>& dependencyCells() { return _dependencyCell; }
    void addDependency(std::string name){ _dependencyCell.insert(name); }
    void delDependency(std::string name){ _dependencyCell.erase(name); }
    // does the node is iob node?
    bool isIOCell();

};




class RTLIL_IR{

private:
    DFG* _dfg;
    std::map<std::string, Cell_IR*> _cells;// using cell's name as index
    std::map<std::string, Wire_IR*> _wires;// using wire's name as index 

protected:
    // std::map<std::string, std::map<std::string, int>> _backedge; // <backedge name, <the dstnation cells name of the edge, the iteration distance>>
    std::map<std::string, std::map<std::string, std::vector<int>>> _backedge; // <backedge name, <the dstnation cells name of the edge, <the iteration distance, logic_latency, type ...>>>
    // std::map<std::string, std::map<std::string, int>> _backedge; // <backedge name, <the dstnation cells name of the edge, the iteration distance>>
    std::map<std::string, int> _iterDist; // <backedge name, iteration distance of this edge>

public:
    RTLIL_IR(std::string fileName, bool Viz);
    ~RTLIL_IR();
    void addCellIR(Cell_IR* cell_ir);
    void addWireIR(Wire_IR* wire_ir);
    void delCellIR(std::string name);
    void delWireIR(std::string name);
    Cell_IR* getCellIR(std::string name);
    Wire_IR* getWireIR(std::string name);
    const std::map<std::string, Cell_IR*>& cellsIR(){ return _cells; }
    const std::map<std::string, Wire_IR*>& wiresIR(){ return _wires; }
    void parseRtlil(std::string fileName); // parse the RTLIL file generated by Yosys
    DFG* Rtlil2DFG();// transform RTLIL to DFG
    void drawDFG(std::string fileName);// draw the DFG in dot format
    // create name for DFG node
    std::string getDfgNodeName(int id);

    DFG* getDFG(){ return _dfg; }

};



























#endif