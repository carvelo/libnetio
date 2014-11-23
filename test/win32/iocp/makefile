GCC		= gcc
NAME	= iocp_test
SRC		= main.c

win32:
	$(GCC) -Wall $(SRC) -o $(NAME)-x86.exe -m32 -lws2_32 -Os -s
	./$(NAME)-x86