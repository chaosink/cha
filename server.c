#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define MAX 250

int main()
{
	int server_sockfd,client_sockfd;
	int server_len,client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	int result;
	fd_set readfds, testfds;
	char name[20][20];

	server_sockfd=socket(AF_INET, SOCK_STREAM, 0);
	server_address.sin_family=AF_INET;
	server_address.sin_addr.s_addr=htonl(INADDR_ANY);
	server_address.sin_port=htons(9734);
	server_len=sizeof(server_address);

	bind(server_sockfd,(struct sockaddr *)&server_address,server_len);
	listen(server_sockfd,5);

	FD_ZERO(&readfds);
	FD_SET(server_sockfd,&readfds);

	while(1)
	{
		char message[MAX+1];
		int fd,fdd;
		int nread;

		testfds=readfds;
		printf("server waiting\n");
		result=select(FD_SETSIZE,&testfds,(fd_set *)0,(fd_set *)0,(struct timeval *)0);
		if(result<1)
		{
			perror("server");
			exit(1);
		}

		for(fd=0;fd<FD_SETSIZE;fd++)
		{
			if(FD_ISSET(fd,&testfds))
			{
				if(fd==server_sockfd)
				{
					client_len=sizeof(client_address);
					client_sockfd=accept(server_sockfd,(struct sockaddr *)&client_address,(socklen_t *)&client_len);
					FD_SET(client_sockfd,&readfds);
					memset(name[client_sockfd],0,sizeof(name[client_sockfd]));
					read(client_sockfd,name[client_sockfd],MAX);
					printf("adding client on fd %d: %s\n",client_sockfd,name[client_sockfd]);
					if(strcmp(name[client_sockfd],"admin")==0) continue;
					for(fdd=0;fdd<FD_SETSIZE;fdd++)
						if(FD_ISSET(fdd,&readfds) && (fdd!=server_sockfd))
						{
							struct tm *tm_ptr;
							time_t the_time;
							time(&the_time);
							tm_ptr=localtime(&the_time);

							sprintf(message,"* %s has been online!\n@ %02d/%02d/%02d %02d:%02d:%02d",
								name[client_sockfd],
								tm_ptr->tm_year-100,tm_ptr->tm_mon+1,tm_ptr->tm_mday,
								tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec);
							write(fdd,message,MAX);
						}
				}
				else
				{
					ioctl(fd,FIONREAD,&nread);

					if(nread==0)
					{
						close(fd);
						FD_CLR(fd,&readfds);
						printf("removing client on fd %d\n", fd);
						if(strcmp(name[fd],"admin")==0) continue;
						for(fdd=0;fdd<FD_SETSIZE;fdd++)
							if(FD_ISSET(fdd,&readfds) && (fdd!=server_sockfd))
							{
								struct tm *tm_ptr;
								time_t the_time;
								time(&the_time);
								tm_ptr=localtime(&the_time);

								sprintf(message,"* %s has been offline!\n@ %02d/%02d/%02d %02d:%02d:%02d",
									name[client_sockfd],
									tm_ptr->tm_year-100,tm_ptr->tm_mon+1,tm_ptr->tm_mday,
									tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec);
									write(fdd,message,MAX);
							}
					}
					else
					{
						memset(message,0,sizeof(message));
						read(fd,message,MAX);
						for(fdd=0;fdd<FD_SETSIZE;fdd++)
							if(FD_ISSET(fdd,&readfds) && (fdd!=server_sockfd))
							{
								char mess[300];
								printf("serving client on fd %d\n",fdd);
								sprintf(mess,"%s: %s",name[fd],message);
								write(fdd,mess,MAX);
							}
					}
				}
			}
		}
	}
}
