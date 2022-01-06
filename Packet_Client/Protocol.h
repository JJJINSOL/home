#pragma once
#include<windows.h>
#define PACKET_HEADER_SIZE 4
#define PACKET_CHAT_MSG 1000

//바이트 패킹, 네트워크를 통한 구조체 전송시 필수!
//구조체가 메모리에 정의되는 형태는 OS와 컴파일러에 따라 달라짐
//동일한 구조체를 서로 다르게 메모리에 정의하고 있는 시스템끼리 
//메모리에 있는 구조체 내용을 그대로 주고 받는다면 구조체의 각 멤버는 서로 다른값을 가지게 됨
#pragma pack(push, 1)
typedef struct
{
	//WORD = 2바이트
	WORD len;
	WORD type;
}PACKET_HEADER;

typedef struct
{
	PACKET_HEADER ph;
	char msg[4096];
}UPACKET, *P_UPACKET;

struct ChatMsg
{
	int index;
	char name[20];
	int damage;
	char message[256];
};
#pragma pack(pop)