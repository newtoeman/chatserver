#include "chatserver.hpp"
#include <functional>
#include <string>
#include "json.hpp"
#include "chatservice.hpp"

using json = nlohmann::json;
using namespace std;
using namespace placeholders;
ChatServer::ChatServer(EventLoop *loop,
                    const InetAddress &listenAddr,
                    const string &nameArg) 
    : _server(loop,listenAddr,nameArg),_loop(loop){
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));

        _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));
        

        _server.setThreadNum(4);

}

void ChatServer::start(){
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn){
    //
    if(!conn->connected()){
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn,Buffer *buffer,Timestamp time){
    string buf = buffer->retrieveAllAsString();
    //decode
    json js = json::parse(buf);
    //js["msgid"]->handler->conn js time
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    msgHandler(conn,js,time);
}