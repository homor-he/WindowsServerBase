#pragma once

#include <memory>
#include <vector>

typedef shared_ptr<vector<char>> share_buff;
#define NEW_SHAREDBUFF make_shared<vector<char>>()

#define LEN_HOSTADDR 256

//connectionpool��������
#define SVR_LINK_TYPE_UNKNOWN 0
#define SVR_LINK_TYPE_CLIENT 100

//��Ϣ���ͽ��
#define SEND_MSG_SUCCESS  0    //������Ϣ�ɹ�
#define SEND_MSG_FAIL	  1    //������Ϣʧ��
#define SEND_MSG_TIMEOUT  2    //������Ϣ��ʱ
#define SEND_MSG_UNKONWERR  3    //������Ϣδ֪����

//rpcЭ��궨��ѡ��
#define PROTOBUF   //ʹ��protobuf
