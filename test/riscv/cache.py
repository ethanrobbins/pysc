import mem_types, params, Utils, thread_utils
import pysc
from collections import deque

print = Utils.sc_print

class CacheLine:
    def __init__(self, name, p :params.ParamsCache):
        self.params = p
        self.name = name
        self.data = bytearray(params.CACHE_WORD)
        self.valid = False
        self.dirty = False
        self.address = 0

    def write(self, req_write : mem_types.RequestWrite):
        if self.valid and self.address == req_write.address: # update
            for i in range(params.CACHE_WORD):
                if req_write.be[i]:
                    self.data[i] = req_write.data[i]
                self.dirty = True
        else: # replace
            assert(not self.dirty)
            assert(req_write.be == [True]*params.CACHE_WORD) # full write
            self.address = req_write.address
            self.data[:] = req_write.data[:]
            self.valid = True
            self.dirty = True

    def read(self, req_read : mem_types.RequestRead):
        assert(self.valid)
        assert(self.address == req_read.address)
        req_read.data[:] = self.data[:]
        req_read.complete(None)

    def flush(self):
        if not self.valid or not self.dirty:
           return None # don't need to flush
        r = mem_types.RequestWrite(self.address, None)
        r.data[:] = self.data[:]
        r.be = [True]*params.CACHE_WORD
        self.dirty = False
        #print("FLUSH: %s %x"%(self.name, self.address))
        return r

    def fill(self, req_read : mem_types.RequestRead):
        assert(not self.valid or not self.dirty)
        self.valid = True
        self.dirty = False
        self.address = req_read.address
        self.data[:] = req_read.data[:]
        #print("FILL %s %x"%(self.name, self.address))

class CacheSet:
    def __init__(self, mem_intf, idx, p):
        self.mem_intf = mem_intf
        self.idx = idx
        self.ways = mem_intf.params.CACHE_ASSOCIATIVITY
        self.way_data = []
        for i in range(self.ways):
            self.way_data.append(CacheLine("s_%x__w%d"%(idx,i),p))
    
    def read(self, req_read : mem_types.RequestRead):
        found = False
        for way_idx in range(self.ways):
            if self.way_data[way_idx].valid and self.way_data[way_idx].address == req_read.address:
                self.way_data[way_idx].read(req_read)
                t = self.way_data.pop(way_idx)
                self.way_data.append(t)
                found = True
                self.mem_intf.stats_cache_hit += 1
                break
        if not found: # need to fill....
            self.mem_intf.stats_cache_miss += 1
            lru = self.way_data[0] # its sorted so this should be the LRU
            flush_req = lru.flush()
            if flush_req:
                self.mem_intf.stats_flush += 1
                self.mem_intf.flush(flush_req)
                flush_req.wait_for_finished()
            self.mem_intf.stats_fill += 1
            lru.fill(self.mem_intf.fill(req_read.address))
            lru.read(req_read)
            t = self.way_data.pop(0)
            self.way_data.append(t)
            self.mem_intf.new_stats.notify()

    def write(self, req_write : mem_types.RequestWrite):
        found = False
        for way_idx in range(self.ways):
            if self.way_data[way_idx].valid and self.way_data[way_idx].address == req_write.address:
                self.way_data[way_idx].write(req_write)
                t = self.way_data.pop(way_idx)
                self.way_data.append(t)
                found = True
                self.mem_intf.stats_cache_hit += 1
                break
        if not found: # need to fill....
            self.mem_intf.stats_cache_miss += 1
            lru = self.way_data[0] # its sorted so this should be the LRU
            flush_req = lru.flush()
            if flush_req:
                self.mem_intf.stats_flush += 1
                self.mem_intf.flush(flush_req)
                flush_req.wait_for_finished()
            self.mem_intf.stats_fill += 1
            lru.fill(self.mem_intf.fill(req_write.address))
            lru.write(req_write)
            t = self.way_data.pop(0)
            self.way_data.append(t)
            self.mem_intf.new_stats.notify()
        
params.add_run_stats("cache_fill")
params.add_run_stats("cache_flush")
params.add_run_stats("cache_hit")
params.add_run_stats("cache_miss")
class Cache:
    def __init__(self, name, mod, p):
        self.params = p
        self.name = name
        self.mod = mod
        self.next = None
        self.num_sets = self.params.CACHE_ENTRIES // self.params.CACHE_ASSOCIATIVITY
        self.sets = []
        for i in range(self.num_sets):
            self.sets.append(CacheSet(self,i,p))

        self.read_sem = thread_utils.Semaphore()
        self.write_sem = thread_utils.Semaphore()
        self.fill_event = pysc.pysc_event.create("fill_event")
        self.comp_event = pysc.pysc_event.create("comp_event")

        self.write_time = pysc.time(self.params.CACHE_WRITE_NS, pysc.time.SC_NS)
        self.read_time = pysc.time(self.params.CACHE_READ_NS, pysc.time.SC_NS)

        self.active = False

        self.mod.SC_THREAD(self.run, "cache")
        self.mod.SC_THREAD(self.stats, "cache_stats")
        self.new_stats = pysc.pysc_event.create("stats")

        self.fill_s = mod.create_signal("%s.fill"%(name))
        self.flush_s = mod.create_signal("%s.flush"%(name))
        self.hit_s = mod.create_signal("%s.hit"%(name))
        self.miss_s = mod.create_signal("%s.miss"%(name))

        self.stats_fill = 0
        self.stats_flush = 0
        self.stats_cache_hit = 0
        self.stats_cache_miss = 0

    def recored_stats(self, perf):
        perf.cache_fill = self.stats_fill
        perf.cache_flush = self.stats_flush
        perf.cache_hit = self.stats_cache_hit
        perf.cache_miss = self.stats_cache_miss

    @Utils.try_dbg
    def stats(self):
        while True:
            pysc.wait(self.new_stats)
            self.fill_s.write(self.stats_fill)
            self.flush_s.write(self.stats_flush)
            self.hit_s.write(self.stats_cache_hit)
            self.miss_s.write(self.stats_cache_miss)

 
    def get_set(self, address):
        a = address//params.CACHE_WORD
        return a % (self.params.CACHE_ENTRIES//self.params.CACHE_ASSOCIATIVITY)

    def read(self, req_read : mem_types.RequestRead):
        self.comp_event.notify()
        self.read_sem.get()
        assert(not self.active)
        self.active = True
        pysc.wait(self.read_time)

        set_idx = self.get_set(req_read.address)
        self.sets[set_idx].read(req_read)

        self.active = False
        self.comp_event.notify()


    def write(self, req_write : mem_types.RequestWrite):
        self.comp_event.notify()
        self.write_sem.get()
        assert(not self.active)
        self.active = True
        pysc.wait(self.write_time)

        set_idx = self.get_set(req_write.address)
        self.sets[set_idx].write(req_write)

        self.active = False
        self.comp_event.notify()

    def flush(self, req_write : mem_types.RequestWrite):
        self.next.write(req_write)

    def fill(self, address):
        req_read = mem_types.RequestRead(address, self.fill_event)
        self.next.read(req_read)
        req_read.wait_for_finished()
        return req_read

    @Utils.try_dbg
    def run(self):
        while True:
            #this is messy and should be moved so some common lib
            # we have something to do...
            while len(self.read_sem.pending):
                while self.active:
                    pysc.wait(self.comp_event)
                self.read_sem.put()
                pysc.wait(self.comp_event)
            if not self.active and len(self.write_sem.pending):
                self.write_sem.put()
                pysc.wait(self.comp_event)
            else:
                pysc.wait(self.comp_event)


