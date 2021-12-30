#if defined WIN32
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h> /* for memset() */
#include "protocol.h"
#define MAX 255
#define PORT 48000

/*Print error handler.*/
void ErrorHandler(char *errorMessage) {
	printf(errorMessage);
}

/*Function that cleans the socket.*/
void ClearWinSock() {
	#if defined WIN32
	WSACleanup();
	#endif
}

int main(void) {
	#if defined WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2 ,2), &wsaData);
	if (iResult != 0) {
		printf ("error at WSASturtup\n");
		return EXIT_FAILURE;
	}
	#endif
	int sock;
	/*Socket initialization.*/
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		ErrorHandler("socket() failed");
	struct sockaddr_in servAddr;
	struct sockaddr_in fromAddr;
	struct hostent *host;
	int msgLen;
	int respStringLen;
	unsigned int fromSize;
	/*Request name and port from the user.*/
	char nameServer[MAX];
	int port;
	printf("Enter the Server name and port.(name:port)\n");
	char serverInfo[MAX];
	scanf("%s",serverInfo);
	if(serverInfo!=NULL){
		char * pch;
		pch = strtok (serverInfo,":");
		strcpy(nameServer,pch);
		pch = strtok (NULL, ":");
		port = atoi(pch);
	} else {
		strcpy(nameServer,"localhost");
		port=48000;
	}
	host = gethostbyname(nameServer);
	/*Saving the data entered by the user.*/
	servAddr.sin_family = PF_INET;
	servAddr.sin_port = htons(port);
	servAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*) host->h_addr_list[0]));
	while(1){
		/*Request for insertion of the operation to the user and sending it to the Server*/
		int incorrect_operators=1;
		struct msgStruct msg;
		do{
			printf("Enter the operation:");
			struct msgStruct msg;
			char c=getchar();
			scanf("%c %d %d",&msg.operator,&msg.n1,&msg.n2);
			if ((msgLen = sizeof(msg)) > MAX)
					ErrorHandler("echo word too long");
			if(msg.operator == '+' || msg.operator == '-' || msg.operator == '*' || (msg.operator == '/' && msg.n2!=0)){
				incorrect_operators=0;
			}else if(msg.operator=='='){
				/*Closing the client.*/
				closesocket(sock);
				return EXIT_SUCCESS;
			}else {
				ErrorHandler("Wrong operator, re-enter data.\n");
				incorrect_operators=1;
			}
			if(incorrect_operators==0){
				/*Sending the operation.*/
				if (sendto(sock, (struct msgStruct*) &msg, msgLen, 0, (struct sockaddr*)&servAddr, sizeof(servAddr)) != msgLen)
					ErrorHandler("sendto() sent different number of bytes than expected");
				/*Receiving the result.*/
				fromSize = sizeof(fromAddr);
				respStringLen = recvfrom(sock, (struct msgStruct*) &msg, MAX, 0, (struct sockaddr*)&fromAddr, &fromSize);
				if (servAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr){
					fprintf(stderr, "Error: received a packet from unknown source.\n");
					exit(EXIT_FAILURE);
				}
				printf("Result received from the server %s , ip %s: %d %c %d = %d.\n",host->h_name,inet_ntoa(*(struct in_addr*) host->h_addr_list[0]),msg.n1,msg.operator,msg.n2,msg.n3);
			}
		} while (incorrect_operators==1);
	}
	closesocket(sock);
	system("pause");
	return EXIT_SUCCESS;
}

