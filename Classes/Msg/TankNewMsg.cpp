#include "TankNewMsg.h"

TankNewMsg::TankNewMsg(GameClient* client, NetClient* netclient)
{
	m_client = client;
	m_netclient = netclient;
}

TankNewMsg::~TankNewMsg()
{

}

void TankNewMsg::send(ODSocket socket, const char* IP, int serverUdpPort)
{
	socket.Send("1", strlen("1"), 0); // �ȷ������ݰ�����, 1:̹�˵�¼��Ϣ

	rapidjson::Document document;
	document.SetObject();
	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
	rapidjson::Value array(rapidjson::kArrayType);
	rapidjson::Value object(rapidjson::kObjectType);
	object.AddMember("tankID", m_client->getTank()->getID(), allocator);
	object.AddMember("tankKind", m_client->getTank()->getKind(), allocator);
	object.AddMember("tank_X", (double)m_client->getTank()->getPositionX(), allocator);
	object.AddMember("tank_Y", (double)m_client->getTank()->getPositionY(), allocator);
	object.AddMember("tank_Dir", m_client->getTank()->getDirection(), allocator);
	array.PushBack(object, allocator);

	document.AddMember("Version", "v1.0", allocator);
	document.AddMember("Data", array, allocator);

	StringBuffer buffer;
	rapidjson::Writer<StringBuffer> writer(buffer);
	document.Accept(writer);

	CCLOG("sending msg:%s",buffer.GetString());

	std::string msg = buffer.GetString();  
	socket.Send(msg.c_str(), strlen(msg.c_str()), 0);  // �������ݰ�
}

void TankNewMsg::parse(ODSocket socket)
{
	// �������� Recv
	char data[1024] = "";
	int result = socket.Recv(data, 1024, 0);

	CCLOG("UDP rec Code : %d", result);

	// ������������ӶϿ���
	if (result <= 0) 
	{
		CCLOG("Disconnected from the server!");
		return ;
	}
	CCLOG("Recv udp data : %s", data);

	rapidjson::Document d;
	d.Parse<0>(data);
	if (d.HasParseError())  
	{
		CCLOG("GetParseError %s",d.GetParseError());
	}

	if (d.IsObject() && d.HasMember("Version")) {
		CCLOG("Version:%s", d["Version"].GetString());
	}
	if (d.IsObject() && d.HasMember("Data"))
	{
		CCLOG("Data:");

		rapidjson::Value& _array = d["Data"];
		for(int i=0; i<_array.Capacity(); i++)
		{
			int tank_id = 0;
			int tank_kind = 0;
			double tank_x = 0.0;
			double tank_y = 0.0;
			int tank_dir = 0;

			rapidjson::Value& arraydoc = _array[i];
			if(arraydoc.HasMember("tankID"))
			{
				tank_id = arraydoc["tankID"].GetInt();
				CCLOG("tankID:%d", tank_id);
				if (m_client->getTank()->getID() == tank_id)
				{
					CCLOG("rec self data , return!");   // �յ��Լ�ת�������ݣ���������
					return;
				}
			}

			if (arraydoc.HasMember("tankKind"))
			{
				tank_kind = arraydoc["tankKind"].GetInt();
				CCLOG("tankKind:%d",tank_kind);
			}
			if(arraydoc.HasMember("tank_X"))
			{
				tank_x = arraydoc["tank_X"].GetDouble();
				CCLOG("tank_X:%lf", tank_x);
			}
			if(arraydoc.HasMember("tank_Y"))
			{
				tank_y = arraydoc["tank_Y"].GetDouble();
				CCLOG("tank_Y:%lf", tank_y);
			}
			if(arraydoc.HasMember("tank_Dir"))
			{
				tank_dir = arraydoc["tank_Dir"].GetInt();
				CCLOG("tank_Dir:%d", tank_dir);
			}
			// ����̹��
			bool exist = false;
			for (int i = 0; i < m_client->getTankList().size(); i ++)
			{
				auto tank = m_client->getTankList().at(i);
				if (tank->getID() == tank_id)
				{
					exist = true;
					break;
				}
			}
			if (!exist)
			{
				// �����Լ���Ϣ������̹�� ʹ�Է��ͻ�����ʾ�Լ�
				auto tnMsg = new TankNewMsg(m_client, m_netclient);
				m_netclient->send(tnMsg);

				// �Լ��ͻ��˴������½�� ʹ�Լ��ͻ�����ʾ�Է�
				m_client->addTank(tank_id, tank_x, tank_y, tank_dir, tank_kind);
			}
		}
	}
}

