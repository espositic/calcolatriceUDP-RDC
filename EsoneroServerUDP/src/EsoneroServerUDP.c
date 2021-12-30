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

/*Addition*/
int add(int n1, int n2){
	return n1+n2;
}

/*Multiplication*/
int mult(int n1, int n2){
	return n1*n2;
}

/*Subtraction*/
int sub(int n1, int n2){
	return n1-n2;
}

/*Division*/
int division(int n1, int n2){
	return n1/n2;
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
	struct sockaddr_in servAddr;
	struct sockaddr_in clntAddr;
	unsigned int cliAddrLen;
	int recvMsgSize;
	/*Socket initialization.*/
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		ErrorHandler("socket() failed");
	/*Construction of the server address.*/
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT);
	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	/*Socket binding.*/
	if ((bind(sock, (struct sockaddr *)&servAddr, sizeof(servAddr))) < 0)
		ErrorHandler("bind() failed");
	/*Communication with the client.*/
	printf("Waiting for a client to connect...\n");
	struct msgStruct msg;
	while(1) {
		cliAddrLen = sizeof(clntAddr);
		recvMsgSize = recvfrom(sock, (struct msgStruct*) &msg, MAX, 0, (struct sockaddr*)&clntAddr, &cliAddrLen);
		struct in_addr addr;
		addr.s_addr = inet_addr(inet_ntoa(clntAddr.sin_addr));
		struct hostent *host;
		host = gethostbyaddr((char *) &addr, 4, AF_INET);
		char* clientName = host->h_name;
		printf("Operation required: '%c %d %d' from the client %s, ip %s \n",msg.operator,msg.n1,msg.n2,clientName,inet_ntoa(clntAddr.sin_addr));
		/*Sending the result to the Client*/
		if(msg.operator=='+'){
			msg.n3=add(msg.n1,msg.n2);
			printf("Addition done.\n");
		} else if(msg.operator=='-'){
			msg.n3=sub(msg.n1,msg.n2);
			printf("Subtraction done.\n");
		} else if(msg.operator=='*'){
			msg.n3=mult(msg.n1,msg.n2);
			printf("Multiplication done.\n");
		} else if(msg.operator=='/'){
			msg.n3=division(msg.n1,msg.n2);
			printf("Division done.\n");
		} else {
			msg.operator='.';
			printf("Operation failed.\n");
		}
		if (sendto(sock, (struct msgStruct*) &msg, recvMsgSize, 0, (struct sockaddr *)&clntAddr, sizeof(clntAddr)) != recvMsgSize) {
			ErrorHandler("sendto() sent different number of bytes than expected");
			closesocket(sock);
		}
	}
	return EXIT_SUCCESS;
}

