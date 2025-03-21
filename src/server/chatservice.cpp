#include "chatservice.hpp"
#include "public.hpp"
#include "group.hpp"
#include <muduo/base/Logging.h>
#include <vector>

using namespace std;
using namespace muduo;

//获取单例对象的接口函数
ChatService* ChatService::instance(){
    static ChatService service;
    return &service;
}
//注册消息对应的回调函数
ChatService::ChatService(){
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,_1,_2,_3)});
    _msgHandlerMap.insert({LOGIN_OUT_MSG,std::bind(&ChatService::loginout,this,_1,_2,_3)});
    _msgHandlerMap.insert({REG_MSG,std::bind(&ChatService::reg,this,_1,_2,_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend,this,_1,_2,_3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::groupChat,this,_1,_2,_3)});

    if(_redis.connect()){
        _redis.init_notify_handler(std::bind(&ChatService::HandleRedisSubscribeMessage,this,_1,_2));
    }
}

void ChatService::reset(){
    //把所有用户状态设置为offline
    _userModel.resetState();
}

//获得消息对应的处理器
MsgHandler ChatService::getHandler(int msgid){
    auto it = _msgHandlerMap.find(msgid);
    //记录错误日志
    if(it == _msgHandlerMap.end()){
        //返回空操作
        return [=](const TcpConnectionPtr &conn,json &js,Timestamp time){
            LOG_ERROR << "msgid:" << msgid <<"cannot find Handler!";
        };

    }
    else{
        return _msgHandlerMap[msgid];
    }
    
}
//处理登录业务
void ChatService::login(const TcpConnectionPtr &conn,json &js,Timestamp time){
    int id = js["id"].get<int>();
    string pwd = js["password"];
    
    User user = _userModel.query(id);
    if(user.getId() == id && user.getPwd() == pwd){
        if(user.getState() == "online"){
            //已登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "账号已登录";
            conn->send(response.dump());
        }

        else{
            {
                //记录用户连接信息
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id,conn});
            }
            //id用户登录成功后向redis订阅channel:id
            _redis.subscribe(id);

            //登录成功,更新用户状态
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            //查询该用户是否有离线消息，有就发送
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty()){
                response["offlinemsg"] = vec;
                //读取离线消息后，删除该用户的离线消息
                _offlineMsgModel.remove(id);
            }
            //查询用户的好友列表
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty()){
                vector<string> vec2;
                for(User &user: userVec){
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
            vector<Group> groupVec = _groupModel.queryGroups(id);
            if(!groupVec.empty()){
                vector<string> groupV;
                for(Group &group : groupVec){
                    json js;
                    js["id"] = group.getId();
                    js["groupname"] = group.getName();
                    js["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for(GroupUser &groupuser : group.getUsers()){
                        json grpusrjs;
                        grpusrjs["id"] = groupuser.getId();
                        grpusrjs["name"] = groupuser.getName();
                        grpusrjs["state"] = groupuser.getState();
                        grpusrjs["role"] = groupuser.getRole();
                        userV.push_back(grpusrjs.dump());
                    }
                    js["users"] = userV;
                    groupV.push_back(js.dump());
                }
                response["groups"] = groupV;
            }


            conn->send(response.dump());
        }
    }
    else{
        //用户不存在
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码错误";
        conn->send(response.dump());
    }
}
//注销业务
void ChatService::loginout(const TcpConnectionPtr &conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if(it != _userConnMap.end()){
            _userConnMap.erase(it);
        }
    }
    //在redis中取消订阅
    _redis.unsubscribe(userid);

    User user(userid,"","offline");
    _userModel.updateState(user);
}

//处理注册业务 name pwd
void ChatService::reg(const TcpConnectionPtr &conn,json &js,Timestamp time){
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if(state){
        //success
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else{
        //failed
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
}


void ChatService::clientCloseException(const TcpConnectionPtr &conn){
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for(auto it = _userConnMap.begin();it != _userConnMap.end();++it){
            if(it->second == conn){
                //map表删除
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    //用户注销，在redis中取消订阅
    _redis.unsubscribe(user.getId());

    //重置状态
    if(user.getId() != -1){
        user.setState("offline");
        _userModel.updateState(user);
    }
    
}

void ChatService::oneChat(const TcpConnectionPtr &conn,json &js,Timestamp time){
    int toid = js["to"].get<int>();
    bool userState = false;//if online?
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if(it != _userConnMap.end()){
            //toid在线，发送    服务器主动推送消息给toid
            it->second->send(js.dump());
            return;
        }
    }
    User user = _userModel.query(toid);
    if(user.getState() == "online"){
        _redis.publish(toid,js.dump());
        return;
    }
    //不在线，存储离线消息
    _offlineMsgModel.insert(toid,js.dump());
}
// msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn,json &js,Timestamp time){
     int userid = js["id"].get<int>();
     int friendid = js["friendid"].get<int>();

     _friendModel.insert(userid,friendid);
}

//创建群组
void ChatService::createGroup(const TcpConnectionPtr &conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    Group group(-1,name,desc);
    if(_groupModel.createGroup(group)){
        _groupModel.addGroup(userid,group.getId(),"creator");
    }
}
//加入群组
void ChatService::addGroup(const TcpConnectionPtr &conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid,groupid,"normal");
}
//群聊
void ChatService::groupChat(const TcpConnectionPtr &conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid,groupid);

    lock_guard<mutex> lock(_connMutex);
    for(int id : useridVec){
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end()){
            //转发群消息
            it->second->send(js.dump());
        }
        else{
            //查询toid是否在线
            User user = _userModel.query(id);
            if(user.getState() == "online"){
                _redis.publish(id,js.dump());
            }
            else{
            //存储离线群消息
            _offlineMsgModel.insert(id,js.dump());
            }
        }
    }
}

//从redis消息队列中获取订阅的消息
void ChatService::HandleRedisSubscribeMessage(int userid,string msg){
    
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end()){
        it->second->send(msg);
        return;
    }
    //存储该用户的离线消息
    _offlineMsgModel.insert(userid,msg);
}