# include "friendmodel.hpp"
# include "db.h"
# include "user.hpp"
// 添加好友
void FriendModel:: insert(int userId, int friendId)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values (%d, %d)", userId, friendId);
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}
// 返回好友列表
vector<User> FriendModel:: query(int userId)
{
    // 创建返回的好友列表
    vector<User> vec;
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, a.state from user a inner join friend b on b.friendid = a.id where b.userid = %d", userId);
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
                    User user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    vec.push_back(user);
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