# ifndef FRIENDMODEL_HPP
# define FRIENDMODEL_HPP

# include "user.hpp"
# include <vector>
using namespace std;

// 提供对friend表的操作方法
class FriendModel
{
public:
    // 添加好友
    void insert(int userId, int friendId);
    // 返回好友列表
    vector<User> query(int userId);
};

# endif