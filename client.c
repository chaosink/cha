#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define MS_MAX 250
#define NM_MAX 25

int main(int argc,char**argv)
{
	int sockfd;
	struct sockaddr_in address;
	int result;
	char message[MS_MAX+1]={'\0'};
	fd_set readfds,testfds;
	char name[NM_MAX];

	if(argc<2)
	{
		puts("Please input your name:");
		fgets(name,NM_MAX,stdin);
		name[strlen(name)-1]='\0';
	}
	else strcpy(name,argv[1]);

	sockfd=socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family=AF_INET;
	address.sin_addr.s_addr=inet_addr("10.110.28.9");
	address.sin_port=htons(9734);

	result=connect(sockfd,(struct sockaddr *)&address,sizeof(address));
	if(result==-1)
	{
		perror("oops: client");
		exit(1);
	}
	FD_ZERO(&readfds);
	FD_SET(sockfd,&readfds);
	FD_SET(0,&readfds);

	write(sockfd,name,MS_MAX);

	while(1)
	{
		int quit=0;

		testfds=readfds;
		result=select(FD_SETSIZE,&testfds,NULL,NULL,NULL);
		if(FD_ISSET(0,&testfds))
		{
			fgets(message,MS_MAX,stdin);
			message[strlen(message)-1]='\0';
			write(sockfd,message,MS_MAX);
			if(strncmp(message,"quit",4)==0) quit=1;;
		}
		read(sockfd,message,MS_MAX);
		printf("%s\n",message);
		
		if(quit) break;
	}

	close(sockfd);
	exit(0);
}
