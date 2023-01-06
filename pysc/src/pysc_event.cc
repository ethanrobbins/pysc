// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#include "pysc_event.h"
#include "pysc_time.h"
#include "pysc.h"

static PyObject* pysc_event_create(PyObject *cls, PyObject *args){
    pysc_event *e = (pysc_event*) pysc_event_Type.tp_alloc(&pysc_event_Type, 0);
    e->e.notify_event = new sc_event();
    e->can_trigger = true;
    return (PyObject*)e;
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


PyTypeObject pysc_event_Type = {
    PyVarObject_HEAD_INIT(NULL,0)
    "pysc.pysc_event",
    sizeof(pysc_event),
};

static PyMethodDef pysc_event_method_def[] = {
    {"create", &pysc_event_create, METH_O | METH_CLASS, "Create a new sc_event object"},
    {"notify", &pysc_event_notify, METH_VARARGS, "Triggers the event"},
    {NULL, NULL, 0, NULL}
};

static void setup(PyObject *pysc_module){
   pysc_event_Type.tp_methods = pysc_event_method_def;

    PyType_Ready(&pysc_event_Type);
    Py_INCREF(&pysc_event_Type);
    PyModule_AddObject(pysc_module, "pysc_event", (PyObject *)&pysc_event_Type);
 
}

static bool is_setup = pysc_add_class(&setup);
