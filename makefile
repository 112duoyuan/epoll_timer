CC = g++
CFLAG = --WALL -g
Target = res
Src:= $(wildcard *.cc)
Objs:= $(patsubst %.cc,%.o,$(Src))

INCLUDES = -I ./

$(Target):$(Objs)
	g++ $(Objs) -o $(Target)

%.o:%.cc
	#equal to g++ -c *.cc -o *.o
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	rm $(Objs)
