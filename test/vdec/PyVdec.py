import pdb, sys, traceback
import builtins as __builtin__

class Factory:
    def __init__(self):
        self.class_reg = {}
    def register(self, cl_name):
        def inner(cls):
            self.class_reg[cl_name] = cls
        return inner
 
    def get_class(self, cls_name, mod):
        print("Factory::get_class(%s)"%(cls_name))
        C = self.class_reg[cls_name]
        return C(mod)

f = Factory()
def get_factory():
    return f

def try_dbg(func):
    def inner(self, *args):
        try:
            func(self, *args)
        except Exception:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            print("*** print_tb:")
            traceback.print_tb(exc_traceback)
            traceback.print_exc()
            pdb.post_mortem(exc_traceback)
    return inner        

@try_dbg 
def sc_main():
    sim.start()

if __name__ == "__main__":
    print("Python is TOP")
    import sim
    sim.set_factory_module(f)

import pysc

__print__ = __builtin__.print
def sc_print(*args, **kwargs):
    t = "%0.1f ns  "%(pysc.cur_time().ps/1000)
    __print__(t, end='')
    __print__(*args, **kwargs)
print = sc_print

@f.register("Mem")
class Mem:
    def __init__(self, mod):
        print("called Mem::init")
        self.mod = mod
        self.target_socket = pysc.tlm_target_socket()
        self.target_socket.register_b_transport(self.b_transport)
        self.mod.set_socket("sock", self.target_socket)
        self.data = bytearray(1024)
        self.READ_TIME = pysc.time(2, pysc.time.SC_NS)
        self.WRITE_TIME = pysc.time(4, pysc.time.SC_NS)

    @try_dbg
    def b_transport(self, trans, delay):
        print("called Mem::b_transport")
        pysc.wait(delay)
        address = trans.get_address()
        if trans.get_command() == trans.TLM_READ_COMMAND:
            print("read %d"%(address))
            pysc.wait(self.READ_TIME)
            d = trans.get_data()
            for i in range(len(d)):
               d[i] = self.data[i+address]
        elif trans.get_command() == trans.TLM_WRITE_COMMAND:
            print("write %d"%(address))
            pysc.wait(self.WRITE_TIME)
            dw = trans.get_data()
            for i in range(len(dw)):
                self.data[i+address] = dw[i]
        
        trans.set_response_status(trans.TLM_OK_RESPONSE)

    def end_of_elaboration(self):
        print("Mem::end_of_elaboration")
    def start_of_simulation(self):
        pass 
    def end_of_simulation(self):
        pass

@f.register("Core")
class Core:
    def __init__(self, mod):
        print("Core::__init__ called")
        self.mod = mod

    def __del__(self):
        print("CALLED __del__ @@@@@@@@@@@@@@@@@@@@")
 
    def end_of_elaboration(self):
        print("Core::end_of_elaboration")

    def start_of_simulation(self):
        print("Core::start_of_simulation")
        self.mod.SC_THREAD(self.run,"run")

    def end_of_simulation(self):
        print("Core::end_of_simulation")
        t = pysc.cur_time()
        print("Finished sim ")

    def write(self, address, data):
        self.wr_en.write(1)
        self.wr_data.write(data)
        self.address.write(address)
        pysc.wait(self.clk_in.posedge_event())
        self.wr_en.write(0)

    def read(self, address):
        self.rd_en.write(1)
        self.address.write(address)
        pysc.wait(self.clk_in.posedge_event())
        self.rd_en.write(0)
        while self.data_val.read()==0:
            pysc.wait(self.clk_in.posedge_event())
        return self.rd_data.read()
    
    @try_dbg
    def run(self):
        print("called Core::run")
        e = self.clk_in.posedge_event()
        pysc.wait(pysc.time(10, pysc.time.SC_NS))
        pysc.wait(self.clk_in.posedge_event())
        self.write(0,42)
        self.write(4,99)
        pysc.wait(pysc.time(10, pysc.time.SC_NS))
        pysc.wait(self.clk_in.posedge_event())

        #a = self.read(4)
        #b = self.read(0)

        #print("0x0:%d"%(b))
        #print("0x4:%d"%(a))
        pysc.wait(pysc.time(15,pysc.time.SC_NS))
        #for i in range(10):
        #    pysc.wait(e)
            #print("tick (run)")
        pysc.stop()
 
    @try_dbg
    def run2(self):
        print("called Core::run2")
        for i in range(10):
            t = pysc.time(2, pysc.time.SC_NS)
            pysc.wait(t)
            #breakpoint()
            print("Done waiting...run2")
        pysc.stop()
        

print("Loading PyVdec")

if __name__ == "__main__":
    print("Python starting")
    sim.start()
    print("Python done")
