#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <unordered_map>
#include <muduo/net/TcpConnection.h>
#include <functional>
#include <mutex>
#include "json.hpp"
#include "usermodel.hpp"
#include "groupmodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "redis.hpp"

using json = nlohmann::json;
using namespace muduo;
using namespace muduo::net;
using namespace std;

using MsgHandler = std::function<void(const TcpConnectionPtr &conn,json &js,Timestamp)>;

class ChatService{
    public:
    //get instance
    static ChatService *instance();
    //
    void login(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //
    void reg(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //一对一聊天
    void oneChat(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //创建群组
    void createGroup(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //加入群组
    void addGroup(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //群聊
    void groupChat(const TcpConnectionPtr &conn,json &js,Timestamp time);
    //注销业务
    void loginout(const TcpConnectionPtr &conn,json &js,Timestamp time);

    MsgHandler getHandler(int msgid);
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    //服务器异常退出
    void reset();

    //从redis消息队列中获取订阅的消息
    void HandleRedisSubscribeMessage(int userid,string msg);
 
    private:
    //id,func
    unordered_map<int,MsgHandler> _msgHandlerMap;
    ChatService();
    //数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
    //存储在线用户的通信连接
    unordered_map<int,TcpConnectionPtr> _userConnMap;

    //保证ConnMap线程安全
    mutex _connMutex;

    //redis操作对象
    Redis _redis;

};

#endif