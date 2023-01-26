ROOT_PWD = $(shell pwd)/../..

PYSC_INC = $(ROOT_PWD)/pysc/include
PYSC_LIB = $(ROOT_PWD)/pysc

SRC = $(wildcard src/*.cc)
_DEPS = top.o mem.o core.o
DEPS = $(patsubst %, build/%, $(_DEPS)) 
HEADERS = $(wildcard src/*.h) $(wildcard $(PYSC_INC)/*.h)

DEFS = -DSC_INCLUDE_DYNAMIC_PROCESSES

INCDIRS = $(ROOT_PWD)/systemc/install/include src $(PYSC_INC)
LIBDIR = $(ROOT_PWD)/systemc/install/lib-linux64 $(PYSC_LIB) $(CONDA_PREFIX)/lib
LIBS = systemc pysc
CXX_FLAGS = $(DEFS) -g -fPIC -std=c++17 $(foreach x, $(INCDIRS), -I$(x)) $(foreach x, $(LIBDIR), -L$(x)) $(foreach x, $(LIBS), -l$(x)) $(shell python3-config --includes)
LD_FLAGS =  -fPIC $(foreach x, $(LIBDIR), -L$(x)) $(foreach x, $(LIBS), -l$(x)) $(foreach x, $(LIBDIR), -Wl,-rpath,$(x)) $(shell python3-config --ldflags)


build:
	mkdir build

clean:
	make -C $(ROOT_PWD)/pysc clean
	rm -rf build

sim: build $(DEPS) $(PYSC_LIB)/libpysc.so
	g++ -o $@ $(DEPS) $(LD_FLAGS) -lpython3.10

sim.so: build $(DEPS) $(PYSC_LIB)/libpysc.so
	g++ -shared -o $@ $(DEPS) $(LD_FLAGS) 

build/%.o: src/%.cc $(HEADERS)
	g++ $(CXX_FLAGS) -o $@ -c $<

test:
	echo $(SRC)
	echo $(DEPS)
	echo $(shell python3-config --ldflags)
	echo $(LD_FLAGS)

$(PYSC_LIB)/libpysc.so:
	make -C $(ROOT_PWD)/pysc libpysc.so

libs: $(PYSC_LIB)/libpysc.so

.PHONY: libs
.PHONY: clean
.PHONY: $(PYSC_LIB)/libpysc.so
.PHONY: sim
.PHONY: sim.so
