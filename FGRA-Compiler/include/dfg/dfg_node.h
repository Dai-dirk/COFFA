#ifndef __DFG_NODE_H__
#define __DFG_NODE_H__

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <assert.h>
#include "graph/graph_node.h"
#include "op/operations.h"


//@yuan: one internal connection for packed node 
struct internalAttr
{
    std::string srcName; 
    int srcPort;
    // int edgeId;
    int width;
};


struct xcoreAttr
{
    //@yuan_xcore: the configuration of XCore
    std::map<int, uint32_t> k_in;//<segment index, K_value>
    std::map<int, std::map<int, uint32_t>> logc;// <segment index, <term index, logc>>
    bool float_flag = false;
    std::map<int, uint32_t> break_points; // <segment index, break_point>
    std::map<int, uint32_t> constant_bias; // <segment index, bias>
    uint32_t n = 0;
    uint32_t opc = 0;
    uint32_t bias_sel = 0;
};


class DFGNode : public GraphNode
{
private:
    std::string _operation;
    int _opLatency = 1; // operation latency
    bool _commutative; // if the inputs(operands) are commutative
    int _LUTsize = 0;
    std::string _LUTconfig;
    // int _bitWidth;
    bool _initSelection = false;
    // Accumulate operation varibales; e.g. for(i = 2; i < 10; i += 2)
    bool _accumulative = false;
    bool _isAccFirst;  // if the accumulative value is the first result; e.g. false
    uint64_t _initVal;  // initial value, 2
    int _cycles = 0;        // update number, (10-2)/2 = 2
    int _interval = 0;      // update interval, 1
    int _repeats = 0;       // repeat number
    // @yuan: the second initial value for double initial value accumulative operations
    int _2ndImmIdx = -1; // the second initial value operand index, -1: no second initial value
    uint64_t _2ndImm; // the second initial value
    //@hw_yuan: flag of MIMD-DFGNode
    bool _isPckedNode = false;
    //@hw_yuan: records the edges for each operation
    std::map<std::string, std::map<int, internalAttr>> _internalAttrMap; // <opName, <operand, attribute>>
    //@hw_yuan: the map of input edge to Eu's operand 
    // std::map<int, >
    std::vector<std::string> _multiOps;
    std::set<int> _immMultiIdx; // <input-index>
    //@hw_yuan: for aa_pack, indicating that this node is currenlty selected to pack
    bool _isCurrentSelected = false;
    //@yuan_hw: used to indicate how to select the output of multiplier
    int mulSel = 0;
    //@yuan_xcore: 
    xcoreAttr _xcoreAttr;
    //@yuan_argIN: 
    bool _constIsArgIn = false; //for the node's operation isn't argIn, this flag means the node's constant is input out of the kernel
    int _argInIdx = -1;

protected:
    // immedaite operand (not exceed 64 bits), <bit-width, <operand-index, value>>
    std::map<int, std::pair<int, int64_t>> _imm;
public:
    DFGNode(): _xcoreAttr(){}
    ~DFGNode(){}
    
    std::string operation(){ return _operation; }
    // set operation, latency, commutative according to operation name
    void setOperation(std::string operation);
    void setOpLatency(int opLat){ _opLatency = opLat; }
    int opLatency(){ return _opLatency; }
    bool commutative(){ return _commutative; }
    void setCommutative(bool cmu){ _commutative = cmu; }
    // set multiple operations after packing
    // void setMultiOps(std::set<std::string> ops){ _multiOps = ops; }

    bool hasImm(int bits){ return _imm.count(bits); }
    std::pair<int, int64_t> imm(int bits){ return _imm[bits]; }
    void setImm(int bits, std::pair<int, int64_t> imm){ _imm[bits] = imm; }
    int immIdx(int bits){ return _imm[bits].first; }
    void setImmIdx(int bits, int immIdx){ _imm[bits].first = immIdx; }
    int64_t immValue(int bits){ return _imm[bits].second; }
    void setImmValue(int bits, int64_t imm){ _imm[bits].second = imm; }
    void setImmIdxs(int idx){ _immMultiIdx.insert(idx); }
    const std::set<int>& immIdxs() {return _immMultiIdx; }
    void clearImmMultiIdxs(){_immMultiIdx.clear();}
    void clearImm(int bits);//@yuan_argIn

    //@yuan: for double initial value accumulative operations
    bool has2ndImm() { return _2ndImmIdx >= 0; }
    void set2ndImm(uint64_t init) { _2ndImm = init; }
    void set2ndImmIdx(int Idx) { _2ndImmIdx = Idx; }
    int get2ndImmIdx() {return _2ndImmIdx;} 
    uint64_t get2ndImm() { return _2ndImm; }


    virtual int numInputs(int bits);
    virtual int numOutputs(int bits);

    // set the parameters about lut node
    int LUTsize(){ return _LUTsize; }
    void setLUTsize(int LUTsize){ _LUTsize = LUTsize; }
    std::string LUTconfig(){ return _LUTconfig; }
    void setLUTconfig(std::string LUTconfig){ _LUTconfig = LUTconfig; }

    // set the parameters about accumulative and init-selection nodes
    bool initSelection() { return _initSelection; }
    void setInitSelection(bool isel){ _initSelection = isel; }
    bool accumulative(){ return _accumulative; }
    void setAccumulative(bool acc){ _accumulative = acc; }
    uint64_t initVal(){ return _initVal; }
    void setInitVal(uint64_t val){ _initVal = val; }
    void setCycles(int cycles){ _cycles = cycles; }
    int cycles(){ return _cycles; }
    void setInterval(int interval){ _interval = interval; }
    int interval(){ return _interval; }
    void setRepeats(int repeats){ _repeats = repeats; }
    int repeats(){ return _repeats; }
    bool isAccFirst(){ return _isAccFirst; }
    void setIsAccFirst(bool isFirst){ _isAccFirst = isFirst; }

    //@hw_yuan: for the node is generated after packing
    void setPacked(bool isPacked){ _isPckedNode = isPacked; }
    bool isPacked(){ return _isPckedNode;}
    void setMultiOps(std::vector<std::string> ops);
    const std::vector<std::string>& multiOps() {return _multiOps; }

    //@hw_yuan: the internal connection
    const std::map<std::string, std::map<int, internalAttr>>& internalAttrMap() { return _internalAttrMap; }
    const std::map<int, internalAttr>& internalAttrMap(std::string opName);
    void addInternalAttribute(std::string opName, std::string srcName, int srcPort, int operand,int width);

    //@yuan_xcore: access the configuration of XCore
    xcoreAttr& getXcoreAttr() { return _xcoreAttr; }
    const xcoreAttr& getXcoreAttr() const { return _xcoreAttr; }
    void setXcoreAttr(const xcoreAttr& attr) { _xcoreAttr = attr; }

    //@yuan_argIn: 
    void setIsArgIn(bool isArgIn) { _constIsArgIn = isArgIn; }
    bool isConstArgIn() { return _constIsArgIn; }
    void setArgIdx( int idx) { _argInIdx = idx; }
    int getArgIdx() { return _argInIdx; }

    virtual void print();
};


// DFG I/O node: INPUT/OUTPUT/LOAD/STORE/CLOAD/CSTORE
class DFGIONode : public DFGNode
{
private:
    std::string _memRefName; // referred memory name for binding, eg. int A[20]; array name is A
    int _memOffset = 0;          // access memory address offset to the array base address, e.g. access A[15]~A[4], offset is 4*4 
    int _reducedMemOffset = 0;   // access address offset to the reduced memory block, e.g. access A[15]~A[4], offset is (15-4)*4 
    int _memSize;                // referred memory size in byte, e.g. size is (15+1-4)*4
    bool _isOverSize = false;
    std::vector<std::pair<int, int>> _pattern; // memory access pattern, nested <stride, loop-cycles>
    std::vector<int> _groupNodes; // other I/O nodes in the same group where nodes access the same array
    bool _isMultiport = false;
    int _branch = 0; // @yuan: the branch that IOB belongs to, to reduce II
    std::set<int> _dependencyNode; //@yuan: used by store nodes, record the input nodes that the store node dependent on <cells id>
    int _bank=0; //@yuan: used for HW, the bank of current IOB
    int _multiportType = 0; // @yuan: the type of multiport for this DFGIONode, 0: no multiport, 1: multiports share 1 bank, 2: multiports share multiple banks without conflict, 3: multiports share multiple banks with conflict
    int _numMultiportBank = 1; // @yuan: how many bank used (i.e. N) by this multiport memory access
    int _Banksize = 1; //@yuan: the size of each bank (i.e. B) for this multiport memory access (the unit of this metric is the index of array)
    int _ControlStep = 0; //@yuan: the control step that this IOBNode belongs to
    int _precision = -1; //@yuan_fp: the precision for the IOB node
public:
    DFGIONode(){}
    // virtual ~DFGIONode(){}
    std::string memRefName();
    std::string realMemRefName(){ return _memRefName; };
    void setMemRefName(std::string name){ _memRefName = name; }
    int memOffset(){ return _memOffset; }
    void setReducedMemOffset(int offset){ _reducedMemOffset = offset; }
    int reducedMemOffset(){ return _reducedMemOffset; }
    void setMemOffset(int offset){ _memOffset = offset; }
    int memSize(){ return _memSize; }
    void setMemSize(int size){ _memSize = size; }
    bool isMultiport(){ return _isMultiport; }
    void setMultiport(bool flag){ _isMultiport = flag; }
    const std::vector<std::pair<int, int>>& pattern(){ return _pattern; }
    int getNestedLevels(){ return _pattern.size(); }
    void addPatternLevel(int stride, int cycles){ _pattern.push_back(std::make_pair(stride, cycles)); }
    // set the dependent nodes
    const std::set<int>& dependencyNodes() { return _dependencyNode; }
    void addDependency(int id){ _dependencyNode.insert(id); }
    void delDependency(int id){ _dependencyNode.erase(id); }

    void addGroupNode(int id){ _groupNodes.push_back(id); }
    const std::vector<int>& groupNodes(){ return _groupNodes; }

    //set the branch
    void setBranch(int branch){ _branch = branch;}
    int Branch(){return _branch;}

    //@yuan: set multiport type
    void setMultiportType (int type) { _multiportType = type;}
    int MultiportType () { return _multiportType;}

    //@yuan: set the number of used multiport banks
    void setNumMultiportBank (int N) { _numMultiportBank = N;} 
    int NumMultiportBank () {return _numMultiportBank;}

    //@yuan: set the size of each multiport bank
    void setMultiportBankSize(int B) { _Banksize = B;}
    int MultiportBankSize() { return _Banksize; }

    //@yuan: set the control step
    void setControlStep (int mode) { _ControlStep = mode; }
    int ControlStep() { return _ControlStep; }

    //@yuan: set the oversize flag
    void setOversie(bool isOversize) { _isOverSize = isOversize; }

    //@yuan_fp: set the precision
    void setPrecision(int precision) { _precision = precision; }
    int getPrecision() { return _precision; }
    bool isSmallPrecision();

    virtual void print();

    //@yuan: for HW
    void setbank(int bank) {_bank = bank;}
    int bank() {return _bank;}
};


#endif