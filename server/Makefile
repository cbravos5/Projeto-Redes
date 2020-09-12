cr-server: c-server
	clear
	@./server
c-server: Server.c ../RawSocket.c
	gcc Server.c ../RawSocket.c ../Protocol.c -o server
debug-server: Server.c ../RawSocket.c ../Protocol.c
	gcc Server.c ../RawSocket.c ../Protocol.c -g
	gdb a.out