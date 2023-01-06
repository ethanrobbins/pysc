import sys
from elftools.elf.elffile import ELFFile
from elftools.elf.segments import Segment
import Utils

class MemSeg:
    def __init__(self, offset, size, RO, data):
        self.offset = offset
        self.size = size
        self.RO = RO
        self.data = bytearray(size)
        self.data[:len(data)] = data

    def write(self, offset, data):
        #assert(offset>self.offset)
        #assert(offset+len(data) <= self.offset+self.size)
        #assert(not self.RO)

        o = offset-self.offset
        if isinstance(data, bytes):
            self.data[o:o+len(data)] =  data
        else:
            self.data[o] = data

    def read(self, offset, sz):
        #assert(offset>self.offset)
        #assert(offset+sz <= self.offset+self.size)

        o = offset-self.offset
        return self.data[o:o+sz]


class ElfMem:
    @Utils.try_dbg
    def __init__(self, filename):
        self.filename = filename
        self.segment = MemSeg(0,0x10000000,False, bytearray(0))
        with open(filename,"rb") as elffile:
            ELF = ELFFile(elffile)
            for segment in ELF.iter_segments():
                if segment.header.p_memsz:
                    self.segment.write(segment.header.p_paddr, segment.data())
                    #ms = MemSeg(segment.header.p_paddr, segment.header.p_memsz, not segment.header.p_flags&2, segment.data())
                    #self.segments.append(ms)
            self.entry = ELF.header.e_entry

    def write(self, offset, data):
        self.segment.write(offset, data)

    def read(self, offset, sz):
        return self.segment.read(offset, sz)
        