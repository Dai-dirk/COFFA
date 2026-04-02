#ifndef __LLVM_CDFG_H__
#define __LLVM_CDFG_H__

// #define DUMP_TMP_CDFG
#define BITWIDTH 8
// #define ALL_COARSE
// #define MEM_PARTITION
// #define INPUT_DUOLICATION

#define MAX_CYCLE 4096
#define MAX_SIZE 8192

//for memory conflict partition
#define BMAX 4096
#define NMAX 8


#include "llvm/Analysis/ScalarEvolution.h"
//#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <queue>
#include <iostream>
#include <fstream>
#include <variant>
#include <Python.h>

#include "llvm_cdfg_node.h"
#include "llvm_cdfg_edge.h"

#include "py_path.h"

using namespace llvm;

class LLVMCDFG
{
private:
    std::string _name;
    std::map<int, LLVMCDFGNode*> _nodes; // <node-id, node>
    std::map<int, LLVMCDFGEdge*> _edges; // <edge-id, edge>
    std::map<Instruction*, LLVMCDFGNode*> _insNodeMap; // <instruction, node>
    // function annotation: size attribute
    std::map<std::string, int> _sizeAttrMap;
    // basic block pairs that there is a back edge in between, <srcBB, dstBB>
    SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 1> _backEdgeBBPs;    
    // the recursively successive basic blocks except the back-edge BBs    
    std::map<const BasicBlock *, std::set<const BasicBlock *>> _succBBsMap;
    //record each BB is dependent by which CTRLNODE
    std::map<BasicBlock*, std::set<std::pair<LLVMCDFGNode*, CondVal>>> _BBctrlDependentNode;
    std::map<int, Loop*> _nestloops;
    std::map<int, std::set<BasicBlock* >> _LoopexclusiveBBs;
    bool _nestLoopisAffine;
    std::map<int, int> _loopsAffineStrides;
    std::map<int, std::pair<varType, varType>> _loopsAffineBounds;
    std::map<int, varType> _loopsAffineCounts;
	std::set<BasicBlock*> _loopBBs;
    //basicblocks including loopBBs and task entry & exit
	std::set<BasicBlock*> _allBBs;
	std::set<std::pair<BasicBlock*,BasicBlock*>> _loopentryBBs;
	std::set<std::pair<BasicBlock*,BasicBlock*>> _loopexitBBs;
    //the BB where the loop_begin/loop_end exist(may be not included in _loopentryBBs when loopsafetycheck)
    std::vector<BasicBlock*> _taskIOBBs;
    // Out of loop control and data node
    // control node : LOOPSTART, LOOPEXIT
    // data node : INPUT, OUTPUT
    std::map<BasicBlock*, LLVMCDFGNode*> _loopStartNodeMap;
    std::map<BasicBlock*, LLVMCDFGNode*> _BB2ExitNodeMap;
    std::map<int, LLVMCDFGNode*> _loop2exitNodesMap;
    std::map<Value*, LLVMCDFGNode*> _ioNodeMap;
	// std::map<LLVMCDFGNode*, Value*> _ioNodeMapReverse;
    // GEP node info map including pointer name
    // GEP base address should be provided by the sceduler
    std::map<LLVMCDFGNode*, std::string> _GEPInfoMap;
    // Input/output node info map, inlcudidng variable name
    std::map<LLVMCDFGNode*, std::string> _IOInfoMap;
    //function input arguements
    std::map<int, Value*> _funcArgs;
    //loop level to idx safety map
    std::map<int, std::vector<std::pair<BasicBlock*, CondVal>>> _safetyMap;
    // std::map<int, std::pair<CmpInst*, CondVal>> _safetyMap;
    // //safety nodes vector
    // std::vector<LLVMCDFGNode*> _safetyNodes;

    //record how many times CDFG is printed
    int printTimes = 0;

    //loop bound value
    std::map<int, std::pair<Value*, Value*>> _loopboundMap;

    //Load Store table
    std::set<LLVMCDFGNode*> _loadList;
    std::set<LLVMCDFGNode*> _storeList;
    std::map<std::string, std::pair<std::set<LLVMCDFGNode *>, std::set<LLVMCDFGNode *>>> _arrayName2LSNode; //irregular access: first-load second-store

    //access conflict
    std::set<LLVMCDFGEdge*> _conflictEdges;

    //instructions in EntryBB, but before loop_begin() & in ExitBB, but after loop_end()
    std::set<Instruction*> _ExInsList;

    //to unify the Size & Offset for NoC of Multi-Task
    std::map<Value*, int> _maxSizeMap;
    std::map<Value*, int> _minOffsetMap;

    void bypassBitcastNode(LLVMCDFGNode *node);

    #ifdef MEM_PARTITION
    //record arrayName the memory partition factors <B, N>
    std::map<std::string, std::pair<unsigned, unsigned>> _arrayName2BN;
    #endif

public:
    ScalarEvolution *SE;
    DominatorTree *DT;
    PostDominatorTree *PDT;
    DependenceInfo *DI;
    const DataLayout *DL;
    bool _noACC = false;
    bool _noPattern = false;
    bool _noMAC = true;
    bool _suppRem = false;
    bool _loopSafety = false;

///TODO:somtimes may have more than one PHINode in a level
    std::map<llvm::PHINode *, int> PHItoLevel;

    LLVMCDFG(std::string name) : _name(name){}
    ~LLVMCDFG();

	void setName(std::string str){_name = str;}
	std::string name(){return _name;}

	const std::map<int, LLVMCDFGNode*>& nodes(){ return _nodes; }
    LLVMCDFGNode* node(int id);
    LLVMCDFGNode* node(Instruction *ins);
    void addNode(LLVMCDFGNode *node);
    LLVMCDFGNode* addNode(Instruction *ins); // create node according to instruction and add node
    LLVMCDFGNode* addNode(std::string customIns, BasicBlock *BB); // create node according to the custom instruction and add node
    LLVMCDFGNode* addNode(std::string customIns); // create node according to the custom instruction for pattern which is not with BB: TODO ?
    LLVMCDFGNode* addCpNode(Instruction *ins);  //for multi-store of subtasks
    LLVMCDFGNode* addCpNode(LLVMCDFGNode *node);  //for multi-store of subtasks
    // delete node and corresponding edges
    void delNode(LLVMCDFGNode *node);
    void delNode(Instruction *ins); 

    // delete node tree(node & all prenodes)
    void delNodeTree(LLVMCDFGNode *node);
    // delete node tree(node & all prenodes)
    void delNodeTree(Instruction *ins);
    // add node tree according to Instruction(node & all prenodes)
    LLVMCDFGNode* addNodeTree(Value* opnode);

    void connectNodes(LLVMCDFGNode *src, LLVMCDFGNode *dst, int idx, EdgeType edgeType, bool isBackEdge = false); 

    std::set<std::pair<LLVMCDFGNode*, int>> addOutputTree(Value* opnode);

    ///handle phi of outer nest loops
    LLVMCDFGNode* addIdxCycle(PHINode* Phi);

	const std::map<int, LLVMCDFGEdge*>& edges(){ return _edges; }
    LLVMCDFGEdge* edge(int id);
    LLVMCDFGEdge* edge(LLVMCDFGNode *src, LLVMCDFGNode *dst);
    // add edge and corresponding src->outEdge and dst->inEdge
	void addEdge(LLVMCDFGEdge *edge);
    LLVMCDFGEdge* addEdge(LLVMCDFGNode *src, LLVMCDFGNode *dst, EdgeType type);
    // delete edge and corresponding src->outEdge and dst->inEdge
    void delEdge(LLVMCDFGEdge *edge);
    void delEdge(int eid);

    // Loop info
    void setLoops(std::map<int, Loop*> nestloops){ _nestloops = nestloops;}
    std::map<int, Loop*> nestloops(){return _nestloops;}
	void setLoopBBs(std::set<BasicBlock*> BBs){_loopBBs = BBs;}
	void setLoopBBs(std::set<BasicBlock*> BBs, std::set<std::pair<BasicBlock*,BasicBlock*>> entryBBs, std::set<std::pair<BasicBlock*,BasicBlock*>> exitBBs){
        _loopBBs = BBs; 
        _loopentryBBs = entryBBs; 
        _loopexitBBs = exitBBs;
    }
    void setLoopBBs(std::set<BasicBlock*> BBs, std::set<std::pair<BasicBlock*,BasicBlock*>> entryBBs, std::set<std::pair<BasicBlock*,BasicBlock*>> exitBBs, std::vector<BasicBlock*> TaskIOBBs){
        _loopBBs = BBs; 
        _loopentryBBs = entryBBs; 
        _loopexitBBs = exitBBs;
        _taskIOBBs = TaskIOBBs;
    }
	const std::set<BasicBlock*>& loopBBs(){return _loopBBs;}

    // Basic block relation
    void setSuccBBMap(std::map<const BasicBlock*,std::set<const BasicBlock*>> succBBsMap){_succBBsMap = succBBsMap;}
	const std::map<const BasicBlock*,std::set<const BasicBlock*>>& succBBMap(){return _succBBsMap;}

    const SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 1>& backEdgeBBPs(){ return _backEdgeBBPs; } 
    void setBackEdgeBBPs(SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 1> backEdgeBBPs){ _backEdgeBBPs = backEdgeBBPs; }
	
    // GEP node info map including pointer name
    const std::map<LLVMCDFGNode*, std::string>& GEPInfoMap(){ return _GEPInfoMap; }
    std::string GEPInfo(LLVMCDFGNode* node);
    void setGEPInfo(LLVMCDFGNode* node, std::string name);

    // Input/output node info map, inlcudidng variable name
    const std::map<LLVMCDFGNode*, std::string>& IOInfoMap(){ return _IOInfoMap; }
    std::string IOInfo(LLVMCDFGNode* node);
    void setIOInfo(LLVMCDFGNode* node, std::string name);

    //set function input args
    void setFuncArgs(std::map<int, Value*> argsMap){ _funcArgs = argsMap;}

    void setExInsList(std::vector<Instruction* > outMarkIns){_ExInsList.insert(outMarkIns.begin(), outMarkIns.end());}

    //to unify the Size & Offset for NoC of Multi-Task
    void setmaxSizeMap(std::map<Value*, int> maxSizeMap){_maxSizeMap = maxSizeMap;}
    std::map<Value*, int> getmaxSizeMap(){return _maxSizeMap;}
    void setminOffsetMap(std::map<Value*, int> minOffsetMap){_minOffsetMap = minOffsetMap;}
    std::map<Value*, int> getminOffsetMap(){return _minOffsetMap;}

    // initialize CDFG according to loopBBs
    void initialize();
    // analyze loop index
    void LoopIdxAnalyze();
    ///find safety branch of variable Idx and modify dependent node
    void FindLoopIdxSafetyBr(int level, Value* bound);

    //accessAnalyze & transform GEP to affine Input
    void accessAnalyze();
    // Set Nestloop Affine Strides
    void setLoopsAffineStrides(int level, int stride){ _loopsAffineStrides[level] = stride;}
    void setLoopsAffineStrides(std::map<int, int> AffineStrides){_loopsAffineStrides = AffineStrides;}
    bool isloopsAffine(){return _nestLoopisAffine;}
    std::map<int, int> getLoopsAffineStrides(){return _loopsAffineStrides;}
    int getLoopsAffineStrides(int level){return _loopsAffineStrides[level];}
    void setLoopsAffineBounds(int level, int leftbound, int rightbound){_loopsAffineBounds[level].first = leftbound;
                                                                        _loopsAffineBounds[level].second = rightbound;}
     
    void setLoopsAffineBounds(int level, varType leftbound, varType rightbound){_loopsAffineBounds[level].first = leftbound;
                                                                        _loopsAffineBounds[level].second = rightbound;}
    std::map<int, std::pair<varType, varType>> getLoopsAffineBounds(){return _loopsAffineBounds;}
    std::pair<varType, varType> getLoopsAffineBounds(int level){return _loopsAffineBounds[level];}
    void setLoopsAffineCounts(int level, varType count){ _loopsAffineCounts[level] = count;}
    void setLoopsAffineCounts(std::map<int, varType> AffineCounts){ _loopsAffineCounts = AffineCounts;}
    std::map<int, varType> getLoopsAffineCounts(){return _loopsAffineCounts;}
    varType getLoopsAffineCounts(int level){return _loopsAffineCounts[level];}

    std::set<LLVMCDFGNode*> getLoadList(){return _loadList;}
    std::set<LLVMCDFGNode*> getStoreList(){return _storeList;}
    
    // add edge between two nodes that have memory dependence (loop-carried)
    void addMemDepEdges();
    // prune stores that are overwritten in-iteration and not observed
    void pruneOverwrittenStores();

    // explore the memory partition method
    void memPartition();

#ifdef MEM_PARTITION
    //add memory partition factor
    void addMemPartition(std::string arrayName, unsigned B, unsigned N){_arrayName2BN[arrayName] = std::make_pair(B, N);}
    /// add memory partition factor
    /// @param first : B
    /// @param second : N
    std::pair<unsigned, unsigned> getMemPartition(std::string arrayName){return _arrayName2BN[arrayName];}
#endif

    // root: output node, no children
	std::vector<LLVMCDFGNode*> getRoots();
    // leaf: node with no parents
    std::vector<LLVMCDFGNode*> getLeafs();
	std::vector<LLVMCDFGNode*> getLeafs(BasicBlock* BB);

    
    // get a map of basicblocks to their control dependent (recursive) predecessors with the respective control value
    std::map<BasicBlock*, std::set<std::pair<BasicBlock*, CondVal>>> getCtrlDepPredBBs();
    // get the control dependent nodes in a BB, including unconditional BranchInst, StoreInst, OutLoopSTORE
    // only TRUE_COND, STORE can be performed
    std::vector<LLVMCDFGNode*> getCtrlDepNodes(BasicBlock *BB);
    // add CTRLAND each CTRL path's node to tem preBB
    std::set<std::pair<LLVMCDFGNode *, CondVal>> ANDctrlPathNodes(std::map<BasicBlock*, std::set<std::pair<BasicBlock*, CondVal>>> &CtrlDepPredBBs, BasicBlock* BB);
    // Connect control dependent node pairs among BBs
    // Conditional Branch predecessor -> control dependent nodes
	void connectCtrlDepBBs();

    // insert Control NOT node behind the condition node of the Branch node
    void insertCtrlNotNodes();

    // transfer the multiple control predecessors (input nodes) into a inverted OR tree 
    // with the root connected to a node and leaves connected to control predecessors
    void createCtrlOrTree();

    // get loop start node. If not exist, create one.
    LLVMCDFGNode* getLoopStartNode(BasicBlock *BB);
    // get loop exit node. If not exist, create one.
    LLVMCDFGNode* getLoopExitNode(BasicBlock *BB);
    // get input node. If not exist, create one.
    LLVMCDFGNode* getInputNode(Value *ins, BasicBlock *BB);
    // get output node. If not exist, create one.
    LLVMCDFGNode* getOutputNode(Value *ins, BasicBlock *BB);

    ///recursively find array assess stride & bound
    std::vector<int> arrayStride(LLVMCDFGNode* opnode, std::map<int, std::pair<varType,std::pair<varType,varType>>>* factortable);
    std::vector<PHINode*> arrayStride(Value* opnode, std::map<int, std::pair<double,std::pair<double,double>>>* factortable);
    
	// Transfer PHI nodes to SELECT nodes
	void handlePHINodes();

    // add mask AND node behind the Shl node with bytewidth less than MAX_DATA_BYTE
    void addMaskAndNodes();

    // Lower llvm.fmuladd.* calls into FMUL + FADD nodes in the CDFG
    void lowerFMulAddCalls();
    // Fold ICMP+SELECT patterns back to Min/Max custom nodes
    void foldCmpSelectToMinMax();
    // Lower llvm.[su]max/[su]min intrinsics into ICMP + SELECT
    void lowerMinMaxCalls();

        // handle conversion operations including trunc, zext, sext
    void handleConversionNodes();

    // fuse constant input node with the mul output node for acc node 
    void fuseAccConstNode();
    // fold (0 - x) feeding const mul/div by negating the constant
    void fuseZeroSubConstOps();

    // simplify the control logic, AND/OR node connected to more than one NOT nodes
    void simplifyCtrlLogic();

    // get the offset of each element in the struct
    std::map<int, int> getStructElemOffsetMap(StructType *ST);

    // transfer GetElementPtr(GEP) node to MUL/ADD/Const tree
    void handleGEPNodes();

    // add loop exit nodes
    std::map<int, LLVMCDFGNode*> addLoopExitNodes();

    // add task level control
    void addTaskCTRL();

    // remove redundant nodes, e.g. Branch
    void removeRedundantNodes();

    // assign final node name
    void assignFinalNodeName();

	// print DOT file of CDFG
    void printDOT(std::string fileName);

    // enforce print DOT file of CDFG
    void enforceprintDOT(std::string fileName);

	// print DOT file of affine DFG
    void printAffineDOT(std::string fileName);

    //print subgraph as a cluster in a WHOLE DFG    
    void printAsSubTask(std::ofstream &ofs, int kernel);

    // print dot with Hirerarchy (loop level)
    void printHierarchyDOT(std::string fileName);

    // generate CDFG
    void generateCDFG();

    // remove affine address calculation & generate INPUT/OUTPUT pattern
    void handleAffineLSNodes();

    // transform add result SELECT tree to add operand SELECT tree
    void transAddOpSelectTree();
    // generate ACC node
    void generateACC(std::set<LLVMCDFGNode *> &InitialSels);
    // generate CACC node
    void generateCACC(std::set<LLVMCDFGNode *> &InitialSels);
    // handle SELECT nodes
    void handleSelectNodes();

    // generate MAC nodes
    void genMACNodes();

    // split llvm.fmuladd.* intrinsic calls into FMUL + FADD nodes
    void splitFmulAddCalls();

    // deduplicate identical memory input nodes (identical base/pattern)
    void dedupMemoryInputNodes();
    void dedupHardwareCallNodes();
 
    // update the IterDist of affine accesses
    void updateIterDist();
			
};


#endif
