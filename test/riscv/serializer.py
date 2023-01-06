import mem_types, params, Utils
from collections import deque

import pysc
print = Utils.sc_print

class Serializer:
    def __init__(self, name, mod):
        self.name = name
        self.mod = mod
        self.event = pysc.pysc_event.create("serializer")
        self.mod.SC_THREAD(self.run, "serializer")
        self.cycle = pysc.time(1, pysc.time.SC_NS)

        self.request_fifo = deque()
        self.pending_trans = mod.create_signal("serializer.pending_trans")
        self.pending_trans.write(0)
        self.pending_trans_priv = 0
        self.trans_cnt_priv = 0
        self.trans_cnt = mod.create_signal("serializer.trans_cnt")
 
        self.next = None

    @Utils.try_dbg
    def run(self):
        while True:
        #for iiii in range(3):
            pysc.wait(self.cycle)
            if len(self.request_fifo):
                req = self.request_fifo.popleft()
                if req.is_write:
                    #print("WRITE %x %x"%(req.address, req.d[0]))
                    #if req.d[0] != 0:
                    #    breakpoint()
                    a = req.address
                    remain = len(req.d)
                    ptr = a
                    ooo = 0
                    while remain>0:
                        this_sz = min(remain, params.CACHE_WORD-(ptr%params.CACHE_WORD))
                        wr = mem_types.RequestWrite(params.CACHE_ADDR(ptr), self.event)
                        offset = ptr%params.CACHE_WORD
                        for i in range(this_sz):
                            wr.data[i+offset] = req.d[i+ooo]
                            wr.be[i+offset] = True
                        req.add_dep(wr)
                        self.next.write(wr)
                        
                        ooo += this_sz
                        remain -= this_sz
                        ptr += this_sz
                if req.is_read:
                    a = req.address
                    remain = len(req.d)
                    ptr = a
                    while remain>0:
                        rd = mem_types.RequestRead(params.CACHE_ADDR(ptr), self.event)
                        req.add_dep(rd)
                        self.next.read(rd)
                        this_sz = params.CACHE_WORD-(ptr%params.CACHE_WORD)
                        remain -= this_sz
                        ptr += this_sz

                req.ordered = True
                req.event.notify()
                #print("internal req done: %x wr:%s rd:%s"%(req.address, req.is_write, req.is_read))
                     

    def new_request(self, request):
        self.trans_cnt_priv += 1
        self.trans_cnt.write(self.trans_cnt_priv)
        r = mem_types.Request(request, self.event)
        #print("REQ: %x wr:%s rd:%s"%(r.address, r.is_write, r.is_read))
        self.request_fifo.append(r)
        self.pending_trans_priv += 1
        self.pending_trans.write(self.pending_trans_priv)
        r.wait_for_finish()
        #for o in range(len(r.d)):
        #    assert(self.next.mem.segment.data[o+r.address] == r.d[o])

       #print("REQ_done: %x wr:%s rd:%s"%(r.address, r.is_write, r.is_read))
        self.pending_trans_priv -= 1
        self.pending_trans.write(self.pending_trans_priv)
