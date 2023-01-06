#include "core.h"
#include <systemc>
using namespace sc_core;

CORE::CORE(sc_module_name name):sc_module(name), py("Core"),
        data_bus("data_bus"),
        clk_in("clk_in"),
        rd_en("rd_en"),
        wr_en("wr_en"),
        address("address"),
        wr_data("wr_data"),
        rd_data("rd_data"),
        data_val("data_val")
{
    //rd_en.write(0);
    //wr_en.write(0);
    //SC_METHOD(run);
    //sensitive<<this->clk_in.pos();

    SC_THREAD(run);
    sensitive<<this->clk_in.pos();

    EXPORT_SIG(clk_in, "clk_in", bool);
    EXPORT_SIG(rd_en, "rd_en", bool);
    EXPORT_SIG(wr_en, "wr_en", bool);
    EXPORT_SIG(address, "address", sc_bv<32>);
    EXPORT_SIG(wr_data, "wr_data", sc_bv<32>);
    EXPORT_SIG(rd_data, "rd_data", sc_bv<32>);
    EXPORT_SIG(data_val, "data_val", bool);
}
void CORE::trace(sc_trace_file *tf){
    TRACE_SIG(tf, clk_in);
    TRACE_SIG(tf, rd_en);
    TRACE_SIG(tf, wr_en);
    TRACE_SIG(tf, address);
    TRACE_SIG(tf, wr_data);
    TRACE_SIG(tf, rd_data);
    TRACE_SIG(tf, data_val);
}

void CORE::run(){
    cout << "CORE::run()..." << endl;
    wait(10);
    unsigned char d[128];
    d[0]=0;
    d[1]=2;
    d[2]=4;
    d[3]=8;
    write(12,d,4);
    read(12,d,4);
    read(12,d,4);
    //cout << "Done" <<endl;
}

void CORE::write(uint64_t addr, unsigned char *ptr, uint len){
    tlm::tlm_generic_payload trans;
    sc_time delay = sc_core::SC_ZERO_TIME;

    trans.set_command(tlm::TLM_WRITE_COMMAND);
    trans.set_data_ptr(ptr);
    trans.set_data_length(len);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans.set_address(addr);

    data_bus->b_transport(trans, delay);
}
void CORE::read(uint64_t addr, unsigned char *ptr, uint len){
    tlm::tlm_generic_payload trans;
    sc_time delay = sc_core::SC_ZERO_TIME;

    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_data_ptr(ptr);
    trans.set_data_length(len);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans.set_address(addr);

    data_bus->b_transport(trans, delay);
}
