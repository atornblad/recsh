defines = 

debug : defines += -DDEBUG_VERBOSE

all: recsh dummy

debug: recsh dummy

dummy: dummy.o
	gcc dummy.o -o dummy

dummy.o: dummy.c
	gcc $(defines) -Wall -Werror -Wpedantic -c dummy.c

recsh: main.o config.o loop.o cleanup.o record.o script.o builtin.o
	gcc main.o config.o loop.o cleanup.o record.o script.o builtin.o -o recsh

main.o: main.c config.h loop.h cleanup.h error.h script.h
	gcc $(defines) -Wall -Werror -Wpedantic -c main.c

config.o: config.c error.h config.h
	gcc $(defines) -Wall -Werror -Wpedantic -c config.c

loop.o: loop.c error.h record.h loop.h script.h builtin.h
	gcc $(defines) -Wall -Werror -Wpedantic -c loop.c

cleanup.o: cleanup.c cleanup.h
	gcc $(defines) -Wall -Werror -Wpedantic -c cleanup.c

record.o: record.c record.h script.h
	gcc $(defines) -Wall -Werror -Wpedantic -c record.c

script.o: script.c script.h
	gcc $(defines) -Wall -Werror -Wpedantic -c script.c

builtin.o: builtin.c builtin.h
	gcc $(defines) -Wall -Werror -Wpedantic -c builtin.c

clean:
	rm recsh *.o
