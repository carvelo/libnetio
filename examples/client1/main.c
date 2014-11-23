#include <stdio.h>
#include <netio.h>

#define ROW_LENGTH 80

int col = 0;

void post_message(const char *str, int success) {
	/* If our column is set, we NEED a status. */
	if (col) {
		printf("%*s\n", (ROW_LENGTH - col - 1), (success) ? "[  OK  ]" : "[ FAIL ]");
	}
	col = strlen(str);
	printf("%s", str);
}

int main(int argc, char *argv[]) {
	/* Local variables */
	netio_port *p;
	netio_socket *s;
	
	netio_callback socket_callback = {
		/* This callback is used for connecting */
		NULL,
		/* This callback is used when data is received */
		NULL,
		/* This callback is used when there's an error */
		NULL
	};
	
	/* Initialize interfaces */
	post_message(" + Initializing Interface...", 1);
	if (!netio.init()) {
		/* Failed to initialize */
		post_message("", 0);
		return 1;
	}
	
	/* Create an asynchronous port */
	post_message(" + Creating a port...", 1);
	if (!(p = netio.port.create())) {
		/* Failed to create a port */
		post_message("", 0);
		return 2;
	}
	
	/* Create a TCP socket */
	post_message(" + Creating TCP socket...", 1);
	if (!(s = netio.socket.create(NETIO_TCP, &socket_callback))) {
		/* Failed to create a TCP socket */
		post_message("", 0);
		return 3;	
	}
	
	/* Set the socket to nonblocking */
	post_message(" + Non-blocking on TCP socket...", 1);
	if (!netio.socket.block(s, 0)) {
		/* Failed to set non-blocking */
		post_message("", 0);
		return 4;
	}
	
	/* Add the socket to the port */
	post_message(" + Adding TCP socket to port...", 1);
	if (!netio.port.add(p, s)) {
		/* Failed to set non-blocking */
		post_message("", 0);
		return 5;	
	}
	
	/* Resolve google.com into an IP address */
	// netio.dns.resolve("www.google.com", on_	resolve);
	post_message("", 1);
	return 0;
}
