/*
 * 
 */
 
#if !defined(NETIO_H)
# define NETIO_H

/*
 * Include
 */

/* Dependent */
# if defined(WIN32)
#  define WIN32_LEAN_AND_MEAN
#  define WIN32_EXTRA_LEAN
#  include <windows.h>
#  include <winsock2.h>
#  include <mswsock.h>
# endif

/* Independent */
#include <stdlib.h>

/*
 * 
 */ 
# if defined(WIN32)
typedef HANDLE netio_port;
# else
typedef int netio_port;
# endif 
 
/*
 * 
 */
typedef enum {
	NETIO_TCP,
	NETIO_UDP
} netio_socket_enum;

/*
 * 
 */
typedef struct {
	/* Socket descriptor */
	int s;
} netio_socket;

/*
 * Callbacks
 */
typedef struct {
	void (*on_connect)(netio_socket *s);
	void (*on_recv)(netio_socket *s);
	void (*on_error)(int error_code, netio_socket *s);
} netio_callback;
 
/*
 * Interface
 */
typedef struct {
	netio_port (*create)();
	int (*add)(netio_port p, netio_socket *s);
} netio_port_type;
 
typedef struct {
	netio_socket *(*create)(netio_socket_enum type, netio_callback *c);
	int (*block)(netio_socket *s, int block);
} netio_socket_type;
 
typedef struct {
	/* netio.init() */
	int(*init)();
	
	/* port namespace */
	netio_port_type port;
	
	/* socket namespace */
	netio_socket_type socket;
} netio_type;
 
/*
 * Export/Import
 */ 
extern netio_type netio;

#endif