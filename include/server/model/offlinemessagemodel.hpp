#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H
#include <string>
#include <vector>
using namespace std;
//提供离线消息表的操作接口方法
class OfflineMsgModel{
    public:
    //存储用户离线消息
    void insert(int userid,string msg);

    void remove(int userid);

    vector<string> query(int userid);
    private:

};

#endif