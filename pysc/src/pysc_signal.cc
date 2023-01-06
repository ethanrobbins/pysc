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


PyTypeObject pysc_signal_Type = {
    PyVarObject_HEAD_INIT(NULL,0)
    "pysc.pysc_signal",
    sizeof(pysc_signal),
};

static PyMethodDef pysc_signal_method_def[] = {
    {"read", &pysc_signal_read, METH_NOARGS, "reads the signal"},
    {"write", &pysc_signal_write, METH_O, "writes the signal"},
    {"event", &pysc_signal_event, METH_NOARGS, "gets the default event for the signal"},
    {"posedge_event", &pysc_signal_posevent, METH_NOARGS, "gets the posedge event for the signal"},
    {"negedge_event", &pysc_signal_negevent, METH_NOARGS, "gets the negedge event for the signal"},
    {NULL, NULL, 0, NULL}
};

static void setup(PyObject *pysc_module){
    pysc_signal_Type.tp_methods = pysc_signal_method_def;

    PyType_Ready(&pysc_signal_Type);
    Py_INCREF(&pysc_signal_Type);
    PyModule_AddObject(pysc_module, "pysc_signal", (PyObject *)&pysc_signal_Type);
 
}

static bool is_setup = pysc_add_class(&setup);
