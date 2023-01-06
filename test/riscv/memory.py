import params, mem_types, ElfMem, Utils
import pysc
import thread_utils
from collections import deque

print = Utils.sc_print

class BackingMemory:
    def __init__(self, name, mod, filename, run_stats, p :params.ParamsMem):
        self.params = p
        self.name = name
        self.mod = mod
        self.run_stats = run_stats
        self.mem = ElfMem.ElfMem(filename)
        self.event = pysc.pysc_event.create("memory")
        self.mod.SC_THREAD(self.run, "memory")

        self.semaphore = thread_utils.Semaphore()

        self.run_stats.memory_reads = 0
        self.run_stats.memory_writes = 0

        self.read_active = self.mod.create_signal("%s.read_active"%(name))
        self.write_active = self.mod.create_signal("%s.write_active"%(name))

        self.read_active_lcl = False
        self.write_active_lcl = False
        

    def read(self, req_read : mem_types.RequestWrite):
        self.semaphore.get()
        self.read_active_lcl = True
        req_read.data[:] = self.mem.read(req_read.address, params.CACHE_WORD)
        req_read.complete(None)
        self.run_stats.memory_reads += 1
 
    def write(self, req_write : mem_types.RequestWrite):
        self.semaphore.get()
        self.write_active_lcl = True
        for o in range(len(req_write.data)):
            if req_write.be[o]:
                #print("COMMIT: %x %x"%(req_write.address+o, req_write.data[o] ))
                self.mem.write(req_write.address+o, req_write.data[o])
        req_write.complete(None)
        self.run_stats.memory_writes += 1
        
    @Utils.try_dbg
    def run(self):
        period = pysc.time(self.params.MEM_NS, pysc.time.SC_NS)
        while True:
            pysc.wait(period)
            self.write_active.write(1 if self.write_active_lcl else 0)
            self.read_active.write(1 if self.read_active_lcl else 0)
            self.write_active_lcl = False
            self.read_active_lcl = False
            self.semaphore.put()