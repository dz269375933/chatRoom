#include <head.h>

struct SocketObject{
    char name[30];
    SOCKET socket;
    char ring[30];
};
/*
struct RingObject{
    char *name;
    struct RingObject *next;
};
*/
int g_count;

struct SocketObject sockets[THREAD_NUM];

void getUserList(char userList[]);
unsigned int _stdcall ThreadFun(void* index);
int is_begin_with(const char * str1,char *str2);



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

    //database

    if(insertData()==0){
        return 0;
    }




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
    //signal(SIGPIPE,SIG_IGN);
    while(1){
        SOCKET new_socket = accept(s,(struct sockaddr *)&client,&c);
        if(new_socket == INVALID_SOCKET || g_count>=THREAD_NUM){
            printf("accept error\n");
            //still need to be done
            //closesocket(new_socket);
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
unsigned int _stdcall ThreadFun(void* index){
    int private_index=-1;
    sqlite3 *db;
    char *err_msg=0;
    int rc = sqlite3_open("dzData", &db);
    printf("\nRC open Database = %d\n",rc);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    int thisIndex=*(int *)index;
    SOCKET new_socket=sockets[thisIndex].socket;
    char nameArray[20];
    char password[20];
    while(1){
        int ret = recv(new_socket, nameArray, 20, 0);
        if(ret <= 0){
            if(thisIndex==g_count){
                g_count--;
            }else{
                sockets[thisIndex]=sockets[g_count--];
            }
            printf("receive error.Closing thread and socket\n");
            closesocket(new_socket);
            WSACleanup();
            return -1;
        } else{
            nameArray[ret]=0x00;
            ret=recv(new_socket,password,20,0);
            if(ret <= 0){
                if(thisIndex==g_count){
                g_count--;
                }else{
                sockets[thisIndex]=sockets[g_count--];
                }
                printf("receive error.\n");
                closesocket(new_socket);
                WSACleanup();
                return -1;
            }
            password[ret]=0x00;

            //search database
            sqlite3_stmt * res;
            char *sql = "SELECT Name,Password FROM User WHERE Name = @id";
            int rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
            if (rc == SQLITE_OK) {
                int idx = sqlite3_bind_parameter_index(res, "@id");
                rc=sqlite3_bind_text(res, idx, nameArray, -1, SQLITE_STATIC);     // the string is static
                printf("\nRC sqltite_exec = %d -> %s\n",rc,sql);
            } else {
                fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            }
            //printf("input name :%s\n",nameArray);
            //printf("input password:%s\n",password);
            int step =sqlite3_step(res);
            if(step!=100 || strcmp(password,sqlite3_column_text(res, 1))!=0){
                printf("Password :%s..\n",sqlite3_column_text(res, 1));
                printf("input:%s..\n",password);
                char * message="Wrong username or password.";
                send(sockets[thisIndex].socket, message, strlen(message), 0);
            }else{
                printf("ACK\n",sqlite3_column_text(res, 1));
                char * returnAck="ACK";
                send(sockets[thisIndex].socket, returnAck, strlen(returnAck), 0);
                break;
            }
        }
    }
    //end loop


    //broadcast ring;
    strcpy(sockets[thisIndex].name,nameArray);
    int k;
    for(k=0;k<=g_count;k++){
        if(strcmp(sockets[k].ring,nameArray)==0){
            char * message[200];
            message[0]=0x00;
            strcat(message,"Ring :");
            strcat(message,nameArray);
            strcat(message," connect.");
            printf("%s\n",message);
            send(sockets[k].socket, message, strlen(message), 0);
        }
    }
    //sockets[thisIndex].name=nameArray;

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
        }else if(is_begin_with(revData,"#Private")==1){
            //printf(revData);
            //printf("|");
            printf("I got private.\n");
            char name[50];
            name[0]=0x00;
            char *p;
            strtok(revData," ");
            while((p=strtok(NULL," "))!=NULL){
                    //printf(p);
                //printf("\n");
               strcat(name,p);
               strcat(name," ");
            }
            if(strlen(name)==0){
                char* errorSend="Sorry, Please input name after #Private (there should be a space after Private).";
                send(sockets[thisIndex].socket, errorSend, strlen(errorSend), 0);
            }else{
                name[strlen(name)-1]=0x00;
                int j;
                printf(name);
                for(j=0;j<=g_count;j++){

                    if(strcmp(sockets[j].name,name)==0)break;

                }
                if(j>g_count){
                    char* errorSend="Sorry, I can't find name in user list.";
                    send(sockets[thisIndex].socket, errorSend, strlen(errorSend), 0);
                }else{
                    private_index=j;
                    char* message="Success be private.";
                    send(sockets[thisIndex].socket, message, strlen(message), 0);
                }
            }

            continue;
        }else if(strcmp(revData,"#Public")==0){
            private_index=-1;
            char* message="Success be private.";
            send(sockets[thisIndex].socket, message, strlen(message), 0);
            continue;
        }else if(is_begin_with(revData,"#Ring")==1){
            printf("I got Ring.\n");
            char name[50];
            name[0]=0x00;
            char *p;
            strtok(revData," ");
            while((p=strtok(NULL," "))!=NULL){
                    //printf(p);
                //printf("\n");
               strcat(name,p);
               strcat(name," ");
            }
            if(strlen(name)==0){
                char* errorSend="Sorry, Please input name after #Ring (there should be a space after Ring).";
                send(sockets[thisIndex].socket, errorSend, strlen(errorSend), 0);
            }else{
                name[strlen(name)-1]=0x00;
                int i;
                for(i=0;i<=g_count;i++){
                    if(strcmp(sockets[i].name,name)==0){
                        char* message="He\\She is online.";
                        send(sockets[thisIndex].socket, message, strlen(message), 0);
                    }
                }
                if(i<=g_count)continue;
                strcpy(sockets[thisIndex].ring,name);
                //printf("%p",sockets[thisIndex].ring);
                /*
                if(sockets[thisIndex].ring==NULL){
                    struct RingObject *head=(struct RingObject*)malloc(sizeof(struct RingObject));
                    strcpy(head->name,name);
                    head->next=NULL;
                    sockets[thisIndex].ring=head;
                    free(head);
                    printf("first\n");

                }else{
                    struct RingObject *q;
                    q=sockets[thisIndex].ring;

                    while(q->next != NULL){
                        printf("%s\n",q->name);
                        q=q->next;
                    }
                    printf("%s\n",name);
                    printf("236\n");
                    struct RingObject *node=(struct RingObject*)malloc(sizeof(struct RingObject));
                    printf("237\n");
                    printf("%p",node);
                    printf("%s\n",node->name);
                    strcpy(node->name,name);
                    printf("238\n");


                    printf("******* %s\n",strerror(errno));
                    printf("%p",node);

                    printf("sss\n");
                    node->next=NULL;
                    p->next=node;
                    printf("sss\n");
                    p=sockets[thisIndex].ring;


                }*/
                char* message="Success save ring.";
                send(sockets[thisIndex].socket, message, strlen(message), 0);
            }
            continue;
        }
        //printf("%s\n",revData);
        int i;
        char broadcast[BROADCAST_MAX];
        broadcast[0]=0x00;
        strcat(broadcast,"From ");
        strcat(broadcast,nameArray);
        strcat(broadcast,":");
        strcat(broadcast,revData);
        if(private_index>=0){
            send(sockets[private_index].socket, broadcast, strlen(broadcast), 0);
        }else{
            for(i=0;i<=g_count;i++){
            //printf("%s\n",sockets[i].name);
            //printf("%p\n",sockets[i].socket);
            send(sockets[i].socket, broadcast, strlen(broadcast), 0);
            }
        }

        broadcast[0]=0x00;

    }
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
int insertData(){
    sqlite3 *db;
    char *err_msg=0;
    int rc = sqlite3_open("dzData", &db);
    printf("\nRC open Database = %d\n",rc);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    char *sql = "DROP TABLE IF EXISTS User;"
                "CREATE TABLE User(Id INT, Name TEXT, Password TEXT);"
                "INSERT INTO User VALUES(1, 'dz', '123');"
                "INSERT INTO User VALUES(2, 'han', '123');"
                "INSERT INTO User VALUES(3, 'haipen', '123');"
                "INSERT INTO User VALUES(4, 'aaa', '123');"
                "INSERT INTO User VALUES(5, 'bbb', '123');"
                "INSERT INTO User VALUES(6, 'ccc', '123');"
                "INSERT INTO User VALUES(7, 'ddd', '123');"
                "INSERT INTO User VALUES(8, 'eee', '123');";
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    printf("\nRC sqltite_exec = %d\n\n",rc);
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 0;
    } else {
        fprintf(stdout, "Table created successfully\n");
    }
    return 1;

}
