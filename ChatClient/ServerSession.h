#pragma once

#include "pch.h"
#include "Service.h"
#include "Session.h"
#include "Protocol.pb.h"
#include "ServerPacketHandler.h"
#include "ChatClientDlg.h"

using ServerSessionRef = shared_ptr<class ServerSession>;
class CChatClientDlg;

class ServerSession : public PacketSession
{
public:
	ServerSession(CChatClientDlg* dig) : _dig(dig) { }
	~ServerSession()
	{
		cout << "~ServerSession" << endl;
	}
	virtual void OnConnected() override
	{
		_dig->_isConnected = true;
		_dig->updateServerStatus();
		_dig->_serverSession = static_pointer_cast<ServerSession>(shared_from_this());

		Protocol::C_LOGIN loginPkt;
		auto sendBuffer = ServerPacketHandler::MakeSendBuffer(loginPkt);
		Send(sendBuffer);
	}

	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
	{
		PacketSessionRef session = GetPacketSessionRef();
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

		ServerPacketHandler::HandlePacket(session, buffer, len);

	}

	virtual void	OnSend(int32 len) override
	{
		//cout << "OnSend Len = " << len << endl;
	}

	virtual void	OnDisconnected() override
	{
		//cout << "OnDisconnected" << endl;
	}

public:
	CChatClientDlg* _dig;
};

