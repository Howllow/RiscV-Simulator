SRCS := main.cc MM.cc machine.cc Read_Elf.cc Simulator.cc fetch.cc decode.cc execute.cc memory.cc writeBack.cc

all:
	g++ -o simulator $(SRCS)

clean:
	rm *.o
	rm simulator