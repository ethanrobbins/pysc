// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#pragma once

#include "py_module.h"

extern PyThreadState *top_thread_state;
extern bool pysc_add_class(void (*setup)(PyObject *));
void pysc_initialize(const char *top_module);
void pysc_initialize_lib(PyObject *_top_module);

extern PyModuleDef sim_ModuleDef;

#define TRACE_SIG(F, X) sc_core::sc_trace(F, X, X.name())

PyObject *sim_PyInit();
#define PYSC_SIM \
extern "C" PyObject *PyInit_sim(){ \
    return sim_PyInit(); \
}
