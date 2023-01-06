// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#pragma once

#include <Python.h>
#include <systemc>
#include "pysc_event.h"
using namespace std;
using namespace sc_core;


class SignalProxyBase{
    public:
        SignalProxyBase(){
        }
        virtual ~SignalProxyBase(){
        }

        // Access function
        virtual uint32_t read() = 0;
        virtual void write(uint32_t value) = 0;
        //TODO add support for wider signals

        // Get event
        virtual pysc_event *default_event() = 0;
        virtual pysc_event *pos_event(){
            return NULL;
        }
        virtual pysc_event *neg_event(){
            return NULL;
        }
    protected:
        enum {SIG,IN,OUT} sig_type;
};

PyObject *pysc_signal_import(SignalProxyBase *t);

template<class TYPE>
class SignalProxy: public SignalProxyBase{
    public:
        SignalProxy(sc_signal<TYPE> *signal){
            this->s.signal = signal;
            this->sig_type = SIG;
        }
        SignalProxy(sc_in<TYPE> *in){
            this->s.in_port = in;
            this->sig_type = IN;
        }
        SignalProxy(sc_out<TYPE> *out){
            this->s.out_port = out;
            this->sig_type = OUT;
        }
        virtual ~SignalProxy(){
        }
        virtual uint32_t read(){
            switch(sig_type){
                case SIG:
                    return s.signal->read().to_int();
                    break;
                case IN:
                    return s.in_port->read().to_int();
                    break;
                case OUT:
                    return s.out_port->read().to_int();
                    break;
                default:
                    cout << "Something is not right, NULL pointer to sig/in/out" << endl;
                    sc_abort();
            }
        }
        virtual void write(uint32_t value){
            switch(sig_type){
                 case SIG:
                    s.signal->write(value);
                    break;
                case IN:
                    cout << "Can't write to an input port " << endl;
                    sc_abort();
                    break;
                case OUT:
                    s.out_port->write(value);
                    break;
                default:
                    cout << "Something is not right, NULL pointer to sig/in/out" << endl;
                    sc_abort();             
            }
        }
        virtual pysc_event *default_event(){
            const sc_event *e;
            switch(sig_type){
                 case SIG:
                    e = &s.signal->default_event();
                    break;
                case IN:
                    e = &s.in_port->default_event();
                    break;
                case OUT:
                    e = &s.out_port->default_event();
                    break;
                default:
                    cout << "Something is not right, NULL pointer to sig/in/out" << endl;
                    sc_abort();             
            }
            pysc_event *r = (pysc_event*) pysc_event_Type.tp_alloc(&pysc_event_Type, 0);
            r->e.event = e;
            r->can_trigger = false;
            return r;
        }
        virtual pysc_event *pos_event(){
            // not supported
            return NULL;
        }
        virtual pysc_event *neg_event(){
            return NULL;
        }

    private:
        union {
            sc_signal<TYPE> *signal;
            sc_in<TYPE> *in_port;
            sc_out<TYPE> *out_port;
        } s;
};

#define DEF_SIG_PROXY(TYPE,RD_CODE,WR_CODE)  \
template<>  \
class SignalProxy<TYPE>: public SignalProxyBase{  \
    public:  \
        SignalProxy(sc_signal<TYPE> *signal){  \
            this->s.signal = signal;  \
            sig_type = SIG;  \
        }  \
        SignalProxy(sc_in<TYPE> *in){  \
            this->s.in_port = in;  \
            sig_type = IN;  \
        }  \
        SignalProxy(sc_out<TYPE> *out){  \
            this->s.out_port = out;  \
            sig_type = OUT;  \
        }  \
        virtual ~SignalProxy(){  \
        }  \
        virtual uint32_t read(){  \
            switch(sig_type){  \
                case SIG:  \
                    return s.signal->read() RD_CODE;  \
                    break;  \
                case IN:  \
                    return s.in_port->read() RD_CODE;  \
                    break;  \
                case OUT:  \
                    return s.out_port->read() RD_CODE;  \
                    break;  \
                default:  \
                    cout << "Something is not right, NULL pointer to sig/in/out" << endl;  \
                    sc_abort();  \
            }     \
        }  \
        virtual void write(uint32_t value){  \
            switch(sig_type){  \
                case SIG:  \
                    s.signal->write(WR_CODE);  \
                    break;  \
                case IN:  \
                    cout << "Can't write to an input port " << endl;  \
                    sc_abort();  \
                    break;  \
                case OUT:  \
                    s.out_port->write(WR_CODE);  \
                    break;  \
                default:  \
                    cout << "Something is not right, NULL pointer to sig/in/out" << endl;  \
                    sc_abort();  \
            }  \
        }  \
        virtual pysc_event *default_event(){  \
            const sc_event *e;  \
            switch(sig_type){  \
                case SIG:  \
                    e = &s.signal->default_event();  \
                    break;  \
                case IN:  \
                    e = &s.in_port->default_event();  \
                    break;  \
                case OUT:  \
                    e = &s.out_port->default_event();  \
                    break;  \
                default:  \
                    cout << "Something is not right, NULL pointer to sig/in/out" << endl;  \
                    sc_abort();  \
            }  \
            pysc_event *r = (pysc_event*) pysc_event_Type.tp_alloc(&pysc_event_Type, 0);  \
            r->e.event = e;  \
            r->can_trigger = false;  \
            return r;  \
        }  \
    private:  \
        union {  \
            sc_signal<TYPE> *signal;  \
            sc_in<TYPE> *in_port;  \
            sc_out<TYPE> *out_port;  \
        } s;  \
};  \


//DEF_SIG_PROXY(bool, +0, value!=0)
DEF_SIG_PROXY(int, +0, value)

template<>
class SignalProxy<bool>: public SignalProxyBase{
    public:
        SignalProxy(sc_signal<bool> *signal){
            this->s.signal = signal;
            sig_type = SIG;
        }
        SignalProxy(sc_in<bool> *in){
            this->s.in_port = in;
            sig_type = IN;
        }
        SignalProxy(sc_out<bool> *out){
            this->s.out_port = out;
            sig_type = OUT;
        }
        virtual ~SignalProxy(){
        }
        virtual uint32_t read(){
            switch(sig_type){
                case SIG:
                    return s.signal->read() ?1:0;
                    break;
                case IN:
                    return s.in_port->read() ?1:0;
                    break;
                case OUT:
                    return s.out_port->read() ?1:0;
                    break;
                default:
                    cout << "Something is not right, NULL pointer to sig/in/out" << endl;
                    sc_abort();
            } 
        } 
        virtual void write(uint32_t value){
            switch(sig_type){
                case SIG:
                    s.signal->write(value!=0);
                    break;
                case IN:
                    cout << "Can't write to an input port " << endl;
                    sc_abort();
                    break;
                case OUT:
                    s.out_port->write(value!=0);
                    break;
                default:
                    cout << "Something is not right, NULL pointer to sig/in/out" << endl;
                    sc_abort();
            }
        }
        virtual pysc_event *default_event(){
            const sc_event *e;
            switch(sig_type){
                case SIG:
                    e = &s.signal->default_event();
                    break;
                case IN:
                    e = &s.in_port->default_event();
                    break;
                case OUT:
                    e = &s.out_port->default_event();
                    break;
                default:
                    cout << "Something is not right, NULL pointer to sig/in/out" << endl;
                    sc_abort();
            }
            pysc_event *r = (pysc_event*) pysc_event_Type.tp_alloc(&pysc_event_Type, 0);
            r->e.event = e;
            r->can_trigger = false;
            return r;
        }
        virtual pysc_event *pos_event(){
            const sc_event *e;
            switch(sig_type){
                case SIG:
                    e = &s.signal->posedge_event();
                    break;
                case IN:
                    e = &s.in_port->posedge_event();
                    break;
                case OUT:
                    e = &s.out_port->posedge_event();
                    break;
                default:
                    cout << "Something is not right, NULL pointer to sig/in/out" << endl;
                    sc_abort();
            }
            pysc_event *r = (pysc_event*) pysc_event_Type.tp_alloc(&pysc_event_Type, 0);
            r->e.event = e;
            r->can_trigger = false;
            return r;
        } 
        virtual pysc_event *neg_event(){ 
            const sc_event *e;
            switch(sig_type){
                case SIG:
                    e = &s.signal->posedge_event();
                    break;
                case IN:
                    e = &s.in_port->posedge_event();
                    break;
                case OUT:
                    e = &s.out_port->posedge_event();
                    break;
                default:
                    cout << "Something is not right, NULL pointer to sig/in/out" << endl;
                    sc_abort();
            }
            pysc_event *r = (pysc_event*) pysc_event_Type.tp_alloc(&pysc_event_Type, 0);
            r->e.event = e;
            r->can_trigger = false;
            return r;
       }
    private:
        union {
            sc_signal<bool> *signal;
            sc_in<bool> *in_port;
            sc_out<bool> *out_port;
        } s;
};

typedef struct {
    PyObject_HEAD;
    SignalProxyBase *sig;
} pysc_signal;

extern PyTypeObject pysc_signal_Type;

