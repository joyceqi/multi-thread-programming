#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>

#define MAXLINE 1024
#define N 10000
#define TIMES 1000
void *thread(void *vargp);

int main() {
	int listenfd, connfd, *connfdp;

	if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("create socket fail\n");
		exit(0);
	}

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0 , sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(6666);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if( bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
		printf("bind socket fail\n");
		exit(0);
	}

	if( listen(listenfd, 10) == -1) {
		printf("listen fail\n");
		exit(0);
	}

	printf("======waiting for client's request======\n");
	struct sockaddr_in clientaddr;
	memset(&clientaddr, 0 , sizeof(clientaddr));
	clientaddr.sin_family = AF_INET;
	int clientlen;
	//char buf[MAXLINE];
	//int n;
	pthread_t tid;
	while(1) {
		connfdp = (int*)malloc(sizeof(int));
		if( (*connfdp = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen)) < 0 ) {
			printf("accept fail\n");
			continue;
		}
		pthread_create(&tid,NULL,thread,connfdp);
		/*
		printf("accept connection from %d %d\n",clientlen,clientaddr.sin_port);
		n = recv(connfd, buf, MAXLINE, 0);
		buf[n] = '\0';
		printf("recv msg from client: %s", buf);
		close(connfd);
		*/
	}
	return 0;
}

char buf[N+2];

void *thread(void *vargp) {
	int n,i;
	printf("new thread");
	int connfd = *((int*)vargp);
	pthread_detach(pthread_self());
	free(vargp);
	for (i=0;i<TIMES;++i){
		printf("heiheihei\n");
		n = recv(connfd, buf, MAXLINE, 0);
		//if (n<=0){
		//	printf("%d\n", n);
		//	return NULL;
		//}
		printf("here\n");
		buf[n] = '\0';
		printf("there\n");
		//printf("recv msg: %s", buf);
		//memset(buf, 'a', 100);
		send(connfd, buf, N, 0);
	}
	close(connfd);
	printf("nani\n");
	return NULL;
}
