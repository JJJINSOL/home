#include "NetUser.h"
void NetUser:: set(SOCKET sock, SOCKADDR_IN addr)
{
	m_connect == true;
	ZeroMemory(m_recvbuffer, sizeof(char) * 2048);
	m_packetpos = 0;
	m_writepos = 0;
	m_readpos = 0;

	m_sock = sock;
	m_addr = addr;
	m_name = inet_ntoa(addr.sin_addr);
	m_port = ntohs(addr.sin_port);
}
int NetUser:: DispatchRead(char* recvbuffer, int recvbyte)
{
	if (m_writepos + recvbyte >= 2048)
	{
		if (m_readpos > 0)
		{
			//memmove = �޸� �̵� �Լ� - ������, �����, ũ��
			memmove(&m_recvbuffer[0], &m_recvbuffer[m_packetpos], m_readpos);
		}
		m_packetpos = 0;
		m_writepos = m_readpos;
	}
	memcpy(&m_recvbuffer[m_writepos], recvbuffer, recvbyte);
	m_writepos += recvbyte;// ���ۿ� ������ ����� ��ġ
	m_readpos += recvbyte; // ��Ŷ���� ��ġ�κ��� ���� ����Ʈ

	if (m_readpos >= PACKET_HEADER_SIZE)
	{
		// ��Ŷ �ؼ� ����
		UPACKET* pPacket = (UPACKET*)&m_recvbuffer[m_packetpos];
		// ��� 1���� ��Ŷ�� �����ߴ�.
		if (pPacket->ph.len <= m_readpos)
		{
			do {
				Packet packet (pPacket->ph.type);
				memcpy(&packet.m_upacket, &m_recvbuffer[m_packetpos], pPacket->ph.len);
				m_packetpool.push_back(packet);

				// ������Ŷ ó��
				m_packetpos += pPacket->ph.len;
				m_readpos -= pPacket->ph.len;
				if (m_readpos < PACKET_HEADER_SIZE)
				{
					break;
				}
				pPacket = (UPACKET*)&m_recvbuffer[m_packetpos];
			} while (pPacket->ph.len <= m_readpos);
		}
	}
	return 1;
}