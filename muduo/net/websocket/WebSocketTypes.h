#pragma once
#include <functional>
#include <memory>
#include <muduo/base/Timestamp.h>
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

struct WebSocketHeader 
{
  bool fin;
  int opcode;
  bool mask;
  uint64_t payload;
  // the lase packge is preasedown, begin prease new header
  bool preaseDown;
  char maskKey[4];
  WebSocketHeader()
      : fin(false), opcode(Opcode::CONTINUATION_FRAME), mask(false), payload(0),
        preaseDown(true) {
    memset(maskKey, 0, 4);
  }
  ~WebSocketHeader() {}
};
typedef std::shared_ptr<WebSocketConnection> WebSocketPtr;
typedef std::shared_ptr<WebSocketHeader> WebSocketHeaderPtr;
typedef std::function<void(WebSocketPtr, Buffer *buf, Timestamp receiveTime)> WebSocketMessageCallback;
}
}
}