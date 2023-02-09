// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#include "pysc_event.h"
#include "pysc_time.h"
#include "pysc.h"

static int pysc_event_init(PyObject *_self, PyObject *args, PyObject *kwds){
    pysc_event *self = (pysc_event*) _self;
    self->e.notify_event = new sc_event();
    self->can_trigger = true;
    self->python_owns = true;
    return 0;
}

static void pysc_event_dealloc(PyObject *_self){
    pysc_event *self = (pysc_event*)_self;
    if(self->python_owns){
        delete self->e.notify_event;
        self->e.notify_event = NULL;
    }
    _self->ob_type->tp_free(_self);
}

static PyObject* pysc_event_notify(PyObject *_self, PyObject *args){
    pysc_event *self = (pysc_event*)_self;
    PyObject *t = NULL;
    if (PyArg_ParseTuple(args, "|O", &t)==0){
        PyErr_Print();
        return NULL;
    }
    assert(self->can_trigger);
    if(t){
        pysc_time *ps_time = (pysc_time*)t;
        self->e.notify_event->notify(ps_time->time);
    }else{
        //cout << "DDD notify " << self->e.notify_event << endl;
        self->e.notify_event->notify();
    }
   Py_RETURN_NONE;
}

static PyMethodDef pysc_event_method_def[] = {
//    {"create", &pysc_event_create, METH_O | METH_CLASS, "Create a new sc_event object"},
    {"notify", &pysc_event_notify, METH_VARARGS, "Triggers the event"},
    {NULL, NULL, 0, NULL}
};


PyTypeObject pysc_event_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type,0)
    .tp_name = "pysc.pysc_event",
    .tp_basicsize = sizeof(pysc_event),
    .tp_itemsize = 0,
    .tp_dealloc = pysc_event_dealloc,
    //.tp_getattro = PyObject_GenericGetAttr,
    //.tp_setattro = PyObject_GenericSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_methods = pysc_event_method_def,
    .tp_init = pysc_event_init,
    .tp_alloc = PyType_GenericAlloc,
    .tp_new = PyType_GenericNew,
    //.tp_free = pysc_module_free,
};

static void setup(PyObject *pysc_module){
   pysc_event_Type.tp_methods = pysc_event_method_def;

    PyType_Ready(&pysc_event_Type);
    Py_INCREF(&pysc_event_Type);
    PyModule_AddObject(pysc_module, "pysc_event", (PyObject *)&pysc_event_Type);
 
}

static bool is_setup = pysc_add_class(&setup);
