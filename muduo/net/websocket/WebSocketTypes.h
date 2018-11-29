#pragma once
#include <functional>
#include <memory>
#include <muduo/base/Timestamp.h>
#include <muduo/net/websocket/WebSocketConnection.h>
namespace muduo
{
namespace net
{
class Buffer;
namespace wss
{ 
const string wssMgic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
class WebSocketConnection;
enum class WebSocketState : char
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
//web socket packeg frame type
enum Opcode : char
{
	CONTINUATION_FRAME,
	TEXT_FRAME,
	BINARY_FRAME,
	CONNECTION_CLOSE_FRAME = 0X8,
	PING_FRAME,
	PONG_FRAME,
};
typedef std::shared_ptr<WebSocketConnection> WebSocketPtr;
typedef std::function<void(WebSocketPtr, Buffer *buf, Timestamp receiveTime)> WebSocketMessageCallback;
}
}
}