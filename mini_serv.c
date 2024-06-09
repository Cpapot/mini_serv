#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define	BUFF		400000
#define	MAXUSER		69000

int		FDtoID[MAXUSER];
char	writeBUFF[BUFF + 100];
int		maxID = 0, maxFD;
fd_set	readFD, writeFD, actualFD;


void printerror(char *msg)
{
	write(2, msg, strlen(msg));
	exit(1);
}

void sendall(int sender)
{
	for (int fd = 0; fd <= maxFD; fd++)
	{
		if (FD_ISSET(fd, &writeFD) && fd != sender)
			send(fd, writeBUFF, strlen(writeBUFF), 0);
	}
	bzero(&writeBUFF, sizeof(writeBUFF));
}

int main(int argc, char **argv)
{
	if (argc != 2)
		printerror("Wrong number of arguments\n");

	struct sockaddr_in servaddr, cli;
	socklen_t	len = sizeof(cli);
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		printerror("Fatal error\n");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(argv[1]));

	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		printerror("Fatal error\n");

	if (listen(sockfd, 4096) != 0)
		printerror("Fatal error\n");

	maxFD = sockfd;
	FD_ZERO(&actualFD);
	FD_SET(sockfd, &actualFD);

	while (42)
	{
		readFD = writeFD = actualFD;

		if (select(maxFD + 1, &readFD, &writeFD, NULL, NULL) < 0)
			continue;

		if (FD_ISSET(sockfd, &readFD))
		{
			int newfd = accept(sockfd, (struct sockaddr *)&cli, &len);
			if (newfd < 0)
				printerror("Fatal error\n");
			if (newfd > maxFD)
				maxFD = newfd;

			FDtoID[newfd] = maxID++;
			FD_SET(newfd, &actualFD);

			sprintf(writeBUFF, "server: client %d just arrived\n", FDtoID[newfd]);
			sendall(newfd);

			continue;
		}
		for (int fd = 0; fd <= maxFD; fd++)
		{
			if (FD_ISSET(fd, &readFD))
			{
				int		nbytes = 1;
				char	readBUFF[BUFF];

				bzero(&readBUFF, sizeof(readBUFF));
				while (nbytes == 1 && readBUFF[strlen(readBUFF) - 1] != '\n')
					nbytes = recv(fd, readBUFF + strlen(readBUFF), 1, 0);

				if (nbytes <= 0)
				{
					sprintf(writeBUFF, "server: client %d just left\n", FDtoID[fd]);
					FD_CLR(fd, &actualFD);
					close(fd);
				}
				else
					sprintf(writeBUFF, "client %d: %s", FDtoID[fd], readBUFF);
				sendall(fd);
			}
		}
	}
}
