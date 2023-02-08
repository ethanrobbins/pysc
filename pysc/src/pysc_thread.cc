// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#include "pysc_thread.h"
#include "pysc_time.h"
#include "pysc_event.h"
#include "pysc.h"

#include <systemc>
#include <list>
using namespace sc_core;
using namespace std;

std::list<Pysc_thread*> pysc_all_threads;

Pysc_thread *cur_thread = NULL;

Pysc_thread::Pysc_thread(PyObject *callable, const char* name):callable(callable){
    Py_INCREF(callable);
    char *c = new char[strlen(name)+1];
    this->name = c;
    strcpy(c,name);
    cout << "New thread: " << this << "   " << this->name << endl;

    sc_core::sc_spawn( sc_core::sc_bind(&Pysc_thread::run, this) );

}
Pysc_thread::Pysc_thread():callable(NULL){
    PyInterpreterState *intr_state = PyInterpreterState_Get();
    this->thread_state = PyThreadState_New(intr_state);
    this->name = "((pure systemc thread))";
}

Pysc_thread::~Pysc_thread(){
    PyThreadState_Clear(thread_state);
    PyThreadState_Delete(thread_state);
}
void Pysc_thread::run(){
    cur_thread = this;
    this->gil_state = PyGILState_Ensure();
    PyInterpreterState *intr_state = PyInterpreterState_Get();
    this->thread_state = PyThreadState_New(intr_state);
    PyThreadState_Swap(this->thread_state);
    PyObject *r = PyObject_CallNoArgs(this->callable);
    PyThreadState_Swap(top_thread_state);
    PyGILState_Release(this->gil_state);
    cur_thread = NULL;
    Py_DECREF(r);
    std::cout << "Finished thread " << name << std::endl;
    //TODO  delete ourselfs...
}

void Pysc_thread::pre_block(){
    PyGILState_Release(this->gil_state);
    //cout << "pre_wait(" << this->name << ")" << endl;
    cur_thread = NULL;
}
void Pysc_thread::post_block(){
    this->gil_state = PyGILState_Ensure();
    PyThreadState_Swap(this->thread_state);
    //cout << "post_wait(" << this->name << ")" << endl;
    cur_thread = this;
}

void pysc_thread_wait(PyObject *trigger){
    if(cur_thread == NULL){
        // We are being called from a pure systemC thread... (and blocking in python)
        sc_abort();
    }
    Pysc_thread *my_thread = cur_thread;
    //cout << "DDDD pysc_thread_wait 1 " << my_thread << " " << my_thread->name << endl;
    my_thread->pre_block();
    if(PyList_Check(trigger)){
        sc_abort(); // TODO
    }else if(PyObject_TypeCheck(trigger, &pysc_time_Type)){
        pysc_time* t = (pysc_time*) trigger;
        sc_core::wait(t->time);
    }else if(PyObject_TypeCheck(trigger, &pysc_event_Type)){
        pysc_event* t = (pysc_event*) trigger;
        if(t->can_trigger){
            //cout << "DDD wait event " << t->e.notify_event << endl;
            sc_core::wait(*t->e.notify_event);
        }else{
            sc_core::wait(*t->e.event);
        }
    }else{
        sc_abort();
    }
    //cout << "DDDD pysc_thread_wait 2 " << my_thread << " " << my_thread->name << endl;
    my_thread->post_block();
}

void pysc_start_thread(PyObject *callable, const char* name){
    Pysc_thread *thread = new Pysc_thread(callable, name);
    pysc_all_threads.push_back(thread);
}

Pysc_thread* pysc_start_dependant_thread(){
    Pysc_thread *th = new Pysc_thread();
    th->post_block();
    return th;
}
void pysc_finish_dependant_thread(Pysc_thread* th){
    th->pre_block();
    delete th;
}

Pysc_thread *pysc_get_thread(){
    return cur_thread;
}
void pysc_set_thread(Pysc_thread *th){
    cur_thread = th;
}
