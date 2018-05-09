# 树莓派3b

#文件
#README.md 帮助文件
#clock.cpp 天气语音闹钟c++源码

#依赖库

#1.libcurl， 多协议文件传输库

sudo apt-get install libcurl4-gnutls-dev

#2.jsoncpp，轻量级的json交互C++库

sudo apt-get install  libjsoncpp-dev

#编译命令

g++ clock.cpp -lcurl -ljsoncpp
