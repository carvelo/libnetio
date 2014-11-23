GCC	 = gcc
NAME = client1
SRC	 = main.c
INC  = ../../include
LIB  = ../../lib

win32:
	$(GCC) -Wall $(SRC) -o $(NAME)-x86.exe -m32 -Os -s -lnetio32 -lws2_32 -I"$(INC)" -L"$(LIB)"
	./$(NAME)-x86