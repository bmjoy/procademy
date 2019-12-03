#pragma comment(lib,"ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include "network.h"


#include "Game.h"

//#include "PacketDefine.h"

#define PORT 5000
#define BUF_SIZE 1024
#define IP "127.0.0.1"

struct Session
{
	SOCKET socket;
	RingBuffer *RecvQ;
	RingBuffer *SendQ;
};

Session g_session;
BOOL g_bSend=false;

bool networkInit(HWND hWnd,int WM_SOCKET)
{
	g_session.RecvQ = new RingBuffer(BUF_SIZE);
	g_session.SendQ = new RingBuffer(BUF_SIZE);
	int ret;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	g_session.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ret = WSAAsyncSelect(g_session.socket, hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);

	if (ret == SOCKET_ERROR)
	{
		OutputDebugString(L"AsyncSelect Error");
		return false;
	}

	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, IP, &addr.sin_addr.s_addr);
	addr.sin_port = htons(PORT);

	ret = connect(g_session.socket, (SOCKADDR *)&addr, sizeof(addr));

	if (ret == SOCKET_ERROR && GetLastError() == !WSAEWOULDBLOCK)
	{
		return false;
	}

	return true;
}

void ProcessSocketMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_CLOSE:
		exit(1);
		break;
	case FD_READ:
		//system("pause");
		//OutputDebugString(L"FD_READ");
		ProcRead(hWnd);
		break;
	case FD_WRITE:
		g_bSend = true;
		ProcWrite();
		//system("pause");
		//OutputDebugString(L"FD_WRITE");
		break;
	default:
		break;
	}
}

void ProcRead(HWND hWnd)
{
	char buf[1024];
	char payLoad[100];
	int enq;
	int size;
	int recv_size;
	BYTE end;
	st_NETWORK_PACKET_HEADER header;

	recv_size = min(g_session.RecvQ->GetFreeSize(), sizeof(buf));

	size = recv(g_session.socket, buf, recv_size, 0);

	if (size == SOCKET_ERROR)
	{
		//int temp = 0;
		//system("pause");
		exit(-1);
	}

	enq = g_session.RecvQ->Enqueue(buf, size);

	if (enq != size)
	{
		int temp = 0;
		//system("pause");
		exit(-1);
	}

	while (1)
	{
		if (sizeof(header) > g_session.RecvQ->GetUseSize())
			break;

		if (g_session.RecvQ->Peek((char *)&header, sizeof(header)) < sizeof(header))
			break;
		if (header.bySize + sizeof(header)+1 > g_session.RecvQ->GetUseSize())
			break;

		g_session.RecvQ->MoveFront(sizeof(header));

		if (g_session.RecvQ->Dequeue(payLoad, header.bySize) != header.bySize)
		{
			exit(-1);
		}

		
		PacketProc(header.byType, payLoad);

		g_session.RecvQ->Dequeue((char *)&end, sizeof(BYTE));

		if (end != dfNETWORK_PACKET_END)
			exit(-1);

		//메시지처리
	}

	//ProcWrite();
}

void PacketProc(BYTE byPacketType, char *Packet)
{
	switch (byPacketType)
	{
	case dfPACKET_SC_CREATE_MY_CHARACTER:
	{
		DWORD id = ((stPACKET_SC_CREATE_MY_CHARACTER *)Packet)->ID;
		BYTE dir = ((stPACKET_SC_CREATE_MY_CHARACTER *)Packet)->Direction;
		WORD x = ((stPACKET_SC_CREATE_MY_CHARACTER *)Packet)->X;
		WORD y = ((stPACKET_SC_CREATE_MY_CHARACTER *)Packet)->Y;
		BYTE hp = ((stPACKET_SC_CREATE_MY_CHARACTER *)Packet)->HP;
		CreatePlayer(id, dir, x, y, hp, true);
	}
		break;
	case dfPACKET_SC_CREATE_OTHER_CHARACTER:
	{
		DWORD id = ((stPACKET_SC_CREATE_MY_CHARACTER *)Packet)->ID;
		BYTE dir = ((stPACKET_SC_CREATE_MY_CHARACTER *)Packet)->Direction;
		WORD x = ((stPACKET_SC_CREATE_MY_CHARACTER *)Packet)->X;
		WORD y = ((stPACKET_SC_CREATE_MY_CHARACTER *)Packet)->Y;
		BYTE hp = ((stPACKET_SC_CREATE_MY_CHARACTER *)Packet)->HP;
		CreatePlayer(id, dir, x, y, hp, false);
	}
		break;
	case dfPACKET_SC_DELETE_CHARACTER:
		DeletePlayer(((stPACKET_SC_DELETE_CHARACTER *)Packet)->ID);
		break;
	case dfPACKET_SC_MOVE_START:
	{
		DWORD ID = ((stPACKET_SC_MOVE_START *)Packet)->ID;
		BYTE Direction = ((stPACKET_SC_MOVE_START *)Packet)->Direction;
		WORD X = ((stPACKET_SC_MOVE_START *)Packet)->X;
		WORD Y = ((stPACKET_SC_MOVE_START *)Packet)->Y;
		MovePlayer(ID, Direction, X, Y);
	}
		break;
	case dfPACKET_SC_MOVE_STOP:
	{
		DWORD ID = ((stPACKET_SC_MOVE_STOP *)Packet)->ID;
		BYTE Direction = ((stPACKET_SC_MOVE_STOP *)Packet)->Direction;
		WORD X = ((stPACKET_SC_MOVE_STOP *)Packet)->X;
		WORD Y = ((stPACKET_SC_MOVE_STOP *)Packet)->Y;
		StopPlayer(ID, Direction, X, Y);
	}
		break;
	case dfPACKET_SC_ATTACK1:
	{
		DWORD ID = ((stPACKET_SC_ATTACK *)Packet)->ID;
		BYTE Direction = ((stPACKET_SC_ATTACK *)Packet)->Direction;
		WORD X = ((stPACKET_SC_ATTACK *)Packet)->X;
		WORD Y = ((stPACKET_SC_ATTACK *)Packet)->Y;
		Attack1Player(ID, Direction, X, Y);
	}
		break;
	case dfPACKET_SC_ATTACK2:
	{
		DWORD ID = ((stPACKET_SC_ATTACK *)Packet)->ID;
		BYTE Direction = ((stPACKET_SC_ATTACK *)Packet)->Direction;
		WORD X = ((stPACKET_SC_ATTACK *)Packet)->X;
		WORD Y = ((stPACKET_SC_ATTACK *)Packet)->Y;
		Attack2Player(ID, Direction, X, Y);
	}
		break;
	case dfPACKET_SC_ATTACK3:
	{
		DWORD ID = ((stPACKET_SC_ATTACK *)Packet)->ID;
		BYTE Direction = ((stPACKET_SC_ATTACK *)Packet)->Direction;
		WORD X = ((stPACKET_SC_ATTACK *)Packet)->X;
		WORD Y = ((stPACKET_SC_ATTACK *)Packet)->Y;
		Attack3Player(ID, Direction, X, Y);
	}
		break;
	case dfPACKET_SC_DAMAGE:
	{
		DWORD AttackID = ((stPACKET_SC_DAMAGE *)Packet)->AttackID;
		DWORD DamageID = ((stPACKET_SC_DAMAGE *)Packet)->DamageID;
		WORD DamageHP = ((stPACKET_SC_DAMAGE *)Packet)->DamageHP;
		DamagePlayer(AttackID, DamageID, DamageHP);
	}
		break;
	default:
		break;
	}
}

void SendPacket(st_NETWORK_PACKET_HEADER *pHeader, char *pPacket)
{ 
	char endCode = dfNETWORK_PACKET_END;
	if (g_session.SendQ->GetFreeSize() < sizeof(st_NETWORK_PACKET_HEADER) + pHeader->bySize + 1)
		exit(-1);
	g_session.SendQ->Enqueue((char *)pHeader, sizeof(st_NETWORK_PACKET_HEADER));
	g_session.SendQ->Enqueue(pPacket, pHeader->bySize);
	g_session.SendQ->Enqueue(&endCode, 1);
	ProcWrite();
}

void ProcWrite()
{
	if (!g_bSend)
	{
		return;
	}
	char buf[1024];
	int size = g_session.SendQ->Peek(buf, 1024);

	int send_size = send(g_session.socket, buf, size, 0);

	if (send_size == SOCKET_ERROR && GetLastError() != WSAEWOULDBLOCK)
	{
		g_bSend = false;
		return;
	}

	g_session.SendQ->MoveFront(send_size);

	return;
}