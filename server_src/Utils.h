#ifndef UTILS_H
#define UTILS_H

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include "MyDB.h"
int socket_bind_and_listen(int port);
bool QueryUser(char* username,MyDB &db);
bool InsertUser(char *username,char *password,MyDB &db);
bool QueryIsMatch(char *username,char *password,MyDB &db);
#endif