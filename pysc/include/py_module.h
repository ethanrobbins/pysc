// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT


#pragma once

#include <systemc>
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include <Python.h>
#include <map>
#include "pysc_signal.h"
#include "pysc_tlm_target_socket.h"

class py_module;

typedef struct pysc_module_s{
    PyObject_HEAD;
    py_module *module;
    PyObject* pdict;
} pysc_module;

class py_module: public sc_module{
        SC_HAS_PROCESS(py_module);
    public:
        py_module(const char* py_class);
        virtual ~py_module();
        virtual void end_of_elaboration();
        virtual void start_of_simulation();
        virtual void end_of_simulation();

        void export_sig(string name, SignalProxyBase *sig);

        void export_target_socket(string name, target_socket_proxy* sock);
        target_socket_proxy *get_target_socket(string name);

        void create_sig(string name, SignalProxyBase *sig);
        void add_trace_sig(string name, sc_signal<int> *sig);

        void trace(sc_trace_file *tf);
    private:
        PyObject *py_instance;
        pysc_module *pysc_instance;

        std::map<string, target_socket_proxy*> target_sockets;
        std::map<string, SignalProxyBase*> py_signals;
        std::map<string, sc_signal<int>*> py_trace_signals; // This needs to be handled better to support more types
    
};

extern PyTypeObject pysc_module_Type;

#define EXPORT_SIG(SIG, NAME, TYPE) \
{ \
    this->py.export_sig(NAME, new SignalProxy<TYPE>(&SIG)); \
}
