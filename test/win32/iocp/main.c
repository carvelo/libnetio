#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <winsock2.h>
#include <mswsock.h>
#include <stdio.h>

#define CONNECTEX_PARAMS \
	SOCKET s,\
  const struct sockaddr *name,\
  int namelen,\
  PVOID lpSendBuffer,\
  DWORD dwSendDataLength,\
  LPDWORD lpdwBytesSent,\
  LPOVERLAPPED lpOverlapped

typedef BOOL(PASCAL*ConnectExType)(CONNECTEX_PARAMS);
ConnectExType ConnectEx; 
	
BOOL WSAConnectEx(CONNECTEX_PARAMS) {
	/* Local variables */
	GUID ConnectExId = WSAID_CONNECTEX;
	DWORD ConnectExRet;
	
	if (!ConnectEx) {
		if (WSAIoctl(
			s, 
			SIO_GET_EXTENSION_FUNCTION_POINTER, 
			&ConnectExId,
			sizeof(ConnectExId),
			&ConnectEx,
			sizeof(void*),
			&ConnectExRet,
			NULL,
			NULL
		)) {
			return 0;
		}
	}
	return ConnectEx(s, name, namelen, lpSendBuffer, dwSendDataLength, lpdwBytesSent, lpOverlapped);
}

/* 
 * Global variables 
 * ----------------
 * This section of our code will house variables that are
 * accessible throughout any scope.
 */

/* I/O Completion Port handle */
HANDLE global_iocp;


void welcome_message() {
	puts("\n IOCP TEST\n"
			 " ---------");
}

char winsock_init() {
	WSADATA wsadata;
	return !WSAStartup(MAKEWORD(2,2), &wsadata);
}

HANDLE iocp_create_port(HANDLE port_handle, HANDLE file_handle) {
	/* Just create the port! */
	return CreateIoCompletionPort(
		/* 
		 * Param 1: File Handle
		 * --------------------
		 * The file handle to insert into the port. When creating
		 * a new completion port, the value must be INVALID_HANDLE_VALUE
		 */
		(port_handle) ? file_handle : INVALID_HANDLE_VALUE, 
		
		/*
		 * Param 2: Existing Completion Port
		 * ---------------------------------
		 * This will be the handle to the existing completion port.
		 * If creating a new completion port, the value can be NULL.
		 */
		(port_handle) ? port_handle : NULL,
		
		/*
		 * Param 3: Completion Key
		 * -----------------------
		 * This is supposedly a pointer that is associated with the file handle
		 * throughout its life cycle in IOCP. Since we're creating an IOCP, we
		 * can leave this as NULL.
		 */
		0,
		
		/*
		 * Param 4: Number of Concurrent Threads
		 * -------------------------------------
		 * The amount of threads allowed to handle requests in the port. We'll
		 * set this to 0, which means the port will use as many threads are there
		 * are logical cores.
		 */
		0
	);
}

SOCKET iocp_create_socket() {
	return WSASocket(
		/*
		 * Param 1: Address Family
		 * -----------------------
		 * Specify the address family for the socket. For
		 * this function, we'll use AF_INET, which is IPv4
		 */
		AF_INET, 
		
		/*
		 * Param 2: Socket type
		 * --------------------
		 * Specify the socket type. For this function, we will
		 * use SOCK_STREAM, which is used for TCP.
		 */
		SOCK_STREAM, 
		
		/*
		 * Param 3: Socket protocol
		 * ------------------------
		 * We'll specify TCP.
		 */
		IPPROTO_TCP, 
		
		/*
		 * Param 4: Protocol Info
		 * ----------------------
		 * This is a pointer to a WSAPROTOCOL_INFO structure that
		 * will determine the socket's characteristics. We can leave
		 * this NULL.
		 */
		NULL,
		
		/*
		 * Param 5: Socket group
		 * ---------------------
		 * 
		 */
		0, 
		
		/*
		 * Param 6: Flags
		 * --------------
		 * Specify the flags on the socket. Since we want to
		 * take advantage of IOCP, we'll specify that this socket
		 * will utilize overlapped(asynchronous) operations.
		 */
		WSA_FLAG_OVERLAPPED
	);
}

int iocp_bind(SOCKET s, unsigned long ip, unsigned short port) {
	/* Local variables */
	struct sockaddr_in sin;
	
	/* Fill in the struct */
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = ip;
	
	/* Bind the socket */
	return bind(s, (struct sockaddr*)&sin, sizeof(sin));
}

BOOL iocp_connect(SOCKET s, unsigned long ip, unsigned short port) {
	/* Local variables */
	struct sockaddr_in sin;
	
	/* Set up the address struct */
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = ip;
	
	OVERLAPPED ol;
	memset(&ol, 0, sizeof(ol));
	
	return WSAConnectEx(
		/* Param 1: Socket */
		s,
		
		/* Param 2: Socket address */
		(struct sockaddr*)&sin,
		
		/* Param 3: Size of the sockaddr struct */
		sizeof(sin),
		
		/* The buffer to be sent after connection */
		NULL,
		
		/* Size of the buffer(0, since it's ignored anyway) */
		0,
		
		/* Pointer to record the amount of bytes sent */
		NULL,
		
		/* Overlapped struct */
		&ol
	);
}

BOOL iocp_check(HANDLE port, DWORD timeout) {
	/* Local variables */
	DWORD bytes;
	DWORD *completion_key;
	OVERLAPPED *ol;
	
	BOOL ret = GetQueuedCompletionStatus(
		/* Param 1: Port handle
		 * --------------------
		 * Specifies the port to use
		 */
		port,
		
		/*
		 * Param 2: Bytes transferred
		 * --------------------------
		 */
		&bytes,
		
		/*
		 *
		 */ 
		(PULONG_PTR)&completion_key,
		
		/*
		 *
		 */
		&ol,
		
		/*
		 *
		 */
		timeout
	);
	
	return ret;
}

int iocp_print_error(int ret) {
	printf("   - Error #%i\n", WSAGetLastError());
	return ret;
}

int application_init() {
	/* Local variables */
	SOCKET s;
	
	/* Attempt to initialize Winsock 2.0 */
	puts(" + Initializing winsock 2.0");
	if (!winsock_init()) {
		/* There was an error */
		return iocp_print_error(1);
	}
	
	/* Create an I/O Completion Port (WIN32 ONLY) */
	puts(" + Creating an I/O Completion Port");
	if (!(global_iocp = iocp_create_port(NULL, NULL))) {
		/* There was an error */
		return iocp_print_error(2);
	}
	
	/* Create a socket */
	puts(" + Creating a socket");
	if ((s = iocp_create_socket()) == INVALID_SOCKET) {
		return iocp_print_error(3);
	}
	
	/* Associate the socket with the socket */
	puts(" + Associating socket with port");
	if (!iocp_create_port(global_iocp, (HANDLE)s)) {
		return iocp_print_error(4);
	}
	
	/* Connect to google */
	puts(" + Binding socket");
	if (iocp_bind(s, INADDR_ANY, 0)) {
		return iocp_print_error(5);
	}
	
	/* Connect to google */
	int c = GetTickCount();
	puts(" + Connecting to Google");
	if (!iocp_connect(s, inet_addr("74.125.225.199"), 80)) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			return iocp_print_error(6);
		}
	}
	
	/* Check queue for completion */
	if (!iocp_check(global_iocp, INFINITE)) {
		return iocp_print_error(7);
	}
	
	printf(" + Took %lums to connect!\n", GetTickCount() - c);
	
	/* Success! */
	return 0;
}

int main(int argc, char *argv[]) {
	/* Display the welcome message */
	welcome_message();
	
	/* Initialize the rest of the application */
	application_init();
	
	return ERROR_SUCCESS;
}
