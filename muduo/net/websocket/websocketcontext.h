#pragma once
#include <muduo/base/Timestamp.h>
#include <muduo/net/Buffer.h>
#include <any>
namespace wss
{
	using namespace muduo;
	using namespace muduo::net;
	const string wssMgic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	class WebSocketContext
	{
	public:
		enum class WebSocketParseState
		{
			kOff,
			kExpectRequestLine,
			kExpectUpgradeLine,
			kExpectConnectionLine,
			kExpectOriginLine,
			kExpectSecWebSocketVersion,
			kExpectSecWebSocketKey,
			kConnectionEstablished,
			kGotAll,
		};
		
		WebSocketContext();
		~WebSocketContext();
		bool manageHandshake(Buffer* buf, Timestamp reciveTime);
		string webSocketHandshakeAccept() const; 
		WebSocketParseState getState()const { return state_; }
	private:
		bool processRequestLine(const char* begin, const char* end);
		bool processUpgradeLine(const char* begin, const char* end);
		bool processConnectionLine(const char* begin, const char* end);
		bool processOriginLine(const char* begin, const char* end);
		bool processWebSocketKeyLine(const char* begin, const char* end);
		bool processWebSocketVeisionLine(const char* begin, const char* end);
		
		WebSocketParseState state_;
		string strSecWebSocketKey_;
	};
}

