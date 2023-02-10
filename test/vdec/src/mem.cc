#include "mem.h"
#include "pysc.h"

MEM::MEM(sc_module_name name):sc_module(name), py("Mem",this),
    clk_in("clk_in"),
    rd_en("rd_en"),
    wr_en("wr_en"),
    address("address"),
    wr_data("wr_data"),
    rd_data("rd_data"),
    data_val("data_val")
{
    target_socket_proxy *sock_p = py.get_target_socket("sock");

    socket.bind(sock_p->socket);

    //socket.register_b_transport(this, &MEM::b_transport);
    //data_val.write(0);
    //rd_data.write(0);
    SC_METHOD(run);
    sensitive<<this->clk_in.pos();

    EXPORT_SIG(clk_in, "clk_in", bool);
    EXPORT_SIG(rd_en, "rd_en", bool);
    EXPORT_SIG(wr_en, "wr_en", bool);
    EXPORT_SIG(address, "address", sc_bv<32>);
    EXPORT_SIG(wr_data, "wr_data", sc_bv<32>);
    EXPORT_SIG(rd_data, "rd_data", sc_bv<32>);
    EXPORT_SIG(data_val, "data_val", bool);   
}
void MEM::trace(sc_trace_file *tf){
    TRACE_SIG(tf, clk_in);
    TRACE_SIG(tf, rd_en);
    TRACE_SIG(tf, wr_en);
    TRACE_SIG(tf, address);
    TRACE_SIG(tf, wr_data);
    TRACE_SIG(tf, rd_data);
    TRACE_SIG(tf, data_val);
}
void MEM::run(){
    if(this->rd_en.read()){
        rd_data.write(memory[this->address.read().to_int()]);
        data_val.write(1);
    }else{
        data_val.write(0);
    }

    if(this->wr_en.read()){
        memory[address->read().to_int()] = wr_data.read().to_uint();
    }
}

void MEM::b_transport(tlm::tlm_generic_payload &trans, sc_time &delay){
    tlm::tlm_command cmd = trans.get_command();
    uint64_t addr = trans.get_address();
    unsigned char *ptr = trans.get_data_ptr();
    uint len = trans.get_data_length();
    unsigned char *byt = trans.get_byte_enable_ptr();
    uint wid = trans.get_streaming_width();
    cout << "MEMEMEMEMEM got a socket request to addr:" << addr << endl;
    // ZERO time for now..
    if(cmd == tlm::TLM_READ_COMMAND){
        memcpy(ptr, &memory[addr], len);
    }else if(cmd == tlm::TLM_WRITE_COMMAND){
        memcpy(&memory[addr], ptr, len);
    }

    trans.set_response_status(tlm::TLM_OK_RESPONSE);

}