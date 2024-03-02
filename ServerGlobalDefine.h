#pragma once

#include <memory>
#include <vector>

typedef shared_ptr<vector<char>> share_buff;
#define NEW_SHAREDBUFF make_shared<vector<char>>()

#define LEN_HOSTADDR 256

//connectionpool连接类型
#define SVR_LINK_TYPE_UNKNOWN 0
#define SVR_LINK_TYPE_CLIENT 100

//消息发送结果
#define SEND_MSG_SUCCESS  0    //发送消息成功
#define SEND_MSG_FAIL	  1    //发送消息失败
#define SEND_MSG_TIMEOUT  2    //发送消息超时
#define SEND_MSG_UNKONWERR  3    //发送消息未知错误

//rpc协议宏定义选项
#define PROTOBUF   //使用protobuf
