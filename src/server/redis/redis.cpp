# include "redis.hpp"
# include <iostream>
using namespace std;

// 构造函数
Redis:: Redis()
: _publish_context(nullptr), _subscribe_context(nullptr){}

Redis::~Redis()
{
    // 如果_publish_context不为空，则释放它
    if(_publish_context)
    {
        redisFree(_publish_context);
    }
    // 如果_subscribe_context不为空，则释放它
    if(_subscribe_context)
    {
        redisFree(_subscribe_context);
    }
}

bool Redis::connect()
{
    // 将负责publish消息的_publish_context连接到Redis服务
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (_publish_context == nullptr)
    {
        cerr << "Error: Failed to connect to Redis server!"<< endl;
        return false;
    }
    // 将负责subscribe消息的_subscribe_context连接到Redis服务
    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if (_subscribe_context == nullptr)
    {
        cerr << "Error: Failed to connect to Redis server!" << endl;
        return false;
    }

    // 在单独的线程中，监听通道上的消息
    thread t([&](){
        observer_channel_message();
    });
    t.detach();
    cout << "Connect redis-server success!" << endl;
    return true;
}

// 向redis指定的通道发布消息
bool Redis:: publish(int channel, string message)
{
    // 将消息发布到指定的通道
    redisReply *reply = (redisReply *)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (reply == nullptr)
    {
        cerr << "Error: Failed to publish message to Redis server!" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// 向redis指定的通道订阅消息
bool Redis:: subscribe(int channel)
{
    // SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，这里只做订阅通道命令的发出，不接收通道消息
    // 订阅执行和通道消息的接收专门在observer_channel_message函数中的独立线程中进行
    // 只负责发送订阅命令，不阻塞接收redis server响应消息（则本线程不会阻塞），否则和回调函数抢占响应资源，导致回调函数迟迟不能进行
    
    // 在订阅上下文中发送订阅命令
    if (REDIS_ERR == redisAppendCommand(this->_subscribe_context, "SUBSCRIBE %d", channel))
    {
        cerr << "subscribe command failed!" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done))
        {
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

// 向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel)
{
    // 在订阅上下文中发送取消订阅命令
    if (REDIS_ERR == redisAppendCommand(this->_subscribe_context, "UNSUBSCRIBE %d", channel))
    {
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done))
        {
            cerr << "unsubscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

// 在独立线程中接收订阅通道中的消息
void Redis::observer_channel_message()
{
    // redisGetReply函数会阻塞等待订阅上下文中订阅的通道中发生消息
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subscribe_context, (void **)&reply))
    {
        // 订阅收到的消息是一个带三元素的数组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            // 调用回调函数给业务层上报通道上发生的消息
            _notify_message_handler(atoi(reply->element[1]->str) , reply->element[2]->str);
        }
        // 释放redisReply对象
        freeReplyObject(reply);
    }

    cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << endl;
}

//初始化回调函数对象
void Redis::init_notify_handler(function<void(int,string)> fn)
{
    this->_notify_message_handler = fn;
}