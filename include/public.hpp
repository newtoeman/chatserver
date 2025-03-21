#ifndef PUBLIC_H
#define PUBLIC_H
/*
public,serve and client
*/
enum EnMsgType{
    LOGIN_MSG = 1,//登录
    LOGIN_MSG_ACK,
    LOGIN_OUT_MSG,//登出
    REG_MSG,//注册
    REG_MSG_ACK,
    ONE_CHAT_MSG, //聊天消息
    ADD_FRIEND_MSG, //添加好友消息
    CREATE_GROUP_MSG, //创建群组
    ADD_GROUP_MSG, //加入群组
    GROUP_CHAT_MSG //群聊天
    

};

#endif