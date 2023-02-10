import pdb, sys, traceback
import builtins as __builtin__
from collections import deque
import pysc

class Semaphore:
    class Token:
        def __init__(self, event):
            self.blocked = True
            self.event = event

        def try_run(self):
            while self.blocked:
                pysc.wait(self.event)

        def go(self):
            self.blocked = False
            self.event.notify()

    def __init__(self):
        self.event = pysc.pysc_event("")
        self.pending = deque()

    def get(self):
        t = Semaphore.Token(self.event)
        self.pending.append(t)
        self.event.notify()
        t.try_run()

    def put(self):
        if len(self.pending):
            t = self.pending.popleft()
            t.go()
