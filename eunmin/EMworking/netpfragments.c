/* IP address structure */

struct in_addr
{
    uint32_t s_addr; /* Address in network byte order (big-endian) */
};

#include <sys/socket.h>
int bind(int sockfd, const struct sockaddr *addr,
         socklen_t addrlen);

// Returns : 0 if OK, -1 on error

#include <sys/socket.h>
int connect(int clientfd, const struct sockaddr *addr,
            socklen_t addrlen);
// Returns: 0 if OK, -1 on error

#include <sys/types.h>
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
// Returns : nonnegative descriptor if OK, −1 on error


#include <sys/socket.h>
int accept(int listenfd, struct sockaddr *addr, int *addrlen);

// Returns: nonnegative connected descriptor if OK, −1 on error


