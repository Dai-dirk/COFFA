#ifndef __PY_INVOKE_H__
#define __PY_INVOKE_H__

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <Python.h>

// invoke python conflictpolytope.py
// 0: no conflict;
// 1: true conflict
int conflictpolytope(int coffs[10], int counts[3]);
int conflictpolytope_same_step(int coffs[11], int counts[3]);
int graph_color_for_II(int totalNum, std::vector<std::pair<int, int>> LSpairs);
std::map<int, int> graph_color_for_CtrlStep(int totalNum, std::vector<std::pair<int, int>> LSpairs);
int minimumIterDistGen(int coffs[10], int counts[3]);
#endif