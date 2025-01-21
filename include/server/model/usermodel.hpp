# ifndef USERMODEL_H
# define USERMODEL_H

#include "user.hpp"

// 提供对User表的操作方法
class UserModel
{
public:
    //User表的增加方法
    bool insert(User& user);

    //根据id查找用户信息
    User query(int id);

    // 更新用户的状态信息
    bool updateState(User user);

    //重置所有用户的状态信息
    void resetState();
};
# endif //USERMODEL_H