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

class py_module_base;

typedef struct pysc_module_s{
    PyObject_HEAD;
    int foo;
    py_module_base *module;
    int foo2;
    PyObject* pdict;
} pysc_module;


class py_module_base: public sc_module{
        SC_HAS_PROCESS(py_module_base);
    public:
        py_module_base(const char* py_class);
        virtual ~py_module_base();
        virtual void end_of_elaboration();
        virtual void start_of_simulation();
        virtual void end_of_simulation();

        void export_sig(string name, SignalProxyBase *sig);

        void export_target_socket(string name, target_socket_proxy* sock);
        target_socket_proxy *get_target_socket(string name);

        void create_sig(string name, SignalProxyBase *sig);
        void add_trace_sig(string name, sc_signal<int> *sig);

        void trace(sc_trace_file *tf);

        virtual PyObject *get_parent() = 0;
     private:
        pysc_module *py_instance;
        //pysc_module *pysc_instance;

        std::map<string, target_socket_proxy*> target_sockets;
        std::map<string, SignalProxyBase*> py_signals;
        std::map<string, sc_signal<int>*> py_trace_signals; // This needs to be handled better to support more types
    
};


template<typename BASE>
class py_module: public py_module_base{
    public:
        py_module(const char* py_class, BASE *parent):py_module_base(py_class){
            this->parent = parent;
            cout << "py_module<>: " << this << endl;
        }
        virtual ~py_module(){
        }

        BASE *parent;

        virtual PyObject* get_parent(){
            cout << "EEEE pre:" << parent << endl;
            PyObject *p = pysc_proxy(parent, "CORE");
            cout << "EEEE post:" << p << endl;
            Py_INCREF(p);
            return p;
            Py_RETURN_NONE;
        }
        
        static BASE* FOO(PyObject *obj){
            pysc_module *inst = (pysc_module*) obj;
            py_module_base *base = inst->module;
            py_module<BASE> *ext = (py_module<BASE> *) base;

            return ext->parent;
        }

};

/*
class py_module: public py_module_base{
    public:
        py_module(const char* py_class, sc_module *parent):py_module_base(py_class){
            this->parent = parent;
            cout << "py_module<>: " << this << endl;
        }
        virtual ~py_module(){
        }

        static sc_module *get_module(PyObject *_base){
            cout << "get_module " << _base << endl;
            pysc_module *base = (pysc_module*) _base;
            py_module *ext = (py_module*) base->module;
            return ext->parent;
        }

        virtual PyObject *

        sc_module *parent;

};*/
extern PyTypeObject pysc_module_Type;

#define EXPORT_SIG(SIG, NAME, TYPE) \
{ \
    this->py.export_sig(NAME, new SignalProxy<TYPE>(&SIG)); \
}
