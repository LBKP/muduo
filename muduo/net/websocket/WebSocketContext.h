#pragma once
#include <muduo/net/Buffer.h>
#include <muduo/net/websocket/WebSocketConnection.h>
#include <muduo/net/websocket/WebSocketTypes.h>
#include <any>
namespace muduo
{
namespace net
{
namespace wss
{
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

	WebSocketContext(WebSocketPtr &websocket);
	~WebSocketContext();
	bool manageHandshake(Buffer *buf, Timestamp reciveTime);
	string webSocketHandshakeAccept() const;
	WebSocketParseState getState() const { return state_; }
	WebSocketPtr &getWebSocketConnection() { return webSocketConn_; }

  private:
	bool processRequestLine(const char *begin, const char *end);
	bool processUpgradeLine(const char *begin, const char *end);
	bool processConnectionLine(const char *begin, const char *end);
	bool processOriginLine(const char *begin, const char *end);
	bool processWebSocketKeyLine(const char *begin, const char *end);
	bool processWebSocketVeisionLine(const char *begin, const char *end);

	WebSocketPtr webSocketConn_;
	WebSocketParseState state_;
	string strSecWebSocketKey_;
};
} // namespace wss
} // namespace net
} // namespace muduo
