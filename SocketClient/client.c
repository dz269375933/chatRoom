#include <winsock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#include <STDIO.H>
#include <stdlib.h>
#define USER_SEND_MAX 280
#define BROADCAST_MAX 500
#define FILE_DATA_MAX 8092
#include <ctype.h>
char name[20];
int is_begin_with(const char* s1,char* s2);
int has_space(char *s);
const char * file_end_ack="BDd8E@XLj605Dsx0zzveRJNhy0qKTQ2T";
const char * FILE_SEND="3q23mgU9hAltcjUMAMOtc@DA*qPgESH";
unsigned int _stdcall Client_thread(SOCKET socket){
    while(1){
		char recData[BROADCAST_MAX];
		int ret = recv(socket, recData, BROADCAST_MAX, 0);
		if (ret > 0)
		{
		    recData[ret] = 0x00;
			//printf("%s\r\n",charArray);
		}else{
            printf("receive error.");
            closesocket(socket);
            WSACleanup();
            return -1;
		}

		if(is_begin_with(recData,"UPLOAD_ACK")){
            char *fileName=NULL;
            strtok(recData,":");
            fileName=strtok(NULL," ");
            if(fileName==NULL){
                printf("receive file name error.\n");
                continue;
            }else{
                //printf("%s\n",p);
                FILE *fp;
                char* data[FILE_DATA_MAX];

                fp=fopen(fileName,"rb");
                if(fp==NULL){
                    printf("file dose not exist.\n");
                    continue;
                }

                while(1){
                    memset((void *)data,0,sizeof(data));
                    //printf("sss\n");
                    int length=fread(data,1,sizeof(data),fp);
                    if(length==0){
                        Sleep(500);
                        send(socket,file_end_ack,strlen(file_end_ack),0);
                        printf("File send Success\n");

                        break;
                    }
                    //printf("%s\n",data);
                    int ret=send(socket,data,length,0);
                    //putchar('.');
                    if(ret==SOCKET_ERROR){
                        printf("Failed to send file.It may be caused by incomplete file.\n");
                        break;
                    }
                }
                fclose(fp);
                continue;
            }
		}else if(strcmp(recData,FILE_SEND)==0){
		    //printf("%s\n",recData);
            printf("try to save file...\n");
            char fileName[100];
            fileName[0]=0x00;
            int ret = recv(socket, fileName, 100, 0);
            if(ret==SOCKET_ERROR){
                printf("failed to receive filename.\n");
                break;
            }
            //printf("fileName:%s\n",fileName);
            FILE* fp;
            char data[FILE_DATA_MAX];
            fp=fopen(fileName,"wb");
            if(fp==NULL){
                printf("fail to save file named %s.\n",fileName);
                break;
            }
            printf("start to save file...\n");

            while(1){
                //printf("receive file.\n");
                memset(data,0,sizeof(data));
                //char * temp[FILE_DATA_MAX];
                int length=recv(socket,data,sizeof(data),0);
                if(length==SOCKET_ERROR){
                    char * message="Fail to save file.It may be caused by incomplete file.";
                    printf("%s\n",message);
                    //send(socket,message,strlen(message),0);
                    break;
                }else if(strcmp(data,file_end_ack)==0){
                    char * message="Save file success.";
                    printf("%s\n",message);
                    //send(socket,message,strlen(message),0);
                    break;
                }else{
                    if(fwrite(data,1,length,fp)<length){
                        printf("Write file failed.\n");
                        break;
                    }
                }
            }
            //printf("finish.\n");
            fclose(fp);
            continue;
		};
		printf("%s\n",recData);

	}
	//end loop
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

    while(1){
        printf("please input your name :");
        gets(name);
        send(sclient, name, strlen(name), 0);
        char password[20];
        printf("please input your password :");
        gets(password);
        send(sclient, password, strlen(password), 0);

        char recData[100];
        int ret = recv(sclient, recData, 100, 0);
        //printf("%d\n",ret);

        if (ret > 0)
        {
            recData[ret] = 0x00;
            if(strcmp(recData,"ACK")==0){
                printf("Welcome!\n");
                break;
            }
            //printf("%s\r\n",charArray);
        }else{
            printf("receive error.\n");
            closesocket(socket);
            WSACleanup();
            return -1;
        }
    }


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
        }else if(is_begin_with(sendData,"#TrfU")==1){
            if(strlen(sendData)<=6 | has_space(sendData)==0){
                printf("Please input fileName after a blank space,\n");
                continue;
            }
            char * fileNameMessage[USER_SEND_MAX];
            strcat(fileNameMessage,sendData);
            char *fileName;
            strtok(fileNameMessage," ");
            fileName=strtok(NULL," ");
            if(fileName==NULL | strlen(fileName)==0){
                printf("Please input fileName after a blank space,\n");
                continue;
            }
            FILE *fp;
            char* data[FILE_DATA_MAX];

            fp=fopen(fileName,"rb");
            if(fp==NULL){
                printf("file dose not exist.\n");
                continue;
            }
            fclose(fp);
            send(sclient, sendData, strlen(sendData), 0);
            continue;


        }
        //printf("sendData:%s",sendData);
		send(sclient, sendData, strlen(sendData), 0);
	}


	system("pause");
	return 0;
}
int is_begin_with(const char * str1,char *str2)
{
  if(str1 == NULL || str2 == NULL)
    return -1;
  int len1 = strlen(str1);
  int len2 = strlen(str2);
  if((len1 < len2) || (len1 == 0 || len2 == 0))
    return -1;
  char *p = str2;
  int i = 0;
  while(*p != '\0')
  {
    if(*p != str1[i])
      return 0;
    p++;
    i++;
  }
  return 1;
}
int has_space(char* s1){
    int i;
    for(i=0;i<strlen(s1);i++){
        if(isspace(s1[i]))break;
    }
    //printf("%d\n",i);
    if(i<strlen(s1))return 1;
    else return 0;
}
