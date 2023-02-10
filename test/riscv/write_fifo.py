import mem_types, params, Utils, thread_utils
import pysc
from collections import deque

class WriteFifo:
    def __init__(self, name, mod, p :params.ParamsFifo):
        self.params = p
        self.name = name
        self.mod = mod
        self.event = pysc.pysc_event("write_fifo")
        self.mod.SC_THREAD(self.run_enque, "write_fifo_enque")
        self.mod.SC_THREAD(self.run_deque, "write_fifo_deque")

        self.enque_semaphore = thread_utils.Semaphore()
        self.read_semaphore = thread_utils.Semaphore()
        self.fifo = deque()

        self.read_time = pysc.time(self.params.READ_NS, pysc.time.SC_NS)
        self.write_time = pysc.time(self.params.WRITE_NS, pysc.time.SC_NS)

        self.read_trans = None

        self.write_hazard = False # set when writes are stalled

        self.next = None

        self.fill_cnt = mod.create_signal("%s.fill"%(self.name))

    def read(self, req_read : mem_types.RequestWrite):
        # use a semaphore for ordering
        if self.read_trans:
            self.read_semaphore.get()
        assert(self.read_trans == None)
        self.read_trans = req_read
        # wait the read time
        pysc.wait(self.read_time)
        # check the fifo for a match (if so wait for the write to commit before sending the read)
        stall_read = True
        while stall_read:
            stall_read = False
            for entry in self.fifo:
                if entry.address == req_read.address:
                    stall_read = True
            if stall_read:
                pysc.wait(self.read_time)
        # push the request to the next block (ie the cache itself)
        self.next.read(req_read)
        self.read_trans = None
        self.write_hazard = False
        
        self.read_semaphore.put()


    def write(self, req_write : mem_types.RequestWrite):
        self.enque_semaphore.get()
        # check for a stalled read that matches req_write address (ie want to stall the write for the read to finish)
        if self.read_trans and self.read_trans.address==req_write.address:
            self.write_hazard = True
        while self.write_hazard:
            pysc.wait(self.write_time)

        entry_found = False
        for entry in self.fifo:
            if entry.address == req_write.address: # merge
                # merge the data and byte_enables
                for i in range(len(entry.data)):
                    if req_write.be[i]:
                        entry.data[i] = req_write.data[i]
                        entry.be[i] = True
                entry_found = True
                break
        if not entry_found:
            self.fifo.append(req_write)
            self.event.notify() # kick the deque thread
        # mark the request as completed
        req_write.complete(None)
       
    @Utils.try_dbg
    def run_enque(self):
        while True:
            pysc.wait(self.write_time)
            if not self.write_hazard:
                if len(self.fifo) < self.params.SIZE:
                    self.enque_semaphore.put()    
    
    @Utils.try_dbg
    def run_deque(self):
        while True:
            self.fill_cnt.write(len(self.fifo))
            while len(self.fifo)==0:
                pysc.wait(self.event)
            self.fill_cnt.write(len(self.fifo))
            self.next.write(self.fifo[0])
            r = self.fifo.popleft()
