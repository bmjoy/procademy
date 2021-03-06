#pragma once

extern SRWLOCK sessionListLock;

enum PROCRESULT { SUCCESS = 0, NONE, FAIL };

BOOL initNetwork();
PROCRESULT CompleteRecvPacket(Session *session);
bool PacketProc(Session *session, BYTE type, Packet &p);
bool SendUnicast(Session* session, Packet& p);
bool SendPacket(LONGLONG sessionID, Packet& p);