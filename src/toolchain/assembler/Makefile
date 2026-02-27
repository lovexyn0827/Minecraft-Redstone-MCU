C_SOURCES=$(wildcard *.c)
C_HEADERS=$(wildcard *.h)

all: $(C_SOURCES) $(C_HEADERS)
	gcc -Wall $(C_SOURCES) $(C_HEADERS) -o asm

.PHONY:clean

clean:
	rm -o asm
