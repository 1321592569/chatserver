# include "usermodel.hpp"
# include "db.h"

//User表的增加方法
bool UserModel:: insert(User& user)
{
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user (name, password, state) values ('%s','%s', '%s')",
            user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    //连接MySQL服务并向其输入sql语句
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            //获取成功插入的数据的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

//根据id查找用户信息
User UserModel:: query(int id)
{
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);
    //连接MySQL服务并向其输入sql语句
    MySQL mysql;
    if(mysql.connect())
    {
        //执行sql语句并返回结果集,res为结果集指针
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr)
        {
            //获取结果集的行向量
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    //如果查询失败，返回一个默认的空的User对象
    return User();
}

// 更新用户的状态信息
bool UserModel::updateState(User user)
{
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());
    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

//重置所有用户的状态信息
void UserModel::resetState()
{
    char sql[1024] = {0};
    sprintf(sql, "update user set state = 'offline' where state = 'online'");
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}




