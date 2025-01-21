
#ifndef CHATSERVER_H
#define CHATSERVER_H
/*
#ifndef CHATSERVER_H检查宏CHATSERVER_H是否已经被定义。
如果没有被定义，那么就执行接下来的代码，即#define CHATSERVER_H，定义了一个宏CHATSERVER_H。
这样，如果这个头文件被多次包含，由于宏CHATSERVER_H已经被定义，就可以避免头文件的重复包含。
*/

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;
using namespace muduo::net;
using namespace muduo;
using namespace placeholders;

// 聊天服务器网络服务类
class ChatServer
{
public:
    // 用构造函数进行初始化
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &name);
    // 启动服务
    void start();

private:
    // TcpConnection 的回调函数，当有新的连接或者中断到来时，会调用这个函数
    void onConnection(const TcpConnectionPtr &conn);
    // TcpConnection 的回调函数，当有消息到达时，会调用这个函数
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);
    // 服务器对象，用于管理 TCP 连接
    TcpServer _server;
    // 事件循环对象，用于监听事件，如连接事件、读写事件等
    EventLoop *_loop;
};

#endif
