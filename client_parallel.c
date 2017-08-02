#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define N 10000
#define M 100
char buffer[M*N];
int completed;
int in_index=0, out_index=0, clientfd;
float t;
FILE * f;

int (* P) (sem_t *);
int (* V) (sem_t *);

sem_t slots, items;

int GetBlockFromNet(char *buf) {
	send(clientfd, "ohoho", 5, 0);
	int r = recv(clientfd, buf, N, 0);
	if (r==0)
		return 1;
	return 0;
}

int WriteBlockToDisk(char *buf){
	//printf("%100s\n",buf);
	f = fopen("dump", "w");
	fprintf(f, "%10000s\n", buf);	
	fclose(f);
	return 0;
}

time_t start, end;
clock_t start_c, end_c;

void* producer() {
	//printf("preoducer\n");
	while(1) {
		P(&slots);
		completed = GetBlockFromNet(buffer+in_index*N);
		in_index = (in_index+1) % M;
		V(&items);
		if(completed){
			printf("producer break\n");
			break;
		}
	}
	return NULL;
}

void* consumer() {
	printf("consumer\n");
	while(1) {
		P(&items);
		WriteBlockToDisk(buffer+out_index*N);
		out_index = (out_index+1) % M;
		V(&slots);
		if(completed && out_index == in_index){
			printf("consumer break\n");
			break;
		}
	}
	//printf("%f\n", time(NULL)-t);;
	//printf("%f\n", time(NULL));
	return NULL;
}

int main(int argc, char **argv) {
	P = sem_wait;
	V = sem_post;
	sem_init(&slots, 0, N);
	sem_init(&items, 0, 0);
	
	if(argc != 2) {
		printf("usage: %s <ipaddr>",argv[0]);
		exit(0);
	}
	
	//f = fopen("dump", "w");

	if( (clientfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("create socket fail\n");
		exit(0);
	}

	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0 , sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(6666);
	if( inet_pton(AF_INET, argv[1], &serveraddr.sin_addr) <= 0) {
		printf("inet_pton fail\n");
		exit(0);
	}
	if( connect(clientfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
		printf("connect  fail\n");
		exit(0);
	}

	printf("======connect to server======\n");
	/*
	char sendline[1024];
	while(1) {
		fgets(sendline, 1024, stdin);
		if( send(clientfd, sendline, strlen(sendline), 0) < 0) {
			printf("send fail\n");
			exit(0);
		}
	}
	*/
	start = time(NULL);
	start_c = clock();
	pthread_t tid1, tid2;
	pthread_create(&tid1, NULL, producer, NULL);
	pthread_create(&tid2, NULL, consumer, NULL);
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);

	close(clientfd);
	//fclose(f);
	end = time(NULL);
	end_c = clock();
	//printf("time=%f\n", difftime(end,start));
	printf("time=%lf\n", (double)(end_c-start_c));
	return 0;
}	
