cr-client: c-client
	clear
	@./client
c-client: Client.c ../RawSocket.c ../Protocol.c
	gcc Client.c ../RawSocket.c ../Protocol.c -o client
debug-client: Client.c ../RawSocket.c ../Protocol.c
	gcc Client.c ../RawSocket.c ../Protocol.c -g
	gdb a.out