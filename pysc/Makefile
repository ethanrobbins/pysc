SRC = $(wildcard src/*.cc)
#_DEPS = py_module.o
#DEPS = $(patsubst %, build/%, $(_DEPS)) 
DEPS = $(patsubst src/%.cc, build/%.o, $(SRC)) 
HEADERS = $(wildcard src/*.h) $(wildcard include/*.h)

DEFS = -DSC_INCLUDE_DYNAMIC_PROCESSES

ROOT_PWD = $(shell pwd)/..
CXX_FLAGS = $(DEFS) -std=c++17 -fPIC -g -I$(ROOT_PWD)/systemc/install/include -Isrc -Iinclude -L$(ROOT_PWD)/systemc/install/lib-linux64 -lsystemc $(shell python3-config --includes)
LD_FLAGS = -fPIC -L$(ROOT_PWD)/systemc/install/lib-linux64 -lsystemc -lm -Wl,-rpath,$(ROOT_PWD)/systemc/install/lib-linux64 $(shell python3-config --ldflags) -lpython3.9
build:
	mkdir build

clean:
	rm -rf build
	rm -f libpysc.so

libpysc.so: build $(DEPS)
	g++ -shared -o $@ $(DEPS) $(LD_FLAGS) 

build/%.o: src/%.cc $(HEADERS)
	g++ $(CXX_FLAGS) -c -o $@ $<

test:
	echo $(SRC)
	echo $(DEPS)
#	echo $(DEPS2)

.PHONY: lib

lib: libpysc.so
