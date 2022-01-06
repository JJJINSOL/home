#pragma once
#include<windows.h>
#define PACKET_HEADER_SIZE 4
#define PACKET_CHAT_MSG 1000

//����Ʈ ��ŷ, ��Ʈ��ũ�� ���� ����ü ���۽� �ʼ�!
//����ü�� �޸𸮿� ���ǵǴ� ���´� OS�� �����Ϸ��� ���� �޶���
//������ ����ü�� ���� �ٸ��� �޸𸮿� �����ϰ� �ִ� �ý��۳��� 
//�޸𸮿� �ִ� ����ü ������ �״�� �ְ� �޴´ٸ� ����ü�� �� ����� ���� �ٸ����� ������ ��
#pragma pack(push, 1)
typedef struct
{
	//WORD = 2����Ʈ
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