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

static PyMethodDef pysc_time_method_def[] = {
    {NULL, NULL, 0, NULL}
};

static PyGetSetDef pysc_time_getset[] = {
    {"us", &pysc_time_get_us, NULL, "Get us from the pysc_time", NULL},
    {"ns", &pysc_time_get_ns, NULL, "Get ns from the pysc_time", NULL},
    {"ps", &pysc_time_get_ps, NULL, "Get ps from the pysc_time", NULL},
    NULL
};

PyTypeObject pysc_time_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    .tp_name = "pysc.time",
    .tp_basicsize = sizeof(pysc_time),
    .tp_itemsize = 0,
    //.tp_dealloc = pysc_time_dealloc,
    .tp_str = &pysc_time_str,
    //.tp_getattro = PyObject_GenericGetAttr,
    //.tp_setattro = PyObject_GenericSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_methods = pysc_time_method_def,
    .tp_getset = pysc_time_getset,
    //.tp_dictoffset = offsetof(pysc_module_s, pdict),
    .tp_init = pysc_time_init,
    .tp_alloc = PyType_GenericAlloc,
    .tp_new = PyType_GenericNew,
    //.tp_free = pysc_module_free,
};


static void setup(PyObject *pysc_module){

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
