#include "chatservice.hpp"
#include "user.hpp"
#include <muduo/base/Logging.h>
#include "public.hpp"
#include <vector>
using namespace std;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 建立不同类型消息的对应的不同回调函数字典（即不同的业务处理方法）
ChatService::ChatService()
{
    //用户基本业务管理相关事件处理的回调函数注册
    _msgHandlerMap.insert({LOGIN_MSG, bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, bind(&ChatService::loginout, this, _1, _2, _3)});
    
    // 群组业务管理相关事件处理的回调函数注册
    _msgHandlerMap.insert({CREATE_GROUP_MSG, bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, bind(&ChatService::groupChat, this, _1, _2, _3)});
    
    // 连接redis服务器
    if (_redis.connect())
    {
        // 设置上报消息的回调函数
        _redis.init_notify_handler(bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

// 获取消息对应的业务处理方法
MsgHandler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    // 该消息没有对应的业务处理方法则返回一个默认方法，什么都不做
    if (it == _msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

// 业务处理方法之一：处理登录业务 ID password password
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int id = js["id"].get<int>();
    string password = js["password"];
    User user = _userModel.query(id); // 根据id查询数据库中该用户的信息
    if (user.getId() == id && user.getPassword() == password)
    {
        if (user.getState() == "online")
        {
            // 该用户已经登录，向客户端返回用户已经登录的消息
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2; // 用户已经登录
            response["errmsg"] = "该账号已经登录，请重新输入新账号";
            conn->send(response.dump());
        }
        else
        {
            // 登录成功
            {
                lock_guard<mutex> lock(_connMutex); // 加锁,{}结束后自动解锁
                /*
                lock_guard<mutex>：这是 C++ 标准库中的一个模板类，用于自动管理互斥锁的加锁和解锁操作
                mutex 是模板参数，指定了要使用的互斥锁类型，这里是 std::mutex。
                lock：是 lock_guard 类的一个对象实例。你可以给这个对象取任何你喜欢的名字，这里取名为 lock。
                _connMutex：是 std::mutex 类型的对象，它是需要加锁的互斥锁。
                */
                _userConnMap.insert({id, conn}); // 将用户id和对应的连接保存到map中
                /*
                虽然每个线程可能是为不同的用户插入不同的 id 和 conn 对，
                但 std::unordered_map 本身不是线程安全的:迭代器可能失效等
                */
            }
            // id 用户登录成功后，向redis订阅channel(id)
            _redis.subscribe(id);

            // 更新数据库中的对应用户状态信息
            user.setState("online");
            _userModel.updateState(user);

            // 向客户端返回登录成功的消息
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0; // 登录成功
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 将离线消息表中该用户的离线消息删除
                _offlineMsgModel.remove(id);
            }

            //  查询该用户的好友列表
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty())
            {
                // 将用户的好友列表序列化为json数据再反序列化为json字符串
                vector<string> vec2;
                for (User &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 返回登录成功后的各种信息
            conn->send(response.dump());
        }
    }
    else
    {
        // 登录失败，向客户端返回用户名或密码错误的消息
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1; // 用户名或密码错误
        response["errmsg"] = "用户名或密码错误";
        conn->send(response.dump());
    }
}
// 业务处理方法之一：处理注册业务 name password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    string name = js["name"];
    string password = js["password"];
    // 向数据库插入注册的用户信息
    User user;
    user.setName(name);
    user.setPassword(password);
    // 调用用户信息使用方法中的insert方法将用户信息插入到数据库中
    bool state = _userModel.insert(user);
    // 注册成功
    if (state)
    {
        json response;
        response["msgid"] = REG_MSG_ACK; // 返回注册相应消息3
        response["errno"] = 0;           // 返回注册成功
        response["id"] = user.getId();   // 返回注册成功后数据库自动赋予的用户id
        conn->send(response.dump());
    }
    // 注册失败
    else
    {
        json response;
        response["msgid"] = REG_MSG_ACK; // 返回注册相应消息3
        response["errno"] = 1;           // 返回注册失败
        conn->send(response.dump());
    }
}

// 业务处理方法之一：处理一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        // 在_userConnMap在线用户字典中找到了接收者，转发给他消息
        if (it != _userConnMap.end())
        {
            it->second->send(js.dump());
            return;
        }
    }

    // 聊天接收者没和发送者在同一个服务器上在线，则查询其是否在其它服务器上在线（查询数据库）
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        // 在其它服务器上在线，则将消息发布到以接收者id命名的通道上
        _redis.publish(toid, js.dump());
        return;
    }
    // 接收者不在线，将发送的消息存到离线消息表中
    _offlineMsgModel.insert(toid, js.dump());
}

// 业务处理方法之一：添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int friendid = js["friendid"].get<int>();
    int userid = js["id"].get<int>();

    // 将好友信息存到数据库中
    _friendModel.insert(userid, friendid);
}

// 业务处理方法之一：创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    string groupname = js["groupname"];
    string groupdesc = js["desc"];

    // 将群组信息存到数据库中
    Group group(-1, groupname, groupdesc);
    if (_groupModel.createGroup(group))
    {
        // 将群组创建者加入群组
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

// 业务处理方法之一：加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    // 将用户加入群组
    _groupModel.addGroup(userid, groupid, "normal");
}

// 业务处理方法之一：群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    // 查找群组中的其它用户，并把消息转发给他们
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    for (int id : useridVec)
    {
        lock_guard<mutex> lock(_connMutex);
        // 在_userConnMap在线用户字典中找到了接收者，转发给他消息
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            // 聊天接收者没和发送者在同一个服务器上在线，则查询其是否在其它服务器上在线（查询数据库）
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                // 在其它服务器上在线，则将消息发布到以接收者id命名的通道上
                _redis.publish(id, js.dump());
            }
            else
            {
                // 接收者不在线，将发送的消息存到离线消息表中
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}

//业务处理方法之一：处理登出业务
void ChatService:: loginout(const TcpConnectionPtr& conn, json& js, Timestamp)
{
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        // 在_userConnMap在线用户字典中找到了要登出的用户后删除该用户
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }
    // 用户登出，在redis中取消订阅通道
    _redis.unsubscribe(userid);

    // 更新数据库中的用户状态信息
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}

// 处理客户端退出的方法
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        // 在_userConnMap中查找该连接对应的用户
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                user.setId(it->first);
                // 从_userConnMap中删除该用户的连接信息（id,conn）
                _userConnMap.erase(it);
                break;
            }
        }
    }

    // 客户端退出, 则用户也下线，在redis中取消订阅通道
    _redis.unsubscribe(user.getId());

    // 更新数据库中的用户状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

// 服务器异常，业务重置方法
void ChatService::reset()
{
    // 把所有online状态的用户设置为offline
    _userModel.resetState();
}

// redis上报消息的回调函数（用于上报通道上的消息）
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    // 在_userConnMap在线用户字典中找到了接收者，转发给他消息
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }
    // 如果接收者不在线，将消息存到离线消息表中
    _offlineMsgModel.insert(userid, msg);
}