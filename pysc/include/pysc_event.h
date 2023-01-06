// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#pragma once

#include <Python.h>
#include <systemc>
using namespace std;
using namespace sc_core;

typedef struct {
    PyObject_HEAD;
    union {
        const sc_event *event;
        sc_event *notify_event;
    }e;
    bool can_trigger;
} pysc_event;

extern PyTypeObject pysc_event_Type;

