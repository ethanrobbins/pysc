import params
import pysc

class BaseRequest:
    def __init__(self, address, ev):
        self.address = address
        self.finished = False
        self.deps = []
        self.prov = []
        self.event = ev

    def complete(self, child):
        if child:
            self.update_data(child)
            for d in self.deps:
                if not d.finished:
                    return
        self.finished = True
        for p in self.prov:
            p.complete(self)
        if self.event:
            self.event.notify()

    def update_data(self, rd):
        assert(False) # This should be overriden in the extened class

    def wait_for_finished(self):
        while not self.finished:
            pysc.wait(self.event)

    def add_dep(self, d):
        if d not in self.deps:
            self.deps.append(d)
        if self not in d.prov:
            d.prov.append(self)

class Request(BaseRequest):
    def __init__(self, gp, ev):
        super().__init__(gp.get_address(), ev)
        self.gp = gp
        self.d = self.gp.get_data()
        self.is_write = self.gp.get_command() == self.gp.TLM_WRITE_COMMAND
        self.is_read = self.gp.get_command() == self.gp.TLM_READ_COMMAND
        self.ordered = False
    
    def get_address(self):
        return self.address

    def get_size(self):
        return len(self.d)

    def update_data(self, req_read):
        if self.is_read:
            for o in range(len(self.d)):
                if o+self.address >= req_read.address and o+self.address < req_read.address+params.CACHE_WORD:
                    self.d[o] = req_read.data[o+self.address-req_read.address]

    def wait_for_finish(self):
        while not self.ordered:
            pysc.wait(self.event)
        super().wait_for_finished()
        
class RequestWrite(BaseRequest):
    def __init__(self, address, ev):
        super().__init__(address, ev)
        self.data = bytearray(params.CACHE_WORD)
        self.be = [False]*params.CACHE_WORD

    def update_data(self, rd):
        pass

class RequestRead(BaseRequest):
    def __init__(self, address, ev):
        super().__init__(address, ev)
        self.data = bytearray(params.CACHE_WORD)

    def update_data(self, rd):
        assert(rd.address == self.address)
        self.data[:] = rd.data[:]

