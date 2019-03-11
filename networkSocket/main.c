#include <head.h>

struct SocketObject{
    char name[30];
    unsigned address;
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
const char* FILE_SEND="3q23mgU9hAltcjUMAMOtc@DA*qPgESH";
char* change_index="K3LvzpYnSQ27wtwM";

struct SocketObject sockets[THREAD_NUM];
void viewFiles(char * path,char * result);
int findThisIndex(unsigned address);
void saveLog(char * message);

void getUserList(char userList[]);
unsigned int _stdcall ThreadFun();
int is_begin_with(const char * str1,char *str2);
int insertData();



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
        time_t rawtime;
        struct tm * timeinfo;
        time(&rawtime);
        timeinfo=localtime(&rawtime);
        char *ipv4=inet_ntoa(client.sin_addr);
        printf("@%s connected...\n",ipv4);
        char saveMessage[50];
        sprintf(saveMessage,("Time:%s\t@%s connected\r\n"),asctime(timeinfo),ipv4);
        printf("%s",saveMessage);
        saveLog(saveMessage);


        unsigned threadAddress;
        _beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))ThreadFun, NULL, 0, &threadAddress);
        struct SocketObject socketObject;
        socketObject.address=threadAddress;
        socketObject.socket=new_socket;
        sockets[++g_count]=socketObject;


    }
    WSACleanup();
    return 0;
}
unsigned int _stdcall ThreadFun(){
    unsigned thisId=GetCurrentThreadId();
    const char* fileTitle="file/";
    const char* file_end_ack="BDd8E@XLj605Dsx0zzveRJNhy0qKTQ2T";
    int private_index=-1;
    sqlite3 *db;
    //char *err_msg=0;
    int rc = sqlite3_open("dzData", &db);
    //printf("\nRC open Database = %d\n",rc);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    //int thisIndex=*(int *)index;
    SOCKET new_socket=sockets[findThisIndex(thisId)].socket;
    char nameArray[20];
    char password[20];

    while(1){
        int ret = recv(new_socket, nameArray, 20, 0);
        if(ret <= 0){

            printf("receive error.Closing thread and socket\n");
            closesocket(new_socket);
            WSACleanup();
            return -1;
        } else{
            nameArray[ret]=0x00;
            ret=recv(new_socket,password,20,0);
            if(ret <= 0){
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
                //printf("\nRC sqltite_exec = %d -> %s\n",rc,sql);
            } else {
                fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            }
            //printf("input name :%s\n",nameArray);
            //printf("input password:%s\n",password);
            int step =sqlite3_step(res);
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            if(step!=100 || strcmp((const char *)password,(const char *)sqlite3_column_text(res, 1))!=0){
                char * message="Wrong username or password.";
                send(new_socket, message, strlen(message), 0);
                char saveLogMessage[100];
                sprintf(saveLogMessage,("Time:%s\t%s login failed\n"),asctime(timeinfo),nameArray);
                saveLog(saveLogMessage);
            }else{
                //printf("ACK\n",sqlite3_column_text(res, 1));
                char * returnAck="ACK";
                send(new_socket, returnAck, strlen(returnAck), 0);
                char saveLogMessage[100];
                sprintf(saveLogMessage,("Time:%s\t%s login success\n"),asctime(timeinfo),nameArray);
                saveLog(saveLogMessage);
                break;
            }
        }
    }
    //end loop


    //broadcast ring;
    strcpy(sockets[findThisIndex(thisId)].name,nameArray);
    int k;
    for(k=0;k<=g_count;k++){
        if(strcmp(sockets[k].ring,nameArray)==0){
            char  message[200];
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
        revData[0]=0x00;
        int ret = recv(new_socket, revData, USER_SEND_MAX, 0);
        if(ret <= 0){
            printf("connect error...\n");
            closesocket(new_socket);
            WSACleanup();
            return -1;
        }
        revData[ret] = 0x00;
        int thisIndex=findThisIndex(thisId);
        if(strcmp(revData,"#Exit")==0){
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime (&rawtime);
            char saveLogMessage[100];
            sprintf(saveLogMessage,("Time:%s%s log out.\n"),asctime(timeinfo),sockets[thisIndex].name);
            saveLog(saveLogMessage);
            if(thisIndex==g_count){
                g_count--;
            }else{
                /*
                char changeInstruct[30];
                changeInstruct[0]=0x00;
                strcat(changeInstruct,change_index);
                strcat(changeInstruct,":");
                char iToChar[2];
                itoa(thisIndex,iToChar,10);
                strcat(changeInstruct,iToChar);
                for(int i=0;i<g_count;i++){
                   send(sockets[i].socket,changeInstruct,strlen(changeInstruct),0);
                }*/

                sockets[thisIndex]=sockets[g_count--];
            }
            printf("a socket is closed.\n");
            return 0;
        /*
        }else if(is_begin_with(revData,change_index)==1){
            if(thisIndex==g_count){
                //char tempIndex[10];
                strtok(revData,":");
                char *p=strtok(NULL,":");
                int newIndex=atoi((const char*)p);
                printf("%d index change to %d\n",thisIndex,newIndex);
                thisIndex=newIndex;
            }
            continue;*/
        }else if(strcmp(revData,"#ListU")==0){
            char userList[1000];
            getUserList(userList);
            send(new_socket, userList, strlen(userList), 0);
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
                send(new_socket, errorSend, strlen(errorSend), 0);
            }else{
                name[strlen(name)-1]=0x00;
                int j;
                printf(name);
                for(j=0;j<=g_count;j++){
                    if(strcmp(sockets[j].name,name)==0)break;
                }
                if(j>g_count){
                    char* errorSend="Sorry, I can't find name in user list.";
                    send(new_socket, errorSend, strlen(errorSend), 0);
                }else{
                    private_index=j;
                    char* message="Success be private.";
                    send(new_socket, message, strlen(message), 0);
                }
            }

            continue;
        }else if(strcmp(revData,"#Public")==0){
            private_index=-1;
            char* message="Success be public.";
            send(new_socket, message, strlen(message), 0);
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
                send(new_socket, errorSend, strlen(errorSend), 0);
            }else{
                name[strlen(name)-1]=0x00;
                int i;
                for(i=0;i<=g_count;i++){
                    if(strcmp(sockets[i].name,name)==0){
                        char* message="He\\She is online.";
                        send(new_socket, message, strlen(message), 0);
                    }
                }
                if(i<=g_count)continue;
                strcpy(sockets[thisIndex].ring,name);

                char* message="Success save ring.";
                send(new_socket, message, strlen(message), 0);
            }
            continue;
        }else if(is_begin_with(revData,"#TrfU")==1){
            printf("I got file.\n");
            char fileName[50];
            fileName[0]=0x00;
            char *tempName=NULL;
            strtok(revData," ");
            tempName=strtok(NULL," ");
            strcat(fileName,fileTitle);
            strcat(fileName,tempName);
            if(!access(fileName,0)){
                char * wrongMessage="Sorry, file exits.Please change your fileName.";
                send(new_socket, wrongMessage, strlen(wrongMessage), 0);
            }else{
                char sendAck[100];
                sendAck[0]=0x00;
                strcat(sendAck,"UPLOAD_ACK:");
                strcat(sendAck,tempName);
                send(new_socket, sendAck, strlen(sendAck), 0);
                printf("try to save file %s\n",tempName);
                FILE* fp;
                char data[FILE_DATA_MAX];
                data[0]=0x00;
                fp=fopen(fileName,"wb");
                if(fp==NULL){
                    printf("fail to save file named %s.\n",fileName);
                    continue;
                }
                printf("start to save file...\n");
                while(1){
                    memset(data,0,sizeof(data));
                    //char * temp[FILE_DATA_MAX];
                    int length=recv(new_socket,data,sizeof(data),0);
                    if(length==SOCKET_ERROR){
                        char * message="Fail to save file.It may be caused by incomplete file.";
                        printf("%s\n",message);
                        send(new_socket,message,strlen(message),0);
                        break;
                    }else if(strcmp(data,file_end_ack)==0){
                        char * message="Save file success.";
                        printf("%s\n",message);
                        send(new_socket,message,strlen(message),0);
                        break;
                    }else{
                        if(fwrite(data,1,length,fp)<length){
                            printf("Write Failed.\n");
                            break;
                        }
                    }
                }
                //printf("finish.\n");
                fclose(fp);
            }
            continue;
            // end #TrfU
        }else if(strcmp(revData,"#ListF")==0){
            char path[200];
            path[0]=0x00;
            char result[1024];
            result[0]=0x00;
            strcat(path,"file");
            viewFiles(path,result);
            //printf("files:%s\n",result);
            send(new_socket,result,strlen(result),0);
            continue;
        }else if(is_begin_with(revData,"#TrfD")==1){

            char  fileName[50];
            fileName[0]=0x00;
            char * p=NULL;
            //fileName[0]=0x00;
            //char *tempName;
            strcat(fileName,"file/");
            strtok(revData," ");
            p=strtok(NULL," ");
            strcat(fileName,p);
            //printf("%s\n",fileName);
            FILE *fp;
            char data[FILE_DATA_MAX];
            data[0]=0x00;

            fp=fopen(fileName,"rb");
            if(fp==NULL){
                char* message="file dose not exist.";
                send(new_socket,message,strlen(message),0);
                printf("%s\n",message);
                continue;
            }
           // char * message="FILE_SEND";
            send(new_socket,FILE_SEND,strlen(FILE_SEND),0);
            Sleep(500);
            send(new_socket,p,strlen(p),0);

            while(1){
                memset((void *)data,0,sizeof(data));
                //printf("sss\n");
                int length=fread(data,1,sizeof(data),fp);
                if(length==0){
                    Sleep(500);
                    send(new_socket,file_end_ack,strlen(file_end_ack),0);
                    printf("File send Success\n");
                    break;
                }
                //printf("%s\n",data);
                int ret=send(new_socket,data,length,0);
                //putchar('.');
                if(ret==SOCKET_ERROR){
                    char * message="Failed to send file.It may be caused by incomplete file.";
                    printf("%s\n",message);
                    send(new_socket,message,strlen(message),0);
                    break;
                }
            }
            fclose(fp);
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
    //printf("\nRC open Database = %d\n",rc);
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
    //printf("\nRC sqltite_exec = %d\n\n",rc);
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
void viewFiles(char * path,char * result){
    struct _finddata_t files;
    //char cFileAddr[300];
    long File_Handle;
    strcat(path,"/*.*");
    //printf("%s\n",path);
    //int i=0;
    File_Handle = _findfirst(path,&files);
    if(File_Handle==-1)
    {
        strcat(result,"Not found.");
    }else{
        do{
            if(files.name[0]!='.' && files.attrib!=_A_SUBDIR){
                strcat(result,files.name);
                strcat(result,"\n");
                //printf("find a file:%s\n",files.name);
            }
        }while( _findnext(File_Handle,&files)==0);
    }
}
int findThisIndex(unsigned address){
    for(int i=0;i<=g_count;i++){
        if(address==sockets[i].address){
            return i;
        }
    }
    return -1;
}
void saveLog(char * message){
    FILE* fp;

    fp=fopen("log.txt","a+");
    if(fp==NULL){
        printf("fail to save log\n");
        return;
    }
    fputs(message, fp);
    fclose(fp);
}
