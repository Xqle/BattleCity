#include "NetClient.h"
#include "Msg/TankNewMsg.h"
#include "Msg/TankMoveMsg.h"
#include "Msg/FireMsg.h"

//	��Socket����
void NetClient::connect(const char* ip, int port)
{
	m_serverIP = ip;
	// ��ʼ��
	m_socket.Init();
	m_socket.Create(AF_INET, SOCK_STREAM, 0); // tcp

	bool result = m_socket.Connect(ip, port);

	if (result) {
		CCLOG("connect to server success!");
	
		// �����������ӻ�����Ϣ��json��ʽ��
		// {"serverUdpPort":9999 ,
		//        "udpPort":10001,
		//         "tankID":101  }
		char connectInfo[MAX_MSG_LEN] = "";
		int result = m_socket.Recv(connectInfo, MAX_MSG_LEN, 0);
		if (result <= 0)
		{
			CCLOG("server stop OR other error!"); // δ���յ����������ݣ�����������ִ��
			return; 
		}
		CCLOG("recv connectInfo data : %s", connectInfo);
		rapidjson::Document d;
		d.Parse<0>(connectInfo);
		if (d.HasParseError())  
		{
			CCLOG("GetParseError %s",d.GetParseError());
		}

		if (d.IsObject() && d.HasMember("serverUdpPort")) {
			// ������������udp�˿ں�
			m_serverUdpPort = d["serverUdpPort"].GetInt();
			CCLOG("serverUdpPort:%d", m_serverUdpPort);
		}
		if (d.IsObject() && d.HasMember("udpPort")) {
			// ����������������ͻ��˵�udp�˿ں�
			m_udpPort = d["udpPort"].GetInt();
			CCLOG("udpPort:%d", m_udpPort);
		}
		if (d.IsObject() && d.HasMember("tankID")) {
			// ����������������ͻ��˵�̹��ID��
			m_tankID = d["tankID"].GetInt();
			CCLOG("tankID:%d", m_tankID);

			// ����̹��ID
			m_parent->getTank()->setID(m_tankID); 
		}

		// �ر�TCP����
		m_socket.Close(); 
	}
	else {
		CCLOG("can not connect to server!");
		return;
	}
	
	// ���������÷������Կ���udp�߳�

	m_udpSocket.Init();
	m_udpSocket.Create(AF_INET, SOCK_DGRAM, 0); // udp
	m_udpSocket.Bind(m_udpPort);	            // ����udp port
	m_udpSocket.Connect(ip, m_serverUdpPort);   // ָ��������IP��udp�˿�
	
	auto tankNewMsg = new TankNewMsg(this->m_parent, this);  // �ͻ������ӷ��ͻ�����Ϣ
	send(tankNewMsg);

	std::thread recvUdpThread = std::thread(&NetClient::UDPrecThread, this);
	recvUdpThread.detach(); // �����̷߳���
}

bool NetClient::init(GameClient* parent)
{
	if (!Node::init())
	{
		return false;
	}
	m_parent = parent;
	m_parent->addChild(this);
	return true;
}

NetClient* NetClient::create(GameClient* parent)
{
	NetClient* pRet = new(std::nothrow) NetClient();
	if (pRet && pRet->init(parent))
	{
		pRet->autorelease();
		return pRet;
	} else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}

void NetClient::UDPrecThread()
{
	while (true) {

		// �������ݰ�����
		char msgType[10] = "";
		int result = m_udpSocket.Recv(msgType, 10, 0);
		CCLOG("udp code for msg type :%d", result);
		
		// ������������ӶϿ���
		if (result <= 0) 
		{
			CCLOG("server stop!");
			break;
		}
		CCLOG("recv udp data msg type: %s", msgType);
		switch (atoi(msgType))
		{
		case 1: // Tank New Msg
			{
				// ��������̹�˵�¼��Ϣ
				auto newTankMsg = new TankNewMsg(m_parent, this);
				newTankMsg->parse(m_udpSocket);
			}
			break;
		case 2: // Tank Move Msg
			{
				// ��������̹���ƶ���Ϣ
				auto tankMoveMsg = new TankMoveMsg(m_parent);
				tankMoveMsg->parse(m_udpSocket);
			}
			break;
		case 3: // Tank Fire Msg
			{
				// ��������̹�˿�����Ϣ
				auto tankFireMsg = new FireMsg(m_parent);
				tankFireMsg->parse(m_udpSocket);
			}
		}
	}
}

void NetClient::send(Msg* msg)
{
	msg->send(m_udpSocket, m_serverIP.c_str(), m_serverUdpPort);
}
