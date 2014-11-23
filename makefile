GCC	 = gcc
AR 	 = ar
NAME = netio
SRC	 = src/netio.c
INC  = include
OBJ  = obj
LIB  = lib

win32:
	$(GCC) -Wall -c $(SRC) -o $(OBJ)/$(NAME)-x86.o -m32 -Os -s -I"$(INC)" -L"$(LIB)"
	$(AR) rcs $(LIB)/lib$(NAME)32.a $(OBJ)/$(NAME)-x86.o
