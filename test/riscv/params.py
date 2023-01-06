from sqlalchemy import Column, Integer, Table, MetaData, insert
import Utils

CACHE_WORD = 64

def CACHE_ADDR(a):
    a = a//CACHE_WORD
    return a*CACHE_WORD

class ParamBase:
    #PARAMS = []
    def __init__(self, name):
        self.name = name

    def add_params(self, TBL):
        for p in self.PARAMS:
            TBL["%s__%s"%(self.name,p)] = Column(Integer)

    def set_params(self, perf):
        for p in self.PARAMS:
            s = getattr(self,p)
            setattr(perf,"%s__%s"%(self.name, p), s)

    
    

class ParamsFifo(ParamBase):
    PARAMS = ["SIZE", "READ_NS", "WRITE_NS"]
    def __init__(self, name):
        super().__init__(name)
        self.SIZE = 4
        self.READ_NS = 1
        self.WRITE_NS = 2

class ParamsCache(ParamBase):
    PARAMS = ["CACHE_ENTRIES", "CACHE_ASSOCIATIVITY", "CACHE_READ_NS", "CACHE_WRITE_NS"]
    def __init__(self, name):
        super().__init__(name)
        self.CACHE_ENTRIES = 128
        self.CACHE_ASSOCIATIVITY = 4
        self.CACHE_READ_NS = 1
        self.CACHE_WRITE_NS = 1

class ParamsMem(ParamBase):
    PARAMS = ["MEM_NS"]
    def __init__(self, name):
        super().__init__(name)
        self.MEM_NS = 100



params_write_fifo = ParamsFifo("write_fifo")
params_mem_fifo = ParamsFifo("mem_fifo")
params_cache = ParamsCache("cache")
params_mem = ParamsMem("mem")

run_stats = []


@Utils.try_dbg
def recored_params(perf):
    params_mem_fifo.set_params(perf)
    params_write_fifo.set_params(perf)
    params_cache.set_params(perf)
    params_mem.set_params(perf)


def add_run_stats(stat_name):
    run_stats.append(stat_name)

def get_TBL(base):
    d = {}
    d["__tablename__"] = "perf"
    d["run_id"] = Column(Integer, primary_key=True)
    d['runtime'] = Column(Integer)
    d["cache_ws"] = Column(Integer)
    params_write_fifo.add_params(d)
    params_mem_fifo.add_params(d)
    params_cache.add_params(d)
    params_mem.add_params(d)

    for s in run_stats:
        d[s] = Column(Integer)

    TBL = type("Perf", (base,), d)
    return TBL
 
