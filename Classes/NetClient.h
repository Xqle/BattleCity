#ifndef __NET_CLIENT_H__
#define __NET_CLIENT_H__

// json ����
#include "json/rapidjson.h"
#include "json/document.h"

// json ��װ
#include "json/writer.h"
#include "json/stringbuffer.h"
using namespace  rapidjson;

#include "cocos2d.h"
#include "GameClient.h"
#include "ODSocket/ODSocket.h"
#include "Msg/Msg.h"
USING_NS_CC;
using namespace cocos2d;

#define MAX_MSG_LEN 1024

class NetClient : public Node
{
public:
	NetClient() {};  
	~NetClient() {};

	static NetClient* create(GameClient* parent);
	virtual bool init(GameClient* parent);

	void connect(const char* ip, int port);
	void send(Msg* msg);

	void UDPrecThread();

private:
	GameClient*  m_parent;
	ODSocket     m_socket;        // ���ڷ���TCP����
	ODSocket     m_udpSocket;     // ���ڷ���udp����
	int          m_udpPort;       // �ͻ���udp�˿ں� : �ɷ���˶�̬����
	int          m_serverUdpPort; // ��������udp�˿ں�
	int          m_tankID;        // ����˶�̬����̹��Ψһ�ı�ʶID
	std::string  m_serverIP;      // ������IP
};

#endif