#ifndef __ADG_NODE_H__
#define __ADG_NODE_H__

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <assert.h>
#include "graph/graph_node.h"


// type of functionnal unit in GPE and IOB
#define TYPE_ALU  0
#define TYPE_LUT  1
#define TYPE_IOC  2  // IO Controller


// configuration data location
struct CfgDataLoc{
    int low;  // lowest bit index 
    int high; // highest bit index
};
// configuration data location
struct EuCfgDataLoc{
    int imuxFGCfgWIdth;  // the configuration width of fine-grained input width
    int imuxCGCfgWIdth;  // the configuration width of Coarse-grained input width
    int CoreCfgWIdth;  // the configuration width of operator_core width
};


class ADG;

// support mixed-grained
class ADGNode : public GraphNode
{
private:
    int _cfgBlkIdx;
    int _x;
    int _y;
    // int _cfgBitLen;
    std::vector<uint64_t> _cfgBits;
    // configuration info, <ID, CfgDataFormat>
    std::map<int, CfgDataLoc> _configInfo;
    // sub-ADG consisting of sub-modules
    ADG* _subADG = nullptr; 
    // @yuan: test for historic usage <dfg node id, usagge>
    std::map<int, int> _historyUsage; 
protected:
    void printADGNode();
public:
    ADGNode(){}
    ADGNode(int id){ _id = id; }
    ~ADGNode(){} // cannot delete _subADG here because of pre-declare class; 

    int cfgBlkIdx(){ return _cfgBlkIdx; }
    int x(){ return _x; }
    int y(){ return _y; }
    // int cfgBitLen(){ return _cfgBitLen; }
    void setCfgBlkIdx(int cfgBlkIdx){ _cfgBlkIdx = cfgBlkIdx; }
    void setX(int x){ _x = x; }
    void setY(int y){ _y = y; }

    // replace bit-width, including _bitWidths, _numInputs, _numOutputs, _inputs, _outputs
    // void replaceBitWidth(int ori_bits, int new_bits);

    // void setCfgBitLen(int cfgBitLen){ _cfgBitLen = cfgBitLen; }
    ADG* subADG(){ return _subADG; }
    void setSubADG(ADG* subADG){ _subADG = subADG; }
    // configuration info, <ID, CfgDataFormat>
    const std::map<int, CfgDataLoc>& configInfo(){ return _configInfo;}
    // get config info for sub-module
    const CfgDataLoc& configInfo(int id);
    // add config info for sub-module
    void addConfigInfo(int id, CfgDataLoc subModuleCfg);
    // virtual void verify();
    // // analyze the connections among the internal sub-modules 
    // virtual void analyzeIntraConnect(){}
    virtual void print();
    //@yuan: historical usage
    void addUsage(int dfgNodeId);
    int  getUsage(int dfgNodeId);
    void clearUsage();

};


// Funtional Unit node, parent class of GPENode and IOBNode
class FUNode : public ADGNode
{
protected:
    std::set<std::string> _operations; // supported operations
    // max delay cycles of the internal shared DelayPipe, <bit-width, max-delay-cycles> 
    std::map<int, int> _maxDelay;
    // number of operands, <bit-width, operand-number>
    std::map<int, int> _numOperands; 
    // indexes of input ports connected to each operand, <bit-width, vector<set<input-port>>>
    std::map<int, std::vector<std::set<int>>> _operandInputs; 
    void printFU();
public:
    using ADGNode::ADGNode; // C++11, inherit parent constructors
    std::map<std::string, int> cfgIdMap; // config item IDs, e.g. BaseAddr, Latency, IsStore
    const std::set<std::string>& operations(){ return _operations; }
    void addOperation(std::string op); // add supported operation
    void delOperation(std::string op); // delete supported operation
    bool opCapable(std::string op); // check if the operation is supported

    int maxDelay(int bits);
    void setMaxDelay(int bits, int maxDelay);
    int numOperands(int bits); 
    void setNumOperands(int bits, int numOperands); // set numOperandCG and resize _operandInputsCG
    const std::set<int>& operandInputs(int bits, int opeIdx); // get input ports connected to this operand
    void addOperandInput(int bits, int opeIdx, int inputIdx); // add input port connected to this operand
    void delOperandInput(int bits, int opeIdx, int inputIdx);
    void setOperandInputs(int bits, int opeIdx, std::set<int> inputIdxs); // set input ports connected to this operand
    int getOperandIdx(int bits, int inputIdx); // get which operand this input is connected
    virtual void print();
};


class GPENode : public FUNode
{
private:
    int _numInputLUT; // LUT input number
    int _numRfRegALU; // number of registers in RegFile for ALU
    int _numRfRegLUT; // number of registers in RegFile for LUT
    bool _hasLUT;
    int _numFgOutput;
    bool _isMIMD = false;
    // indexes of input ports connected to each DMR control signal, <bit-width, vector<set<input-port>>>
    std::map<int, std::map<int, std::set<int>>> _ConDMRInputs; 
    //@hw_yuan: record the EUs have coarse-grained output (and the ordered Id of EU)
    std::map<std::string, int> _CGEusIdx;
    //@hw_yuan: record the EUs have fine-grained output (and the ordered Id of EU)
    std::map<std::string, int> _FGEusIdx;
    // configuration info, <Eu's name, CfgDataFormat>
    std::map<std::string, EuCfgDataLoc> _euConfigInfo;
public:
    using FUNode::FUNode; // C++11, inherit parent constructors
    void sethasLUT(bool hasLut){ _hasLUT = hasLut;}
    bool hasLUT(){ return _hasLUT;}
    int numInputLUT(){ return _numInputLUT; }
    void setNumInputLUT(int num){ _numInputLUT = num; }
    int numRfRegALU(){ return _numRfRegALU; } 
    void setNumRfRegALU(int numRfReg){ _numRfRegALU = numRfReg; }
    int numRfRegLUT(){ return _numRfRegLUT; } 
    void setNumRfRegLUT(int numRfReg){ _numRfRegLUT = numRfReg; }
    // fine-grained operand order: ALU fg-input, LUT input
    // operand 0 is ALU fg-input if there is one;
    // get the operand index in PE according to the operand index in LUT
    int getOperandIdxLUT(int idxLUT);
    // @yuan: get the operand index in PE for fine-grained signal excepted LUT
    int getOperandIdxFg(int numCghOp, int idxFg);
    virtual void print();
    //@yuan: for DMR control signal
    // void setNumConACCINput(int bits, int numConACCInputs); // resize _ConACCInputs
    const std::set<int>& ConDMRInputs(int bits, int opeIdx); // get input ports connected to this DMR control signal
    void addConDMRInput(int bits, int opeIdx, int inputIdx); // add input port connected to this DMR control signal
    void delConDMRInput(int bits, int opeIdx, int inputIdx);
    void setConDMRInput(int bits, int opeIdx, std::set<int> inputIdxs); // set input ports connected to this DMR control signal
    int getConDMRIdx(int bits, int inputIdx); // get which DMR control signal this input is connected
    //@hw_yuan: indicating whether the GPE support MIMD
    void setMIMD(bool isMIMD){ _isMIMD = isMIMD; }
    bool isMIMD(){ return _isMIMD; }

    void addCGEusId(std::string Eu, int idx);
    const std::map<std::string, int>& CGEusId() { return _CGEusIdx;}
    int CGEusId(std::string Eu);
    void addFGEusId(std::string Eu, int idx);
    const std::map<std::string, int>& FGEusId() { return _FGEusIdx;}
    int FGEusId(std::string Eu);
    // configuration info, <Eu's name, CfgDataFormat>
    const std::map<std::string, EuCfgDataLoc>& euConfigInfo(){ return _euConfigInfo;}
    // get config info for sub-module
    const EuCfgDataLoc& euConfigInfo(std::string name);
    // add config info for sub-module
    void addEuConfigInfo(std::string name, EuCfgDataLoc euCfg);
};

class IOBNode : public FUNode
{
private:
    int _index; // index in all the IOBs
    int _mode;
public:
    using FUNode::FUNode; // C++11, inherit parent constructors
    //std::map<std::string, int> iocCfgIdMap; // IO Controller config item IDs, e.g. BaseAddr, Latency, IsStore

    int index(){ return _index; }
    void setIndex(int index){ _index = index; }
    int getOperandIdxIOB(int idxIOB);
    virtual void print();
};


class GIBNode : public ADGNode
{
private:
    bool _trackReged; // if there are registers in the track output ports
    std::map<int, bool> _outReged; // if there are registers in the output port 
    // inputs connected to each output, <output-index, set<input-index>>
    std::map<int, std::set<int>> _out2ins;
    // outputs connected to each input, <input-index, set<output-index>>
    std::map<int, std::set<int>> _in2outs; 
public:
    using ADGNode::ADGNode; // C++11, inherit parent constructors
    bool trackReged(){ return _trackReged; }
    void setTrackReged(bool trackReged){ _trackReged = trackReged; }
    bool outReged(int idx);
    void setOutReged(int idx, bool reged);
    // inputs connected to each output
    std::set<int> out2ins(int outPort);
    // outputs connected to each input
    std::set<int> in2outs(int inPort);
    // add input connected to the output
    void addOut2ins(int outPort, int inPort);
    // add output connected to the input
    void addIn2outs(int inPort, int outPort);
    // check if the input and output port are connected
    bool isInOutConnected(int inPort, int outPort); 
    virtual void print();
};




// // Load/Store Unit
// class LSUNode : public ADGNode
// {
// private:
//     int _maxDelay; // max delay cycles of the internal DelayPipe 
//     std::vector<std::set<int>> _operandInputs; // indexes of input ports connected to each operand
// public:
//     using ADGNode::ADGNode; // C++11, inherit parent constructors
//     int maxDelay(){ return _maxDelay; }
//     void setMaxDelay(int maxDelay){ _maxDelay = maxDelay; }
//     const std::set<int>& operandInputs(int opeIdx); // get input ports connected to this operand
//     void addOperandInputs(int opeIdx, int inputIdx); // add input port connected to this operand
//     void delOperandInputs(int opeIdx, int inputIdx);
//     void addOperandInputs(int opeIdx, std::set<int> inputIdxs); // add input ports connected to this operand
//     int getOperandIdx(int inputIdx); // get which operand this input is connected
//     virtual void print();
// };




#endif