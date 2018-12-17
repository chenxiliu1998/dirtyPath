//
// Created by hrc on 18-12-3.
//

#ifndef ANALYSE_SEARCHDIRTYDEST_H
#define ANALYSE_SEARCHDIRTYDEST_H

#include "head.h"

void SearchDirtyDest(); // 通过g_vector_module遍历分析模块,赋值g_vector_[func_define|func_call|dirty_position]

#endif //ANALYSE_SEARCHDIRTYDEST_H
