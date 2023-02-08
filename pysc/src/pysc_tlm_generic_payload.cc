// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#include "pysc_tlm_generic_payload.h"
#include "pysc.h"

PyObject* pysc_tlm_generic_payload_import(tlm::tlm_generic_payload &payload){
    pysc_tlm_generic_payload *r = PyObject_New(pysc_tlm_generic_payload, &pysc_tlm_generic_payload_Type);
    //pysc_tlm_generic_payload *r = (pysc_tlm_generic_payload*) pysc_tlm_generic_payload_Type.tp_alloc(&pysc_tlm_generic_payload_Type,0);
    r->payload = &payload;
    r->py_owns_payload = false;
    //cout << "GP import::"<<r<<endl;
    PyObject *rr = PyObject_Init((PyObject*)r, &pysc_tlm_generic_payload_Type);
    return rr;
}

int pysc_tlm_generic_payload_init(PyObject *_self, PyObject *args, PyObject *kwds){
    pysc_tlm_generic_payload *self = (pysc_tlm_generic_payload*)_self;
    self->payload = new tlm::tlm_generic_payload();
    self->payload->acquire();
    self->py_owns_payload = true;
    //cout << "GP init::"<<self<<endl;
    return 0;
}
void pysc_tlm_generic_payload_del(PyObject *_self){
    pysc_tlm_generic_payload *self = (pysc_tlm_generic_payload*)_self;
    //cout << "GP del::" << _self << endl;
    if(self->py_owns_payload){
        // cleanup the data storage memory?
        unsigned char *ptr = self->payload->get_data_ptr();
        if(ptr){
            delete[] ptr;
        }
        delete self->payload;
    }
    Py_TYPE(self)->tp_free(self);
}

PyObject* pysc_tlm_generic_payload_str(PyObject *_self){
    pysc_tlm_generic_payload *self = (pysc_tlm_generic_payload*)_self;
    //PyObject *cnt = PyLong_FromLong(self->time.value());
    //TODO
    //PyObject *r = PyObject_Str(42);
    //Py_DECREF(cnt);
    //return r;
    Py_RETURN_NONE;
}

static PyObject* pysc_tlm_generic_payload_set_data(PyObject *_self, PyObject *args){
    pysc_tlm_generic_payload *self = (pysc_tlm_generic_payload*)_self;
    Py_buffer buffer;
    sc_abort();
     
    if (PyArg_ParseTuple(args, "y*", &buffer)==0){
        PyErr_Print();
        return NULL;
    }
    if(self->py_owns_payload){
        if(self->payload->get_data_ptr()){
            cout << "For py owned gp, the ptr is already set..." << endl;
            sc_abort();
        }
        unsigned char *ptr = new unsigned char[buffer.len];
        self->payload->set_data_ptr(ptr);
        self->payload->set_data_length(buffer.len);
        PyBuffer_ToContiguous(self->payload->get_data_ptr(), &buffer, buffer.len, 'C');
        PyBuffer_Release(&buffer);
    }else{
        //check that the length matches
        if(buffer.len != self->payload->get_data_length()){
            cout << "payload lenth does not match...." << endl;
            sc_abort();
        }
        PyBuffer_ToContiguous(self->payload->get_data_ptr(), &buffer, buffer.len, 'C');
        PyBuffer_Release(&buffer);
    }
    Py_RETURN_NONE;
}

static PyObject* pysc_tlm_generic_payload_get_data(PyObject *_self, PyObject *args){
    pysc_tlm_generic_payload *self = (pysc_tlm_generic_payload*)_self;
 
    return PyMemoryView_FromObject(_self);
}

static PyObject* pysc_tlm_generic_payload_set_address(PyObject *_self, PyObject *args){
    pysc_tlm_generic_payload *self = (pysc_tlm_generic_payload*)_self;
    self->payload->set_address(PyLong_AsLong(args));
    Py_RETURN_NONE;
}
static PyObject* pysc_tlm_generic_payload_get_address(PyObject *_self, PyObject *args){
    pysc_tlm_generic_payload *self = (pysc_tlm_generic_payload*)_self;
    return PyLong_FromUnsignedLongLong(self->payload->get_address());
}
static PyObject* pysc_tlm_generic_payload_set_command(PyObject *_self, PyObject *args){
    pysc_tlm_generic_payload *self = (pysc_tlm_generic_payload*)_self;
    self->payload->set_command((tlm::tlm_command)PyLong_AsLong(args));
    Py_RETURN_NONE;
}
static PyObject* pysc_tlm_generic_payload_get_command(PyObject *_self, PyObject *args){
    pysc_tlm_generic_payload *self = (pysc_tlm_generic_payload*)_self;
    return PyLong_FromLong(self->payload->get_command());
}
static PyObject* pysc_tlm_generic_payload_set_response_status(PyObject *_self, PyObject *args){
    pysc_tlm_generic_payload *self = (pysc_tlm_generic_payload*)_self;
    self->payload->set_response_status((tlm::tlm_response_status)PyLong_AsLong(args));
    Py_RETURN_NONE;
}

PyTypeObject pysc_tlm_generic_payload_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type,0)
    "pysc.tlm_generic_payload",
    sizeof(pysc_tlm_generic_payload),
};

static PyMethodDef pysc_tlm_generic_payload_method_def[] = {
    {"set_data", &pysc_tlm_generic_payload_set_data, METH_VARARGS, "Sets the data/len part of the generic payload"},
    {"get_data", &pysc_tlm_generic_payload_get_data, METH_NOARGS, "Gets the data/len part of the generic payload"},
    {"set_address", &pysc_tlm_generic_payload_set_address, METH_O, "Sets the address"},
    {"get_address", &pysc_tlm_generic_payload_get_address, METH_NOARGS, "Gets the address"},
    {"set_command", &pysc_tlm_generic_payload_set_command, METH_O, "Sets the command"},
    {"get_command", &pysc_tlm_generic_payload_get_command, METH_NOARGS, "Gets the command"},
    {"set_response_status", &pysc_tlm_generic_payload_set_response_status, METH_O, "Sets the response_status"},
    {NULL, NULL, 0, NULL}
};

int pysc_tlm_generic_payload_getbuffer(PyObject *_self, Py_buffer *view, int flags){
    pysc_tlm_generic_payload *self = (pysc_tlm_generic_payload*)_self;
    view->buf = self->payload->get_data_ptr();
    view->obj = _self;
    view->len = self->payload->get_data_length();
    view->readonly = 0;
    view->itemsize = 1;
    view->format = NULL;
    view->ndim = 1;
    self->shape[0] = view->len;
    view->shape = self->shape;
    self->strides[0] = 1;
    view->strides = self->strides;
    view->suboffsets = NULL;
    view->internal = NULL;
    Py_INCREF(_self);
    //cout << "BUF_get: " << view << endl;
    return 0;
}
void pysc_tlm_generic_payload_releasebuffer(PyObject *_self, Py_buffer *view){
    //cout << "BUF_rel: " << view << " obj:" << _self << " view->obj: " << view->obj << endl;
    //Py_DECREF(_self);
}

PyBufferProcs pysc_tlm_generic_payload_buffer_def{
    pysc_tlm_generic_payload_getbuffer,
    pysc_tlm_generic_payload_releasebuffer
};


static void setup(PyObject *pysc_module){
    pysc_tlm_generic_payload_Type.tp_methods = pysc_tlm_generic_payload_method_def;
    pysc_tlm_generic_payload_Type.tp_init = &pysc_tlm_generic_payload_init;
    pysc_tlm_generic_payload_Type.tp_str = &pysc_tlm_generic_payload_str;
    pysc_tlm_generic_payload_Type.tp_new = PyType_GenericNew;
    pysc_tlm_generic_payload_Type.tp_dealloc = &pysc_tlm_generic_payload_del;
    pysc_tlm_generic_payload_Type.tp_as_buffer = &pysc_tlm_generic_payload_buffer_def;

    PyObject *dict = PyDict_New();
    PyDict_SetItemString(dict, "TLM_READ_COMMAND", PyLong_FromLong(tlm::TLM_READ_COMMAND));
    PyDict_SetItemString(dict, "TLM_WRITE_COMMAND", PyLong_FromLong(tlm::TLM_WRITE_COMMAND));
    PyDict_SetItemString(dict, "TLM_IGNORE_COMMAND", PyLong_FromLong(tlm::TLM_IGNORE_COMMAND));
    PyDict_SetItemString(dict, "TLM_OK_RESPONSE", PyLong_FromLong(tlm::TLM_OK_RESPONSE));
    PyDict_SetItemString(dict, "TLM_INCOMPLETE_RESPONSE", PyLong_FromLong(tlm::TLM_INCOMPLETE_RESPONSE));
    PyDict_SetItemString(dict, "TLM_GENERIC_ERROR_RESPONSE", PyLong_FromLong(tlm::TLM_GENERIC_ERROR_RESPONSE));
    PyDict_SetItemString(dict, "TLM_ADDRESS_ERROR_RESPONSE", PyLong_FromLong(tlm::TLM_ADDRESS_ERROR_RESPONSE));
    PyDict_SetItemString(dict, "TLM_COMMAND_ERROR_RESPONSE", PyLong_FromLong(tlm::TLM_COMMAND_ERROR_RESPONSE));
    PyDict_SetItemString(dict, "TLM_BURST_ERROR_RESPONSE", PyLong_FromLong(tlm::TLM_BURST_ERROR_RESPONSE));
    PyDict_SetItemString(dict, "TLM_BYTE_ENABLE_ERROR_RESPONSE", PyLong_FromLong(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE));
    pysc_tlm_generic_payload_Type.tp_dict = dict;

    PyType_Ready(&pysc_tlm_generic_payload_Type);
    Py_INCREF(&pysc_tlm_generic_payload_Type);
    PyModule_AddObject(pysc_module, "tlm_generic_payload", (PyObject *)&pysc_tlm_generic_payload_Type);
 
}

static bool is_setup = pysc_add_class(&setup);
