#ifndef PUBLIC_H
# define PUBLIC_H

// server和client的公共头文件
enum ENMsgType
{
    LOGIN_MSG = 1, // 登录消息
    LOGIN_MSG_ACK, // 登录响应消息
    LOGINOUT_MSG, // 登出消息
    REG_MSG, // 注册消息
    REG_MSG_ACK, // 注册响应消息
    ONE_CHAT_MSG, // 一对一聊天消息
    ADD_FRIEND_MSG, // 添加好友消息

    CREATE_GROUP_MSG, // 创建群组消息
    ADD_GROUP_MSG, // 加入群组消息
    GROUP_CHAT_MSG // 群组聊天消息
};
/*
enum为枚举类型，在 C/C++ 的枚举类型中，
如果没有显式地为枚举常量赋值，那么它的值是前一个枚举常量的值加 1
*/

#endif // PUBLIC_H 


