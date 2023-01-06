// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#pragma once

#include <Python.h>
#include <systemc>
using namespace std;
using namespace sc_core;

typedef struct {
    PyObject_HEAD;
    sc_time time;
} pysc_time;

extern PyTypeObject pysc_time_Type;


PyObject *pysc_time_import(sc_time t);
