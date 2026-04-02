#ifndef __DFG_H__
#define __DFG_H__

#include "dfg/dfg_node.h"
#include "dfg/dfg_edge.h"
#include "dfg/python_path.h"
#include "graph/graph.h"
#include "Py_Tools/py_invoke.h"
#include <list>
#include "spdlog/spdlog.h"
#include <sstream>
#include <deque>
#include <queue>
#include <unordered_set>

//@yuan: one banking solution 
struct BankingSolution
{
    int II = 1;
    int N = 1;
    int B = 1;
    std::map< int, std::vector<int>> scheduledSteps; // <<control step>, <IO node's id> >
};


struct BS_Cmp{
	bool operator () (const BankingSolution &a, const BankingSolution &b) {
        //min-heap
        if(a.II > b.II){
            return true;
        }else if(a.II == b.II && a.N > b.N){
            return true;
        }else if(a.II == b.II && a.N == b.N && a.B > b.B){
            return true;
        }else{
            return false;
        }
    }
};

typedef std::priority_queue<BankingSolution, std::vector<BankingSolution>, BS_Cmp> BS_Queue;

class DFG : public Graph
{
private:
    bool _hasFineGrained;
    std::map<int, DFGNode*> _nodes;   // <node-id, node>
    std::map<int, DFGEdge*> _edges;   // <edge-id, edge>
    std::set<int> _ioNodes; // IO Node IDs, including INPUT, OUTPUT, LOAD, STORE nodes
    std::set<int> _lutNodes; // IO Node IDs, including INPUT, OUTPUT, LOAD, STORE nodes
    int _numPckedNode = 0;
    DFG(const DFG&) = delete; // disable the default copy construct function
    // @yuan: loops on the DFG, maybe we can delete it later
    std::list<std::list<DFGEdge*>*> _loops;
    // loops containing the back edges, <back-edge-id, <<loop-edges>>>
    std::map<int, std::vector<std::vector<int>>> _backEdgeLoops;
    // <mem-name, piar<<input-ids>, <output-ids>>>>
    std::map<std::string, std::pair<std::vector<int>, std::vector<int>>> _multiportIOs;
    // <mem-name, num_branch>
    std::map<std::string, int> _multiportIObr;
    //@yuan: multiport control steps: <mem-name, <step, <ids>>>
    std::map<std::string, std::map<int, std::vector<int>>> _multiportIOSteps;
    // @yuan: record all the back edges
    std::set<int> _backEdges;
    //@yuan: the coarse-grain width of the DFG
    int _CG_Width = 32; // default is 32
    int _MII = 1; // Minimum II
    int _MPII = 1; // II due to multiport access
    //@yuan: all the banking solutions
    std::map<std::string, BS_Queue> _multiportBnakingSolutions;
    //@yuan_argIn: the flag indicates that this DFG has argIn
    int _numArgIn = 0;
protected:
    // DFG nodes in topological order, DFG should be DAG
    std::vector<int> _topoNodes;
    int _numPyInit = 0;

    // depth-first search, sort dfg nodes in topological order
    void dfs(DFGNode* node, std::map<int, bool>& visited);

public:
    DFG();
    ~DFG();

    const std::map<int, DFGNode*>& nodes(){ return _nodes; }
    const std::map<int, DFGEdge*>& edges(){ return _edges; }
    DFGNode* node(int id);
    DFGEdge* edge(int id);
    void addNode(DFGNode* node);
    void addEdge(DFGEdge* edge);
    void delNode(int id);
    void delEdge(int id);

    const std::set<int>& ioNodes(){ return _ioNodes; }
    void addIONode(int id){ _ioNodes.insert(id); }
    void delIONode(int id){ _ioNodes.erase(id); }
    bool isIONode(int id){ return _ioNodes.count(id); }
    // In nodes: INPUT node, LOAD node without input
    std::set<int> getInNodes();
    // Out nodes: OUTPUT/STORE node
    std::set<int> getOutNodes();
    // @yuan: End nodes: The end nodes in the graph, including output nodes and the nodes only has backedge outputs
    std::set<int> getEndNodes();

    
    const std::set<int>& lutNodes(){ return _lutNodes; }
    void addLUTNode(int id){ _lutNodes.insert(id); }
    void delLUTNode(int id){ _lutNodes.erase(id); }
    bool isLUTNode(int id){ return _lutNodes.count(id); }

    // DFG nodes in topological order
    const std::vector<int>& topoNodes(){ return _topoNodes; }
    // sort dfg nodes in topological order
    void topoSortNodes();

    //if the DFG has fine-grained operators, set the variable as true
    void setFineGrained(bool FineGrained){_hasFineGrained = FineGrained;}
    bool hasFineGrained(){ return _hasFineGrained; }

    // set/get the coarse-grain width
    void setCGWidth(int width){ _CG_Width = width; }
    int CGWidth(){ return _CG_Width; }

    // ====== operators >>>>>>>>>>
    // DFG copy
    DFG& operator=(const DFG& that);

    // multiport Load/Store
    const std::map<std::string, std::pair<std::vector<int>, std::vector<int>>>& multiportIOs(){ return _multiportIOs; }
    void detectMultiportIOs();
    int getMultiportNum(std::string name);
    int getMultiportNum(DFGNode* node, std::string name);
    bool isMultiportIoNode(DFGIONode* node){ return _multiportIOs.count(node->memRefName()); }
    bool isMultiportIoNode(int id);
    // return multiport branch 
    const std::map<std::string, int>& multiportIObr(){ return _multiportIObr;}
    //@yuan: return  multiport IO control steps
    const std::map<std::string, std::map<int, std::vector<int>>>& multiportIOSteps() {return _multiportIOSteps;}


    // @yuan: get the loops on the DFG
    void getLoops();
    void DFS_loop(int headId, int currentId, std::list<DFGEdge*>* t_erasedEdges, std::list<DFGEdge*>* t_currentCycle,\
    std::list<std::list<DFGEdge*>*>* t_cycles);
    

    void print();


    const std::map<int, std::vector<std::vector<int>>>& backEdgeLoops(){ return _backEdgeLoops; }
    // detect loops based on backedge
    void detectBackEdgeLoops();
    // delete back edge loop
    void deleteBackEdgeLoop(int backedgeId);
    // delete memory-dependent edge
    void deleteMemEdge();

    const std::set<int>& backEdges(){return _backEdges; }



    int MPII(){ return _MPII; }
    void setMPII(int II) { _MPII = II; }

    int MII(){ return _MII; }
    void setMII(int II) { _MII = II; }

    //@yuan: generate all the banking solutions
    void genBankingSolution(bool modify, int Nmax, int Bmax, int maxBank);

    //@yuan: update the N, B, Control step for the array
    void updateBankingSettings(std::string memName, bool isFirst);
    
    //@Gao: get current Banking solution (the top element of the min-heap)
    BankingSolution getCurrBankingSolution(std::string arrayname) {return _multiportBnakingSolutions[arrayname].top();}

    //@yuan: set the size overflow flag here
    void setOverflag(int Bmax, int DateinByte);

    //@yuan: tune the partition scheme accroding to the #bank limitation
    // void partitionCheck(int maxBank);


    //@yuan: check if there is flow dependecy between two nodes
    bool hasFlowDependency(DFGIONode* node0, DFGIONode* node1);
    
    // void updateNonMultiPortBank();
    
    //@hw_yuan: generate the dot format DFG
    void dumpDFG(std::string resultDir, bool isPack);
    //@hw_yuan: get the Eu's type for one specific operation
    std::string getEuType(std::string op);
    void setNumPacked(int num){ _numPckedNode = num;}
    int numPackedNode() { return _numPckedNode; }

    //@hw_yuan_op: update the DFG, where the custom Op is replaced by the sub-DFG
    void UpdateCustomOPandArgIn(bool viz, std::string resultDir);

    //@yuan_argIn
    void setNumArgIn(int numArg) { _numArgIn = numArg; }
    int numArgIn() {return _numArgIn;}
};




#endif
