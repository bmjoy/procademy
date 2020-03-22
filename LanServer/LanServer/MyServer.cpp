#include <Windows.h>
#include <map>
#include <stack>
#include "Packet.h"
#include "RingBuffer.h"
#include "Session.h"
#include "PacketPtr.h"
#include "LanServer.h"
#include "Player.h"
#include "MyServer.h"


CMyServer::CMyServer()
{
}

bool CMyServer::OnConnectionRequest(WCHAR *ClientIP, int Port)
{
	return true;
}

void CMyServer::OnRecv(DWORD sessionID, Packet *p)
{
	Echo(sessionID, p);
}

void CMyServer::OnSend(DWORD sessionID, int sendsize)
{
	return;
}
void CMyServer::OnClientJoin(DWORD sessionID)
{
	LanServerHeader header;
	Player *player = new Player(sessionID);

	

	Packet *p = new Packet;
	header.len = 8;
	p->PutData((char *)&header, sizeof(header));
	*p << 0x7fffffffffffffff;

	//�ڵ�ȭ �׽�Ʈ
	//PacketPtr *pPtr =new PacketPtr;
	//pPtr->GetPacket()->PutData((char *)&header, sizeof(header));
	//*pPtr << 0x7fffffffffffffff;
	//�ڵ�ȭ �׽�Ʈ

	AcquireSRWLockExclusive(&playerListLock);
	playerList.insert(std::make_pair(sessionID, player));
	SendPacket(sessionID, p);

	//�ڵ�ȭ �׽�Ʈ
	//AutoSendPacket(sessionID, pPtr);
	//�ڵ�ȭ �׽�Ʈ

	ReleaseSRWLockExclusive(&playerListLock);
	
}
void CMyServer::OnClientLeave(DWORD sessionID)
{
	AcquireSRWLockExclusive(&playerListLock);
	auto iter = playerList.find(sessionID);

	if (iter == playerList.end())
	{
		ReleaseSRWLockExclusive(&playerListLock);
		return;
	}
	Player *player = iter->second;
	playerList.erase(playerList.find(sessionID));
	player->Lock();
	ReleaseSRWLockExclusive(&playerListLock);
	player->UnLock();
	delete player;
}
void CMyServer::OnError(int errorcode, WCHAR *)
{}


bool CMyServer::Echo(LONGLONG sessionID, Packet *p)
{
	//echo�ϱ� ���� player�� �����Ǹ� echo�� ������ ����
	AcquireSRWLockExclusive(&playerListLock);
	auto iter = playerList.find(sessionID);

	if (iter == playerList.end())
	{
		ReleaseSRWLockExclusive(&playerListLock);
		return FALSE;
	}
	Player *player = iter->second;
	ReleaseSRWLockExclusive(&playerListLock);

	Packet *sendPacket = new Packet;
	LanServerHeader header;

	LONGLONG data;

	*p >> data;

	header.len = sizeof(data);
	sendPacket->PutData((char *)&header, sizeof(header));
	*sendPacket << data;

	SendPacket(sessionID, sendPacket);


	//�ڵ�ȭ �׽�Ʈ
	//PacketPtr *pPtr = new PacketPtr;
	//pPtr->GetPacket()->PutData((char *)&header, sizeof(header));
	//*pPtr << data;
	//
	//AutoSendPacket(sessionID, pPtr);
	//�ڵ�ȭ �׽�Ʈ

	return TRUE;
}