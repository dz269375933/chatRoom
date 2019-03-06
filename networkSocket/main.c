#include <head.h>
struct SocketObject{
    char name[30];
    SOCKET socket;
};
struct SocketObject sockets[THREAD_NUM];
int g_count;
void getUserList(char userList[]);
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

unsigned int _stdcall ThreadFun(void* index){
    int thisIndex=*(int *)index;
    SOCKET new_socket=sockets[thisIndex].socket;
    char nameArray[20];
    int ret = recv(new_socket, nameArray, 20, 0);
    if(ret <= 0){
        printf("receive error.");
        closesocket(new_socket);
        WSACleanup();
        return -1;
    }else{
        nameArray[ret]=0x00;
        strcpy(sockets[thisIndex].name,nameArray);
        //sockets[thisIndex].name=nameArray;
    }
    while(1){
        char revData[USER_SEND_MAX];
        int ret = recv(new_socket, revData, USER_SEND_MAX, 0);
        if(ret <= 0){
            printf("connect error...\n");
            closesocket(new_socket);
            WSACleanup();
            return -1;
        }
        revData[ret] = 0x00;
        if(strcmp(revData,"#Exit")==0){
            if(thisIndex==g_count){
                g_count--;
            }else{
                sockets[thisIndex]=sockets[g_count--];
            }
            printf("a socket is closed.\n");
            return 0;
        }else if(strcmp(revData,"#ListU")==0){
            char userList[1000];
            getUserList(userList);
            send(sockets[thisIndex].socket, userList, strlen(userList), 0);
            userList[0]=0x00;
            continue;
        }else if(is_begin_with(revData,"#Private")){

        }
        printf("%s\n",revData);
        int i;
        char broadcast[BROADCAST_MAX];
        broadcast[0]=0x00;
        strcat(broadcast,"From ");
        strcat(broadcast,nameArray);
        strcat(broadcast,":");
        strcat(broadcast,revData);
        for(i=0;i<=g_count;i++){
            //printf("%s\n",sockets[i].name);
            //printf("%p\n",sockets[i].socket);
            send(sockets[i].socket, broadcast, strlen(broadcast), 0);
        }
        broadcast[0]=0x00;

    }
}
void getUserList(char userList[]){
    strcat(userList,"users\n----------\n");
    int i=0;
    for(i=0;i<=g_count;i++){
        strcat(userList,sockets[i].name);
        strcat(userList,"\n");
    }
    strcat(userList,"----------");
}
int main(int arg,char *argv[])
{
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server,client;
    int c;
    //char* message;
    //HANDLE handle[THREAD_NUM];
    g_count=-1;
    /*
    struct sockaddr_in server;
    char* message,server_reply[2000];
    int recv_size;
    */

    printf("Initialising Winsock ...\n ");
    if(WSAStartup(MAKEWORD(2,2),&wsa) !=0)
    {
        printf("Failed,error code : %d",WSAGetLastError());
        return 1;
    }
    printf("Initialised.\n");

    //Create a socket
    if((s = socket(AF_INET,SOCK_STREAM,0))== INVALID_SOCKET)
    {
        printf("Could not create socket:%d",WSAGetLastError());
        return 1;
    }

    printf("Socket created\n");

    //prepare the sockaddr_in structure
    server.sin_addr.s_addr=INADDR_ANY;//google.fr
   // server.sin_addr.s_addr=inet_addr("205.178.189.131");//bourse.com

    server.sin_family = AF_INET;//ipv4
    server.sin_port = htons(8888); // port number 80

    //Bind
    if(bind(s,(struct sockaddr *)&server,sizeof(server))==SOCKET_ERROR)
    {
        printf("Bind failed with error code:%d",WSAGetLastError());
    }
    puts("Bind done");

    listen(s,3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");


    c=sizeof(struct sockaddr_in);

    while(1){
        SOCKET new_socket = accept(s,(struct sockaddr *)&client,&c);
        if(new_socket == INVALID_SOCKET || g_count>=THREAD_NUM){
            printf("accept error\n");
            closesocket(new_socket);
            break;
        }
        printf("connected...\r\n");
        struct SocketObject socketObject;
        socketObject.socket=new_socket;
        sockets[++g_count]=socketObject;
        _beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))ThreadFun, &g_count, 0, NULL);

    }
    WSACleanup();
    return 0;
}
