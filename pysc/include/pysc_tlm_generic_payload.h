// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#pragma once

#include <Python.h>
#include <systemc>
#include "tlm.h"
using namespace std;
using namespace sc_core;

typedef struct {
    PyObject_HEAD;
    tlm::tlm_generic_payload *payload;
    bool py_owns_payload;
    // Buffer related stuff
    Py_ssize_t shape[1];
    Py_ssize_t strides[1];
    
} pysc_tlm_generic_payload;

extern PyTypeObject pysc_tlm_generic_payload_Type;


PyObject* pysc_tlm_generic_payload_import(tlm::tlm_generic_payload &payload);
