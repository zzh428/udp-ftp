#include "inc.h"
#include "const.h"

int handleArgs(int * port, char * path, int argc, char **argv)
{
	switch(argc)
	{
		case 1:
			return 1;
			break;
		case 3:
			if(!strcmp(argv[1],"-port"))
			{
				int temp = atoi(argv[2]);
				if (temp >0 && temp < 65536)
				{
					*port = temp;
					return 1;
				}
				else
				{
					printf("Invalid Port Number %s\n",argv[2]);
					return 0;
				}
			}
			if(!strcmp(argv[1],"-root"))
			{
				strcpy(path,argv[2]);
				return 1;
			}
			printf("Invalid Arguments\n");
			return 0;
			break;
		case 5:
			if(!strcmp(argv[1],"-port") && !strcmp(argv[3],"-root"))
			{
				int temp = atoi(argv[2]);
				if (temp > 0 && temp < 65536)
				{
					*port = temp;
					strcpy(path,argv[4]);
					return 1;
				}
				else
				{
					printf("Invalid Port Number %s\n",argv[2]);
					return 0;
				}
			}
			if(!strcmp(argv[3],"-port") && !strcmp(argv[1],"-root"))
			{
				int temp = atoi(argv[4]);
				if (temp > 0 && temp < 65536)
				{
					*port = temp;
					strcpy(path,argv[2]);
					return 1;
				}
				else
				{
					printf("Invalid Port Number %s\n",argv[4]);
					return 0;
				}
			}
			printf("Invalid Arguments\n");
			return 0;
			break;
		default:
			printf("Invalid Number of Arguments\n");
			return 0;
			break;
	}
	return 0;
}

int sendStringtoClient(int connfd, char * sentence)
{
	int len = strlen(sentence);
	int p = 0;
	while (p < len) {
		int n = write(connfd, sentence + p, len - p);
		if (n < 0) {
			printf("Error write(): %s(%d)\n", strerror(errno), errno);
			close(connfd);
			return 0;
		} 
		else if(n == 0)
		{
			break;
		}
		else {
			p += n;
		}			
	}
	return 1;
}

int receiveStringfromClient(int connfd, char * sentence)
{
	int p = 0;
	while (1) {
		int n = read(connfd, sentence + p, 8191 - p);
		if (n < 0) {
			printf("Error read(): %s(%d)\n", strerror(errno), errno);
			close(connfd);
			return 0;
		} else if (n == 0) {
			break;
		} else {
			p += n;
			if (sentence[p - 1] == '\n') {
				break;
			}
		}
	}
	//socket接收到的字符串并不会添加'\0'
	sentence[p - 2] = '\0';
	return 1;
}

int parseCmd(char * str, char * cmd, char * args)
{
	int i = 0;
	for(; i < strlen(str); i++)
	{
		if(str[i] == ' ')
			break;
	}
	strncpy(cmd,str,i);
	str += i;
	while (*str == ' ')
		str++;
	strcpy(args, str);
	return 1;
}

int initSocket(int port)
{
	int listenfd = 0;
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	
	//设置本机的ip和port
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听"0.0.0.0"
			
	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}

	//开始监听socket
	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return listenfd;
}

int connectUser(char * ip, int port)
{
	int connfd = 0;
	if ((connfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &addr.sin_addr);
	if(connect(connfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	return connfd;
}

int sendFile(int fd, char * args, int rest)
{
	FILE* f = fopen(args, "rb");
	if (f == NULL)
		return 0;
	if(rest)
	{
		if(!fseek(f,(long)rest,SEEK_SET))
			return 0;
	}
	char buf[8192];
	int len = 0;
	while (!feof(f))
	{
		len = fread(buf, sizeof(char), sizeof(buf), f);
		write(fd, buf, len);
	}
	fclose(f);
	close(fd);
	return 1;
}

int recvFile(int fd, char * args, char * cmd)
{
	if(!strcmp(cmd, "STOR"))
	{
		FILE* f = fopen(args, "wb");
	}
	else if(!strcmp(cmd, "APPE"))
	{
		FILE* f = fopen(args, "ab");
	}
	else
		return 0;
	if (f == NULL)
		return 0;
	char buf[8192];
	int len = 0;
	while(1)
	{
		len = read(fd, buf, 8192);
		if (len == 0)
			break;
		else if (len < 0)
		{
			fclose(f);
			return 0;
		}
		fwrite(buf, sizeof(char), len, f);
	}
	fclose(f);
	close(fd);
	return 1;
}

int parseIP(char * args, char * IP, int * port)
{
	char * split = strtok(args, ",");
	int count = 1;
	while (split != NULL)
	{
		if (count <= 4)
		{
			strcat(IP, split);
			if (count <= 3)
				strcat(IP, ".");
		}
		else if(count == 5)
		{
			*port = atoi(split) * 256;
		}
		else if (count == 6)
		{
			*port += atoi(split);
		}
		count++;
		split = strtok(NULL, ",");
	}
	if (count != 6)
		return 0;
	else
		return 1;
}

char * getLocalIP()
{
	//reference: https://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine
	struct ifaddrs * ifAddrStruct = NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr = NULL;
	char * addressBuffer = (char *)malloc(INET_ADDRSTRLEN);
    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
		if (ifa->ifa_addr->sa_family==AF_INET)
        {
			tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
        	inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			if(strcmp(addressBuffer,"127.0.0.1") != 0)
				break;
		} 
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
	return addressBuffer;
}

void generatePASVMessage(char * message, int port, char * localIP)
{
	strcpy(message, str_pasv);
	char tempLocalIP[20];
	strcpy(tempLocalIP, localIP);
	for (int i = 0; i < strlen(tempLocalIP); i++)
	{
		if (tempLocalIP[i] == '.')
			tempLocalIP[i] = ',';
	}
	strcat(message, tempLocalIP);
	strcat(message, ",");
	char temp[10];
	sprintf(temp,"%d",port/256);
	//itoa(port / 256, temp, 10);
	strcat(message, temp);
	strcat(message, ",");
	memset(temp, 0, 10);
	sprintf(temp,"%d",port%256);
	//itoa(port % 256, temp, 10);
	strcat(message, temp);
	strcat(message, ")\r\n");
}

int getFullPath(char * root, char * cwd, char * args, char * result)
{
	if (args[0] == '/')
	{
		strcpy(result, root);
		strcat(result, args);
	}
	else
	{
		strcpy(result, root);
		strcat(result, cwd);
		if(cwd[strlen(cwd)-1] != '/')
			strcat(result, "/");
		strcat(result, args);
	}
	char checktemp[1000];
	if(realpath(result,checktemp) != NULL)
	{
		if(!strncmp(root,checktemp,strlen(root)))
		{
			memset(result,0,1000);
			strcpy(result,checktemp);
			return 1;
		}
		else
			return 0;
	}
	else
		return -1;
}

int sendDirList(int fd, char * args)
{
	FILE * f = NULL;
	char shellcmd[200] = "ls -l ";
	if( *args == 0)
	{
		f = popen(shellcmd, "r");
	}
	else
	{
		strcat(shellcmd,args);
		f = popen(shellcmd, "r");
	}
	if (f == NULL)
		return 0;
	char buf[8192];
	int len = 0;
	while (fgets(buf, sizeof(buf), f) != NULL)
	{
		len = strlen(buf);
		write(fd, buf, len);
	}
	pclose(f);
	close(fd);
	return 1;
}

int checkPath(char * path)
{
    char *pattern = "\\.\\.";
    regex_t reg;
    regmatch_t pmatch[1];
	regcomp(&reg,pattern,REG_EXTENDED);
    int t = regexec(&reg,path,1,pmatch,0);
    if(t == REG_NOMATCH){
		return 1;
	}
	return 0;
}

int startClientSession(char *localIP, int connfd, char * rootPath)
{
	char curPath[1000] = "/";
	sendStringtoClient(connfd, str_welcome);
	int status = STATUS_NOUSERNAME;
	int mode = 0; //0=PORT, 1=PASV
	char client_IP[20];
	int dataport = 0;
	int pasv_listenfd = 0;
	int rnfr_status = 0;
	char rnfr_path[1000];
	int rest = 0;
	while(1)
	{
		char str[8192];
		memset(str,0,8192);
		if(receiveStringfromClient(connfd, str) == 0)
			return 0;
		char cmd[10],args[8192];
		memset(cmd,0,10);
		memset(args,0,8192);
		parseCmd(str,cmd,args);
		if(!strcmp(cmd,"USER"))
		{
			if(status == STATUS_NOUSERNAME || status == STATUS_NOPASSWORD)
			{
				if(!strcmp(args,"anonymous"))
				{
					sendStringtoClient(connfd,str_password);
					status = STATUS_NOPASSWORD;
				}
				else
				{
					sendStringtoClient(connfd, str_wronguser);
				}
			}
			else
			{
				sendStringtoClient(connfd, str_login);
			}	
		}
		else if(!strcmp(cmd,"PASS"))
		{
			if(status == STATUS_NOPASSWORD)
			{
				if (args[0] != 0)
				{
					sendStringtoClient(connfd, str_login);
					status = STATUS_NOPORT;
				}
				else
					sendStringtoClient(connfd, str_nopassword);
			}
			else if(status == STATUS_NOUSERNAME)
			{
				sendStringtoClient(connfd, str_nouser);
			}
			else if(status > STATUS_NOPASSWORD)
			{
				sendStringtoClient(connfd, str_login);
			}	
		}
		else if(!strcmp(cmd,"REST"))
		{
			rest = atoi(args);
			sendStringtoClient(connfd, str_rest);
		}
		else if(!strcmp(cmd,"RETR"))
		{
			int file_fd = 0;
			char path[1000];
			memset(path, 0, 1000);
			if (status == STATUS_READY)
			{
				int p = getFullPath(rootPath, curPath, args, path);
				if(p == 1)
				{
					char message[1000];
					memset(message, 0, 1000);
					strcpy(message, str_open);
					strcat(message, args);
					strcat(message, "\r\n");
					sendStringtoClient(connfd, message);
					if (mode == MODE_PORT)
					{
						file_fd = connectUser(client_IP, dataport);
						if(sendFile(file_fd, path, rest))
							sendStringtoClient(connfd, str_finish);
						else
							sendStringtoClient(connfd, str_fileFail);

					}
					else
					{
						file_fd = accept(pasv_listenfd, NULL, NULL);
						if (sendFile(file_fd, path, rest))
							sendStringtoClient(connfd, str_finish);
						else
							sendStringtoClient(connfd, str_fileFail);
					}
					
				}
				else if(p == 0)
				{
					sendStringtoClient(connfd,str_permission);
				}
				else
				{
					sendStringtoClient(connfd,str_noFile);
				}
			}
			else if (status == STATUS_NOPORT)
			{
				sendStringtoClient(connfd, str_noconnection);
			}
			else
			{
				sendStringtoClient(connfd,str_permission);
			}
			rest = 0;	
		}
		else if(!strcmp(cmd,"STOR") || !strcmp(cmd,"APPE"))
		{
			int file_fd = 0;
			char path[1000];
			memset(path, 0, 1000);
			if (status == STATUS_READY)
			{
				if(getFullPath(rootPath, curPath, args, path) != 0)
				{
					char message[1000];
					memset(message, 0, 1000);
					strcpy(message, str_open);
					strcat(message, args);
					strcat(message, "\r\n");
					sendStringtoClient(connfd, message);
					if (mode == MODE_PORT)
					{
						file_fd = connectUser(client_IP, dataport);
						if (recvFile(file_fd, path, cmd))
							sendStringtoClient(connfd, str_finish);
						else
							sendStringtoClient(connfd, str_fileFail);
					}
					else
					{
						file_fd = accept(pasv_listenfd, NULL, NULL);
						if (recvFile(file_fd, path, cmd))
							sendStringtoClient(connfd, str_finish);
						else
							sendStringtoClient(connfd, str_fileFail);
					}
				}
				else
				{
					sendStringtoClient(connfd,str_permission);
				}
			}
			else if (status == STATUS_NOPORT)
			{
				sendStringtoClient(connfd, str_noconnection);
			}
			else
			{
				sendStringtoClient(connfd, str_permission);
			}
		}
		else if(!strcmp(cmd,"QUIT") || !strcmp(cmd,"ABOR"))
		{
			sendStringtoClient(connfd, str_goodbye);
			return 0;
		}
		else if(!strcmp(cmd,"SYST"))
		{
			if(status >= STATUS_NOPORT)
				sendStringtoClient(connfd, str_syst);
			else
				sendStringtoClient(connfd, str_permission);
		}
		else if(!strcmp(cmd,"TYPE"))
		{
			if (status >= STATUS_NOPORT)
			{
				if (!strcmp(args, "I"))
					sendStringtoClient(connfd, str_type);
				else
					sendStringtoClient(connfd, str_typeerror);
			}
			else
			{
				sendStringtoClient(connfd, str_permission);
			}
		}
		else if(!strcmp(cmd,"PORT"))
		{
			if (status >= STATUS_NOPORT)
			{
				if (mode == MODE_PASV)
					close(pasv_listenfd);
				mode = MODE_PORT;
				memset(client_IP, 0, 20);
				parseIP(args, client_IP, &dataport);
				status = STATUS_READY;
				sendStringtoClient(connfd, str_port);
			}
			else
			{
				sendStringtoClient(connfd, str_permission);
			}
		}
		else if(!strcmp(cmd,"PASV"))
		{
			if (status >= STATUS_NOPORT)
			{
				if(mode == MODE_PASV)
					close(pasv_listenfd);
				mode = MODE_PASV;
				while (1)
				{
					dataport = rand() % 45536 + 20000;
					if ((pasv_listenfd = initSocket(dataport)) == -1)
						continue;
					break;
				}
				char message[200];
				memset(message, 0, 200);
				generatePASVMessage(message, dataport, localIP);
				sendStringtoClient(connfd, message);
				status = STATUS_READY;
			}
			else
			{
				sendStringtoClient(connfd, str_permission);
			}
		}
		else if(!strcmp(cmd,"MKD"))
		{
			if (status >= STATUS_NOPORT)
			{
				char path[1000];
				memset(path, 0, 1000);
				if(getFullPath(rootPath, curPath, args, path) != 0)
				{
					
					if(mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
					{
						sendStringtoClient(connfd, str_dirok);
					}
					else
					{
						sendStringtoClient(connfd, str_mkdirFail);
					}
				}
				else
				{
					sendStringtoClient(connfd,str_permission);
				}
			}
			else
			{
				sendStringtoClient(connfd, str_permission);
			}

		}
		else if(!strcmp(cmd,"CWD"))
		{
			if (status >= STATUS_NOPORT)
			{
				char path[1000];
				char * pp = path;
				memset(path, 0, 1000);
				int p = getFullPath(rootPath, curPath, args, path);
				if(p == 1)
				{
					if (chdir(path) == 0)
					{
						getcwd(path, 255);
						if (strncmp(path, rootPath, strlen(rootPath)) == 0)
						{
							pp += strlen(rootPath);
							if (*pp == 0)
								*pp = '/';
							memset(curPath, 0, 1000);
							strcpy(curPath, pp);
							sendStringtoClient(connfd, str_dirok);
						}
						else
						{
							memset(path, 0, 1000);
							strcpy(path, rootPath);
							strcat(path, curPath);
							chdir(path);
							sendStringtoClient(connfd, str_chdirFail);
						}
					}
					else
					{
						sendStringtoClient(connfd, str_chdirFail);
					}
				}
				else if(p == 0)
				{
					sendStringtoClient(connfd, str_permission);
				}
				else
				{
					sendStringtoClient(connfd, str_noFile);
				}
			}
			else
			{
				sendStringtoClient(connfd, str_permission);
			}
			
		}
		else if(!strcmp(cmd,"PWD"))
		{
			if (status >= STATUS_NOPORT)
			{
				char message[1000];
				memset(message, 0, 1000);
				strcpy(message, "257 ");
				strcat(message, curPath);
				strcat(message, "\r\n");
				sendStringtoClient(connfd, message);
			}
			else
			{
				sendStringtoClient(connfd, str_permission);
			}
		}
		else if(!strcmp(cmd,"LIST"))
		{
			if (status == STATUS_READY)
			{
				char path[1000];
				memset(path, 0, 1000);
				int p = getFullPath(rootPath, curPath, args, path);
				if(p == 1)
				{
					int file_fd = 0;
					char message[1000];
					memset(message, 0, 1000);
					strcpy(message, str_open);
					strcat(message, "list of current directory\r\n");
					sendStringtoClient(connfd, message);
					if (mode == MODE_PORT)
					{
						file_fd = connectUser(client_IP, dataport);
						if(sendDirList(file_fd, path))
							sendStringtoClient(connfd, str_finish);
						else
							sendStringtoClient(connfd, str_fileFail);
					}
					else
					{
						file_fd = accept(pasv_listenfd, NULL, NULL);
						if (sendDirList(file_fd, path))
							sendStringtoClient(connfd, str_finish);
						else
							sendStringtoClient(connfd, str_fileFail);
					}
				}
				else if(p == 0)
				{
					sendStringtoClient(connfd, str_permission);
				}
				else
				{
					sendStringtoClient(connfd, str_noFile);
				}
			}
			else if (status == STATUS_NOPORT)
			{
				sendStringtoClient(connfd, str_noconnection);
			}
			else
			{
				sendStringtoClient(connfd, str_permission);
			}
		}
		else if(!strcmp(cmd,"RMD"))
		{
			if (status >= STATUS_NOPORT)
			{
				char path[1000];
				memset(path, 0, 1000);
				int p = getFullPath(rootPath, curPath, args, path);
				if(p == 1)
				{
					
					if (rmdir(path) == 0)
					{
						sendStringtoClient(connfd, str_dirok);
					}
					else
					{
						sendStringtoClient(connfd, str_permission);
					}
				}
				else if(p == 0)
				{
					sendStringtoClient(connfd, str_permission);
				}
				else
				{
					sendStringtoClient(connfd, str_noFile);
				}
			}
			else
			{
				sendStringtoClient(connfd, str_permission);
			}
		}
		else if(!strcmp(cmd,"RNFR"))
		{
			if (status >= STATUS_NOPORT)
			{
				char path[1000];
				memset(path, 0, 1000);
				int p = getFullPath(rootPath, curPath, args, path);
				if(p == 1)
				{
					if (access(path, F_OK) == 0)
					{
						strcpy(rnfr_path, path);
						sendStringtoClient(connfd, str_dirok);
						rnfr_status = 1;
					}
					else
					{
						sendStringtoClient(connfd, str_noFile);
					}
				}
				else if(p == 0)
				{
					sendStringtoClient(connfd, str_permission);
				}
				else
				{
					sendStringtoClient(connfd, str_noFile);
				}
			}
			else
			{
				sendStringtoClient(connfd, str_permission);
			}
		}
		else if(!strcmp(cmd,"RNTO"))
		{
			if (status >= STATUS_NOPORT)
			{
				if(rnfr_status)
				{
					char path[1000];
					memset(path, 0, 1000);
					if(getFullPath(rootPath, curPath, args, path) != 0)
					{
						if (rename(rnfr_path, path) == 0)
						{
							rnfr_status = 0;
							sendStringtoClient(connfd, str_dirok);
						}
						else
						{
							sendStringtoClient(connfd, str_fileFail);
						}
					}
					else
					{
						sendStringtoClient(connfd, str_permission);
					}
				}
				else
				{
					sendStringtoClient(connfd, str_nornfr);
				}
			}
			else
			{
				sendStringtoClient(connfd, str_permission);
			}
		}
		else
		{
			sendStringtoClient(connfd, str_nocmd);
		}
	}
}

int main(int argc, char **argv) {
	char * localIP; //for PASV mode
	int listenfd, connfd;		//监听socket和连接socket不一样，后者用于数据传输
	int port = 21;
	char rootPath[500] = "/tmp";
	localIP = getLocalIP();

	if(!handleArgs(&port, rootPath, argc, argv))
		return 1;
	if(chdir(rootPath) != 0)
		return 1;
	getcwd(rootPath, 255);
	
	if ((listenfd = initSocket(port)) == -1)
	{
		return 1;
	}

	//持续监听连接请求
	while (1) {
		//等待client的连接 -- 阻塞函数
		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
		}
		else
		{
			int pid = fork();
			if(pid == 0)
			{
				close(listenfd);
				startClientSession(localIP,connfd,rootPath);
				close(connfd);
				exit(0);
			}
			close(connfd);
		}
		
	}
	close(listenfd);
	free(localIP);
	return 0;
}

