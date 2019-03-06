#include <winsock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#include <STDIO.H>
#include <stdlib.h>
#define USER_SEND_MAX 280
#define BROADCAST_MAX 500
char name[20];
unsigned int _stdcall Client_thread(SOCKET socket){
    while(1){
		char recData[BROADCAST_MAX];
		int ret = recv(socket, recData, BROADCAST_MAX, 0);
		if (ret > 0)
		{
		    recData[ret] = 0x00;
			printf("%s\n",recData);
			//printf("%s\r\n",charArray);
		}else{
            printf("receive error.");
            closesocket(socket);
            WSACleanup();
            return -1;
		}
	}
}

int main(int argc, char* argv[])
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;


	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}

	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("invalid socket !");
		return 0;
	}

	struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (connect(sclient, (struct sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("connect error !");
		closesocket(sclient);
		return 0;
	}
	//for (int i = 0; i < 50; i++)
	//{
    printf("please input your name :");
    gets(name);
    send(sclient, name, strlen(name), 0);
    HANDLE handle=_beginthreadex(NULL, 0, (unsigned int (_stdcall*)(void *))Client_thread,sclient, 0, NULL);
	while(1){
        char sendData[USER_SEND_MAX];
        //printf("$%s: ",name);
        gets(sendData);
        if(strcmp(sendData,"#Exit")==0){
            TerminateThread(handle , 0 );
            send(sclient,sendData,strlen(sendData),0);
            closesocket(sclient);
            WSACleanup();
            break;
        }
        /*
        char charArray[500];
        strcat(charArray,"From ");
        strcat(charArray,name);
        strcat(charArray,":");
        strcat(charArray,sendData);
        */
        //printf("sendData is %s\n",sendData);
		send(sclient, sendData, strlen(sendData), 0);
	}


	system("pause");
	return 0;
}
