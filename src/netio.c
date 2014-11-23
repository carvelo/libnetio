#include <netio.h>

/*
 * Prototypes
 * ----------
 * We'll declare these here so that we do not
 * declare them in the header. We declare these
 * here in order to declare our interface.
 */
int netio_init();
netio_port netio_create_port();
int netio_add_port(netio_port p, netio_socket *s);
netio_socket *netio_create_socket(netio_socket_enum type, netio_callback *c);
int netio_block_socket(netio_socket *s, int block);

/*
 * Interface
 * ---------
 * This is the NetI/O interface. For the time being, it
 * will utilize a namespace approach.
 */
netio_type netio = {
	netio_init, 
	{
		netio_create_port,
		netio_add_port
	}, 
	{
		netio_create_socket,
		netio_block_socket
	}
};

/*
 * netio.init()
 * ------------
 * Initializes the NetI/O interface.
 */
int netio_init() {
#if defined(WIN32)
	/* In Microsoft Windows, we MUST initalize networking */
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2,2), &wsadata)) {
		/* There was an error with initialization */
		return 0;
	}
#endif

	/* Successful call */
	return 1;
}

/*
 * netio.port.add()
 * ----------------
 * Adds a socket to an existing port
 */
int netio_add_port(netio_port p, netio_socket *s) {
#if defined(WIN32)
	if (!CreateIoCompletionPort(
		/* File handle */
		(HANDLE)s->s,
		
		/* Port */
		p,
		
		/* Completion Key */
		(ULONG_PTR)NULL,
		
		/* Concurrent threads */
		0
	)) {
		return 0;
	}
#endif
	return 1;
}

netio_port netio_create_port() {
#if defined(WIN32)
	/* Local variables */
	HANDLE port;
	
	/* Create the port */
	port = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		NULL,
		0,
		0
	);
	
	if (port) {
		/* Successful call */
		return port;
	}
#elif defined(__linux__)
	epoll()
#elif defined(MAC_OS_X)
	kqueue();
#endif

	return 0;
}


/*
 * netio.socket.create()
 * ---------------------
 * Creates a socket. 
 */
netio_socket *netio_create_socket(netio_socket_enum type, netio_callback *c) {
	/* Local variables */
	int s;
	netio_socket *new_s;
	
	/* Initialize */
	s = 0;
	
#if defined(WIN32)
	s = WSASocket(
			AF_INET,
			SOCK_STREAM,
			IPPROTO_TCP,
			NULL,
			(GROUP)NULL,
			WSA_FLAG_OVERLAPPED
	);
	
	if (s == (int)INVALID_HANDLE_VALUE) {
		return 0;
	}
#endif
	/* Create the new socket object */
	new_s = (s) ? (netio_socket *)malloc(sizeof(netio_socket)) : NULL;
	memset(new_s, 0, sizeof(netio_socket));
	new_s->s = s;
	
	/* Return the new socket object */
	return new_s;
}

/*
 * netio.socket.block()
 * --------------------
 */
int netio_block_socket(netio_socket *s, int block) {
	/* Local variables */
	unsigned long yes;
	
	/* Set the local variable (used for ioctl) */
	yes = 1;
	
#if defined(WIN32)
	/* Local variables */
	unsigned long ret, retsize;
	
	if (WSAIoctl(
		/* Socket to perform the operation on */
		s->s,
		
		/* Non-blocking */
		FIONBIO,
		
		/* Pointer to the value 1. No blocking. */
		&yes,
		sizeof(yes),
		
		/* Return information */
		&ret,
		sizeof(ret),
		&	retsize,
		
		/* No overlapping */
		NULL,
		NULL
	)) {
		/* ioctl failed */
		return 0;
	}
#elif defined(O_NONBLOCK)
	/* Local variables */
	unsigned long flags;
	
	flags = fcntl(s->s, F_GETFL);
	fcntl(s->s, F_SETFL, flags | O_NONBLOCK);
#else
	ioctl(s->s, FIONBIO, &yes);
#endif

	return 1;
}
