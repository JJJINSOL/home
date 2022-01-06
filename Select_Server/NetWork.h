#pragma once
#include "NetUser.h"
class NetWork
{
public:
	SOCKET m_listensock;

	bool Initnetwork();
	bool Initserver(int protocal, int port, const char* ip);
	bool Closenetwork();
};

