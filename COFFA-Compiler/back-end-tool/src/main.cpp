#include <iostream>
#include <set>
#include <cstdlib>
#include <ctime>
#include <regex>
#include <sstream>
#include <getopt.h>
#include "op/operations.h"
#include "ir/adg_ir.h"
#include "ir/dfg_ir.h"
#include "mapper/mapper_sa.h"
#include "spdlog/spdlog.h"
#include "spdlog/cfg/argv.h"
#include <vector>
#include <fstream>
#include <stdexcept>
#include "rtlil/rtlil.h"
#include "rtlil/rtlil_ir.h"

// split string using regex
std::vector<std::string> split(const std::string& str, const std::string& delim){
    std::regex re{ delim };
    return std::vector<std::string>{
        std::sregex_token_iterator(str.begin(), str.end(), re, -1),
        std::sregex_token_iterator()
    };
}

// remove the prefix path  
std::string fileNameRemovePath(const std::string& filename) {
  size_t lastindex = filename.find_last_of(".");
  std::string res = filename.substr(0, lastindex);

  lastindex = filename.find_last_of("\\/");
  if (lastindex != std::string::npos) {
    res = res.substr(lastindex + 1);
  }
  return res;
}
// get the design name
std::string design_name(const std::string& filename) {
  size_t index1 = filename.find_last_of("\\/");
  size_t index2 = filename.find_last_of(".");
  if (index1 == std::string::npos) {
    return filename.substr(0, index2);
  }
   return filename.substr(index1+1, index2-index1 -1);
}
// get the file directory
std::string fileDir(const std::string& filename) {
  size_t lastindex = filename.find_last_of("\\/");
  if (lastindex == std::string::npos) {
    return std::string("./");
  }
  return filename.substr(0, lastindex);
}

void printMetric(int dfg_succeed, double II, double latency, double usage, int numDfg) {
    std::string path1 = "./result/latency.txt";
    std::string path2 = "./result/ii.txt";
    std::string path3 = "./result/mappingFailureRate.txt";
    std::string path4 = "./result/usage.txt";
    // std::cout << "II: " << II<< std::endl;

    double mapp_rate = dfg_succeed / (double)numDfg;
    // double av_II = II - MII;
    // double av_t = run_t / ((double)numDfg * 10);


    // if(mapp_rate != 1){
    //     mapp_rate = 0;
    // }

        std::ofstream ofs1(path1);
        std::ofstream ofs2(path2);
        std::ofstream ofs3(path3);
        std::ofstream ofs4(path4);
        ofs1 << latency << std::endl;
        ofs2 << II << std::endl;
        ofs3 << 1 - mapp_rate << std::endl;
        ofs4 << usage << std::endl;
    
    
}

int main(int argc, char* argv[]) {
    // spdlog load log level from argv
    // ./examlpe SPDLOG_LEVEL=info, mylogger=trace
    spdlog::cfg::load_argv_levels(argc, argv);

    static struct option long_options[] = {
        // {"verbose",        no_argument,       nullptr, 'v',},
        {"dump-config",     required_argument, nullptr, 'c',},  // true/false
        {"dump-mapped-viz", required_argument, nullptr, 'm',},  // true/false
        {"obj-opt",         required_argument, nullptr, 'o',},  // true/false
        {"timeout-ms",      required_argument, nullptr, 't',},
        {"max-iters",       required_argument, nullptr, 'i',},
        {"op-file",         required_argument, nullptr, 'p',},
        {"adg-file",        required_argument, nullptr, 'a',},
        {"dfg-files",       required_argument, nullptr, 'd',},  // can input multiple files, separated by " " or ","
        {"yosys-quiet-mode", required_argument, nullptr, 'q',}, // true/false, whether the Yosys is working at quiet mode( dosen't print internal message)
        {"RTLIL-Viz", required_argument, nullptr, 'v',}, // true/false, if true, Yosys will generate the schematics of the design, and the mapper will generate the DFG in dot format
        {"initialization-interval", required_argument, nullptr, 'I',}, // custom II
        {"cgra-only", required_argument, nullptr, 'C',}, // @yuan: coarse-only
        {0, 0, 0, 0,}
    };
    static char* const short_options = (char *)"c:m:o:t:i:p:a:d:q:v:I:C:";

    std::string op_fn;  // "resources/ops/operations.json";  // operations file name
    std::string adg_fn; // "resources/adgs/my_cgra_test.json"; // ADG filename
    std::vector<std::string> dfg_fns; // "resources/dfgs/conv3.dot"; // DFG filenames
    int timeout_ms = 3600000;
    int max_iters = 2000;
    int custom_ii = -1;
    bool dumpConfig = true;
    bool dumpMappedViz = true;
    bool objOpt = true;
    bool YosysQuiet = false;
    bool RTLILViz = false;
    bool CGOnly = false;
    std::string resultDir = "";
    std::string design_Name = "";
    std::string tempDir = "./Syn";

    int opt;
    while ((opt = getopt_long(argc, argv, short_options, long_options, nullptr)) != -1) {
        switch (opt) {
        //   case 'v': verbose = true; break;
            case 'c': std::istringstream(optarg) >> std::boolalpha >> dumpConfig; break;
            case 'm': std::istringstream(optarg) >> std::boolalpha >> dumpMappedViz; break;
            case 'o': std::istringstream(optarg) >> std::boolalpha >> objOpt; break;
            case 'q': std::istringstream(optarg) >> std::boolalpha >> YosysQuiet; break;
            case 'v': std::istringstream(optarg) >> std::boolalpha >> RTLILViz; break;
            case 't': timeout_ms = atoi(optarg); break;
            case 'i': max_iters = atoi(optarg); break;
            case 'p': op_fn = optarg; break;
            case 'a': adg_fn = optarg; break;
            case 'd': dfg_fns = split(optarg, "[\\s,?]+"); break;   
            case 'I': custom_ii = atoi(optarg); break;    
            case 'C': std::istringstream(optarg) >> std::boolalpha >> CGOnly; break;     
            case '?': std::cout << "Unknown option: " << optopt << std::endl; exit(1);
        }
    }
    if(op_fn.empty()){
        std::cout << "Please input operation file!" << std::endl; 
        exit(1);
    }
    if(adg_fn.empty()){
        std::cout << "Please input ADG file!" << std::endl; 
        exit(1);
    }
    if(dfg_fns.empty()){
        std::cout << "Please input at least one DFG file!" << std::endl; 
        exit(1);
    }

    unsigned seed = time(0); // random seed using current time
    srand(seed);  // set random generator seed 
    std::cout << "Parse Operations: " << op_fn << std::endl;
    Operations::Instance(op_fn);
    // Operations::print();

    std::cout << "Parse ADG: " << adg_fn << std::endl;
    ADGIR adg_ir(adg_fn);
    ADG* adg = adg_ir.getADG();
    int numADGNodes = adg->numGpeNodes();
    std::cout << "numADGNodes: " << numADGNodes << std::endl;
    int numIOBNodes = adg->numIobNodes();
    std::cout << "numADGNodes: " << numADGNodes << std::endl;
    int maxLUTinput = adg->maxLUTinput();
    // std::cout << "maxLUTinput: " << maxLUTinput << std::endl;
    std::vector<float>storePEusage;
    std::vector<int>bestLatency;
    std::vector<int>AchieveII;
    // adg->print();

    // map DFG to ADG
    MapperSA mapper(adg, timeout_ms, max_iters, objOpt);
    // MapperSA mapper(adg, dfg, 3600000, 2000);
    int numDfg = dfg_fns.size();
    int numSucceed = 0;
    for(auto& dfg_fn : dfg_fns){
        std::cout << "Parse DFG: " << dfg_fn << std::endl;
        DFGIR dfg_ir(dfg_fn);
        // OpTypeCount result = dfg_ir.getNumType();
        // int total = result.numaddsub+result.numlogic+result.nummul+result.numcomp;
	    // std::cout << "Operation Count:\n";
	    // std::cout << "ADDandSUB: " << result.numaddsub << ", " << result.numaddsub*100/total << "%\n";
        // std::cout << "MUL: " << result.nummul << ", " << result.nummul*100/total << "%\n";
        // std::cout << "LOGIC: " << result.numlogic << ", " << result.numlogic*100/total << "%\n";
        // std::cout << "COMP:" << result.numcomp << ", " << result.numcomp*100/total << "%\n";
        // std::ofstream fout("dfgoptype.txt");
        // fout << result.numaddsub*100/total << "\n" << result.nummul*100/total << "\n" << result.numlogic*100/total << "\n" << result.numcomp*100/total;
        // fout.close();
        DFG* dfg = dfg_ir.getDFG();
        int numNodes = dfg->nodes().size();
        int numopNodes = dfg->nodes().size() - dfg->ioNodes().size();
        int numioNodes = dfg->ioNodes().size();
        bool hasFinedGrain = dfg->hasFineGrained();
        std::cout << "node num: " << numNodes << " num edge: " << dfg->edges().size() << " num io node: " << dfg->ioNodes().size() << std::endl;
        // exit(0);
        if(hasFinedGrain && !CGOnly){
            //dfg->print();
            // map DFG to ADG
            resultDir = fileDir(dfg_fn);
            design_Name = design_name(dfg_fn);
            RTLIL* rtlil = dfg_ir.getRTLIL();
            rtlil->dumpRtlil(resultDir, design_Name);
            rtlil->runYosys(YosysQuiet, resultDir, RTLILViz, maxLUTinput);
            delete rtlil;// after running Yosys, rtlil can be deleted
            RTLIL_IR rtlil_ir(resultDir,RTLILViz);
            dfg = rtlil_ir.getDFG();
            // std::cout << "t-mapped node num: " << dfg->nodes().size() << " num edge: " << dfg->edges().size() << " num io node: " << dfg->ioNodes().size() << std::endl;
            // continue;
            if(custom_ii > 0){
              dfg->setMII(custom_ii);
            }
            numNodes = dfg->nodes().size();
            numopNodes = dfg->nodes().size() - dfg->ioNodes().size();
            numioNodes = dfg->ioNodes().size();
            // mapper.setStartTime();
            mapper.setDFG(dfg);
            std::cout << "fine-grain mapping, before mapping!!" << std::endl;
            // exit(0);
            bool succeed = mapper.execute(dumpConfig, dumpMappedViz, resultDir, CGOnly);
            if(!succeed){
                break;
            }
            numSucceed++;
        }else{
          // dfg->print();
          // map DFG to ADG
          delete dfg_ir.getRTLIL();
          if(custom_ii > 0){
            dfg->setMII(custom_ii);
          }
          mapper.setDFG(dfg);
          resultDir = fileDir(dfg_fn);
          std::cout << "coarse grain mapping, before mapping!!" << std::endl;
          // exit(0);
          bool succeed = mapper.execute(dumpConfig, dumpMappedViz, resultDir, CGOnly);
          if(!succeed){
            break;
          }
          numSucceed++;
        }
        float usagePE = (float)numopNodes/numADGNodes;
        storePEusage.push_back(usagePE);
        std::cout << "PE usage: " << usagePE << std::endl;
        float usageIO = (float)numioNodes/numIOBNodes;
        std::cout << "IO usage: " << usageIO << std::endl;
        bestLatency.push_back(mapper.getMaxLat());
        std::cout << "DFG latency: " << mapper.getMaxLat() << std::endl;
        AchieveII.push_back(mapper.getII());
    }
    float avgPEUsage = std::accumulate(storePEusage.begin(), storePEusage.end(), 0.0) / storePEusage.size();
    float avgLatency = std::accumulate(bestLatency.begin(), bestLatency.end(), 0) / (float)bestLatency.size();
    float avgII = std::accumulate(AchieveII.begin(), AchieveII.end(), 0) / (float)AchieveII.size();
    printMetric(numSucceed, avgII, avgLatency, avgPEUsage, numDfg);
    // std::cout << "=============================================\n";
    // if(numSucceed == numDfg){
    //     std::cout << "Succeed to map all the DFGs, number:  " << numSucceed << std::endl;
    //     float avgPEUsage = std::accumulate(storePEusage.begin(), storePEusage.end(), 0.0) / storePEusage.size();
    //     float avgLatency = std::accumulate(bestLatency.begin(), bestLatency.end(), 0) / (float)bestLatency.size();
    //     std::ofstream fout("PEWaste.txt");
    //     fout << (1-avgPEUsage) << std::endl;
    //     fout.close();
    //     std::ofstream fout2("bestLatency.txt");
    //     fout2 << avgLatency << std::endl;
    //     fout2.close();
    //     std::cout << "The average usage of PE is:  " << avgPEUsage << std::endl;
    //     std::cout << "The average best max latency is:  " << avgLatency << std::endl;
    // }else{
    //     std::cout << "Fail to map the DFG:  " << dfg_fns[numSucceed] << std::endl;
    // }
    // std::cout << "=============================================\n";
    
    
    // return numSucceed != numDfg;
}
