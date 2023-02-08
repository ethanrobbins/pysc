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

/*py_module_base::py_module_base(const char* py_class):sc_module(sc_module_name("py")){
    cout << "Called py_module:" << py_class << endl;
    pysc_instance = PyObject_New(pysc_module, &pysc_module_Type);
    PyObject_Init((PyObject*)pysc_instance, &pysc_module_Type);
    pysc_module_Type.tp_init((PyObject*)pysc_instance, NULL, NULL);
    pysc_instance->module = this;
    this->py_instance = PyObject_CallMethod(factory_module, "get_class", "sO", py_class, pysc_instance);
    Py_INCREF(this->py_instance);
}*/
py_module_base::py_module_base(const char* py_class):sc_module(sc_module_name("py")){
    cout << "Called py_module:" << py_class << endl;
    this->py_instance = (pysc_module*) PyObject_CallMethod(factory_module, "get_class", "s", py_class);
    if(!this->py_instance){
        PyErr_Print();
    }
    Py_INCREF(this->py_instance);
    cout << "DDD: py_module_base:" << this->py_instance << endl;
    this->py_instance->module = this;
}


py_module_base::~py_module_base(){
    cout << "py_module_base::~py_module_base()" << endl;
    //nothing for now
}
#define SC_STAGE_HOOK(stage) \
void py_module_base::stage(){ \
    cout << "STAGE " << #stage << this->py_instance << "  " << this->py_instance->ob_base.ob_type->tp_name << endl; \
    PyObject *fname = PyUnicode_FromString(#stage); \
    PyObject *call = PyObject_GetAttr((PyObject*)this->py_instance, fname); \
    if(call){ \
        cout << "STAGE (call) " << #stage << endl; \
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
    std::cout << "EFR trace" <<endl;
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

static PyObject* pysc_module_get_parent(PyObject *_self, PyObject *args){
    pysc_module *self = (pysc_module*)_self;
    return self->module->get_parent();
}

static int pysc_module_init(PyObject *_self, PyObject *args, PyObject *kwds){
    cout << "DDD: pysc_module_init:" << _self << endl;
    pysc_module *self = (pysc_module*)_self;
    self->pdict = PyDict_New();
    Py_INCREF(self->pdict);
    self->foo = 0;
    self->foo2 = 0;
    return 0;
}

static void pysc_module_free(void *p){
    cout << "pysc_module_free " << p << endl;
    PyObject_Free(p);
}

static void pysc_module_dealloc(PyObject *_self){
    cout << "pysc_module_dealloc " << _self << endl;
    PyObject_Del(_self);
}


PyTypeObject pysc_module_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "pysc.py_module",
    sizeof(pysc_module),
};

static PyMethodDef pysc_module_method_def[] = {
    {"SC_THREAD",&pysc_module_thread, METH_VARARGS, "Create a new SC_THREAD"},
    {"set_socket", &pysc_module_set_socket, METH_VARARGS, "Registers a socket, so SystemC can see it and bind to it"},
    {"create_signal", &pysc_module_create_signal, METH_VARARGS, "create a new signal"},
    {"get_parent", &pysc_module_get_parent, METH_NOARGS, "return a cppyy object of the parent"},
    {NULL, NULL, 0, NULL}
};

static void setup(PyObject *pysc_module){
    pysc_module_Type.tp_init = pysc_module_init;
    pysc_module_Type.tp_methods = pysc_module_method_def;
    pysc_module_Type.tp_dictoffset = offsetof(pysc_module_s, pdict);
    pysc_module_Type.tp_getattro = PyObject_GenericGetAttr;
    pysc_module_Type.tp_setattro = PyObject_GenericSetAttr;
    pysc_module_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    pysc_module_Type.tp_basicsize = sizeof(pysc_module);
    pysc_module_Type.tp_itemsize = 0;
    pysc_module_Type.tp_new = PyType_GenericNew;
    pysc_module_Type.tp_alloc = PyType_GenericAlloc;
    pysc_module_Type.tp_free = pysc_module_free;
    pysc_module_Type.tp_dealloc = pysc_module_dealloc;
    // HERE add the destructors (check that they are not called)

    PyType_Ready(&pysc_module_Type);
    Py_INCREF(&pysc_module_Type);
    PyModule_AddObject(pysc_module, "py_module", (PyObject*)&pysc_module_Type);
};

static bool is_setup = pysc_add_class(&setup);
