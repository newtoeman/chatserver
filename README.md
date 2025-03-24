基于muduo网络库实现的聊天服务器，涵盖服务器ChatServer和客户端ChatClient，系统具备用户登录、注册、一对一聊天、群聊、添加好友以及处理离线消息等功能。服务器使用 Redis 进行消息的发布与订阅，利用 MySQL 存储用户数据。
项目结构
chatserver/
├── CMakeLists.txt
├── bin/  # 编译生成的可执行文件存放目录
├── include/
│   ├── server/
│   │   ├── db/
│   │   ├── model/
│   │   ├── redis/
│   └── public.hpp
├── src/
│   ├── client/
│   │   ├── main.cpp
│   │   └── CMakeLists.txt
│   └── server/
│       ├── chatservice.cpp
│       ├── model/
│       │   └── groupmodel.cpp
│       ├── redis/
│       └── CMakeLists.txt
└── build/
    └── compile.md
构建步骤
创建构建目录并进入：
bash
mkdir build
cd build

运行 CMake：
cmake ..

编译项目：
make

编译完成后，可在bin目录下找到生成的可执行文件ChatServer和ChatClient。
