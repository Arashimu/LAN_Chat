#include "MyDB.h"
#include <iostream>
#include <string>
using namespace std;

MyDB::MyDB(){
    mysql=mysql_init(NULL);
    if(mysql==nullptr){
        cout<<"Errot: "<<mysql_error(mysql);
        exit(1);
    }
}
MyDB::~MyDB(){
    if(mysql!=nullptr){
        mysql_close(mysql);
    }
}
bool MyDB::InitDB(string host,string user,string pwd,string db_name){
    mysql=mysql_real_connect(mysql,host.c_str(),user.c_str(),pwd.c_str(),db_name.c_str(),0,nullptr,0);
    if(mysql==NULL){
        cout<<"Error: "<<mysql_error(mysql)<<"\n";
        exit(1);
    }
    return true;
}
bool MyDB::ExeSql(string cmd){
    if(mysql_query(mysql,cmd.c_str())){
        cout<<"Query Error: "<<mysql_error(mysql);
        return false;
    }else{
        res=mysql_store_result(mysql);
        if(res){
            int num_fields=mysql_num_fields(res);
            int num_rows=mysql_num_rows(res);
            if(num_rows==0) return false;
        }else{
            if(mysql_field_count(mysql)==0){  //代表执行的是update,insert,delete类的非查询语句  
                int num_rows=mysql_affected_rows(mysql);
            }else{
                cout<<"Get result error: "<<mysql_error(mysql)<<"\n";
                return false;
            }
        }
    }
    return true;
}
