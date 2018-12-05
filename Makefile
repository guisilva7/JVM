FILES = $(wildcard ./*.c) #$(wildcard ./*.h)

FLAGS= -std=c99 -O0 -lm -ggdb #-fdiagnostics-color

EXEC = jvm

COMPILER = gcc

#comando para remover pastas
RMDIR = rm -rf
#comando para remover arquivos
RM = rm -f
CD = cd

ifeq ($(OS),Windows_NT)

EXEC := $(EXEC).exe

FLAGS= -std=c99 -O0 -ggdb

#comando para remover um diret�rio recursivamente
RMDIR= rd /s /q
#comando para deletar um �nico arquivo
RM = del

endif

all:
	$(COMPILER) $(FILES) -o $(EXEC) $(FLAGS)

clean:
	$(RM) $(EXEC) *.txt

