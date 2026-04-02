#include "Py_Tools/py_invoke.h"

// invoke python conflictpolytope.py
// 0: no conflict;
// 1: true conflict
int conflictpolytope(int coffs[10], int counts[3]){
 
    // 3、调用python文件名，不用写后缀
	PyObject* pModule = PyImport_ImportModule("conflictpolytope");
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
        int Count = counts[i]-1 < 0 ? 0 : counts[i]-1;
        PyTuple_SetItem(pArgs, 2*i + 11, Py_BuildValue("i", Count)); 
    }
    
    
    // 6、使用C++的python接口调用该函数
    PyObject* pReturn = PyEval_CallObject(pFunc, pArgs);
    
    // 7、接收python计算好的返回值
    int nResult;
    // i表示转换成int型变量。
    // 在这里，最需要注意的是：PyArg_Parse的最后一个参数，必须加上“&”符号
    PyArg_Parse(pReturn, "i", &nResult);
    // std::cout << "return result is " << nResult << "\n";


    // 0: no conflict;
    // 1: true conflict
    return nResult;
}

// invoke python conflictpolytope_same_step.py
// 0: no conflict;
// 1: true conflict
int conflictpolytope_same_step(int coffs[11], int counts[3]){
 
    // 3、调用python文件名，不用写后缀
	PyObject* pModule = PyImport_ImportModule("conflictpolytope_same_step");
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
    PyObject* pArgs = PyTuple_New(17);
    
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
        int Count = counts[i]-1 < 0 ? 0 : counts[i]-1;
        PyTuple_SetItem(pArgs, 2*i + 11, Py_BuildValue("i", Count)); 
    }

    PyTuple_SetItem(pArgs, 16, Py_BuildValue("i", coffs[10])); 

    
    
    // 6、使用C++的python接口调用该函数
    PyObject* pReturn = PyEval_CallObject(pFunc, pArgs);
    
    // 7、接收python计算好的返回值
    int nResult;
    // i表示转换成int型变量。
    // 在这里，最需要注意的是：PyArg_Parse的最后一个参数，必须加上“&”符号
    PyArg_Parse(pReturn, "i", &nResult);
    // std::cout << "return result is " << nResult << "\n";


    // 0: no conflict;
    // 1: true conflict
    return nResult;
}

int graph_color_for_II(int totalNum, std::vector<std::pair<int, int>> LSpairs){
 
    // 3、调用python文件名，不用写后缀
	PyObject* pModule = PyImport_ImportModule("graph_color");
	if( pModule == NULL ){
		std::cout <<"module not found\n";
	}
    // 4、调用函数
	PyObject* pFunc = PyObject_GetAttrString(pModule, "runII");
	if( !pFunc || !PyCallable_Check(pFunc)){
		std::cout <<"not found function run\n";
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
        // std::cout << LSpairs[i].first << ", " << LSpairs[i].second  << "; ";
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
    // std::cout << "return result is " << nResult << "\n";

    // II after graph_color algorithms
    return nResult;
}

std::map<int, int> graph_color_for_CtrlStep(int totalNum, std::vector<std::pair<int, int>> LSpairs){
 
    // 3、调用python文件名，不用写后缀
	PyObject* pModule = PyImport_ImportModule("graph_color");
	if( pModule == NULL ){
		std::cout <<"module not found\n";
	}
    // 4、调用函数
	PyObject* pFunc = PyObject_GetAttrString(pModule, "runCtrlStep");
	if( !pFunc || !PyCallable_Check(pFunc)){
		std::cout <<"not found function run\n";
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
        // std::cout << LSpairs[i].first << ", " << LSpairs[i].second  << "; ";
        PyTuple_SetItem(pArgs, i*2+1, Py_BuildValue("i", LSpairs[i].first)); 
        PyTuple_SetItem(pArgs, i*2+2, Py_BuildValue("i", LSpairs[i].second)); 
    }
    
    // 6、使用C++的python接口调用该函数
    PyObject* pReturn = PyEval_CallObject(pFunc, pArgs);
    
    // 7、接收python计算好的返回值
    std::map<int, int> Mem2CtrlStep;

    if (PyDict_Check(pReturn)) {
        // std::cout << "PyDict YES!\n";
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
