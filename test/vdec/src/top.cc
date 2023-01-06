#include <systemc>
using namespace sc_core;
using namespace std;

#include "mem.h"
#include "core.h"

class TOP: public sc_module {
    public:
    SC_HAS_PROCESS(TOP);
    MEM MEM_i;
    CORE CORE_i;

    sc_clock clk;
    sc_signal<bool> rd_en;
    sc_signal<bool> wr_en;
    sc_signal<sc_bv<32>> address;
    sc_signal<sc_bv<32>> wr_data;
    sc_signal<sc_bv<32>> rd_data;
    sc_signal<bool> data_val;


    TOP(sc_module_name name) : sc_module(name), 
        MEM_i("mem"),
        CORE_i("CORE"),
        clk("clk", 1, SC_NS),
        rd_en("rd_en"),
        wr_en("wr_en"),
        address("address"),
        wr_data("wr_data"),
        rd_data("rd_data"),
        data_val("data_val")
    {
        SC_THREAD(run);
        sensitive<<clk.posedge_event();

        MEM_i.clk_in(clk);
        MEM_i.rd_en(rd_en);
        MEM_i.wr_en(wr_en);
        MEM_i.address(address);
        MEM_i.wr_data(wr_data);
        MEM_i.rd_data(rd_data);
        MEM_i.data_val(data_val);

        CORE_i.clk_in(clk);
        CORE_i.rd_en(rd_en);
        CORE_i.wr_en(wr_en);
        CORE_i.address(address);
        CORE_i.wr_data(wr_data);
        CORE_i.rd_data(rd_data);
        CORE_i.data_val(data_val);

        CORE_i.data_bus.bind(MEM_i.socket);
       

    }

    void run(){
        cout << "Hello Wolrd!!!" <<endl;

        for(int i=0;i<100;i++){
            wait();
            if(i%10==0){
                cout << sc_time_stamp() << " TICK" << endl;
            }
        }
        cout << "done: " << sc_time_stamp() << endl;
        sc_stop();
    }

    void trace(sc_trace_file *tf){
        MEM_i.trace(tf);
        CORE_i.trace(tf);
    }
};

sc_trace_file *tf;

int sc_main(int nargs, char* args[]){
    tf = sc_create_vcd_trace_file("trace");
    pysc_initialize("PyVdec");
    TOP top("top");
    top.trace(tf);
    sc_start();
    sc_close_vcd_trace_file(tf);
    return 0;
}

PYSC_SIM
