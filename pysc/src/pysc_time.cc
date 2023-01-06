// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#include "pysc_time.h"
#include "pysc.h"

PyObject *pysc_time_import(sc_time t){
    pysc_time *r = PyObject_New(pysc_time, &pysc_time_Type);
    r->time = t;
    return (PyObject*)PyObject_Init((PyObject*) r, &pysc_time_Type);
}

PyObject *pysc_time_get_us(PyObject *self, void*){
    pysc_time *t = (pysc_time*) self;
    uint64_t ticks = t->time.value() / sc_time(1,SC_US).value();

    return PyLong_FromLong(ticks);
}
PyObject *pysc_time_get_ns(PyObject *self, void*){
    pysc_time *t = (pysc_time*) self;
    uint64_t ticks = t->time.value() / sc_time(1,SC_NS).value();

    return PyLong_FromLong(ticks);
}
PyObject *pysc_time_get_ps(PyObject *self, void*){
    pysc_time *t = (pysc_time*) self;
    uint64_t ttt = sc_time(1,SC_PS).value();
    if(ttt==0){
        ttt=1;
    }
    uint64_t ticks = t->time.value() / ttt;//sc_time(1,SC_PS).value();

    return PyLong_FromLong(ticks);
}

int pysc_time_init(PyObject *_self, PyObject *args, PyObject *kwds){
    uint64_t units;
    int scale;
    PyArg_ParseTuple(args, "ki", &units, &scale);
    pysc_time *self = (pysc_time*)_self;
    self->time = sc_core::sc_time(units, sc_core::sc_time_unit(scale));

    return 0;
}

PyObject* pysc_time_str(PyObject *_self){
    pysc_time *self = (pysc_time*)_self;
    PyObject *cnt = PyLong_FromLong(self->time.value());
    PyObject *r = PyObject_Str(cnt);
    Py_DECREF(cnt);
    return r;
}

//TODO add support for creating a new time object

PyTypeObject pysc_time_Type = {
    PyVarObject_HEAD_INIT(NULL,0)
    "pysc.time",
    sizeof(pysc_time),
};

static PyMethodDef pysc_time_method_def[] = {
    {NULL, NULL, 0, NULL}
};

static PyGetSetDef pysc_time_getset[] = {
    {"us", &pysc_time_get_us, NULL, "Get us from the pysc_time", NULL},
    {"ns", &pysc_time_get_ns, NULL, "Get ns from the pysc_time", NULL},
    {"ps", &pysc_time_get_ps, NULL, "Get ps from the pysc_time", NULL},
    NULL
};


static void setup(PyObject *pysc_module){
    pysc_time_Type.tp_getset = pysc_time_getset;
    pysc_time_Type.tp_methods = pysc_time_method_def;
    pysc_time_Type.tp_init = &pysc_time_init;
    pysc_time_Type.tp_str = &pysc_time_str;
    pysc_time_Type.tp_new = PyType_GenericNew;

    PyObject *dict = PyDict_New();
    PyDict_SetItemString(dict, "SC_MS", PyLong_FromLong(SC_MS));
    PyDict_SetItemString(dict, "SC_US", PyLong_FromLong(SC_US));
    PyDict_SetItemString(dict, "SC_NS", PyLong_FromLong(SC_NS));
    PyDict_SetItemString(dict, "SC_PS", PyLong_FromLong(SC_PS));
    pysc_time_Type.tp_dict = dict;

    PyType_Ready(&pysc_time_Type);
    Py_INCREF(&pysc_time_Type);
    PyModule_AddObject(pysc_module, "time", (PyObject *)&pysc_time_Type);
 
}

static bool is_setup = pysc_add_class(&setup);
