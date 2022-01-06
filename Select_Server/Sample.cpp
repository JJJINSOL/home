#include "NetWork.h"
list<NetUser> userlist;
int SendMsg(SOCKET sock, char* msg, WORD type)
{
	//1. ��Ŷ ����
	UPACKET packet;
	//��Ŷ ����ü �޸� ����
	ZeroMemory(&packet, sizeof(packet));
	packet.ph.len = strlen(msg) + PACKET_HEADER_SIZE;
	packet.ph.type = type;
	memcpy(packet.msg, msg, strlen(msg));
	//2. ��Ŷ ����
	//������ ������ ������ ���� �ٷ� ������ ���� �ƴ϶�, �ü�� ���ۿ� ���ĺ���
	//�ü�� sendbuffer/recvbuffer �� ���� ������ �ޱ� �ȵ�
	char* pmsg = (char*)&packet;
	int sendsize = 0;
	do
	{
		int sendbyte = send(sock, &pmsg[sendsize], packet.ph.len - sendsize, 0);
		if (sendbyte == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				//���������� ����
				return -1;
			}
		}
		sendsize += sendbyte;
	} while (sendsize < packet.ph.len);
	return sendsize;
}
int SendMsg(SOCKET sock, UPACKET& packet)
{
	char* msg = (char*)&packet;
	int sendsize = 0;
	do
	{
		int sendbyte = send(sock, &msg[sendsize], packet.ph.len- sendsize, 0);
		if (sendbyte == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				//���������� ����
				return -1;
			}
		}
		sendsize += sendbyte;
	} while (sendsize < packet.ph.len);
	return sendsize;
}
int AddUser(SOCKET sock)
{
	SOCKADDR_IN clientaddr;
	int len = sizeof(clientaddr);
	SOCKET clientsock = accept(sock, (sockaddr*)&clientaddr, &len);
	if (clientsock == SOCKET_ERROR)
	{
		return -1;
	}
	else
	{
		NetUser user;
		user.set(clientsock, clientaddr);
		userlist.push_back(user);

		cout << "ip = " << inet_ntoa(clientaddr.sin_addr)
			<< " port = " << ntohs(clientaddr.sin_port)
			<< "  " << endl;

		cout << userlist.size() << "�� ������!" << endl;
	}
	return 1;
}
int RecvUser(NetUser& user)
{
	char recvbuffer[1024] = { 0, };
	int recvbyte = recv(user.m_sock, recvbuffer, 1024, 0);
	if (recvbyte == 0)
	{
		return 0;
	}
	if (recvbyte == SOCKET_ERROR)
	{
		return -1;
	}
	user.DispatchRead(recvbuffer, recvbyte);
	return 1;
}
void main()
{
	NetWork nw;
	nw.Initnetwork();
	nw.Initserver(SOCK_STREAM, 10000,nullptr);
	cout << "���� ������~" << endl;
	//select�� ����ϸ� �ͺ��ŷ�� �ƴϾ ��

	FD_SET rSet; // �б� ��
	FD_SET wSet; // ���� ��

	timeval timeout; // Ÿ�Ӿƿ� ��
	timeout.tv_sec = 0; // ��
	timeout.tv_usec = 1; // ����ũ�� ��

	while (1)
	{
		//1.���� �� ����(�ʱ�ȭ)
		//2.���� �¿� ������ ����. �¿� ���� �� �ִ� �ִ� ���� = 64
		//3.select() �Լ� ȣ��
		//4.select() �Լ� ���� ��, ���� �¿� ���� �ִ� ��� ���Ͽ� ���� ������ ���� �Լ��� ȣ�� ó��
		//1-4 �ݺ� 

		//FD_ZERO =  �ʱ�ȭ
		FD_ZERO(&rSet);
		FD_ZERO(&wSet);

		FD_SET(nw.m_listensock, &rSet);

		list<NetUser>::iterator userIter;

		for (userIter = userlist.begin(); userIter != userlist.end();)
		{
			if ((*userIter).m_connect == false)
			{
				cout << (*userIter).m_name << " ���� ����Ǿ���" << endl;
				userIter = userlist.erase(userIter);
				continue;
			}
			FD_SET((*userIter).m_sock, &rSet);
			if ((*userIter).m_packetpool.size() > 0)
			{
				FD_SET((*userIter).m_sock, &wSet);
			}
			userIter++;
		}

		int ret = select(0, &rSet, &wSet, NULL, &timeout);
		if (ret == 0)
		{
			continue;
		}
		if (FD_ISSET(nw.m_listensock, &rSet))
		{
			if (AddUser(nw.m_listensock) <= 0)
			{
				break;
			}
		}
		for (NetUser& user : userlist)
		{
			if (FD_ISSET(user.m_sock, &rSet))
			{
				if (RecvUser(user) <= 0)
				{
					user.m_connect = false;
				}
			}
		}
		for (NetUser& user : userlist)
		{
			if (FD_ISSET(user.m_sock, &wSet))
			{
				if (user.m_packetpool.size() > 0)
				{
					list<Packet>::iterator iter;
					for (iter = user.m_packetpool.begin(); iter != user.m_packetpool.end();)
					{
						for (NetUser& senduser : userlist)
						{
							int ret = SendMsg(senduser.m_sock, (*iter).m_upacket);
							if (ret <= 0)
							{
								senduser.m_connect = false;
							}
						}
						iter = user.m_packetpool.erase(iter);
					}
				}
			}
		}
				
	}
		nw.Closenetwork();
}