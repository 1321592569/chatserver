# include "chatserver.hpp"
# include "chatservice.hpp"

ChatServer:: ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &name)
    : _server(loop, listenAddr, name), _loop(loop)
{
    // 给服务器注册用户连接的创建和断开配置回调函数
    // 使用 std::bind 将 onConnection 函数与 this 指针绑定，确保能正确调用 ChatServer 的成员函数
    _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));
    // 给服务器注册用户读写事件配置回调函数
    // 使用 std::bind 将 onMessage 函数与 this 指针绑定，确保能正确调用 ChatServer 的成员函数
    _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));

    /* 因为我们使用 std::bind 绑定了 onConnection 函数和 this 指针，
    所以 _server 可以正确调用 ChatServer 对象的 onConnection 成员函数，并将相应的连接信息作为参数传递给它。
    如果不使用 std::bind，直接将 onConnection 函数作为回调函数传递给 _server.setConnectionCallback，
    那么 _server 就不知道应该使用哪个 ChatServer 类的实例来调用 onConnection，因为 onConnection 是一个成员函数，而不是一个静态函数或普通的函数。
    */

    // 设置服务器端的线程数量，1 个 I/O 线程，3 个 worker 线程
    _server.setThreadNum(4);
}

void ChatServer:: start()
{
    // 启动服务器
    _server.start();
}


void ChatServer:: onConnection(const TcpConnectionPtr &conn)
{
    // 如果连接已经建立
    if (conn->connected())
    {
        // 打印连接的远程地址和本地地址，表示连接已建立
        cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " is connected" << endl;
    }
    else
    {
        // 打印连接的远程地址和本地地址，表示连接已断开
        cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " is disconnected" << endl;
        // 关闭连接时，调用 ChatService 对象的 clientCloseException 函数处理用户异常退出
        ChatService:: instance() -> clientCloseException(conn);
        // 关闭连接
        conn->shutdown();
    }
}

// TcpConnection 的回调函数，当有消息到达时，会调用这个函数
void ChatServer:: onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    // 从接收缓冲区 buf 中读取所有数据，并将其存储为字符串
    string msg = buf->retrieveAllAsString();
    json js = json::parse(msg); //反序列化为json数据格式
    // 达到的目的：完全解耦网络模块的代码和业务模块的代码
    // 利用ChatService对象地址调用getHandler函数获取js["msgid"]所对应的业务处理方法
    MsgHandler msgHandler = ChatService:: instance() -> getHandler(js["msgid"].get<int>());
    msgHandler(conn, js, time); //调用业务处理方法
}


