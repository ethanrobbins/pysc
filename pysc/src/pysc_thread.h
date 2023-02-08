// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#pragma once

#include <Python.h>
#include <systemc>
using namespace sc_core;

class Pysc_thread{
    public:
        Pysc_thread(PyObject *callable, const char *name);
        Pysc_thread();
        virtual ~Pysc_thread();

        void run();
        void pre_block();
        void post_block();

        const char *name;
    private:
        PyObject *callable;
        PyGILState_STATE gil_state;
        PyThreadState *thread_state;
};

Pysc_thread *pysc_get_thread();
void pysc_set_thread(Pysc_thread *th);

void pysc_thread_wait(PyObject *events);
void pysc_start_thread(PyObject *callable, const char* name);

Pysc_thread* pysc_start_dependant_thread();
void pysc_finish_dependant_thread(Pysc_thread* th);
