#pragma once
#include <memory>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/websocket/WebSocketTypes.h>

namespace muduo::net
{
class Buffer;
namespace wss
{

class WebSocketConnection : public TcpConnection
{
public:
	WebSocketConnection(EventLoop* loop,
		const string& name,
		int sockfd,
		const InetAddress& localAddr,
		const InetAddress& peerAddr,
		ssl::sslAttrivutesPtr sslAttr = ssl::sslAttrivutesPtr());
	virtual ~WebSocketConnection();
	virtual bool connected() const;
	virtual bool disconnected() const;

	// void send
	virtual void send(const void* message, int len);
	virtual void send(const StringPiece& message);
	// void send buffer
	virtual void send(Buffer* message);
	virtual size_t sendToChannel(const void* message, size_t len);
	// prease message
	bool preaseMessage(Buffer* buf, Timestamp receiveTime);

	void setOpcode(Opcode frame)
	{
		frame_ = frame;
	}
	void shutdown();
	void forceClose();
	void forceCloseWithDelay(double secondes);

private:
	virtual void handleRead(Timestamp receiveTime);
	// called when TcpServer accepts a new connection
	virtual void connectEstablished(); // should be called only once
	// called when TcpServer has removed me from its map
	virtual void connectDestroyed(); // should be called only once

	//handshake
	bool manageHandshake(Timestamp reciveTime);
	string webSocketHandshakeAccept() const;
	bool processRequestLine(const char *begin, const char *end);
	bool processUpgradeLine(const char *begin, const char *end);
	bool processConnectionLine(const char *begin, const char *end);
	bool processOriginLine(const char *begin, const char *end);
	bool processWebSocketKeyLine(const char *begin, const char *end);
	bool processWebSocketVeisionLine(const char *begin, const char *end);

	//prease message
	void fetchFIN(Buffer* buf);
	bool fecthOpcode(Buffer* buf);
	void fetchMask(Buffer* buf);
	void fetchMaskingKey(Buffer* buf);
	bool fetchPayloadLength(Buffer* buf);
	void fetchPayload(Buffer* buf);


private:
	Buffer recivedBuf_;
	Opcode frame_;
	WebSocketHeaderPtr receiveHeader_;

	WebSocketState state_;
	string strSecWebSocketKey_;
	// openssl optional
	ssl::sslAttrivutesPtr sslAttr_;
};
} // namespace wss
} // namespace muduo::net