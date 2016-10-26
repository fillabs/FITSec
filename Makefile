all: tests
clean:
	-make -C tests clean
	-make -C libfitsec clean
	-make -C cshared clean

tests: libfitsec FORCE
	make -C tests all

libfitsec: cshared FORCE
	make -C libfitsec all

cshared: FORCE
	make -C cshared all
FORCE: