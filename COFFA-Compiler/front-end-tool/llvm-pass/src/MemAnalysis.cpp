#include "llvm_cdfg.h"


// invoke python conflictpolytope.py
// 0: no conflict;
// 1: true conflict
inline int conflictpolytope(int coffs[10], int counts[3]){
 
    // 3、调用python文件名，不用写后缀
	PyObject* pModule = PyImport_ImportModule("conflictpolytope");
	if( pModule == NULL ){
		errs() <<"module not found\n";
	}
    // 4、调用函数
	PyObject* pFunc = PyObject_GetAttrString(pModule, "run");
	if( !pFunc || !PyCallable_Check(pFunc)){
		errs() <<"not found function run\n";
	}
    
    //5、给python传参数
    // 函数调用的参数传递均是以元组的形式打包的,2表示参数个数
    // 如果AdditionFc中只有一个参数时，写1就可以了
    PyObject* pArgs = PyTuple_New(16);
    
    int a[12] = {2, 1, 3, 4, 0, 0, 0, 10, 0, 5, 0, 0};

    // for(int i = 0; i < 12; i++){
    //     // 第i个参数，传入 int 类型的值 coffs[i]
    //     PyTuple_SetItem(pArgs, i, Py_BuildValue("i", a[i])); 
    // }

    for(int i = 0; i < 10; i++){
        // 第i个参数，传入 int 类型的值 coffs[i]
        PyTuple_SetItem(pArgs, i, Py_BuildValue("i", coffs[i])); 
    }

    for(int i = 0; i < 3; i++){
        // 第2*i + 5 & 2*i + 6个参数，传入 int 类型的值 coffs[i]
        PyTuple_SetItem(pArgs, 2*i + 10, Py_BuildValue("i", 0)); 
        PyTuple_SetItem(pArgs, 2*i + 11, Py_BuildValue("i", counts[i]-1)); 
    }
    
    
    // 6、使用C++的python接口调用该函数
    PyObject* pReturn = PyEval_CallObject(pFunc, pArgs);
    
    // 7、接收python计算好的返回值
    int nResult;
    // i表示转换成int型变量。
    // 在这里，最需要注意的是：PyArg_Parse的最后一个参数，必须加上“&”符号
    PyArg_Parse(pReturn, "i", &nResult);
    // errs() << "return result is " << nResult << "\n";


    // 0: no conflict;
    // 1: true conflict
    return nResult;
}

inline int graph_color_for_II(int totalNum, std::vector<std::pair<int, int>> LSpairs){
 
    // 3、调用python文件名，不用写后缀
	PyObject* pModule = PyImport_ImportModule("graph_color");
	if( pModule == NULL ){
		errs() <<"module not found\n";
	}
    // 4、调用函数
	PyObject* pFunc = PyObject_GetAttrString(pModule, "runII");
	if( !pFunc || !PyCallable_Check(pFunc)){
		errs() <<"not found function run\n";
	}
    
    //5、给python传参数
    // 函数调用的参数传递均是以元组的形式打包的,2表示参数个数
    // 如果AdditionFc中只有一个参数时，写1就可以了
    PyObject* pArgs = PyTuple_New(LSpairs.size()*2+1);

    // for(int i = 0; i < 12; i++){
    //     // 第i个参数，传入 int 类型的值 coffs[i]
    //     PyTuple_SetItem(pArgs, i, Py_BuildValue("i", a[i])); 
    // }
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("i", totalNum)); 
    
    for(int i = 0; i < LSpairs.size(); i++){
        // 第 i & i+1 个参数，传入 int 类型的值 LSpair.first & LSpair.second
        // errs() << LSpairs[i].first << ", " << LSpairs[i].second  << "; ";
        PyTuple_SetItem(pArgs, i*2+1, Py_BuildValue("i", LSpairs[i].first)); 
        PyTuple_SetItem(pArgs, i*2+2, Py_BuildValue("i", LSpairs[i].second)); 
    }
    
    // 6、使用C++的python接口调用该函数
    PyObject* pReturn = PyEval_CallObject(pFunc, pArgs);
    
    // 7、接收python计算好的返回值
    int nResult;
    // i表示转换成int型变量。
    // 在这里，最需要注意的是：PyArg_Parse的最后一个参数，必须加上“&”符号
    PyArg_Parse(pReturn, "i", &nResult);
    // errs() << "return result is " << nResult << "\n";

    // II after graph_color algorithms
    return nResult;
}

inline std::map<int, int> graph_color_for_CtrlStep(int totalNum, std::vector<std::pair<int, int>> LSpairs){
 
    // 3、调用python文件名，不用写后缀
	PyObject* pModule = PyImport_ImportModule("graph_color");
	if( pModule == NULL ){
		errs() <<"module not found\n";
	}
    // 4、调用函数
	PyObject* pFunc = PyObject_GetAttrString(pModule, "runCtrlStep");
	if( !pFunc || !PyCallable_Check(pFunc)){
		errs() <<"not found function run\n";
	}
    
    //5、给python传参数
    // 函数调用的参数传递均是以元组的形式打包的,2表示参数个数
    // 如果AdditionFc中只有一个参数时，写1就可以了
    PyObject* pArgs = PyTuple_New(LSpairs.size()*2+1);

    // for(int i = 0; i < 12; i++){
    //     // 第i个参数，传入 int 类型的值 coffs[i]
    //     PyTuple_SetItem(pArgs, i, Py_BuildValue("i", a[i])); 
    // }
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("i", totalNum)); 
    
    for(int i = 0; i < LSpairs.size(); i++){
        // 第 i & i+1 个参数，传入 int 类型的值 LSpair.first & LSpair.second
        // errs() << LSpairs[i].first << ", " << LSpairs[i].second  << "; ";
        PyTuple_SetItem(pArgs, i*2+1, Py_BuildValue("i", LSpairs[i].first)); 
        PyTuple_SetItem(pArgs, i*2+2, Py_BuildValue("i", LSpairs[i].second)); 
    }
    
    // 6、使用C++的python接口调用该函数
    PyObject* pReturn = PyEval_CallObject(pFunc, pArgs);
    
    // 7、接收python计算好的返回值
    std::map<int, int> Mem2CtrlStep;

    if (PyDict_Check(pReturn)) {
        std::cout << "PyDict YES!\n";
        PyObject *key, *value;
        Py_ssize_t pos = 0;

        while (PyDict_Next(pReturn, &pos, &key, &value)) {
            const char* KeyStr = PyUnicode_AsUTF8(key);
            int cKey = std::stoi(KeyStr);
            int cValue = PyLong_AsLong(value);
            Mem2CtrlStep[cKey] = cValue;
        }
    }

    // i表示转换成int型变量。
    // 在这里，最需要注意的是：PyArg_Parse的最后一个参数，必须加上“&”符号
    // PyArg_Parse(pReturn, "i", &nResult);
    

    // II after graph_color algorithms
    return Mem2CtrlStep;
}

inline int minimumIterDistGen(int coffs[10], int counts[3]){
 
    // 3、调用python文件名，不用写后缀
	PyObject* pModule = PyImport_ImportModule("iterDistGen");
	if( pModule == NULL ){
		std::cout <<"module not found\n";
	}
    // 4、调用函数
	PyObject* pFunc = PyObject_GetAttrString(pModule, "run");
	if( !pFunc || !PyCallable_Check(pFunc)){
		std::cout <<"not found function run\n";
	}
    
    //5、给python传参数
    // 函数调用的参数传递均是以元组的形式打包的,2表示参数个数
    // 如果AdditionFc中只有一个参数时，写1就可以了
    PyObject* pArgs = PyTuple_New(14);

    for(int i = 0; i < 8; i++){
        // 第i个参数，传入 int 类型的值 coffs[i]
        PyTuple_SetItem(pArgs, i, Py_BuildValue("i", coffs[i])); 
    }
    // PyTuple_SetItem(pArgs, 0, Py_BuildValue("i", totalNum)); 
    
    for(int i = 0; i < 3; i++){
        // 第2*i + 5 & 2*i + 6个参数，传入 int 类型的值 coffs[i]
        PyTuple_SetItem(pArgs, 2*i + 8, Py_BuildValue("i", 0)); 
        int Count = counts[i]-1 < 0 ? 0 : counts[i]-1;
        PyTuple_SetItem(pArgs, 2*i + 9, Py_BuildValue("i", Count)); 
    }
    
    // 6、使用C++的python接口调用该函数
    PyObject* pReturn = PyEval_CallObject(pFunc, pArgs);
    
    // 7、接收python计算好的返回值
    int minIterDist;

    PyArg_Parse(pReturn, "i", &minIterDist);

    // i表示转换成int型变量。
    // 在这里，最需要注意的是：PyArg_Parse的最后一个参数，必须加上“&”符号
    // PyArg_Parse(pReturn, "i", &nResult);
    

    // II after graph_color algorithms
    return minIterDist;
}


// add edge between two nodes that have memory dependence (loop-carried)
void LLVMCDFG::addMemDepEdges()
{
    for(auto &elem : _arrayName2LSNode){
        std::vector<LLVMCDFGNode *> LSNodes; // load/store nodes
        errs() << "after handling irregular access of array: " << elem.first << "\n";
        LSNodes.insert(LSNodes.end(), elem.second.first.begin(), elem.second.first.end());
        LSNodes.insert(LSNodes.end(), elem.second.second.begin(), elem.second.second.end());
        int N = LSNodes.size();
        errs() << "N is: " << N << "\n";
        std::set<LLVMCDFGNode*> highestNodes;
        std::set<LLVMCDFGNode*> lowestNodes;
        std::set<LLVMCDFGNode*> interNodes;
        // analyze dependence between every two LSNodes
        for(int i = 0; i < N; i++){
            for(int j = i + 1; j < N; j++){
                LLVMCDFGNode *srcNode = LSNodes[i];
                Instruction *srcIns = srcNode->instruction();
                LLVMCDFGNode *dstNode = LSNodes[j];
                Instruction *dstIns = dstNode->instruction();
                bool isLoopDistConst = false;
                bool reverse = false;
                int loopIterDist = 1; // loop-carried iteration distance, e.g. a[i+1] = a[i] : 1; default = 1
                int inputidx = -1;
                if(auto D = DI->depends(srcIns, dstIns, true)){
                    outs() << "Found memory dependence between " << LSNodes[i]->getName() 
                        << "(src) and " << LSNodes[j]->getName() << "(dst)\n";
                    // outs() << "Levels:" << D->getLevels() <<"\n";
                    DepType type = NON_DEP; // dependence type
                    if(D->isFlow()){ // RAW, read after write
                        type = FLOW_DEP;
                        outs() << "FLOW_DEP, ";
                    }else if(D->isAnti()){ // WAR
                        type = ANTI_DEP;
                        outs() << "ANTI_DEP, ";
                    }else if(D->isOutput()){ // WAW
                        type = OUTPUT_DEP;
                        outs() << "OUTPUT_DEP, ";
                    }else if(D->isInput()){ // RAR (need no dependence)
                        type = INPUT_DEP;
                        outs() << "INPUT_DEP, continue\n";
                        #ifdef INPUT_DUOLICATION
                            continue;
                        #endif
                        continue;
                    }
                    if(D->isConfused() || 
                        (srcNode->getLSArrayName() == dstNode->getLSArrayName() && srcNode->isLSaffine() && dstNode->isLSaffine())){//TODO: How to handle confused conditions
                        outs() << "Confused!---->judge manually\n";
                        bool srcRead = dyn_cast<LoadInst>(srcIns); bool srcWrite = !srcRead;
                        bool dstRead = dyn_cast<LoadInst>(dstIns); bool dstWrite = !dstRead;
                        if(srcWrite && dstRead){ // RAW, read after write
                            type = FLOW_DEP;
                            outs() << "FLOW_DEP, ";
                        }else if(srcRead && dstWrite){ // WAR
                            type = ANTI_DEP;
                            outs() << "ANTI_DEP, ";
                        }else if(srcWrite && dstWrite){ // WAW
                            type = OUTPUT_DEP;
                            outs() << "OUTPUT_DEP, ";
                        }else if(srcRead && dstRead){ // RAR (need no dependence)
                            type = INPUT_DEP;
                            outs() << "INPUT_DEP, continue\n";
                            #ifdef INPUT_DUOLICATION
                                continue;
                            #endif
                            continue;
                        }else{
                            assert(false && "something wrong");
                        }
                        if(srcNode->getLSArrayName() == dstNode->getLSArrayName()){
                            if(srcNode->isLSaffine() && srcNode->isLSaffine()){
                                auto stride0 = srcNode->getLSstride(); auto stride1 = dstNode->getLSstride();
                                auto offset0 = srcNode->getLSstart(); auto offset1 = dstNode->getLSstart();
                                int elemsize = (srcNode->getLSbounds())[2].getNum<int>();
                                bool offsetZero = (offset0 - offset1 == 0); // is all the offset is the same
                                bool allCoeffsZero = true; // is all the coeffs of loop levels the same
                                for(int l = 0; l < stride0.size(); l++){
                                    if(!(stride0[i] - stride1[i] == 0)){
                                        allCoeffsZero = false;
                                        break;
                                    }
                                }
                                if(allCoeffsZero && offsetZero){
                                    errs() << "totally the same address\n";
                                    isLoopDistConst = true;
                                    loopIterDist = 0;
                                    inputidx = -1;
                                }else if(allCoeffsZero){
                                    errs() << "constant distance\n";
                                    isLoopDistConst = true;
                                    loopIterDist = (offset0 - offset1).getNum<int>() / elemsize;
                                    if(loopIterDist < 0){
                                        reverse = true;
                                        loopIterDist = -loopIterDist;
                                        if(type == FLOW_DEP){
                                            type = ANTI_DEP;
                                        }else if(type == ANTI_DEP){
                                            type = FLOW_DEP;
                                        }
                                    }
                                }else{
                                    errs() << "non-constant distance\n";
                                    isLoopDistConst = false;
                                    loopIterDist = 1;
                                }
                            }else{
                                errs() << "can not analyze\n";
                                isLoopDistConst = false;
                                loopIterDist = 1;
                            }
                        }
                    }else{
                        if(D->isFlow()){ // RAW, read after write
                            type = FLOW_DEP;
                            outs() << "FLOW_DEP, ";
                        }else if(D->isAnti()){ // WAR
                            type = ANTI_DEP;
                            outs() << "ANTI_DEP, ";
                        }else if(D->isOutput()){ // WAW
                            type = OUTPUT_DEP;
                            outs() << "OUTPUT_DEP, ";
                        }else if(D->isInput()){ // RAR (need no dependence)
                            type = INPUT_DEP;
                            outs() << "INPUT_DEP, continue\n";
                            #ifdef INPUT_DUOLICATION
                                continue;
                            #endif
                            continue;
                        }
                        outs() << "Not Confused!---->judge by llvm built-in pass\n";
                        if(D->isLoopIndependent()){
                            outs() << "Loop independent\n";
                            if(!D->isConsistent()){//TODO: is this right?
                                outs() << "the dependece is not consistent, continue\n";
                                //continue;
                            }
                            
                            isLoopDistConst = true;
                            loopIterDist = 0;
                            inputidx = -1;
                        }
                        else{    
                            outs() << "Loop carried dependence\n";
                            int nestedLevels = D->getLevels(); // nested loop levels, [1, nestedLevels], 1 is the outer-most loop
                            // assert(nestedLevels > 0);
                            if(nestedLevels == 0){
                                errs() << "[WARNING] CAN NOT analyze memory dependence\n";
                                continue;
                            }
                            // target at the inner-most loop
                            const SCEV *dist = D->getDistance(nestedLevels);
                            const SCEVConstant *distConst = dyn_cast_or_null<SCEVConstant>(dist);
                            if(distConst){
                                loopIterDist = distConst->getAPInt().getSExtValue();
                                outs() << "const distance: " << loopIterDist << "\n";
                                isLoopDistConst = true;
                                if(loopIterDist < 0){
                                    reverse = true;
                                    loopIterDist = -loopIterDist;
                                    if(type == FLOW_DEP){
                                        type = ANTI_DEP;
                                    }else if(type == ANTI_DEP){
                                        type = FLOW_DEP;
                                    }
                                }                   
                            }else{ /// if no subscript in the source or destination mention the induction variable associated with the loop at this level.
                                isLoopDistConst = false;   
                                outs() << "non-const distance\n";                               
                            }                
                        }

                    }

                    
                    if(isLoopDistConst && (loopIterDist == 0)){
                        //dfs to find if there is guarantee of the dependence
                        srcIns->dump(); dstIns->dump();
                        BasicBlock* srcBB = srcIns->getParent();
                        BasicBlock* dstBB = dstIns->getParent();
                        bool src2dst, dst2src;
                        if(srcBB == dstBB){
                            for (auto &I : *srcBB) {
                                if(&I == dstIns){
                                    dst2src = true;
                                    src2dst = false;
                                    break;
                                }
                                if(&I == srcIns){
                                    dst2src = false;
                                    src2dst = true;
                                    break;
                                }
                            }
                        }else{
                            src2dst = _succBBsMap[srcBB].find(dstBB) != _succBBsMap[srcBB].end();
                            dst2src = _succBBsMap[dstBB].find(srcBB) != _succBBsMap[dstBB].end();
                        }
                        
                        // assert((src2dst || dst2src) && "unknow memDep topology");
                        if(!(src2dst || dst2src)){
                            errs() << "the same address accesses are mutually exclusive\n";
                            continue;
                        }
                        
                        if(dst2src){
                            std::swap(srcNode, dstNode);
                            std::swap(srcIns, dstIns);
                            if(type == FLOW_DEP){
                                type = ANTI_DEP;
                            }else if(type == ANTI_DEP){
                                type = FLOW_DEP;
                            }
                        }
                        //to check if the accesses with the same address have true dataflow relationship in original Graph
                        std::vector<LLVMCDFGNode*> toVisit = srcNode->outputNodes();
                        bool hasInloopDep = false;
                        while(!toVisit.empty()){
                            auto temNode = toVisit.back();
                            // errs() << "temNode is: " << temNode->getName() << "\n";
                            toVisit.pop_back();
                            if(temNode == dstNode){
                                hasInloopDep = true;
                                break;
                            }
                            auto temOuts = temNode->outputNodes();
                            for(auto outNode : temOuts){
                                if(!temNode->isOutputBackEdge(outNode)){//to skip the cycle in graph
                                    toVisit.push_back(outNode);
                                }
                            }
                        }
                        //update highest & lowest set of LSNodes                    
                        bool findSrcinH = std::find(highestNodes.begin(), highestNodes.end(), srcNode) != highestNodes.end();
                        bool findSrcinL = std::find(lowestNodes.begin(), lowestNodes.end(), srcNode) != lowestNodes.end();
                        bool findSrcinI = std::find(interNodes.begin(), interNodes.end(), srcNode) != interNodes.end();
                        bool findDstinH = std::find(highestNodes.begin(), highestNodes.end(), dstNode) != highestNodes.end();
                        bool findDstinL = std::find(lowestNodes.begin(), lowestNodes.end(), dstNode) != lowestNodes.end();
                        bool findDstinI = std::find(interNodes.begin(), interNodes.end(), dstNode) != interNodes.end();
                        if(!findSrcinH && !findSrcinL && !findSrcinI){
                            highestNodes.insert(srcNode);
                        }else if(findSrcinL || (findSrcinL && findDstinL)){
                            lowestNodes.erase(srcNode);
                            interNodes.insert(srcNode);
                        }
                        if(!findDstinH && !findDstinL && !findDstinI){
                            lowestNodes.insert(dstNode);
                        }else if(findDstinH || (findSrcinH && findDstinH)){
                            highestNodes.erase(dstNode);
                            interNodes.insert(dstNode);
                        }
                        if(hasInloopDep){
                            outs() << "have memory dependence in loop, need not to connect\n";
                            continue;
                        }
                    }else if(isLoopDistConst){
                        if(type == ANTI_DEP){
                            errs() << "fake dependence, skip\n";
                            continue;
                        }
                    }else{
                        if(type == ANTI_DEP){
                            reverse = true;
                        }
                    }
                    
                    // add mem dep edges
                    DependInfo dep;
                    dep.type = type;
                    dep.isConstDist = isLoopDistConst;
                    dep.distance = loopIterDist;
                    LLVMCDFGNode* trueSrc, * trueDst;
                    if(reverse){
                        trueSrc = dstNode;
                        trueDst = srcNode;
                    }else{
                        trueSrc = srcNode;
                        trueDst = dstNode;
                    }
                    if(_loadList.count(trueDst)){
                        inputidx = 1;
                    }else if(_storeList.count(trueDst)){
                        inputidx = 2;
                    }else{
                        errs() << "[WARNING] no LSlist record\n";
                    }
                    trueSrc->addDstDep(trueDst, dep);
                    trueDst->addSrcDep(trueSrc, dep);
                    if(loopIterDist == 0)
                        connectNodes(trueSrc, trueDst, inputidx, EDGE_TYPE_MEM);
                    else
                        connectNodes(trueSrc, trueDst, inputidx, EDGE_TYPE_MEM, true);
                }
            }
        }
        errs() << "highest nodes: ";
        for(auto elem : highestNodes){
            errs() << elem->getName() << " ";
        }
        errs() << "\nlowest nodes: ";
        for(auto elem : lowestNodes){
            errs() << elem->getName() << " ";
        }
        errs() << "\n";
        DependInfo dep;
        dep.type = FLOW_DEP;
        dep.isConstDist = true;
        dep.distance = 1;
        int inputidx = 1;
        for(auto elemL = lowestNodes.begin(); elemL != lowestNodes.end();){
            auto nodeL = *elemL;
            if(_storeList.count(nodeL) && !nodeL->isLSaffine()){
                for(auto nodeH : highestNodes){
                    if(_storeList.count(nodeH)){
                        dep.type = OUTPUT_DEP;
                        inputidx = 2;
                    }
                    nodeL->addDstDep(nodeH, dep);
                    nodeH->addSrcDep(nodeL, dep);
                    connectNodes(nodeL, nodeH, inputidx, EDGE_TYPE_MEM, true);
                }
                elemL = lowestNodes.erase(elemL);
            }else{
                elemL++;
            }
        }
        for(auto nodeH : highestNodes){
            if(!nodeH->isLSaffine()){
                for(auto nodeL : lowestNodes){
                    if(_loadList.count(nodeL)){
                        continue;
                    }
                    if(_storeList.count(nodeH)){
                        dep.type = OUTPUT_DEP;
                        inputidx = 2;
                    }
                    nodeL->addDstDep(nodeH, dep);
                    nodeH->addSrcDep(nodeL, dep);
                    connectNodes(nodeL, nodeH, inputidx, EDGE_TYPE_MEM, true);
                }
            }
        }


    }

    //8、结束python接口初始化
    Py_Finalize();
}

// explore the memory partition method
void LLVMCDFG::memPartition(){
    std::vector<LLVMCDFGNode*> LSNodes;
    int Level = nestloops().size();
    LSNodes.insert(LSNodes.end(), _loadList.begin(), _loadList.end());
    LSNodes.insert(LSNodes.end(), _storeList.begin(), _storeList.end());
    std::map<std::string, std::vector<LLVMCDFGNode*>> arraynameTale;
    for(auto LSNode : LSNodes){
        auto arrayname = LSNode->getLSArrayName();
        // errs() << "node: " << node->getName() << "; arrayName: " << arrayname << "\n";
        // errs() << "offset: " << LSoffset << "\n";arraynameTale
        arraynameTale[arrayname].push_back(LSNode);
    }

    #ifndef MEM_PARTITION
    for(auto &elem : arraynameTale){
        auto arrayname = elem.first;
        std::vector<LLVMCDFGNode*> confLSNodes = elem.second;
        if(confLSNodes.size() == 1){
            (*confLSNodes.begin())->setMultiPort(0);
        }else{
            for(auto LSNode : confLSNodes){
                LSNode->setMultiPort(1);
            }
        }
    }
    #else
    // 1、初始化python接口  
	Py_Initialize();
	if(!Py_IsInitialized()){
		errs() << "python init fail\n";
	}
    // 2、初始化python系统文件路径，保证可以访问到 .py文件
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('/home/dai-dirk/fdra/app-compiler/llvm-pass/src/Py_Tools')");
    
    for(auto &elem : arraynameTale){
        auto arrayname = elem.first;
        std::vector<LLVMCDFGNode*> confLSNodes = elem.second;
        if(confLSNodes.size() == 1){
            (*confLSNodes.begin())->setMultiPort(0);
            continue;
        }
        int ConfNodesNum = confLSNodes.size();
        errs() << "now handle the conflicts among array: " << arrayname << " totalNum: " << ConfNodesNum << "\n";
        //B,N pairs ---> vector<L/SNode pairs>
        //first: B  second: N
        //L/SNode pairs means the numbers (in this loop) of conflicting memory access
        std::map<std::pair<int, int>, std::vector<std::pair<int, int>>> ConflictTable;
        unsigned minII = 0; unsigned finB = BMAX; unsigned finN = NMAX;
        bool hasNoConflictBN = false;
        for(int N = 1; N <= NMAX && !hasNoConflictBN; N *= 2){
            for(int B = 1; B <= BMAX && !hasNoConflictBN; B *= 2){
                bool noConflict = true;
                for(int i = 0; i < confLSNodes.size(); i++){
                    auto Node0 = confLSNodes[i];
                    for(int j = i+1; j < confLSNodes.size(); j++){
                        bool varFailure = false;
                        auto Node1 = confLSNodes[j];
                        int elemsize = (Node1->getLSbounds())[2].getNum<int>();
                        if(!Node0->isLSaffine() || !Node1->isLSaffine()){
                            errs() << Node0->getName() << " or " << Node1->getName() << " is not affine, skip\n"; 
                            continue;
                        }
                        auto stride0 = Node0->getLSstride(); auto stride1 = Node1->getLSstride();
                        auto offset0 = Node0->getLSstart(); auto offset1 = Node1->getLSstart();
                        int coffs[10] = {0};
                        int counts[3] = {0};
                        for(int l = 0; l < Level; l++){
                            if(stride1[l].index() == 2 || stride0[l].index() == 2 || _loopsAffineCounts[l].index() == 2){
                                varFailure = true;
                                break;
                            }
                            coffs[l] = stride0[l].getNum<int>() / elemsize;
                            coffs[l+4] = stride1[l].getNum<int>() / elemsize;
                            counts[l] = _loopsAffineCounts[l].getNum<int>();
                        }
                        if(offset0.index() == 2 || offset1.index()){
                            varFailure = true;
                        }
                        auto varValue = offset1 - offset0;
                        if(varFailure){
                            errs() << "varType Failure, skip\n"; 
                            continue;
                        }
                        coffs[3] = offset0.getNum<int>() / elemsize;
                        coffs[7] = offset1.getNum<int>() / elemsize;
                        coffs[8] = N;
                        coffs[9] = B;
                        if(conflictpolytope(coffs, counts)){
                            ConflictTable[std::make_pair(B, N)].push_back(std::make_pair(i, j));
                            noConflict = false;
                        }
                    }
                }
                if(noConflict && N != 1){
                    finB = B;
                    finN = N;
                    hasNoConflictBN = true;
                }
            }
        }
        if(hasNoConflictBN){
            errs() << "B_" << finB << " N_" << finN << ": all the mamory accesses do not conflict\n";
            for(auto LSNode : confLSNodes){
                LSNode->setMultiPort(2);
                LSNode->setCtrlStep(0);
            }
            minII = 1;
        }else{
            for(auto &elem : ConflictTable){
                auto BNpair = elem.first;
                auto LSpairs = elem.second;
                errs() << "B_" << BNpair.first << "\tN_" << BNpair.second << "\t: ";
                for(auto LSpair : LSpairs){
                    errs() << LSpair.first << "<->" << LSpair.second << " ";
                }
                int II = graph_color_for_II(ConfNodesNum, LSpairs);
                errs() << " II: " << II << "\n";
                if(II < minII || minII == 0){
                    minII = II;
                    finB = BNpair.first;
                    finN = BNpair.second;
                }else if(II == minII && BNpair.second < finN){
                    finB = BNpair.first;
                    finN = BNpair.second;
                }
            }
            std::map<int, int> Mem2CtrlStep = graph_color_for_CtrlStep(ConfNodesNum, ConflictTable[std::make_pair(finB, finN)]);
            for(auto &elem : Mem2CtrlStep){
                auto LSNode = confLSNodes[elem.first];
                // errs() << "tem Node: " << LSNode->getName() << "\n";
                // errs() << "elem.first: " << elem.first;
                // errs() << " elem.second: " << elem.second;
                LSNode->setMultiPort((finN == 1) ? 1 : 3);
                LSNode->setCtrlStep(elem.second);
            }
        }
        errs() << "ok, now the final values: " << "II_" << minII << " B_" << finB << " N_" << finN << "\n";
        addMemPartition(arrayname, finB, finN);
    }
    //8、结束python接口初始化
    Py_Finalize();
    #endif
}

//nestloop access behavior Analyze
void LLVMCDFG::accessAnalyze(){
	outs() << ">>>>>> analyze Load/Store's behavior\n";
    //find LSU & analyze their access pattern
    for(auto &elem : _nodes){
        auto node = elem.second;
        Instruction *ins = node->instruction();
        GetElementPtrInst *GEP = NULL;
        Value *pointerOp = NULL;
        varType startoffset = 0;
        bool isGEPaffine = true;
        std::map<int, varType> LSstride;
        varType LSbounds[3] = {0, 0, 0};
        int addressidx;
        Type *insType;
        //initial LSstride for each nest loop
        for(int i = 0; i < getLoopsAffineStrides().size(); i++){
            LSstride[i] = 0;
        }
        if(ins == NULL){
            continue;
        }
        if(auto Loadins = dyn_cast<LoadInst>(ins)){
            insType = Loadins->getPointerOperandType();
            pointerOp = Loadins->getPointerOperand();
            addressidx = 0;
        }else if(auto Storeins = dyn_cast<StoreInst>(ins)){
            insType = Storeins->getPointerOperandType();
            pointerOp = Storeins->getPointerOperand();
            addressidx = 1;
        }else{
            continue;
        }
        GEP = dyn_cast<GetElementPtrInst>(pointerOp);
        errs()<<"--------------come accross LSU: "<<node->getName()<<" instruction: ";ins->dump();
        if(node->getInputPort(addressidx) == NULL || node->getInputPort(addressidx)->hasConst()
                            || node->getInputPort(addressidx)->customInstruction() == "INPUT"){
            errs() << node->getName() << "'s address is totally fixed\n";
            node->setLSstart("");
            if(isloopsAffine()){
                node->setLSstride(LSstride);
            }
            auto insTypesize = DL->getTypeAllocSize(insType);
            LSbounds[0] = 0;
            LSbounds[1] = 0;
            LSbounds[2] = (int)insTypesize;
            node->setLSbounds(LSbounds);
            continue;
        }
        ///array element dimension's scale
        bool allCoeffsZerostride = true;
        errs() << "handle LSPattern:\n";
        int elemsize = DL->getTypeAllocSize(insType);
        // if(GEP != NULL){
        //     elemsize = DL->getTypeAllocSize(GEP->getResultElementType());
        // }else{
        //     elemsize = DL->getTypeAllocSize(ins->getType());
        // }
        LSbounds[2] = elemsize;
        errs() << "LS size is " << elemsize <<" \n";
        /*
            here factortable's key is nestloop level; value is for this levelindex
            temporary dimension's stride & bounds: first element represent stride; second element represent boundspair
            for boundspair: first element represent leftbounds; second element represent rightbounds
        */
        std::map<int, std::pair<varType, std::pair<varType,varType>>> factortable;
        ///factortable initialize is must!!
        for(int i = 0; i < getLoopsAffineStrides().size(); i++){
            factortable[i].first = 0;
            factortable[i].second.first = getLoopsAffineBounds(i).first;
            factortable[i].second.second = getLoopsAffineBounds(i).second;
        }

        if(isloopsAffine()){
            enforceprintDOT("beforearrayStride.dot");
            arrayStride(node->getInputPort(addressidx), &factortable);
        }

        if(factortable.empty()){
            errs() << "\t\t[this GEP is not affine]\n";
            isGEPaffine = false;
        }
        else
        {
            for (int i = 0; i < factortable.size(); i++)
            {
                varType left, right;
                varType stride = factortable[i].first;
                left = factortable[i].second.first;
                right = factortable[i].second.second;
                bool BoundExchange = false;
                if (stride.index()==0){
                    if(std::get<int>(stride.value) < 0){
                        BoundExchange = true;
                    }
                }
                else if (stride.index()==1){
                    if(std::get<double>(stride.value) < 0){
                        BoundExchange = true;
                    }
                }
                if(BoundExchange){
                    right = factortable[i].second.first;
                    left = factortable[i].second.second;
                }

                // if (left.index()!=2 && right.index()!=2)
                // {
                //     if(std::get<int>(left.value) > std::get<int>(right.value)){
                //         right = factortable[i].second.first;
                //         left = factortable[i].second.second;
                //     }
                // }
                LSstride[i] += (stride);
                if (!(stride == 0))
                { // if stride, this index is calculated in this dimension
                    allCoeffsZerostride = false;
                    startoffset += (factortable[i].second.first);
                    LSbounds[0] += left;
                    LSbounds[1] += right;
                }
                if(0 > LSbounds[0]){
                    LSbounds[0] = 0;
                }
                errs() << "\t\tin this GEP " << i << " stride: " << factortable[i].first.to_string() << " ; leftbound: " << factortable[i].second.first.to_string() << " ; rightbound: " << factortable[i].second.second.to_string() << "\n";
            }
        }
        varType varLSoffset = node->getLSoffset();
        startoffset += varLSoffset;
        if(allCoeffsZerostride)
            startoffset = "";///assign for totally constant GEP
        if(isGEPaffine){
            node->setLSstride(LSstride);
            node->setLSbounds(LSbounds);
            node->setLSstart(startoffset);
        }
        else{
            Type *currType;
            if(GEP != NULL){
                currType = GEP->getSourceElementType();
            }else{
                errs() << "[WARNING] opaque size for the memory space\n";
                currType = insType;
            }
            auto currTypesize = DL->getTypeAllocSize(currType);
            LSbounds[0] = 0;
            LSbounds[1] = (int)currTypesize-elemsize;
            LSbounds[2] = elemsize;
            node->setLSbounds(LSbounds);
        }
    }

}

 void LLVMCDFG::updateIterDist(){
    std::cout << "--------------updateIterDist start---------------" << std::endl;
    // 1、初始化python接口  
    Py_Initialize();
    if(!Py_IsInitialized()){
        std::cout << "python init fail\n";
    }
    // 2、初始化python系统文件路径，保证可以访问到 .py文件
    PyRun_SimpleString("import sys");
    std::string str1 = "sys.path.append('";
    std::string str2 = PROJECT_PATH;
    std::string str3 = "/llvm-pass/src/Py_Tools')";
    std::string pyCMD = str1 + str2 + str3;
    // std::cout << pyCMD << "\n";
    PyRun_SimpleString(pyCMD.c_str());
    // if(_numPyInit == 0){
    //     // 1、初始化python接口  
    //     Py_Initialize();
    //     if(!Py_IsInitialized()){
    //         std::cout << "python init fail\n";
    //     }
    //     // 2、初始化python系统文件路径，保证可以访问到 .py文件
    //     PyRun_SimpleString("import sys");
    //     std::string str1 = "sys.path.append('";
    //     std::string str2 = PROJECT_PATH;
    //     std::string str3 = "/src/Py_Tools')";
    //     std::string pyCMD = str1 + str2 + str3;
    //     // std::cout << pyCMD << "\n";
    //     PyRun_SimpleString(pyCMD.c_str());
    // }
    int count = 0;
    std::vector<LLVMCDFGEdge*> toDelList; 
    for(auto elem : edges()){
        LLVMCDFGEdge* edge = elem.second;
        if(edge->type() == EdgeType::EDGE_TYPE_MEM){
            LLVMCDFGNode* srcNode = edge->src();
            LLVMCDFGNode* dstNode = edge->dst();
            if(!(srcNode->isLSaffine() && dstNode->isLSaffine())){
                errs() << srcNode->getName() << " or " << dstNode->getName() << " is not affine, skip\n"; 
                continue;
            }
            bool varFailure = false;
            int coffs[8] = {0};
            int counts[3] = {0};
            int Level = nestloops().size();

            auto stride0 = srcNode->getLSstride(); auto stride1 = dstNode->getLSstride();
            auto offset0 = srcNode->getLSstart(); auto offset1 = dstNode->getLSstart();
            int elemsize = (srcNode->getLSbounds())[2].getNum<int>();
            for(int l = 0; l < Level; l++){
                if(stride1[l].index() == 2 || stride0[l].index() == 2 || _loopsAffineCounts[l].index() == 2){
                    varFailure = true;
                    break;
                }
            }
            if(offset0.index() == 2 || offset1.index()){
                varFailure = true;
            }
            if(varFailure){
                errs() << "varType Failure, skip\n"; 
                continue;
            }
            std::vector<std::pair<int, int>> pattern0;
            varType resetstride = 0;
            for(int i = 0; i < stride0.size(); i++){
                pattern0.push_back(std::make_pair((stride0[i]-resetstride).getNum<int>(), std::stoi(getLoopsAffineCounts(i).to_string())));
                // std::cout << (stride0[i]-resetstride).to_string() <<", " << getLoopsAffineCounts(i).to_string();
                resetstride += (getLoopsAffineCounts(i)-1)*stride0[i];
                // if(i < stride0.size()-1)
                //     std::cout << ", ";
            }
            // std::cout << std::endl;
            std::vector<std::pair<int, int>> pattern1;
            resetstride = 0;
            for(int i = 0; i < stride1.size(); i++){
                pattern1.push_back(std::make_pair((stride1[i]-resetstride).getNum<int>(), std::stoi(getLoopsAffineCounts(i).to_string())));
                // std::cout << (stride0[i]-resetstride).to_string() <<", " << getLoopsAffineCounts(i).to_string();
                resetstride += (getLoopsAffineCounts(i)-1)*stride1[i];
                // if(i < stride0.size()-1)
                //     std::cout << ", ";
            }
            // auto getABC = ([] (std::map<int, varType> stride, std::map<int, varType> affineCount){
            //     std::vector<int> result;
            //     int size = stride.size();
            //     if(size == 0){
            //         assert(false && "no pattern?");
            //     }else{
            //         result.push_back(stride[0].getNum<int>());
            //         for(int i = 1; i < size; i++){
            //             result.push_back(stride[i].getNum<int>() - stride[i-1].getNum<int>() + 
            //                                 result.back() * (affineCount[i-1].getNum<int>()-1));
            //         }
            //     }
            //     return result;
            // });
            //@yuan: fix the bug
            auto getABC = ([] (std::vector<std::pair<int, int>> pattern){
                std::vector<int> result;
                int size = pattern.size();
                if(size == 0){
                    assert(false && "no pattern?");
                }else{
                    result.push_back(pattern[0].first);
                    for(int i = 1; i < size; i++){
                        result.push_back(pattern[i].first - pattern[i-1].first + 
                                            result.back() * pattern[i-1].second);
                    }
                }
                return result;
                }
            );
            // auto ABC_Src = getABC(stride0, _loopsAffineCounts);
            // auto ABC_Dst = getABC(stride1, _loopsAffineCounts);
            auto ABC_Src = getABC(pattern0);
            auto ABC_Dst = getABC(pattern1);
            for(int l = 0; l < Level; l++){
                std::cout << "ABC_Src[l]: " << l << " " << ABC_Src[l] << std::endl;
            }
            
            //@yuan_ddp: the input node first
            for(int l = 0; l < Level; l++){
                coffs[l] = ABC_Src[l] / elemsize;
                coffs[l+4] = ABC_Dst[l] / elemsize;
                counts[l] = _loopsAffineCounts[l].getNum<int>();
            }
            coffs[3] = offset0.getNum<int>() / elemsize;
            coffs[7] = offset1.getNum<int>() / elemsize;
            int minIterDist = minimumIterDistGen(coffs, counts);
            std::cout << "Setting iteration distance for edge from: "<<srcNode->getName() << " to " << dstNode->getName() << " with: " << srcNode->getDstDep(dstNode).distance << " -> " <<minIterDist << std::endl; 
            edge->setIterDist(minIterDist);
            if(minIterDist == 0){
                //dfs to find if there is guarantee of the dependence
                auto srcIns = srcNode->instruction();
                auto dstIns = dstNode->instruction();
                srcIns->dump(); dstIns->dump();
                BasicBlock* srcBB = srcIns->getParent();
                BasicBlock* dstBB = dstIns->getParent();
                
                bool src2dst, dst2src;
                if(srcBB == dstBB){
                    for (auto &I : *srcBB) {
                        if(&I == dstIns){
                            dst2src = true;
                            src2dst = false;
                            break;
                        }
                        if(&I == srcIns){
                            dst2src = false;
                            src2dst = true;
                            break;
                        }
                    }
                }else{
                    src2dst = _succBBsMap[srcBB].find(dstBB) != _succBBsMap[srcBB].end();
                    dst2src = _succBBsMap[dstBB].find(srcBB) != _succBBsMap[dstBB].end();
                }
                // assert((src2dst || dst2src) && "unknow memDep topology");
                if(!(src2dst || dst2src)){
                    errs() << "the same address accesses are mutually exclusive\n";
                    continue;
                }
                
                if(dst2src){
                    srcNode->delOutputNode(dstNode); srcNode->delOutputEdge(edge->id());
                    dstNode->delInputNode(srcNode); dstNode->delInputEdge(edge->id());
                    DepType type = srcNode->getDstDep(dstNode).type;
                    srcNode->delDstDep(dstNode); dstNode->delSrcDep(srcNode);

                    std::swap(srcNode, dstNode);
                    std::swap(srcIns, dstIns);

                    srcNode->addOutputNode(dstNode); srcNode->addOutputEdge(edge->id());
                    dstNode->addInputNode(srcNode); dstNode->addInputEdge(edge->id());
                    
                    edge->setSrc(srcNode); edge->setDst(dstNode);

                    DependInfo dep;
                    dep.isConstDist = true;
                    dep.distance = 0;
                    if(type == FLOW_DEP){
                        type = ANTI_DEP;
                    }else if(type == ANTI_DEP){
                        type = FLOW_DEP;
                    }
                    dep.type = type;
                    srcNode->addDstDep(dstNode, dep); dstNode->addSrcDep(srcNode, dep);
                }

                srcNode->setOutputBackEdge(dstNode, false);
                dstNode->setInputBackEdge(srcNode, false);

                //to check if the accesses with the same address have true dataflow relationship in original Graph
                std::vector<LLVMCDFGNode*> toVisit = srcNode->outputNodes();
                bool hasInloopDep = false;
                while(!toVisit.empty()){
                    auto temNode = toVisit.back();
                    // errs() << "temNode is: " << temNode->getName() << "\n";
                    toVisit.pop_back();
                    if(temNode == dstNode){
                        hasInloopDep = true;
                        break;
                    }
                    auto temOuts = temNode->outputNodes();
                    for(auto outNode : temOuts){
                        if(!temNode->isOutputBackEdge(outNode)){//to skip the cycle in graph
                            toVisit.push_back(outNode);
                        }
                    }
                }
                if(hasInloopDep){
                    toDelList.push_back(edge);
                }
            }
        }
    }
    // exit(0);
    //8、结束python接口初始化
    Py_Finalize();
    for(auto &elem : toDelList){
        delEdge(elem);
    }
    std::cout << "--------------updateIterDist end---------------" << std::endl;
}

