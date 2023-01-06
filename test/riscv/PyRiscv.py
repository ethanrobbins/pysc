import pdb, sys, traceback
import builtins as __builtin__
from Utils import Factory, try_dbg, sc_print, f
import ElfMem
import params
import os

from sqlalchemy import Column, Integer, Table, MetaData, insert
from sqlalchemy.orm import Session
from sqlalchemy.ext.declarative import declarative_base
import sqlalchemy as db

meta = MetaData()

db_file = "foo.sqlite"
if os.path.exists(db_file):
    os.remove(db_file)
engine = db.create_engine("sqlite:///%s"%(db_file))

Base = declarative_base()


@try_dbg 
def sc_main():
    sim.start()

if __name__ == "__main__":
    print("Python is TOP")
    import sim
    sim.set_factory_module(f)

import pysc
import serializer,memory,write_fifo,cache

Perf = params.get_TBL(Base)

Base.metadata.create_all(engine)

session = Session(engine)


run_stats = None

@f.register("Memory")
class Memory:
    elf_file = "RISC-V-TLM/tests/C/dhrystone/dhrystone"

    @try_dbg
    def __init__(self, mod):
        print("called Memory::init")
        self.mod = mod
        self.target_socket = pysc.tlm_target_socket()
        self.target_socket.register_b_transport(self.b_transport)
        self.mod.set_socket("sock", self.target_socket)

        self.serializer = serializer.Serializer("serializer",mod)
        self.write_fifo = write_fifo.WriteFifo("write_fifo",mod,params.params_write_fifo)
        self.cache = cache.Cache("cache", mod, params.params_cache)
        self.mem_fifo = write_fifo.WriteFifo("mem_fifo",mod, params.params_mem_fifo)
        self.memory = memory.BackingMemory("mem", mod, self.elf_file, run_stats, params.params_mem)

        self.serializer.next = self.write_fifo
        self.write_fifo.next = self.cache
        self.cache.next = self.mem_fifo
        self.mem_fifo.next = self.memory

    @try_dbg
    def b_transport(self, trans, delay):
        #print("called Mem::b_transport")
        pysc.wait(delay)
        self.serializer.new_request(trans)
        trans.set_response_status(trans.TLM_OK_RESPONSE)

    def end_of_elaboration(self):
        print("Mem::end_of_elaboration")
    def start_of_simulation(self):
        pass 
    def end_of_simulation(self):
        self.cache.recored_stats(run_stats)
        pass

print("Loading PyVdec")

if __name__ == "__main__":
    print("Python starting")
    print = sc_print
    for cache_ws in [32,64,128]:
        for mem_ns in [100,200]:
            for cache_size in [2,4,8,16,32]:
                for assciativity in [1,2,4,8]:
                    params.params_cache.CACHE_ENTRIES = cache_size*1024//cache_ws
                    params.params_cache.CACHE_ASSOCIATIVITY = assciativity
                    params.params_mem.MEM_NS = mem_ns
                    params.CACHE_WORD = cache_ws

                    if params.params_cache.CACHE_ENTRIES<params.params_cache.CACHE_ASSOCIATIVITY:
                        next

                    perf = Perf(cache_ws=params.CACHE_WORD)

                    params.recored_params(perf)
                    run_stats = perf
                    sim.start(["-L 3","RISC-V-TLM/tests/C/dhrystone/dhrystone.hex"])
        
                    perf.runtime = pysc.cur_time().ns
                    session.add(perf)
                    session.commit()

                    sim.restart()
        
    print("Python done")
