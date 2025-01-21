#include "groupmodel.hpp"
#include "db.h"

// 创建群组
bool GroupModel::createGroup(Group &group)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup (groupname, groupdesc) values ('%s','%s')",
            group.getName().c_str(), group.getDesc().c_str());
    // 连接MySQL服务并向其输入sql语句
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取成功插入的数据的主键id
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}
// 加入群组
void GroupModel::addGroup(int userId, int groupId, string role)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser values (%d, %d, '%s')",
            groupId, userId, role.c_str());
    // 连接MySQL服务并向其输入sql语句
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 查询用户所在群组信息
vector<Group> GroupModel:: queryGroups(int userId)
{
    {
        // 定义一个Group数组，用于存储查询到的用户所在群组信息
        vector<Group> groupVec;
        // 组装sql语句
        char sql[1024] = {0};
        sprintf(sql, "select a.id, a.groupname, a.groupdesc from allgroup a inner join groupuser b on a.id = b.groupid where b.userid = %d", userId);
        // 连接MySQL服务并向其输入sql语句
        MySQL mysql;
        if (mysql.connect())
        {
            // 执行sql语句并返回结果集,res为结果集指针
            MYSQL_RES *res = mysql.query(sql);
            if (res != nullptr)
            {
                MYSQL_ROW row;
                // 不断获取结果集的行向量，并将该行的消息存入vec中
                while (row = mysql_fetch_row(res))
                {
                    if (row[0] != nullptr)
                    {
                        Group group;
                        group.setId(atoi(row[0]));
                        group.setName(row[1]);
                        group.setDesc(row[2]);
                        groupVec.push_back(group);
                    }
                    else
                    {
                        break;
                    }
                }
                mysql_free_result(res);
            }
        }
        return groupVec;
    }
}
// 向群组中的其它用户发送消息（获取他们的id）
vector<int> GroupModel:: queryGroupUsers(int userId, int groupId)
{
        {
        // 定义一个int数组，用于存储需要接收消息的用户id
        vector<int> idVec;
        // 组装sql语句
        char sql[1024] = {0};
        sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", groupId, userId);
        // 连接MySQL服务并向其输入sql语句
        MySQL mysql;
        if (mysql.connect())
        {
            // 执行sql语句并返回结果集,res为结果集指针
            MYSQL_RES *res = mysql.query(sql);
            if (res != nullptr)
            {
                MYSQL_ROW row;
                // 不断获取结果集的行向量，并将该行的消息存入vec中
                while (row = mysql_fetch_row(res))
                {
                    if (row[0] != nullptr)
                    {
                        idVec.push_back(atoi(row[0]));
                    }
                    else
                    {
                        break;
                    }
                }
                mysql_free_result(res);
            }
        }
        return idVec;
    }
}