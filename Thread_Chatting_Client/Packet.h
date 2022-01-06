#pragma once
#include "Protocol.h"
#include <string>
using namespace std;

class Packet
{
public:
	UPACKET m_upacket;
	char* m_offset;
	void PutData(const char* data, int size);
	void GetData(const char* data, int size);

	Packet& operator << (int data);
	Packet& operator << (long data);
	Packet& operator << (short data);
	Packet& operator << (float data);
	Packet& operator << (char* data);
	Packet& operator << (string data);

	Packet& operator >> (int data);
	Packet& operator >> (long data);
	Packet& operator >> (short data);
	Packet& operator >> (float data);
	Packet& operator >> (char* data);
	Packet& operator >> (string data);



	Packet();
	Packet(const Packet& ipacket);
	Packet(WORD type);
	~Packet();
};

