#ifndef CONST_H
#define CONST_H

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
char * str_port = "200 PORT command succeed\r\n";
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
char * str_nornfr = "503 No RNFR command\r\n";
char * str_rest = "350 REST Okey\r\n";
char * str_rnfr = "350 RNFR Okey\r\n";

#endif