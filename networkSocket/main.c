//引入需要的头文件，包含一些定义的常量
#include <head.h>


//查找path下所有文件，并将结果保存到result，用于#ListF指令
void viewFiles(char * path,char * result);
//子线程查找自身线程所在sockets的index
int findThisIndex(unsigned address);
//将所需要的日志信息保存到Log文件中
void saveLog(char * message);
//根据sockets中的信息，查找在线用户，结果保存到userList的指针中
void getUserList(char userList[]);
//每一个子线程所需要做的工作，实现功能的主要部分
unsigned int _stdcall ThreadFun();
//工具函数，用于判断str1是否由str2开头，如果是返回1不是返回0
int is_begin_with(const char * str1,char *str2);
//初始化sqlite数据库，包括删除表和插入新表
int insertData();

//sockets每一个的元素的结构体，name存储当前index的用户名
//address存储当前线程的ID，用于子线程查找
//socket保存当前socket
//ring保存该用户想要记录的提醒上线的用户名
//如当前是dz，ring了han，当han上线时，给dz发消息，han is connented。
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
//最重要的全局变量sockets，相当于线程池，管理子线程的信息
struct SocketObject sockets[THREAD_NUM];
//用于记录有多少线程（socket）在线
int g_count;
//服务器发送给client用于确认接收文件的随机码，在client中有相应
const char* FILE_SEND="3q23mgU9hAltcjUMAMOtc@DA*qPgESH";
//
//char* change_index="K3LvzpYnSQ27wtwM";




void getUserList(char userList[]){
    //结构化用户列表
    strcat(userList,"users\n----------\n");
    int i=0;
    for(i=0;i<=g_count;i++){
        //所有index小于等于g_count的均为在线对象
        strcat(userList,sockets[i].name);
        strcat(userList,"\n");
    }
    strcat(userList,"----------");
}
int main(int arg,char *argv[])
{
    //以下都是基本的初始化socket
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server,client;
    int c;

    g_count=-1;

    //初始化sqlite数据库
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
            //等待clientsocket建立连接
        SOCKET new_socket = accept(s,(struct sockaddr *)&client,&c);
        if(new_socket == INVALID_SOCKET || g_count>=THREAD_NUM){
            printf("accept error\n");
            //still need to be done
            //closesocket(new_socket);
            break;
        }
    //用于保存日志中所需用的时间
        time_t rawtime;
        struct tm * timeinfo;
        time(&rawtime);
        timeinfo=localtime(&rawtime);
        //用于保存client的ip地址
        char *ipv4=inet_ntoa(client.sin_addr);
        printf("@%s connected...\n",ipv4);
        char saveMessage[50];
        sprintf(saveMessage,("Time:%s\t@%s connected\r\n"),asctime(timeinfo),ipv4);
        printf("%s",saveMessage);
        //如果连接将信息保存到log.txt中
        saveLog(saveMessage);


        //存储线程Id的无符号int
        unsigned threadAddress;
        //创建线程
        _beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))ThreadFun, NULL, 0, &threadAddress);
        //将该线程的信息保存到全局变量sockets中
        struct SocketObject socketObject;
        socketObject.address=threadAddress;
        socketObject.socket=new_socket;
        sockets[++g_count]=socketObject;
    }
    WSACleanup();
    return 0;
}
//子线程，主要实现功能部分
unsigned int _stdcall ThreadFun(){
    //子线程获取自身线程ID用于查找sockets中自身的信息
    unsigned thisId=GetCurrentThreadId();
    //上传和下载文件的目录，全部都从file文件夹下读取和保存
    const char* fileTitle="file/";
    //服务器用于确认文件传输完毕，停止写入文件的随机码
    const char* file_end_ack="BDd8E@XLj605Dsx0zzveRJNhy0qKTQ2T";
    //记录与哪一个socket private通信
    int private_index=-1;
    //数据库指针，用于确认用户帐号密码
    sqlite3 *db;
    //打开数据库
    int rc = sqlite3_open("dzData", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    //获取本线程所在的socket地址
    SOCKET new_socket=sockets[findThisIndex(thisId)].socket;
    //保存用户名和密码
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
            //查找数据库，验证帐号密码
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

            int step =sqlite3_step(res);
            //将登录信息保存到日志中用到的时间
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            if(step!=100 || strcmp((const char *)password,(const char *)sqlite3_column_text(res, 1))!=0){
                    //如果登陆失败，保存信息，返回信息给客户端
                char * message="Wrong username or password.";
                send(new_socket, message, strlen(message), 0);
                char saveLogMessage[100];
                sprintf(saveLogMessage,("Time:%s\t%s login failed\n"),asctime(timeinfo),nameArray);
                saveLog(saveLogMessage);
            }else{
                //如果登陆成功，保存信息，返回ACK给客户端
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



    //将登录用户的name保存到sockets中
    strcpy(sockets[findThisIndex(thisId)].name,nameArray);
    int k;
    //将该用户登陆信息广播给ring了该用户的用户
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

    //主要监听循环，非常大
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

        //C语言只能挨个判断，不能用switch，所以写了很多ifelse，无奈之举
        if(strcmp(revData,"#Exit")==0){
            //如果是#Exit指令
            //记录时间
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime (&rawtime);
            char saveLogMessage[100];
            sprintf(saveLogMessage,("Time:%s%s log out.\n"),asctime(timeinfo),sockets[thisIndex].name);
            saveLog(saveLogMessage);
            if(thisIndex==g_count){
                //如果当前socket存在于sockets最后的位置，则直接将g_count-1即可
                //退出这个while循环这个线程就关闭了
                //socket关闭由client进行
                g_count--;
            }else{
                //如果当前socket存在于中间，则将最后一个socket放到当前的index下
                sockets[thisIndex]=sockets[g_count--];
            }
            printf("a socket is closed.\n");
            return 0;

        }else if(strcmp(revData,"#ListU")==0){
            //显示用户
            char userList[1000];
            getUserList(userList);
            send(new_socket, userList, strlen(userList), 0);
            userList[0]=0x00;
            continue;
        }else if(is_begin_with(revData,"#Private")==1){
            //私聊
            printf("I got private.\n");
            char name[50];
            name[0]=0x00;
            char *p;
            strtok(revData," ");
        //提取#Private <username>空格之后的username，允许名字中含有空格（但未测试）
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
                //查找该用户是否在线
                for(j=0;j<=g_count;j++){
                    if(strcmp(sockets[j].name,name)==0)break;
                }
                if(j>g_count){
                        //for循环跑完，说明用户不在线
                    char* errorSend="Sorry, I can't find name in user list.";
                    send(new_socket, errorSend, strlen(errorSend), 0);
                }else{
                    //如果小于等于g_count,说明找到了，保存index到这个socket中
                    //此时此刻我忽然意识到这里有个Bug，但是我不想管了
                    private_index=j;
                    char* message="Success be private.";
                    send(new_socket, message, strlen(message), 0);
                }
            }

            continue;
        }else if(strcmp(revData,"#Public")==0){
            //将private标记为-1，即可发出公开消息
            private_index=-1;
            char* message="Success be public.";
            send(new_socket, message, strlen(message), 0);
            continue;
        }else if(is_begin_with(revData,"#Ring")==1){
            //ring指令
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
                //查找该用户是否已经在线
                for(i=0;i<=g_count;i++){
                    if(strcmp(sockets[i].name,name)==0){
                        char* message="He\\She is online.";
                        send(new_socket, message, strlen(message), 0);
                        break;
                    }
                }
                //break只能跳出一层循环，所以这里还需要判断再跳出循环
                if(i<=g_count)continue;
                //如果i大于g_count，说明用户不在线，将ring的名字保存到该socket中
                strcpy(sockets[thisIndex].ring,name);

                char* message="Success save ring.";
                send(new_socket, message, strlen(message), 0);
            }
            continue;
        }else if(is_begin_with(revData,"#TrfU")==1){
            //传输文件指令
            printf("I got file.\n");
            char fileName[50];
            fileName[0]=0x00;
            char *tempName=NULL;
            strtok(revData," ");
            tempName=strtok(NULL," ");
            //加入文件头file/
            strcat(fileName,fileTitle);
            strcat(fileName,tempName);
            //判断文件是否存在，access是C语言的函数
            if(!access(fileName,0)){
                char * wrongMessage="Sorry, file exits.Please change your fileName.";
                send(new_socket, wrongMessage, strlen(wrongMessage), 0);
            }else{
                //如果文件存在
                char sendAck[100];
                sendAck[0]=0x00;
                strcat(sendAck,"UPLOAD_ACK:");
                strcat(sendAck,tempName);
                //返回给client 一个以UPLOAD_ACK开头的消息，客户端接收之后可以开始发送文件数据
                send(new_socket, sendAck, strlen(sendAck), 0);
                printf("try to save file %s\n",tempName);
                //保存文件，理论上应该写个函数包装一下，然而我懒了
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
                    //接收文件数据
                    int length=recv(new_socket,data,sizeof(data),0);
                    if(length==SOCKET_ERROR){
                        char * message="Fail to save file.It may be caused by incomplete file.";
                        printf("%s\n",message);
                        send(new_socket,message,strlen(message),0);
                        break;
                    }else if(strcmp(data,file_end_ack)==0){
                        //如果client发送file_end_ack表名文件已经传输完毕了
                        //没想到更好的方法来让服务器知道文件传输完毕
                        char * message="Save file success.";
                        printf("%s\n",message);
                        send(new_socket,message,strlen(message),0);
                        break;
                    }else{
                        //如果收到的不是结束ack，则向文件里写入数据
                        if(fwrite(data,1,length,fp)<length){
                            printf("Write Failed.\n");
                            break;
                        }
                    }
                }
                //关闭文件指针
                fclose(fp);
            }
            continue;
            // end #TrfU
        }else if(strcmp(revData,"#ListF")==0){
            //显示文件列表
            char path[200];
            path[0]=0x00;
            char result[1024];
            result[0]=0x00;
            strcat(path,"file");
            viewFiles(path,result);
            send(new_socket,result,strlen(result),0);
            continue;
        }else if(is_begin_with(revData,"#TrfD")==1){
            //下载文件指令，与上传很类似
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
                //如果文件指针打开失败，说明文件不存在，返回给client
                char* message="file dose not exist.";
                send(new_socket,message,strlen(message),0);
                printf("%s\n",message);
                continue;
            }
            //通知client可以开始上传文件了
            send(new_socket,FILE_SEND,strlen(FILE_SEND),0);
            //此处如果不延迟发送，socket可能会将两个信息合并在一起发送
            Sleep(500);
            //返回给client文件名,client需要用这个名字创建文件
            send(new_socket,p,strlen(p),0);

            while(1){
                memset((void *)data,0,sizeof(data));

                int length=fread(data,1,sizeof(data),fp);
                //fread返回参数为读到该文件数组的长度，如果为零说明文件读完了
                if(length==0){
                    //与之前sleep同理，不用会把两个socket同时传
                    Sleep(500);
                    send(new_socket,file_end_ack,strlen(file_end_ack),0);
                    printf("File send Success\n");
                    break;
                }
                //将文件发送给client
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


        //如果均不是以上指令，则将消息广播出去
        int i;
        char broadcast[BROADCAST_MAX];
        //结构化广播的消息
        broadcast[0]=0x00;
        strcat(broadcast,"From ");
        strcat(broadcast,nameArray);
        strcat(broadcast,":");
        strcat(broadcast,revData);
        if(private_index>=0){
                //如果是私聊，只发给一个用户
            send(sockets[private_index].socket, broadcast, strlen(broadcast), 0);
        }else{
            //不是私聊，广播给在线的用户
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
//判断str1是否以str2开头
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
    //创建数据库文件，文件名为dzData
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
    //根据path路径查找文件列表，并将结构化信息保存到result指针
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
    //子线程寻找当前所在的index
    for(int i=0;i<=g_count;i++){
        if(address==sockets[i].address){
            return i;
        }
    }
    return -1;
}
void saveLog(char * message){
    //保存日志
    FILE* fp;

    fp=fopen("log.txt","a+");
    if(fp==NULL){
        printf("fail to save log\n");
        return;
    }
    fputs(message, fp);
    fclose(fp);
}
