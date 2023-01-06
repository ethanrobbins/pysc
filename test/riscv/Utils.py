import pdb, sys, traceback
import builtins as __builtin__
from collections import deque

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
        print("get_class ok")
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

__print__ = __builtin__.print
def sc_print(*args, **kwargs):
    import pysc
    t = "%0.1f ns  "%(pysc.cur_time().ns)
    __print__(t, end='')
    __print__(*args, **kwargs)

