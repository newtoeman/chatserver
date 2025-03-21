#ifndef USERMODEL_H
#define USERMODEL_H

#include "User.hpp"

class UserModel{
    public:
    bool insert(User &user);
    //查询用户信息
    User query(int id);

    //更新用户状态
    bool updateState(User user);
    
    //
    void resetState();
};

#endif