#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include "mapper/mapping.h"



// configuration data
struct CfgData{
    int len; // config data length
    std::vector<uint32_t> data; // config data
    int argInIdx = -1; //@yuan_argIn: this flag indicates current configuration is transmitted outside the kernel
    CfgData(){}
    CfgData(int len_) : len(len_){}
    CfgData(int len_, uint32_t data_){
        len = len_;
        data.push_back(data_);
    }
    CfgData(int len_, std::vector<uint32_t> data_){
        len = len_;
        data = data_;
    }
    CfgData& operator=(const CfgData& that){
        if(this == &that) return *this;
        len = that.len;
        data = that.data;
        argInIdx = that.argInIdx;//@yuan_argIn
        return *this;
    }
};


// configuration data packet
struct CfgDataPacket{
    unsigned addr; // config address
    std::vector<uint32_t> data; // config data
    int argInIdx = -1;//@yuan_argIn
    CfgDataPacket(unsigned addr_) : addr(addr_){}
    CfgDataPacket(unsigned addr_, uint32_t data_){
        addr = addr_;
        data.push_back(data_);
    }
};


// CGRA Configuration
class Configuration
{
private:
    Mapping* _mapping;
    std::map<int, int> _dfgIoSpadAddrs; // base address in spad for each DFG IO, <id, addr>
    std::map<std::string, int> _partitionedStartBank; //@yuan: the start bank index for the partitioned serious memory banks, <mem name, index>, notice: the index is a relative value
    const std::map<std::string, int> _unifiedCfgMap;
public:
    Configuration(Mapping* mapping) : _mapping(mapping),  
    _unifiedCfgMap({{"ADD", 0}, {"ADC", 1}, {"SUB", 4}, {"ULT", 4}, {"SLT", 4}, {"UGE", 5}, {"SGE", 5}, {"UGT", 2}, {"SGT", 2}, {"ULE", 3}, {"SLE", 3},
    {"OR", 0}, {"AND", 1}, {"XNOR", 3}, {"XOR", 2}, {"NE", 2}, {"EQ", 6}, {"CSHR", 0}, {"CSHL", 1}, {"LSHR", 2}, {"SHL", 3}, {"ASHR", 4},
    {"FADD", 0}, {"FSUB", 1}, {"FEQ", 0}, {"FLE", 1}, {"FLT", 2}, {"FEXADD", 0}, {"FEXSUB", 0}}) {}
    ~Configuration(){}
    const std::map<int, int>& dfgIoSpadAddrs(){ return _dfgIoSpadAddrs; }
    void setDfgIoSpadAddr(int id, int addr){ _dfgIoSpadAddrs[id] = addr; }
    // get config data for GPE, return<LSB-location, CfgData>
    std::map<int, CfgData> getGpeCfgData(GPENode* node);
    // get config data for GIB, return<LSB-location, CfgData>
    std::map<int, CfgData> getGibCfgData(GIBNode* node);
    // get config data for IOB, return<LSB-location, CfgData>
    std::map<int, CfgData> getIobCfgData(IOBNode* node);
    // get config data for ADG node
    void getNodeCfgData(ADGNode* node, std::vector<CfgDataPacket>& cfg);
    // get config data for ADG
    void getCfgData(std::vector<CfgDataPacket>& cfg);
    // dump config data
    void dumpCfgData(std::ostream& os);
    //@yuan: get/set the start bank index for the partitioned serious memory banks
    const std::map<std::string, int>& partitionedStartBank(){ return _partitionedStartBank; }
    void setpartitionedStartBank(std::string memName, int index) { _partitionedStartBank[memName] = index; }
    const std::map<std::string, int>& cfgMap() {return _unifiedCfgMap;}
    //@yuan_hw: get the real operation for accumulated-Ops
    std::string realOp(std::string op);
};






#endif