#include "friendmodel.hpp"
#include "db.h"

void FriendModel::insert(int userid,int friendid){
    //组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"insert into Friend values(%d,%d)",userid,friendid);
    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}
//返回用户好友列表 friendid ->User
vector<User> FriendModel::query(int userid){
    char sql[1024] = {0};
    //userid->Friendid
    sprintf(sql, "SELECT "
        "    CASE "
        "        WHEN b.userid = %d THEN b.friendid "
        "        WHEN b.friendid = %d THEN b.userid "
        "    END AS friend_id, "
        "    u.id, "
        "    u.name, "
        "    u.state "
        "FROM "
        "    Friend b "
        "JOIN "
        "    User u ON (b.userid = %d AND b.friendid = u.id) OR (b.friendid = %d AND b.userid = u.id) "
        "WHERE "
        "    b.userid = %d OR b.friendid = %d;",
   userid, userid, userid, userid, userid, userid);


    vector<User> vec;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr){
            //把userid用户的所有离线消息放入vec返回
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!=nullptr){
                User user;
                user.setId(atoi(row[1]));
                user.setName(row[2]);
                user.setState(row[3]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec; 
        }
    }
    return vec;
}