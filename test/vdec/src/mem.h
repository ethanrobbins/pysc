#include <systemc>
#include "pysc.h"
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

class MEM: public sc_module {
    py_module<MEM> py;
    SC_HAS_PROCESS(MEM);
    public:
    MEM(sc_module_name name);
    void run();
    void trace(sc_trace_file *tf);

    //tlm_utils::simple_target_socket<MEM> socket;
    tlm::tlm_target_socket<> socket;
    virtual void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay);

    
    uint32_t memory[1024]; // 4KB of memory

    sc_in<bool> clk_in;
    sc_in<bool> rd_en;
    sc_in<bool> wr_en;
    sc_in<sc_bv<32>> address;
    sc_in<sc_bv<32>> wr_data;
    sc_out<sc_bv<32>> rd_data;
    sc_out<bool> data_val;

    //sc_signal<sc_bv<32>> d;
    //sc_signal<bool> d_v;
};