ROOT_PWD = $(shell pwd)/../..

PYSC_INC = $(ROOT_PWD)/pysc/include
PYSC_LIB = $(ROOT_PWD)/pysc

SRC = $(wildcard RISC-V-TLM/src/*.cpp)

DEPS = $(patsubst RISC-V-TLM/src/%.cpp, build/%.o, $(SRC))
HEADERS = $(wildcard RISC-V-TLM/inc/*.h) $(wildcard $(PYSC_INC)/*.h)

DEFS = 

INCDIRS = $(ROOT_PWD)/systemc/install/include RISC-V-TLM/inc $(PYSC_INC)
LIBDIR = $(ROOT_PWD)/systemc/install/lib-linux64 $(PYSC_LIB)
LIBS = systemc pysc fmt
CXX_FLAGS = $(DEFS) -g -fPIC -std=c++17 $(foreach x, $(INCDIRS), -I$(x)) $(foreach x, $(LIBDIR), -L$(x)) $(foreach x, $(LIBS), -l$(x)) $(shell python3-config --includes) $(CPPFLAGS)
LD_FLAGS =  -fPIC $(foreach x, $(LIBDIR), -L$(x)) $(foreach x, $(LIBS), -l$(x)) $(foreach x, $(LIBDIR), -Wl,-rpath,$(x)) $(shell python3-config --ldflags)


build:
	mkdir build

clean:
	rm -rf build

sim: build $(DEPS) $(PYSC_LIB)/libpysc.so
	g++ -o $@ $(DEPS) $(LD_FLAGS)

sim.so: build $(DEPS) $(PYSC_LIB)/libpysc.so
	g++ -shared -o $@ $(DEPS) $(LD_FLAGS) 

build/%.o: RISC-V-TLM/src/%.cpp $(HEADERS)
	g++ $(CXX_FLAGS) -o $@ -c $<

dhrystone:
	export PATH=/home/efr/pysc/install/bin:$(PATH); make -C RISC-V-TLM/tests/C/dhrystone
	echo ./sim -L 3 RISC-V-TLM/tests/C/dhrystone/dhrystone.hex

func1:
	export PATH=/home/efr/pysc/install/bin:$(PATH); make -C RISC-V-TLM/tests/C/func1


test:
	echo $(ROOT_PWD)
	echo $(SRC)
	echo $(DEPS)

.PHONY: clean
.PHONY: sim
