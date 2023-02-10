// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#include "py_module.h"
#include <iostream>
#include <Python.h>
#include "pysc.h"
#include "pysc_thread.h"
#include "py_module.h"
#include "pysc_tlm_target_socket.h"
#include "pysc_signal.h"
using namespace std;

extern PyObject *factory_module;

py_module_base::py_module_base(const char* py_class):sc_module(sc_module_name("py")){
    PyObject *o = PyObject_CallMethod(factory_module, "get_class", "s", py_class);
    PyTypeObject *t = (PyTypeObject *)o;
    this->py_instance = (pysc_module*) t->tp_new(t, Py_BuildValue("()"), Py_BuildValue("{}"));
    if(!this->py_instance){
        PyErr_Print();
    }
    Py_INCREF(this->py_instance);
    this->py_instance->module = this;
    t->tp_init((PyObject*)this->py_instance, Py_BuildValue("()"), Py_BuildValue("{}"));
}

py_module_base::~py_module_base(){
    cout << "py_module_base::~py_module_base()" << endl;
    //nothing for now
}
#define SC_STAGE_HOOK(stage) \
void py_module_base::stage(){ \
    PyObject *fname = PyUnicode_FromString(#stage); \
    PyObject *call = PyObject_GetAttr((PyObject*)this->py_instance, fname); \
    if(call){ \
        PyObject *r = PyObject_CallNoArgs(call); \
        if(r==NULL){ \
            PyErr_Print(); \
            return; \
        } \
        Py_DECREF(r); \
        Py_DECREF(call); \
    } else{\
    PyErr_Print();\
    } \
    Py_DECREF(fname); \
}

SC_STAGE_HOOK(end_of_elaboration)
SC_STAGE_HOOK(start_of_simulation)
SC_STAGE_HOOK(end_of_simulation)

#undef SC_STAGE_HOOK

void py_module_base::export_sig(string name, SignalProxyBase *sig){
    pysc_signal *ps_signal = (pysc_signal*) pysc_signal_import(sig); //pysc_signal_Type.tp_alloc(&pysc_signal_Type, 0);
    ps_signal->sig = sig;
    this->py_instance->ob_base.ob_type->tp_setattro((PyObject*)this->py_instance, PyUnicode_FromString(name.c_str()), (PyObject*)ps_signal);
}

void py_module_base::export_target_socket(string name, target_socket_proxy* sock){
    target_sockets[name] = sock;
}

void py_module_base::create_sig(string name, SignalProxyBase* sig){
    py_signals[name] = sig;
}
void py_module_base::add_trace_sig(string name, sc_signal<int>* sig){
    py_trace_signals[name] = sig;
}
target_socket_proxy *py_module_base::get_target_socket(string name){
    return target_sockets[name];
}

void py_module_base::trace(sc_trace_file *tf){
    for(auto const& s: py_trace_signals){
        sc_trace(tf, s.second->read(), name()+("."+s.first));
    }
}

///   Python related functions ///

static PyObject* pysc_module_thread(PyObject *_self, PyObject *args){
    pysc_module *self = (pysc_module*)_self;
    PyObject *run_func = NULL;
    const char *name = NULL;
    
    if (PyArg_ParseTuple(args, "Os", &run_func, &name)==0){
        PyErr_Print();
        return NULL;
    }

    pysc_start_thread(run_func, name);

    Py_RETURN_NONE;
}

static PyObject* pysc_module_set_socket(PyObject *_self, PyObject *args){
    pysc_module *self = (pysc_module*)_self;
    const char *name;
    PyObject *socket = NULL;
        
    if (PyArg_ParseTuple(args, "sO", &name, &socket)==0){
        PyErr_Print();
        return NULL;
    }
    pysc_tlm_target_socket *s = (pysc_tlm_target_socket *)socket;
    self->module->export_target_socket(name, s->socket);

    Py_RETURN_NONE;
}

static PyObject* pysc_module_create_signal(PyObject *_self, PyObject *args){
    pysc_module *self = (pysc_module*)_self;
    const char *name;
    PyArg_ParseTuple(args, "s", &name);
    sc_signal<int> *sig = new sc_signal<int>(name);
    self->module->add_trace_sig(name, sig);
    SignalProxy<int> *sp = new SignalProxy<int>(sig);
    self->module->create_sig(name, sp);
    return pysc_signal_import(sp);
}
static PyObject* pysc_module_abort(PyObject *_self, PyObject *args){
    sc_abort();
    Py_RETURN_NONE;
}

static int pysc_module_init(PyObject *_self, PyObject *args, PyObject *kwds){
    pysc_module *self = (pysc_module*)_self;
    self->pdict = PyDict_New();
    return 0;
}

static void pysc_module_free(void *p){
    cout << "pysc_module_free " << p << endl;
    sc_abort();
    PyObject_Free(p);
}

static void pysc_module_dealloc(PyObject *_self){
    pysc_module *self = (pysc_module*)_self;
    cout << "pysc_module_dealloc " << _self << endl;
    sc_abort();
    PyObject_Del(_self);
}

static PyMethodDef pysc_module_method_def[] = {
    {"SC_THREAD",&pysc_module_thread, METH_VARARGS, "Create a new SC_THREAD"},
    {"set_socket", &pysc_module_set_socket, METH_VARARGS, "Registers a socket, so SystemC can see it and bind to it"},
    {"create_signal", &pysc_module_create_signal, METH_VARARGS, "create a new signal"},
    {"abort", &pysc_module_abort, METH_NOARGS, "calls abort in the ctx of the module (for GDB debug)"},
    {NULL, NULL, 0, NULL}
};

static PyTypeObject pysc_module_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    .tp_name = "pysc.py_module",
    .tp_basicsize = sizeof(pysc_module),
    .tp_itemsize = 0,
    .tp_dealloc = pysc_module_dealloc,
    .tp_getattro = PyObject_GenericGetAttr,
    .tp_setattro = PyObject_GenericSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_methods = pysc_module_method_def,
    .tp_dictoffset = offsetof(pysc_module_s, pdict),
    .tp_init = pysc_module_init,
    .tp_alloc = PyType_GenericAlloc,
    .tp_new = PyType_GenericNew,
    .tp_free = pysc_module_free,
};


static void setup(PyObject *pysc_module){
 
    if(PyType_Ready(&pysc_module_Type)<0){
        sc_abort();
    }
    Py_INCREF(&pysc_module_Type);
    PyModule_AddObject(pysc_module, "py_module", (PyObject*)&pysc_module_Type);
};

static bool is_setup = pysc_add_class(&setup);
