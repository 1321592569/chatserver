# ifndef GROUPMODEL_H__
# define GROUPMODEL_H__

#include <vector>
#include <string>
#include "group.hpp"
using namespace std;

// 提供对于群组信息（包括AllGroup表和GroupUser表）的操作方法
class GroupModel
{
public:
    // 创建群组
    bool createGroup(Group& group);
    // 加入群组
    void addGroup(int userId, int groupId, string role);
    // 查询用户所在群组信息
    vector<Group> queryGroups(int userId);
    // 向群组中的其它用户发送消息（获取他们的id）
    vector<int> queryGroupUsers(int userId, int groupId);
};
# endif // GROUPMODEL_H__