#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>

/*Allowed functions: write, close, select, socket, accept,
 listen, send, recv, bind, strstr, malloc, realloc, free,
 calloc, bzero, atoi, sprintf, strlen, exit, strcpy, strcat, memset
*/

struct clientdata
{
	int		id;
	char	*msg;
}typedef t_clientdata;

t_clientdata	clientBUFF[1024];
fd_set readFD, actualFD, writeFD;
char writeBUFF[400200];
int maxFD = 0, maxID = 0;

void debugprint(char *print)
{
	write(1, print, strlen(print));
	write(1, "\n", 1);
}

void sendall(int senderfd)
{
	for (int fd = 0; fd <= maxFD; fd++)
	{
		if (FD_ISSET(fd, &writeFD) && fd != senderfd)
		{
			send(fd, &writeBUFF, strlen(writeBUFF), 0);
		}
	}
	bzero(&writeBUFF, sizeof(char [400000]));
}

void printerror(char *message)
{
	write(2, message, strlen(message));
	exit(1);
}

int main(int argc, char **argv)
{
	if (argc != 2)
		printerror("Wrong number of arguments\n");

	int sockfd;
	socklen_t len;
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		printerror("Fatal error\n");

	//bzero(&clientBUFF, sizeof(t_clientdata [1024]));
	bzero(&servaddr, sizeof(servaddr));
	FD_ZERO(&actualFD);
	maxFD = sockfd;
	FD_SET(sockfd ,&actualFD);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(argv[1]));

	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		printerror("Fatal error\n");

	if (listen(sockfd, 10) != 0)
		printerror("Fatal error\n");

	while (42)
	{
		readFD = writeFD = actualFD;

		if (select(maxFD + 1, &readFD, &writeFD, NULL, NULL) < 0)
			continue;
		for (int fd = 0; fd <= maxFD; fd++)
		{
			if (FD_ISSET(fd, &readFD) && sockfd == fd)
			{
				int newfd = accept(sockfd, (struct sockaddr *)&servaddr, &len);
				if (newfd < 0)
					continue;
				if (newfd > maxFD)
					maxFD = newfd;
				clientBUFF[newfd].id = maxID++;
				FD_SET(newfd, &actualFD);
				sprintf(writeBUFF, "server: client %d just arrived\n", clientBUFF[newfd].id);
				sendall(newfd);
				break;
			}
			if (FD_ISSET(fd, &readFD) && sockfd != fd)
			{
				char readBUFF[400000];
				bzero(readBUFF, sizeof(readBUFF));
				int recvbyte = 1;
				while (recvbyte == 1 && readBUFF[strlen(readBUFF) - 1] != '\n')
					recvbyte = recv(fd, readBUFF + strlen(readBUFF), 1, 0);
				if (recvbyte <=0)
				{
					sprintf(writeBUFF, "server: client %d just left\n", clientBUFF[fd].id);
					sendall(fd);
					FD_CLR(fd, &actualFD);
					close(fd);
					break ;
				}
				else
				{
					sprintf(writeBUFF, "client %d: %s", clientBUFF[fd].id, readBUFF);
					sendall(fd);
				}
			}
		}
	}
}
