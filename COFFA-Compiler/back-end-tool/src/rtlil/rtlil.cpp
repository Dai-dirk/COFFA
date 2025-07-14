#include "rtlil/rtlil.h"
/*****************Cell**********************/
// return edge id
int Cell::inputWire(int index){
    if(_inputWires.count(index)){
        return _inputWires[index];
    }else{
        return -1; 
    }
}

// return set<edge-id>
std::set<int> Cell::outputWire(int index){
    if(_outputWires.count(index)){
        return _outputWires[index];
    }else{
        return {}; // return empty set
    }
}

// add input edge
void Cell::addInputWire(int index, int wireId){
    _inputWires[index] = wireId;
}

// add output edge
void Cell::addOutputWire(int index, int wireId){
    _outputWires[index].emplace(wireId);
}

// delete input edge
void Cell::delInputWire(int index){
    _inputWires.erase(index);
}

// delete output edge
void Cell::delOutputWire(int index, int wireId){
    assert(_outputWires.count(index));
    _outputWires[index].erase(wireId);
}


// does the node is io node?
bool Cell::isIOCell(){
    if(_operation == "INPUT" || _operation == "OUTPUT" || _operation == "LOAD" || _operation == "STORE" || _operation == "CLOAD" || _operation == "CSTORE"|| _operation == "CINPUT" || _operation == "COUTPUT" 
        || _operation == "TLOAD" || _operation == "TSTORE" || _operation == "TCLOAD" || _operation == "TCSTORE"){
        return true;    
    }else{
        return false;
    }
}

/*****************RTLIL********************/

void RTLIL::setInWire(int index, int width){
    _inputWire[index] = width;
}

void RTLIL::setOutWire(int index, int width){
    _outputWire[index] = width;
}

void RTLIL::delInWire(int index){
    assert(_inputWire.count(index));
    _inputWire.erase(index);
}

void RTLIL::delOutWire(int index){
    assert(_outputWire.count(index));
    _outputWire.erase(index);
}

void RTLIL::addCell(Cell* cell){
    int id = cell->id();
    _cells[id] = cell;
}


Cell* RTLIL::getCell(int id){
    if(_cells.count(id)){
        return _cells[id];
    } else {
        return nullptr;
    }  
}


Wire* RTLIL::getWire(int id){
    if(_wires.count(id)){
        return _wires[id];
    } else {
        return nullptr;
    }  
}

void RTLIL::addWire(Wire* wire){
    int id = wire->id();
    _wires[id] = wire;
    int srcId = wire->srcId();
    int dstId = wire->dstId();
    int srcPort = wire->srcPortIdx();
    int dstPort = wire->dstPortIdx();
    Cell* src = getCell(srcId);
    assert(src);
    src->addOutputWire(srcPort, id);
           
    Cell* dst = getCell(dstId);
    assert(dst);
    dst->addInputWire(dstPort, id);
}


void RTLIL::delCell(int id){
    Cell* cell = getCell(id);
    for(auto& elem : cell->inputWires()){
        delWire(elem.second);
    }
    for(auto& elem : cell->outputWires()){
        for(auto eid : elem.second){
            delWire(eid);
        }        
    }
    _cells.erase(id);
    delete cell;
}


void RTLIL::delWire(int id){
    Wire* e = getWire(id);
    int srcId = e->srcId();
    int dstId = e->dstId();
    int srcPortIdx = e->srcPortIdx();
    int dstPortIdx = e->dstPortIdx();

    Cell* srcCell = getCell(srcId);       
    srcCell->delOutputWire(srcPortIdx, id);
    
    Cell* dstCell = getCell(dstId);   
    dstCell->delInputWire(dstPortIdx);

    _wires.erase(id);
    delete e;
}

//generate RTLIL file, which is used by Yosys
//dirname: the path of the design
void RTLIL::dumpRtlil(std::string dirname, std::string design_name){
    //std::set<int> logicConst;
    std::string filename = dirname + "/temp_rtlil.il";
    std::ofstream ofs(filename);
    ofs << "module \\"<< design_name <<std::endl;
    ofs << std::endl;
    //print input/output wires
    //int ioindex = 1;
    /*for(auto elem : inWires()){
        //for coarse-grain wires, set "keep" attribute, which directs Yosys don't remove/process them while optimizing
        if(elem.second > 1){
            ofs <<"  attribute \\keep " << "\"coarse_input_" << std::to_string(ioindex)<< "\"" << std::endl;
        }
        ofs <<"  wire width " << std::to_string(elem.second) << " input " <<  std::to_string(ioindex) << " \\input" << std::to_string(elem.first) << std::endl;
        ofs << std::endl;
        ioindex += 1;
    }   
    for(auto elem : outWires()){
        //for coarse-grain wires, set "keep" attribute, which directs Yosys don't remove/process them while optimizing
        if(elem.second > 1){
            ofs <<"  attribute \\keep " << "\"coarse_output_" << std::to_string(ioindex) << "\"" << std::endl;
        }
        ofs <<"  wire width " << std::to_string(elem.second) << " output " <<  std::to_string(ioindex) << " \\output" << std::to_string(elem.first) << std::endl;
        ofs << std::endl;
        ioindex += 1;
    } */ 

    //print the internal wires according to the output ports of the nodes 
    for(auto elem : cells()){
        for(auto outwires : elem.second->outputWires()){
            int outWireId = *(outwires.second.begin()); // for the wires that from same port, their width are the same
            int wireWidth = getWire(outWireId)->Width();
            //for coarse-grain wires, set "keep" attribute, which directs Yosys don't remove/process them while optimizing
            if(wireWidth > 1){
                ofs <<"  attribute \\keep " << "\"coarse_wire_" << elem.second->id() << "_" << outwires.first << "\"" << std::endl;
                ofs <<"  wire width " << wireWidth <<  " $" << elem.second->id() << "_" << outwires.first << std::endl;
                ofs << std::endl;
                continue;
            }else{
                // for(auto& e: outwires.second){
                //     auto wire = getWire(e);
                //     Cell* dstCell = getCell(wire->dstId());
                //     if(dstCell->bitWidths().size() != 1){
                //         ofs <<"  attribute \\keep " << "\"fused_wire_" << elem.second->id() << "_" << outwires.first << "\"" << std::endl;
                //         break;
                //     }
                // }
            }
            ofs <<"  wire width " << wireWidth <<  " $" << elem.second->id() << "_" << outwires.first << std::endl;
            ofs << std::endl;
        }    
    }

    //print all the cells
    for(auto elem : cells()){
        ofs << std::endl;
        if(elem.second->bitWidths().size() == 1){ 
            //determine the grain of the cell
            int width = *(elem.second->bitWidths().begin());
            if(width == 1){// fine-grain
            //for the fine-grain nodes, only the logic operations using the Yosys internal cells
                if(elem.second->operation() == "AND" || elem.second->operation() == "EQ" || elem.second->operation() == "XOR" || elem.second->operation() == "OR" || elem.second->operation() == "NOT"){
                    //using Yosys internal cells lib
                    std::string op = elem.second->operation();
                    std::transform(op.begin(), op.end(), op.begin(), tolower);
                    ofs <<"  cell " << "$" << op <<  " $" << elem.second->name() << std::endl;

                    //print signed message of the input ports, which is not useful for FGRA, but is needed by Yosys
                    int portASCII = 65;
                    for(auto inwires : elem.second->inputWires()){
                        ofs <<"     parameter " << "\\" << char(portASCII + inwires.first) <<  "_SIGNED 0"<< std::endl;
                    }

                    //print input wires parameters
                    for(auto inwire : elem.second->inputWires()){
                        ofs <<"     parameter " << "\\" << char(portASCII + inwire.first) <<  "_WIDTH " << getWire(inwire.second)->Width()<< std::endl;
                    }

                    //print output wires parameters
                    portASCII = 89;
                    for(auto outwire : elem.second->outputWires()){
                        int outWireId = *(outwire.second.begin()); // for the wires that from same port, their width are the same
                        ofs <<"     parameter " << "\\" << char(portASCII + outwire.first) <<  "_WIDTH " << getWire(outWireId)->Width()<< std::endl;
                    } 

                    //print input connections
                    portASCII = 65;
                    for(auto inwire : elem.second->inputWires()){
                        Wire* inWire = getWire(inwire.second);
                        int srcId = inWire->srcId();
                        int srcPortIdx = inWire->srcPortIdx();
                        ofs <<"     connect " << "\\" << char(portASCII + inwire.first) <<  " $" << srcId <<"_" << srcPortIdx << std::endl;
                        //auto srcCell = getCell(srcId);
                        /*if(srcCell->operation()== "CONST"){
                            int constValue = srcCell->Value();
                            ofs <<"     connect " << "\\" << char(portASCII + inwire.first) <<  " 1'" << constValue << std::endl;    
                            logicConst.emplace(srcId);
                        }else{
                        }*/
                    }

                    //print output connections
                    portASCII = 89;
                    for(auto outwire : elem.second->outputWires()){
                        ofs <<"     connect " << "\\" << char(portASCII + outwire.first) <<  " $" << elem.second->id() << "_" <<outwire.first<< std::endl;
                    }  
                
                    ofs << "  end" <<std::endl;              
                }else{
                    /*if(logicConst.count(elem.second->id())){
                        continue;
                    }*/
                    //for the fine-grain cells that do not perform logic operations, set "keep" attribute
                    ofs <<"  attribute \\keep " << "\"fine_node_" << elem.second->id() << "\"" << std::endl;                
                    ofs <<"  cell " << "\\" << elem.second->operation() <<  " $" << elem.second->name() << std::endl;
                    
                    //print the parameters of the node
                    auto& parameters = elem.second->Parameters();
                    for(auto& eachPar : parameters){
                        ofs <<"     parameter " << "\\" << eachPar.first <<  " \"" << eachPar.second<< "\""<< std::endl;
                    }

                    //print input wires parameters
                    int portASCII = 65;
                    for(auto inwire : elem.second->inputWires()){
                        ofs <<"     parameter " << "\\" << char(portASCII + inwire.first) <<  "_WIDTH " << getWire(inwire.second)->Width()<< std::endl;
                    }
                    //print output wires parameters
                    portASCII = 89;
                    for(auto outwire : elem.second->outputWires()){
                        int outWireId = *(outwire.second.begin()); // for the wires that from same port, their width are the same
                        ofs <<"     parameter " << "\\" << char(portASCII + outwire.first) <<  "_WIDTH " << getWire(outWireId)->Width()<< std::endl;
                    }

                    //print the parameter of value for CONST node   
                    if(elem.second->operation() == "CONST"){
                        ofs <<"     parameter " << "\\VALUE " <<elem.second->Value() << std::endl;
                    }

                    //print input connections
                    portASCII = 65;
                    for(auto inwire : elem.second->inputWires()){// connect ID_Port -> SrcID_Port
                        Wire* inWire = getWire(inwire.second);
                        int srcId = inWire->srcId();
                        int srcPortIdx = inWire->srcPortIdx();
                        ofs <<"     connect " << "\\" << char(portASCII + inwire.first) <<  " $" << srcId <<"_" << srcPortIdx << std::endl;
                    }

                    //print output connections
                    portASCII = 89;
                    for(auto outwire : elem.second->outputWires()){// connect ID_Port -> ID_Port
                        ofs <<"     connect " << "\\" << char(portASCII + outwire.first) <<  " $" << elem.second->id() << "_" <<outwire.first<< std::endl;
                    }
                    ofs << "  end" <<std::endl;

                }
            }else{// coase-grain, default one node has one output port
                //for the coarse-grain cells, set "keep" attribute
                ofs <<"  attribute \\keep " << "\"coarse_node_" << elem.second->id() << "\"" << std::endl;
                ofs <<"  cell " << "\\" << elem.second->operation() <<  " $" << elem.second->name() << std::endl;

                //print the parameters of the node
                auto& parameters = elem.second->Parameters();
                for(auto& eachPar : parameters){
                    ofs <<"     parameter " << "\\" << eachPar.first <<  " \"" << eachPar.second<< "\""<< std::endl;
                }
                // for iob cell, pirnt it's dependent cells
                if(elem.second->isIOCell()){
                    if(!elem.second->dependencyCells().empty()){
                        ofs << "     parameter "<< "\\Dependent \"";
                        auto cells = elem.second->dependencyCells();
                        int cellSize = cells.size();
                        int Idx = 1;
                        for(auto& c : cells){
                            ofs << c ;
                            if(Idx == cellSize){
                                ofs << "\"" << std::endl;
                            }else{
                                ofs << " " << std::endl;
                            }
                            Idx ++;
                        }
                    }
                }
                //print input wires parameters
                int portASCII = 65;
                bool hasBack = false;
                std::map<char, std::vector<int>> backPair; // backedge input port and iteration distance <Port, dist>
                for(auto inwire : elem.second->inputWires()){
                    auto wire = getWire(inwire.second);
                    ofs <<"     parameter " << "\\" << char(portASCII + inwire.first) <<  "_WIDTH " << wire->Width()<< std::endl;
                    // std::cout << "isloop: " << isloop << std::endl;
                    if(wire->isBackEdge()){
                        hasBack = true;
                        int iterdist = wire->iterDist();
                        backPair[char(portASCII + inwire.first)].push_back(iterdist);
                        int logicLat = wire->logicLat();
                        backPair[char(portASCII + inwire.first)].push_back(logicLat);
                        int edgeType = wire->Type();
                        backPair[char(portASCII + inwire.first)].push_back(edgeType);
                        // ofs <<"     parameter " << "\\" << "backedge \"" <<  char(portASCII + inwire.first) << "\"" << std::endl;  
                        // ofs <<"     parameter " << "\\" << char(portASCII + inwire.first) <<  "_DISTANCE " << iterdist<< std::endl; 
                    }
                }
                if(hasBack){
                    ofs <<"     parameter " << "\\" << "backedge \"";
                    int Idx = 1;
                    int PairSize = backPair.size();
                    for(auto & elem : backPair){
                        ofs << elem.first << ","; 
                        int psize = elem.second.size();
                        int pidx = 1;
                        for(auto & p : elem.second){
                            ofs << p;
                            if(pidx < psize){
                                ofs << "," ;
                            }
                            pidx++;
                        }
                        if(Idx < PairSize){
                            ofs << " " ;
                        }  
                        Idx++;        
                    }
                    ofs << "\"" << std::endl;
                }
                //print output wires parameters
                portASCII = 89;
                for(auto outwire : elem.second->outputWires()){
                    int outWireId = *(outwire.second.begin()); // for the wires that from same port, their width are the same
                    ofs <<"     parameter " << "\\" << char(portASCII + outwire.first) <<  "_WIDTH " << getWire(outWireId)->Width()<< std::endl;
                } 

                //print the parameter of value for CONST node   
                if(elem.second->operation() == "CONST"){
                    ofs <<"     parameter " << "\\VALUE " <<elem.second->Value() << std::endl;
                }

                //print input connections
                portASCII = 65;
                for(auto inwire : elem.second->inputWires()){// connect ID_Port -> SrcID_Port
                    Wire* inWire = getWire(inwire.second);
                    int srcId = inWire->srcId();
                    int srcPortIdx = inWire->srcPortIdx();
                    ofs <<"     connect " << "\\" << char(portASCII + inwire.first) <<  " $" << srcId <<"_" << srcPortIdx << std::endl;
                }

                //print output connections
                portASCII = 89;
                for(auto outwire : elem.second->outputWires()){// connect ID_Port -> ID_Port
                    ofs <<"     connect " << "\\" << char(portASCII + outwire.first) <<  " $" << elem.second->id() << "_" <<outwire.first<< std::endl;
                }
                ofs << "  end" <<std::endl;
            }
        }else{
            //for the fused-grain cells (such as "sel" node), set "keep" attribute
            ofs <<"  attribute \\keep " << "\"node_" << elem.second->id() << "\"" << std::endl;
            ofs <<"  cell " << "\\" << elem.second->operation() <<  " $" << elem.second->name() << std::endl;


            //print the parameters of the node
            auto& parameters = elem.second->Parameters();
            for(auto& eachPar : parameters){
                ofs <<"     parameter " << "\\" << eachPar.first <<  " \"" << eachPar.second<< "\""<< std::endl;
            }
            // for iob cell, pirnt it's dependent cells
            if(elem.second->isIOCell()){
                if(!elem.second->dependencyCells().empty()){
                    ofs << "     parameter "<< "\\Dependent \"";
                    auto cells = elem.second->dependencyCells();
                    int cellSize = cells.size();
                    int Idx = 1;
                    for(auto& c : cells){
                        ofs << c ;
                        if(Idx == cellSize){
                            ofs << "\"" << std::endl;
                        }else{
                            ofs << " " << std::endl;
                        }
                        Idx ++;
                    }
                }
            }

            //print input wires parameters
            int portASCII = 65;
            bool hasBack = false;
            std::map<char, std::vector<int>> backPair; // backedge input port and iteration distance <Port, dist>
            for(auto inwire : elem.second->inputWires()){
                auto wire = getWire(inwire.second);
                ofs <<"     parameter " << "\\" << char(portASCII + inwire.first) <<  "_WIDTH " << wire->Width()<< std::endl;
                // std::cout << "isloop: " << isloop << std::endl;
                if(wire->isBackEdge()){
                    hasBack = true;
                    int iterdist = wire->iterDist();
                    backPair[char(portASCII + inwire.first)].push_back(iterdist);
                    int logicLat = wire->logicLat();
                    backPair[char(portASCII + inwire.first)].push_back(logicLat);
                    int edgeType = wire->Type();
                    backPair[char(portASCII + inwire.first)].push_back(edgeType);
                    // ofs <<"     parameter " << "\\" << "backedge \"" <<  char(portASCII + inwire.first) << "\"" << std::endl;
                    // ofs <<"     parameter " << "\\" << char(portASCII + inwire.first) <<  "_DISTANCE " << iterdist<< std::endl; 
                }
            }
            if(hasBack){
                ofs <<"     parameter " << "\\" << "backedge \"";
                int Idx = 1;
                int PairSize = backPair.size();
                for(auto & elem : backPair){
                    ofs << elem.first << ","; 
                    int psize = elem.second.size();
                    int pidx = 1;
                    for(auto & p : elem.second){
                        ofs << p;
                        if(pidx < psize){
                            ofs << "," ;
                        }
                        pidx++;
                    }
                    if(Idx < PairSize){
                        ofs << " " ;
                    }  
                    Idx++;        
                }
                ofs << "\"" << std::endl;
            }
            //print output wires parameters
            portASCII = 89;
            for(auto outwire : elem.second->outputWires()){
                int outWireId = *(outwire.second.begin()); // for the wires that from same port, their width are the same
                ofs <<"     parameter " << "\\" << char(portASCII + outwire.first) <<  "_WIDTH " << getWire(outWireId)->Width()<< std::endl;
            } 

            //print the parameter of value for CONST node   
            if(elem.second->operation() == "CONST"){
                ofs <<"     parameter " << "\\VALUE " <<elem.second->Value() << std::endl;
            }

            //print input connections
            portASCII = 65;
            for(auto inwire : elem.second->inputWires()){// connect ID_Port -> SrcID_Port
                Wire* inWire = getWire(inwire.second);
                int srcId = inWire->srcId();
                int srcPortIdx = inWire->srcPortIdx();
                ofs <<"     connect " << "\\" << char(portASCII + inwire.first) <<  " $" << srcId <<"_" << srcPortIdx << std::endl;
            }

            //print output connections
            portASCII = 89;
            for(auto outwire : elem.second->outputWires()){// connect ID_Port -> ID_Port
                ofs <<"     connect " << "\\" << char(portASCII + outwire.first) <<  " $" << elem.second->id() << "_" <<outwire.first<< std::endl;
            }
            ofs << "  end" <<std::endl;
        }    
    }
    ofs << "end" <<std::endl;   

    std::cout << "Generate RTLIL file: " << filename << std::endl;
}

//run Yosys
//YosysQuiet: whether the Yosys is working at quiet mode( dosen't print internal message)
//Viz: whether generate schematics in Yosys
void RTLIL::runYosys(bool YosysQuiet, std::string dirname, bool Viz, int lut){
    //int lutWidth = 3;
    std::string inFIle = dirname + "/temp_rtlil.il";
    std::string resultDir = dirname + "/mapped_rtlil.il";
    std::string command = "yosys -p \"tcl ./Syn/Syn.tcl ~i " + inFIle + " ~l " + std::to_string(lut) + " ~o "+ resultDir;
    if(Viz){
        command = command + " ~v true \"";
    }else{
        command = command + " ~v false \"";
    }
    if(YosysQuiet){
        command = command + " -q";
    }
    system(command.c_str());
    //std::cout << command << std::endl;
    if(!YosysQuiet){
        std::cout << "\n=============================================================" << std::endl;
    }
    std::cout << "Run Yosys finished!"<< std::endl;
    std::cout << "The mapped RTLIL file is generated, storing at: " << resultDir << std::endl;

}

