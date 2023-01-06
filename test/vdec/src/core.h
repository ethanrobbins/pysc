#include <systemc>
#include "pysc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/tlm_quantumkeeper.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;


class CORE: public sc_module {
    SC_HAS_PROCESS(CORE);
    public:
        py_module py;
        CORE(sc_module_name name);
        void run();
        void trace(sc_trace_file *tf);

        void write(uint64_t addr, unsigned char *ptr, uint len);
        void read(uint64_t addr, unsigned char *ptr, uint len);

        tlm_utils::simple_initiator_socket<CORE> data_bus;
 
        sc_in<bool> clk_in;
        sc_out<bool> rd_en;
        sc_out<bool> wr_en;
        sc_out<sc_bv<32>> address;
        sc_out<sc_bv<32>> wr_data;
        sc_in<sc_bv<32>> rd_data;
        sc_in<bool> data_val;

};