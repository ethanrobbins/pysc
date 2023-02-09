// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#include "pysc_signal.h"
#include "pysc.h"

PyObject *pysc_signal_import(SignalProxyBase *t){
    pysc_signal *r = PyObject_New(pysc_signal, &pysc_signal_Type);
    r->sig = t;
    return (PyObject*)PyObject_Init((PyObject*) r, &pysc_signal_Type);
}

static PyObject* pysc_signal_read(PyObject *_self, PyObject *args){
    pysc_signal *self = (pysc_signal*)_self;
    uint32_t i = self->sig->read();
    return PyLong_FromLong(i);
}
static PyObject* pysc_signal_write(PyObject *_self, PyObject *args){
    pysc_signal *self = (pysc_signal*)_self;
    long i = PyLong_AsLong(args);
    self->sig->write(i);
    Py_RETURN_NONE;
}
static PyObject* pysc_signal_event(PyObject *_self, PyObject *args){
    pysc_signal *self = (pysc_signal*)_self;
    return (PyObject*) self->sig->default_event();
}
static PyObject* pysc_signal_posevent(PyObject *_self, PyObject *args){
    pysc_signal *self = (pysc_signal*)_self;
    return (PyObject*) self->sig->pos_event();
}
static PyObject* pysc_signal_negevent(PyObject *_self, PyObject *args){
    pysc_signal *self = (pysc_signal*)_self;
    return (PyObject*) self->sig->neg_event();
}

static PyMethodDef pysc_signal_method_def[] = {
    {"read", &pysc_signal_read, METH_NOARGS, "reads the signal"},
    {"write", &pysc_signal_write, METH_O, "writes the signal"},
    {"event", &pysc_signal_event, METH_NOARGS, "gets the default event for the signal"},
    {"posedge_event", &pysc_signal_posevent, METH_NOARGS, "gets the posedge event for the signal"},
    {"negedge_event", &pysc_signal_negevent, METH_NOARGS, "gets the negedge event for the signal"},
    {NULL, NULL, 0, NULL}
};

PyTypeObject pysc_signal_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    .tp_name = "pysc.py_signal",
    .tp_basicsize = sizeof(pysc_signal),
    .tp_itemsize = 0,
    //.tp_dealloc = pysc_module_dealloc, //TODO this will need to free the systemc type?
    //.tp_getattro = PyObject_GenericGetAttr,
    //.tp_setattro = PyObject_GenericSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_methods = pysc_signal_method_def,
    //.tp_dictoffset = offsetof(pysc_module_s, pdict),
    //.tp_init = pysc_module_init,
    //.tp_alloc = PyType_GenericAlloc,
    //.tp_new = PyType_GenericNew,
    //.tp_free = pysc_module_free,
};

static void setup(PyObject *pysc_module){

    PyType_Ready(&pysc_signal_Type);
    Py_INCREF(&pysc_signal_Type);
    PyModule_AddObject(pysc_module, "pysc_signal", (PyObject *)&pysc_signal_Type);
 
}

static bool is_setup = pysc_add_class(&setup);
