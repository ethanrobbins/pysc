# Using Python for high level Architectural analysis
This is a proof of concept project and should not be used unless you are exploring the stated goals.

* NOTE this is subject to refactor, deletion, abandonment at anytime *

`pysc` is a library that links in with your systemC simulation to allow you to write the implementation of modules in python.

## Goals
* I wanted to explore using python to replace individual modules in a systemC simulation. 
* Use Python (and available libraries) to run sweeps and report results.
* Quick and easy way to do high level architectural exploration
   * Can code something up at a high level quickly
   * Some components can by cycel accurate systemc models (or verilog using verilator)
* Leverage pythons interactive debugger (pdb)

## Examples
### Implementing a module
```
class Cache:
   def __init__(self, name, mod):
      self.mod = mod
      self.name = name
      self.fill_event = pysc.pysc_event.create("fill_event") # create a sc_event
      self.access_time = pysc.time(10, pysc.time.SC_NS) # ie sc_time(10,SC_NS)
      self.flush_trace = mod.create_signal("cache.flush_trace") # creates an sc_signal that is traced in VCD files
      self.mod.SC_THREAD(self.run_thread, "cache_run_thread") # create a systemC thread to call self.run_thread()
   ...

   def run_thread(self):
      while(True):
         self.flush_trace.write(0) # set the signal to 0
         pysc.wait(self.access_time)  # waits 10ns
         pysc.wait(self.fill_event)  # waits until the event occurs
   ...
```

### Top level simulation
```
for cache_size in [2,4,8,16]:
   gbl.cache_size = cache_size
   sim.start()  # calls sc_main(..)

   runtime = pysc.cur_time().ns
   # Record stats in a file or sqlite database

   sim.restart() # restarts simulation from time 0 (and re-elab the design)
```

## What is included
* pysc\ - This is a library that links against SystemC and provides python bindings to some systemC components, link this library to your systemC project
   * src\ - Source code (C++)
   * include\ - includes for building your own SystemC simulation
   * libpysc.so - generate binary to link with your systemC simulation
* systemc\ - Working directory to download and build systemC
* tests\ -
   * vdec\ - my initial test case, may not run anymore.
   * riscv\ - more advanced testcase.  Pulls in a fork of mariusmm/RISC-V-TLM and replaces the memory module with a cache in pysc

## Dependancies
* Python 3.10  (later version may work, but not tested)
   * python-dev  (needed for Python.h)
* pyelftools
* pandas - for test/riscv/report.py
* matplotlib - for test/riscv/report.py

## Running
* Install steps:
   * build systemc
   ```
   cd systemc;
   make install
   ```

   * build pysc
   ```
   cd ../pysc
   make lib
   ```

* Running test case (see tests/riscv/README.md)
   * Download fork of mariusmm/RISC-V-TLM from ethanrobbins/RISC-V-TLM
   ```
   git submodule update ???
   ```

   * Build dhrystone
   ```
    cd test/riscv
    make dhrystone
    ```

   * build the systemC lib
   ```
    make sim.so
    ```

   * Run the test case
   ```
    python3 PyRiscv.py
    ```

## Documentation
Due to the early nature of this project there is not much documentation yet.  The Goal is not to provide a usable library, but to answer the questions "Can Python be used to do PPA?"

## Next steps
* Review the pysysc project from Accelera
   * Uses the python lib cppyy to bind to C++
   * What are the differences
* Debug/Test
* Memory leaks
   * tons of leaks
* Complete API
   * Initial work was to prove goals, not produce a (usable) product
   * more data types
   * TLM1/2
   * tlm_generic_payload
* Documentation


## Notes

## License
pysc is Licensed under the MIT License

Copyright 2023 Ethan Robbins

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


*Note the test/riscv contains code using a different license. And is not part of the pysc libray.*

*Note systemc is under its own license and is not part of the pysc library.*

