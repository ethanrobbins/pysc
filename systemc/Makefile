PWD = $(shell pwd)

2.3.4.tar.gz:
	wget https://github.com/accellera-official/systemc/archive/refs/tags/2.3.4.tar.gz

systemc-2.3.4: 2.3.4.tar.gz
	tar -xzf 2.3.4.tar.gz

build_dir:
	mkdir build_dir

install: build_dir systemc-2.3.4
	cd systemc-2.3.4; ./config/bootstrap
	cd build_dir; ../systemc-2.3.4/configure --prefix=$(PWD)/install --enable-debug CXXFLAGS="-DSC_CPLUSPLUS=201703L -std=c++17"
	make -C build_dir install
	
build:
	make -C build_dir install


clean:
	rm -rf build_dir
	rm -rf install

clean-full: clean
	rm -rf 2.3.4.tar.gz
	rm -rf systemc-2.3.4
