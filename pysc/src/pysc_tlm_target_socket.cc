// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#include "pysc_tlm_target_socket.h"
#include "pysc.h"
#include "pysc_thread.h"

target_socket_proxy::target_socket_proxy(){
    target__b_transport = NULL;
    socket.register_b_transport(this, &target_socket_proxy::b_transport);
}
target_socket_proxy::~target_socket_proxy(){

}
void target_socket_proxy::b_transport(tlm::tlm_generic_payload &trans, sc_time &delay){
    PyObject *gp = pysc_tlm_generic_payload_import(trans);
    PyObject *t = pysc_time_import(delay);

    if(target__b_transport){
        // check if we are in a systemC thread
        Pysc_thread *dep_thread = NULL;
        if(pysc_get_thread() == NULL){
            //cout << "Createing DEP thread" << endl;
            dep_thread = pysc_start_dependant_thread();
        }
        PyObject *r = PyObject_CallFunction(target__b_transport, "OO", gp, t);
        if(dep_thread){
            pysc_finish_dependant_thread(dep_thread);
        }
        if(r==NULL){
            PyErr_Print();
        }
        //PyObject *r = PyObject_CallFunction(target__b_transport, "");
        Py_DECREF(r);
    }else{
        cout << "Called b_transport without a registered handler..." << endl;
        sc_abort();
    }
    Py_DECREF(gp);
    Py_DECREF(t);
}
// Python code below

int pysc_tlm_target_socket_init(PyObject *_self, PyObject *args, PyObject *kwds){
    cout << "pysc_tlm_target_socket_init " << _self << endl;
    pysc_tlm_target_socket *self = (pysc_tlm_target_socket*)_self;
    self->socket = new target_socket_proxy();
    return 0;
}
void pysc_tlm_target_socket_del(PyObject *_self){
    pysc_tlm_target_socket *self = (pysc_tlm_target_socket*)_self;
}

static PyObject* pysc_tlm_target_socket_register_b_transport(PyObject *_self, PyObject *args){
    pysc_tlm_target_socket *self = (pysc_tlm_target_socket*)_self;
    Py_INCREF(args);
    self->socket->target__b_transport = args;
    Py_RETURN_NONE;
}


static PyMethodDef pysc_tlm_target_socket_method_def[] = {
    {"register_b_transport", &pysc_tlm_target_socket_register_b_transport, METH_O, "Sets the handler for the b_transport call"},
    {NULL, NULL, 0, NULL}
};

PyTypeObject pysc_tlm_target_socket_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    .tp_name = "pysc.tlm_target_socket",
    .tp_basicsize = sizeof(pysc_tlm_target_socket),
    .tp_itemsize = 0,
    //.tp_dealloc = pysc_module_dealloc,
    //.tp_getattro = PyObject_GenericGetAttr,
    //.tp_setattro = PyObject_GenericSetAttr,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_methods = pysc_tlm_target_socket_method_def,
    //.tp_dictoffset = offsetof(pysc_module_s, pdict),
    .tp_init = pysc_tlm_target_socket_init,
    .tp_alloc = PyType_GenericAlloc,
    .tp_new = PyType_GenericNew,
    //.tp_free = pysc_module_free,
};

static void setup(PyObject *pysc_module){
    PyType_Ready(&pysc_tlm_target_socket_Type);
    Py_INCREF(&pysc_tlm_target_socket_Type);
    PyModule_AddObject(pysc_module, "tlm_target_socket", (PyObject *)&pysc_tlm_target_socket_Type);
 
}

static bool is_setup = pysc_add_class(&setup);
