#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <fcntl.h>
#include <net/if.h> 
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#define MODE_PORT 0
#define MODE_PASV 1
#define STATUS_NOUSERNAME 0
#define STATUS_NOPASSWORD 1
#define STATUS_NOPORT 2
#define STATUS_READY 3

char * str_welcome = "220 FTP server ready\r\n";
char * str_wronguser = "530 Invalid username\r\n";
char * str_nopassword = "530 No password\r\n";
char * str_nouser = "503 Need username before password\r\n";
char * str_password = "331 Guest login ok, send your complete e-mail address as password\r\n";
char * str_login = "230 Guest login ok\r\n";
char * str_syst = "215 UNIX Type: L8\r\n";
char * str_type = "200 Type set to I.\r\n";
char * str_typeerror = "501 Type error\r\n";
char * str_port = "200 PORT command successful\r\n";
char * str_open = "150 Opening BINARY mode data connection for ";
char * str_fileFail = "451 File operation failed\r\n";
char * str_noFile = "451 No such file or directory\r\n";
char * str_finish = "226 Transfer complete\r\n";
char * str_pasv = "227 Entering Passive Mode (";
char * str_goodbye = "221 Goodbye\r\n";
char * str_permission = "550 Permission denied\r\n";
char * str_dirok = "250 Okey\r\n";
char * str_mkdirFail = "550 Mkdir failed\r\n";
char * str_rmdirFail = "550 Rmdir failed\r\n";
char * str_chdirFail = "550 Chdir failed\r\n";
char * str_renameFail = "550 Rename failed\r\n";
char * str_noconnection = "425 No connection\r\n";
char * str_nocmd = "502 No such command\r\n";

char * localIP; //for PASV mode



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
			if(!strcmp(argv[1],"-path"))
			{
				strcpy(path,argv[2]);
				return 1;
			}
			printf("Invalid Arguments\n");
			return 0;
			break;
		case 5:
			if(!strcmp(argv[1],"-port") && !strcmp(argv[3],"-path"))
			{
				int temp = atoi(argv[2]);
				if (temp >0 && temp < 65536)
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
			if(!strcmp(argv[3],"-port") && !strcmp(argv[1],"-path"))
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
	p = 0;
	while (p < len) {
		int n = write(connfd, sentence + p, len + 1 - p);
		if (n < 0) {
			printf("Error write(): %s(%d)\n", strerror(errno), errno);
			close(connfd);
			return 0;
		} else {
			p += n;
		}			
	}
	return 1;
}

int receiveStringfromClient(int connfd, char * sentence)
{
	p = 0;
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
	sentence[p - 1] = '\0';
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
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);	//监听"0.0.0.0"

												//将本机的ip和port与socket绑定
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

int sendFile(int fd, char * args)
{
	FILE* f = fopen(args, "rb");
	if (f == NULL)
		return 0;
	char buf[8192];
	int len = 0;
	while (!feof(f))
	{
		len = fread(buf, sizeof(char), sizeof(buf), f);
		write(fd, buf, len);
	}
	fclose(f);
	return 1;
}

int recvFile(int fd, char * args)
{
	FILE* f = fopen(args, "wb");
	if (f == NULL)
		return 0;
	char buf[8192];
	int len = 0;
	while(1)
	{
		if ((len = read(fd, buf, 8192)) <= 0)
			break;
		fwrite(buf, sizeof(char), len, f);
	}
	fclose(f);
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
	int fd;
	struct sockaddr_in addr;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFADDR, &ifr) == 0) {
		memcpy(&addr, &ifr.ifr_addr, sizeof(ifr.ifr_addr));
		return inet_ntoa(addr.sin_addr);
	}
}

void generatePASVMessage(char * message, int port)
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
	itoa(port / 256, temp, 10);
	strcat(message, temp);
	strcat(message, ",");
	memset(temp, 0, 10);
	itoa(port % 256, temp, 10);
	strcat(message, temp);
	strcat(message, ")\r\n");
}

void getFullPath(char * root, char * cwd, char * args, char * result)
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
		strcat(result, args);
	}
}

void getDirList(char * buf)
{
	char path[1000];
	memset(path, 0, 1000);
	path = getcwd(NULL, NULL);
	DIR *dir = opendir(path);
	struct dirent *dirptr;
	while ((dirptr = readdir(dir)) != NULL)
	{
		strcat(buf, dirptr->d_name);
		if (dirptr->d_type == DT_DIR)
			strcat(buf, " DIR\n");
		else if(dirptr->d_type == DT_REG)
			strcat(buf, " FILE\n");
		else
			strcat(buf, " OTHER\n");
	}
	strcat(buf, "\r\n");
}

int startClientSession(int connfd, char * rootPath)
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
	while(1)
	{
		char str[8192];
		receiveStringfromClient(connfd, str);
		char cmd[10],args[8192];
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
					status = 2;
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
		else if(!strcmp(cmd,"RETR"))
		{
			int file_fd = 0;
			char path[1000];
			memset(path, 0, 1000);
			getFullPath(rootPath, curPath, args, path);
			if (status == STATUS_READY)
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
					if(sendFile(file_fd, path))
						sendStringtoClient(connfd, str_finish);
					else
						sendStringtoClient(connfd, str_fileFail);

				}
				else
				{
					file_fd = accept(pasv_listenfd, NULL, NULL);
					if (sendFile(file_fd, path))
						sendStringtoClient(connfd, str_finish);
					else
						sendStringtoClient(connfd, str_fileFail);
				}
				close(file_fd);
			}
			else if (status == STATUS_NOPORT)
			{
				sendStringtoClient(connfd, str_noconnection);
			}
			else
			{
				sendStringtoClient(connfd,str_permission)
			}
		}
		else if(!strcmp(cmd,"STOR"))
		{
			int file_fd = 0;
			char path[1000];
			memset(path, 0, 1000);
			getFullPath(rootPath, curPath, args, path);
			if (status == STATUS_READY)
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
					if (recvFile(file_fd, path))
						sendStringtoClient(connfd, str_finish);
					else
						sendStringtoClient(connfd, str_fileFail);
				}
				else
				{
					file_fd = accept(pasv_listenfd, NULL, NULL);
					if (recvFile(file_fd, path))
						sendStringtoClient(connfd, str_finish);
					else
						sendStringtoClient(connfd, str_fileFail);
				}
				close(file_fd);
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
				if(pasv_listenfd != 0)
					close(pasv_listenfd);
				mode = MODE_PASV;
				while (1)
				{
					dataport = rand() % (65535 - 1 + 20000) + 20000;
					if ((pasv_listenfd = initSocket(dataport) == -1)
						continue;
					break;
				}
				char message[200];
				memset(message, 0, 200);
				generatePASVMessage(message, dataport);
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
				getFullPath(rootPath, curPath, args, path);
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
				sendStringtoClient(connfd, str_permission);
			}
		}
		else if(!strcmp(cmd,"CWD"))
		{
			if (status >= STATUS_NOPORT)
			{
				char path[1000];
				memset(path, 0, 1000);
				getFullPath(rootPath, curPath, args, path);
				if (chdir(path) == 0)
				{
					path = getcwd(NULL, NULL);
					if (strncmp(path, rootPath, strlen(rootPath)) == 0)
					{
						path += strlen(rootPath);
						if (*path == 0)
							*path == '/';
						memset(curPath, 0, 1000);
						strcpy(curPath, path);
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
				else
				{
					sendStringtoClient(connfd, str_permission);
				}
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
				char message[1000];
				memset(message, 0, 1000);
				strcpy(message, str_open);
				strcat(message, "list of current dircttory\r\n");
				sendStringtoClient(connfd, message);
				char buf[100000];
				memset(buf, 0, 100000);
				getDirList(buf);
				if (mode == MODE_PORT)
				{
					file_fd = connectUser(client_IP, dataport);
					int len = strlen(buf);
					p = 0;
					while (p < len) {
						int n = write(file_fd, buf + p, len + 1 - p);
						if (n < 0) {
							sendStringtoClient(connfd, str_fileFail);
							close(file_fd);
						}
						else {
							p += n;
						}
					}
					sendStringtoClient(connfd, str_finish);
				}
				else
				{
					file_fd = accept(pasv_listenfd, NULL, NULL);
					int len = strlen(buf);
					p = 0;
					while (p < len) {
						int n = write(file_fd, buf + p, len + 1 - p);
						if (n < 0) {
							sendStringtoClient(connfd, str_fileFail);
							close(file_fd);
						}
						else {
							p += n;
						}
					}
					sendStringtoClient(connfd, str_finish);
				}
				close(file_fd);
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
				getFullPath(rootPath, curPath, args, path);
				if (rmdir(path) == 0)
				{
					sendStringtoClient(connfd, str_dirok);
				}
				else
				{
					sendStringtoClient(connfd, str_permission);
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
				getFullPath(rootPath, curPath, args, path);
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
			else
			{
				sendStringtoClient(connfd, str_permission);
			}
		}
		else if(!strcmp(cmd,"RNTO"))
		{
			if (status >= STATUS_NOPORT)
			{
				char path[1000];
				memset(path, 0, 1000);
				getFullPath(rootPath, curPath, args, path);
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
			sendStringtoClient(connfd, str_nocmd);
		}
	}
}

int main(int argc, char **argv) {
	int listenfd, connfd;		//监听socket和连接socket不一样，后者用于数据传输
	struct sockaddr_in addr;
	char sentence[8192];
	int p;
	int len;
	int port = 21;
	char path[500] = "/tmp";

	if(chdir(path) != 0)
		return 1;
	path = getcwd(NULL, NULL);
	localIP = getLocalIP();

	if(!handleArgs(&port, path, argc, argv))
		return 1;

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
			if(pid < 0)
			{
				printf("fork failed");
				return -1;
			}	
			if(pid == 0)
			{
				close(listenfd);
				startClientSession(connfd, path);
				close(connfd);
				return 0;
			}
		

		}
		
		//榨干socket传来的内容
		p = 0;
		while (1) {
			int n = read(connfd, sentence + p, 8191 - p);
			if (n < 0) {
				printf("Error read(): %s(%d)\n", strerror(errno), errno);
				close(connfd);
				continue;
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
		sentence[p - 1] = '\0';
		len = p - 1;
		

		//发送字符串到socket
 		
		close(connfd);
	}
	close(listenfd);
	return 0;
}

