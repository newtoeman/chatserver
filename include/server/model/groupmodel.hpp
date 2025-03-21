#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include <vector>
#include <string>
#include "group.hpp"

using namespace std;

class GroupModel{
    public:
    //创建群组
    bool createGroup(Group &group);
    //加入群组
    void addGroup(int userid,int groupid,string role);
    //查询用户所在群组信息
    vector<Group> queryGroups(int userid);
    //查询指定groupid群组的userid列表，用于群发消息
    vector<int> queryGroupUsers(int userid,int groupid);
};

#endif