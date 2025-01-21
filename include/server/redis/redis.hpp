# ifndef REDIS_H__
# define REDIS_H__

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>
using namespace std;

class Redis
{
public:
    Redis();
    ~Redis();

    // 连接redis服务器
    bool connect();

    // 向redis指定的通道发布消息
    bool publish(int channel, string message);

    // 向redis指定的通道订阅消息
    bool subscribe(int channel);

    // 向redis指定的通道取消订阅
    bool unsubscribe(int channel);

    // 在独立线程中监听通道消息（负责订阅、取消订阅、调用回调函数上报消息）
    void observer_channel_message();

    // 初始化回调函数对象
    void init_notify_handler(function<void(int, string)> fn);

private:
    // hiredis同步上下文对象(相当于redis客户端），负责publish消息
    redisContext* _publish_context;
    // hiredis同步上下文对象(相当于redis客户端），负责subscribe消息（以及unsubscribe）
    redisContext* _subscribe_context;

    //回调函数：收到订阅的消息，给service层上报消息及来自哪个通道
    function<void(int, string)> _notify_message_handler;
};

#endif