# ifndef CHATSERVICE_H
#define CHATSERVICE_H

# include <mutex>
#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
using namespace muduo;
using namespace muduo::net;
using namespace std;

# include "redis.hpp"
# include "usermodel.hpp"
# include "offlinemessagemodel.hpp"
# include "friendmodel.hpp"
# include "groupmodel.hpp"
# include "json.hpp"
using json = nlohmann::json;
// 定义消息的业务处理方法函数为MsgHandler类型
using MsgHandler = std::function<void(const TcpConnectionPtr& conn, json& js, Timestamp)>;

//聊天服务器业务类
class ChatService
{
public:
    //获取单例对象的接口函数
    static ChatService* instance();
    //业务处理方法之一：处理登录业务
    void login(const TcpConnectionPtr& conn, json& js, Timestamp);
    //业务处理方法之一：处理登出业务
    void loginout(const TcpConnectionPtr& conn, json& js, Timestamp);
    //业务处理方法之一：处理注册业务
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp);
    //业务处理方法之一：处理一对一聊天业务
    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp);
    //业务处理方法之一：添加好友业务
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp);
    //业务处理方法之一：创建群组业务
    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp);
    //业务处理方法之一：加入群组业务
    void addGroup(const TcpConnectionPtr& conn, json& js, Timestamp);
    //业务处理方法之一：群组聊天业务
    void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp);
    
    // 获取消息对应的业务处理方法
    MsgHandler getHandler(int msgid);
    
    // 处理客户端退出的方法
    void clientCloseException(const TcpConnectionPtr& conn);
    //服务器异常，业务重置方法
    void reset();
private:
    //构造函数,用于初始化_msgHandlerMap
    ChatService();
    //存储消息id和其对应业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;
    //存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    //互斥锁，保证不同用户对_userConnMap操作的线程安全
    mutex _connMutex;

    //数据操作类对象
    UserModel _userModel;
    //离线消息表的操作类对象
    OfflineMsgModel _offlineMsgModel;
    //好友表的操作类对象
    FriendModel _friendModel;
    //群组表的操作类对象
    GroupModel _groupModel;

    // redis 操作对象
    Redis _redis;
    // redis上报消息的回调函数（用于上报通道上的消息）
    void handleRedisSubscribeMessage(int channel, string msg);
};
# endif // CHATSERVICE_H
