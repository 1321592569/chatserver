# include "offlinemessagemodel.hpp"
# include "db.h"

void OfflineMsgModel:: insert(int userId, string msg)
{
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values (%d, '%s')", userId, msg.c_str());
    //连接MySQL服务并向其输入sql语句
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

void OfflineMsgModel::remove(int userId)
{
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid = %d", userId);
    //连接MySQL服务并向其输入sql语句
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

vector<string> OfflineMsgModel:: query(int userId)
{
    //定义一个字符串数组，用于存储查询到的离线消息
    vector<string> vec;
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d", userId);
    //连接MySQL服务并向其输入sql语句
    MySQL mysql;
    if(mysql.connect())
    {
        //执行sql语句并返回结果集,res为结果集指针
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            //不断获取结果集的行向量，并将该行的消息存入vec中
            while(row = mysql_fetch_row(res))
            {
                if (row[0] != nullptr)
                {
                    vec.push_back(row[0]);
                }
                else
                {
                    break;
                }
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}
