
#include "mapper/visualize.h"


Graphviz::Graphviz(Mapping* mapping, std::string dirname) : _mapping(mapping), _dirname(dirname) {}


// // create name for DFG node
// std::string Graphviz::getDfgNodeName(int id){
//     std::string name;
//     DFG* dfg = _mapping->getDFG();
//     if(id != dfg->id()){
//         name = dfg->node(id)->name();
//     }else if(isDfgInput){
//         name = dfg->inputName(idx);
//     }else{
//         name = dfg->outputName(idx);
//     }
//     return name;
// }


void Graphviz::drawDFG(){
    std::string filename = _dirname + "/mapped_dfg.dot";
    std::ofstream ofs(filename);
    DFG* dfg = _mapping->getDFG();
    int dfgId = dfg->id();
    ofs << "Digraph G {\n";
    ofs << "edge [colorscheme=paired12];\n";
    int II = _mapping->II();
    for(auto& elem : dfg->nodes()){
        auto node = elem.second;
        auto& attr = _mapping->dfgNodeAttr(node->id());
        auto name = node->name();
        std::string quoteName = "\"" + name + "\"";
        ofs << quoteName << "[label = \"\\N\\nlat=" << attr.lat << "\"];\n";
        if(dfg->isIONode(node->id())){
            DFGIONode* ioNode = dynamic_cast<DFGIONode*>(node);
            if(!ioNode->dependencyNodes().empty()){
                for(auto c : ioNode->dependencyNodes()){
                    std::string srcName = dfg->node(c)->name();
                    std::string quoteSrcName = "\"" + srcName + "\"";
                    ofs << quoteSrcName << "->" << quoteName ;
                    ofs << "[style = dashed, color = blue];\n";
                }
            }
        }
        for(auto& bitInputs : node->inputEdges()){
            int bits = bitInputs.first;
            for(auto& input : bitInputs.second){
                DFGEdge* edge = dfg->edge(input.second);
                if(edge->isMemEdge()) continue;
                auto& edgeattr = _mapping->dfgEdgeAttr(edge->id());
                int lat = edgeattr.lat;
                int rdu = edgeattr.delay;
                int srcNodeId = edge->srcId();
                int srclogiclat = edge->logicLat()*II;
                int operandIdx = edge->dstPortIdx();
                std::string srcName = dfg->node(srcNodeId)->name();
                std::string quoteSrcName = "\"" + srcName + "\"";
                ofs << quoteSrcName << "->" << quoteName << "[weight = 4, color = " << ((bits%12)+1) << ", label =" << "\"lat = " << lat << "\\nRDU = " << rdu << "\\nlogiclat=" << srclogiclat 
                    << "\\nop=" << operandIdx;
                if(edge->isBackEdge()){
                    ofs << "\\niterdist="<< edge->iterDist() << "\"" <<", style = dashed" <<"];\n";
                }else{
                    ofs <<"\"];\n";
                }
                
            }
        }
    }
    ofs << "}\n";
}



// create name for ADG node
// std::string Graphviz::getAdgNodeName(int id){
//     ADG* adg = _mapping->getADG();
//     return adg->node(id)->name();
// }


void Graphviz::drawADG(){
    std::string filename = _dirname + "/mapped_adg.dot";
    std::ofstream ofs(filename);
    ADG* adg = _mapping->getADG();
    int adgId = adg->id();
    DFG* dfg = _mapping->getDFG();
    int dfgId = dfg->id();
    ofs << "Digraph G {\nlayout = sfdp;\noverlap = scale;\n";
    for(auto& elem : adg->nodes()){
        auto node = elem.second;
        auto& attr = _mapping->adgNodeAttr(node->id());
        auto name = node->name();
        ofs << name << "[label = \"" << name;
        auto dfgNodes = _mapping->mappedNode(node);
        for(auto& dfgNode : dfgNodes){
            ofs << "\\nDFG:" << dfgNode->name();
        }
        if(dfgNodes.empty()){
            ofs << "\", color = black];\n";
        }else{
            ofs << "\", color = blue];\n";
        }
        std::set<int> srcNodeIds;
        for(auto& bitInputs : node->inputs()){
            for(auto& input : bitInputs.second){
                int srcNodeId = input.second.first;
                srcNodeIds.emplace(srcNodeId);
            }    
        }
        for(auto srcNodeId : srcNodeIds){
            std::string srcName = adg->node(srcNodeId)->name();
            ofs << srcName << "->" << name << "[color = gray80];\n";  
        }
    }
    ofs << "edge [colorscheme=paired12];\n";
    for(auto& elem : dfg->edges()){
        int eid = elem.first;
        auto& attr = _mapping->dfgEdgeAttr(eid);
        auto& edgeLinks = attr.edgeLinks;
        int i = edgeLinks.size();
        if(i==1) continue;
        for(auto& edgeLink : edgeLinks){
            i--;
            auto adgNode = edgeLink.adgNode;
            auto name = adgNode->name();
            if(i > 0){
                ofs << name << "->";           
            }else{
                ofs << name << "[weight = 4, color = " << ((eid%12)+1) << "];\n";
            }
        }
    }
    ofs << "}\n";
}


// print edge path
void Graphviz::printDFGEdgePath(){
    DFG* dfg = _mapping->getDFG();
    for(auto& elem : dfg->edges()){
        int eid = elem.first;
        auto& attr = _mapping->dfgEdgeAttr(eid);
        auto& edgeLinks = attr.edgeLinks;
        // print edge path
        auto dfgSrcNodeName = dfg->node(elem.second->srcId())->name();
        auto dfgDstNodeName = dfg->node(elem.second->dstId())->name();
        std::cout << dfgSrcNodeName << " => " << dfgDstNodeName << ":\n";
        int i = edgeLinks.size();
        for(auto& edgeLink : edgeLinks){
            auto adgNodeName = edgeLink.adgNode->name();
            int adgNodeSrcPort = edgeLink.srcPort;
            int adgNodeDstPort = edgeLink.dstPort;
            std::cout << "(" << adgNodeName << ", " << adgNodeSrcPort << ", " << adgNodeDstPort;
            i--;
            if(i > 0){
                std::cout << ") -> ";
            }else{
                std::cout << ");\n";
            }
        }
    }
}


// dump mapped DFG IO ports with mapped ADG IO and latency annotated
void Graphviz::dumpDFGIO(){
    std::string filename = _dirname + "/mapped_dfgio.txt";
    std::ofstream ofs(filename);
    ADG* adg = _mapping->getADG();
    DFG* dfg = _mapping->getDFG();
    int ioid = 0;
    ofs << "# format: DFG-IO-Tag, DFG-IO-Name, ADG-IOB-Id, ADG-IOB-Index, DFG-IO-Latency, Spad-Bank\n";
    auto outNodeIds = dfg->getOutNodes(); // OUTPUT/STORE nodes
    for(auto& id : dfg->ioNodes()){
        auto dfgIONode = dfg->node(id);
        auto& attr =  _mapping->dfgNodeAttr(id);
        int iobId = attr.adgNode->id();
        int iobIdx = dynamic_cast<IOBNode*>(adg->node(iobId))->index();
        std::string tag = dfgIONode->operation() + "_" + std::to_string(ioid++);    
        int lat = attr.lat;
        int bank = dynamic_cast<DFGIONode*>(dfgIONode)->bank();
        if(!outNodeIds.count(id)){ // INPUT/LOAD node latency is the input port latency
            lat = attr.lat - dfgIONode->opLatency();
        }    
        ofs << tag << ", " << dfgIONode->name() << ", " << iobId << ", " << iobIdx << ", " << lat <<", "<< bank << std::endl;
    }
}