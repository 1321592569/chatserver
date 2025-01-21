# ifndef OFFLINEMESSAGEMODEL_H
# define OFFLINEMESSAGEMODEL_H

#include <vector>
#include <string>
using namespace std;

class OfflineMsgModel
{
public:

    //插入一条离线消息给接收用户
    void insert(int userId, string msg);

    //删除该用户收到的所有离线消息
    void remove(int userId);

    //查询某个用户的离线消息
    vector<string> query(int userId);
};

# endif // OFFLINEMESSAGEMODEL_H