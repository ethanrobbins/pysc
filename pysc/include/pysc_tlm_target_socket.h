// SPDX-FileCopyrightText: © 2023 Ethan Robbins
// SPDX-License-Identifier: MIT

#pragma once

#include <Python.h>
#include <systemc>
#include "tlm.h"
#include "pysc_tlm_generic_payload.h"
#include "pysc_time.h"
#include "tlm_utils/simple_target_socket.h"

using namespace std;
using namespace sc_core;

class target_socket_proxy;
typedef struct {
    PyObject_HEAD;
    target_socket_proxy *socket;
} pysc_tlm_target_socket;

class target_socket_proxy{
    public:
        target_socket_proxy();
        virtual ~target_socket_proxy();

        tlm_utils::simple_target_socket<target_socket_proxy> socket;

        // forwared calls
        void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay);

        PyObject *target__b_transport;
};

extern PyTypeObject pysc_tlm_target_socket_Type;

