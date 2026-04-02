#ifndef __DFG_EDGE_H__
#define __DFG_EDGE_H__

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <assert.h>
#include "graph/graph_edge.h"


enum EdgeType{ 
    EDGE_TYPE_DATA, // data dependence
    EDGE_TYPE_CTRL, // control dependence
    EDGE_TYPE_MEM,  // loop-carried memory dependence 
};

class DFGEdge : public GraphEdge
{
private:
    EdgeType _type = EDGE_TYPE_DATA;
    bool _backedge = false;  
    bool _realbackedge = false;  //@yuan: for some edges are not cause loops
    int _logicLat = 0; // due to multport add a logic lat //mulrport      
    int _iterDist = 0; // iteration distance for loop-carried dependence
    bool _isDontTouch = false;  
    int _addDelay = 0;//@hw_yuan: the additional delay requirement caused by packing  
public:
    using GraphEdge::GraphEdge; // C++11, inherit parent constructors
    EdgeType type(){ return _type; }
    void setType(EdgeType type){ _type = type; }
    bool isMemEdge(){ return _type == EDGE_TYPE_MEM; }
    bool isMemBackEdge(){ return _type == EDGE_TYPE_MEM && _backedge;}
    bool isBackEdge(){ return _backedge; }
    void setBackEdge(bool back){ _backedge = back; }
    bool isrealBackEdge(){ return _realbackedge; }
    void setrealBackEdge(bool back){ _realbackedge = back; }
    void setlogicLat(int logicLat){ _logicLat = logicLat; }
    int logicLat(){ return _logicLat; }
    void setIterDist(int dist){ _iterDist = dist; }
    int iterDist(){ return _iterDist; }
    void setDontTouch(bool touch){ _isDontTouch = touch; }
    bool isDontTouch(){ return _isDontTouch; }
    void setAddDelay(int adddelay){ _addDelay = adddelay; }
    int addDelay(){ return _addDelay; }
};





#endif