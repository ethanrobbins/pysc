// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#include <Python.h>
#include <iostream>
#include <systemc>
#include <list>
using namespace std;
using namespace sc_core;

#include "pysc_time.h"
#include "pysc_thread.h"

#include "CPyCppyy/API.h"

PyObject *factory_module;

extern "C" PyObject* pysc_proxy(void *obj, string t){
    return CPyCppyy::Instance_FromVoidPtr(obj, t, false);
}

/// pysc module ///
static PyObject* pysc_wait(PyObject *self, PyObject *args){
    pysc_thread_wait(args);
    Py_RETURN_NONE;
}

static PyObject* pysc_sc_time_stamp(PyObject *self, PyObject *args){
    pysc_time *t = (pysc_time*) pysc_time_Type.tp_alloc(&pysc_time_Type, 0);
    t->time = sc_time_stamp();

    return (PyObject*) t;
} 

static PyObject* pysc_stop(PyObject *self, PyObject *args){
    sc_stop();

    Py_RETURN_NONE;
} 

//static PyObject* pysc_print(PyObject *self, PyObject *args){
//    Py
//}

static PyMethodDef pysc_method_def[] = {
    {"wait", &pysc_wait, METH_O, "In the context of a thread, suspends execution until an event (from args occures) (standard systemc wait)"},
    {"cur_time", &pysc_sc_time_stamp, METH_NOARGS, "Get the current timestamp"},
    {"stop", &pysc_stop, METH_NOARGS, "Stops the simulation"},
    {NULL, NULL, 0, NULL}
};
static struct PyModuleDef pysc_module_def = {
    PyModuleDef_HEAD_INIT,
    "pysc",
    "Python interface to SystemC",
    -1,
    pysc_method_def
};


/// Initilizer code ///
list<void (*)(PyObject *)> *class_setup_list;
PyThreadState *top_thread_state;
static bool pysc_initialized = false;
void pysc_initialize(const char *top_module_name){
    if(pysc_initialized){
        return;
    }
    Py_Initialize();
    PyInterpreterState *intr_state = PyInterpreterState_Get();
    top_thread_state = PyThreadState_New(intr_state);
 
    PyRun_SimpleString("import sys,os\nsys.path.append(os.getcwd())");
 
    PyObject *pysc_module = PyModule_Create(&pysc_module_def);
    for(auto it: *class_setup_list){
        it(pysc_module);
    }


    PyObject *sys_modules = PyImport_GetModuleDict();
    PyDict_SetItemString(sys_modules, "pysc", pysc_module);
    Py_DECREF(pysc_module);
    Py_DECREF(sys_modules);

    PyObject *top_module = PyImport_ImportModule(top_module_name);
    if(top_module==NULL){
        PyErr_Print();
        cout << "Error loading:" << top_module_name << endl;
    }

    factory_module = PyObject_CallMethod(top_module, "get_factory", "");
 
    if(factory_module==NULL){
        PyErr_Print();
        cout << "Error loading:" << top_module_name << endl;
    }
    Py_INCREF(factory_module);
    pysc_initialized = true;
}

void pysc_initialize_lib(){
    PyInterpreterState *intr_state = PyInterpreterState_Get();
    top_thread_state = PyThreadState_Get();//PyThreadState_New(intr_state);

    // setup cppyy
    //PyObject *cppyy = PyImport_ImportModule("cppyy");
//    CPyCppyy::Import("cppyy");
    //PyRun_SimpleString("cppyy.add_include_path(\"../../systemc/install/include\")");
    //PyRun_SimpleString("cppyy.add_include_path(\"src\")");
    //PyRun_SimpleString("cppyy.add_include_path(\"../../pysc/include\")");
    //PyRun_SimpleString("#cppyy.include(\"systemc.h\")");
    //PyRun_SimpleString("cppyy.include(\"core.h\")");

    PyObject *pysc_module = PyModule_Create(&pysc_module_def);
    for(auto it: *class_setup_list){
        it(pysc_module);
    }

    PyObject *sys_modules = PyImport_GetModuleDict();
    PyDict_SetItemString(sys_modules, "pysc", pysc_module);
    Py_DECREF(pysc_module);
    Py_DECREF(sys_modules);

 }

bool pysc_add_class(void (*setup)(PyObject *)){
    if(class_setup_list==NULL){
        class_setup_list = new list<void (*)(PyObject *)>();
    }
    class_setup_list->push_back(setup);
    return true;
}

static PyObject* pysc_sim_start(PyObject *_self, PyObject *args){
    PyObject *sc_main_args = NULL;
    cout << "SIM:: start" << endl;
    PyArg_ParseTuple(args,"|O", &sc_main_args);
    int nargs = 0;
    char * *argv;
    if(sc_main_args){
        nargs = PyList_Size(sc_main_args);
        argv = new char*[nargs];
        for(int i=0;i<nargs;i++){
            PyObject *t = PyList_GetItem(sc_main_args, i);
            Py_ssize_t len;
            const char *s = PyUnicode_AsUTF8AndSize(t,&len);
            argv[i] = new char[len+1];
            strncpy(argv[i],s,len+1);
        }
        sc_elab_and_sim(nargs, argv);

        //sc_main(nargs, argv);
    }else{
        sc_main(0, NULL);
    }
    cout << "SIM:: done" << endl;

    Py_RETURN_NONE;
}
static PyObject* pysc_sim_restart(PyObject *_self, PyObject *args){
    sc_get_curr_simcontext()->reset();
    Py_RETURN_NONE;
}

static PyObject* pysc_sim_set_factory_module(PyObject *_self, PyObject *args){
    factory_module = args;
    Py_INCREF(factory_module);
    pysc_initialized = true;

    cout << "SIM:: set_factory_module" << endl;
    Py_RETURN_NONE;
}
static PyObject* pysc_sim_abort(PyObject *_self, PyObject *args){
    sc_abort();
    Py_RETURN_NONE;
}


static PyMethodDef sim_Method[] = {
    {"start", &pysc_sim_start, METH_VARARGS, "Starts a simulation"},
    {"restart", &pysc_sim_restart, METH_NOARGS, "Restart a simulation"},
    {"abort", &pysc_sim_abort, METH_NOARGS, "Calls sc_abort (so you can debug in GDB)"},
    {"set_factory_module", &pysc_sim_set_factory_module, METH_O, "Tells systemC how to find other modules"},
    {NULL,NULL,0,NULL}
};

PyModuleDef sim_ModuleDef = {
    PyModuleDef_HEAD_INIT,
    "sim",
    "Top level SystemC model for starting a simulation",
    0,
    sim_Method,
    NULL,
    NULL,
    NULL,
    NULL
};

PyObject *sim_PyInit(){ \
    pysc_initialize_lib(); \
    return  PyModule_Create(&sim_ModuleDef); \
}
