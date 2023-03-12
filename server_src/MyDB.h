#ifndef MYDB_H
#define MYDB_H

#include <iostream>
#include <mysql/mysql.h>
#include <string>
using namespace std;

class MyDB{
public:
    MyDB();
    ~MyDB();
    bool InitDB(string host,string user,string pwd,string db_name);
    bool ExeSql(string cmd);
private:
    MYSQL *mysql;
    MYSQL_RES *res;
    MYSQL_ROW row;
};
#endif