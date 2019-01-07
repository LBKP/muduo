#pragma once
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <muduo/base/Types.h>
#include <muduo/net/Openssl.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/websocket/WebSocketTypes.h>

using std::shared_ptr;
namespace muduo
{
namespace net
{
namespace wss
{
class WebSocketServer : public TcpServer
{
public:
	WebSocketServer(EventLoop *loop,
					const InetAddress &addr,
					const string &name,
					TcpServer::Option option = TcpServer::kNoReusePort,
					ssl::sslAttrivutesPtr sslAttr = ssl::sslAttrivutesPtr());
	virtual ~WebSocketServer();
	void setOpcode(Opcode frame)
	{
		frame_ = frame;
	}

private:
	virtual TcpConnectionPtr createConnectiong(EventLoop* ioLoop, 
											   const string &nameArg,
											   int sockfd,
											   const InetAddress &localAddr,
											   const InetAddress &peerAddr);
	
	static void defaultOnMessageCallback(TcpConnectionPtr websocket, Buffer *buf, Timestamp receiveTime);
private:
	//openssl is open
	bool openSsl_;
	ssl::sslAttrivutesPtr sslAttributes_;
	Opcode frame_;
};
} // namespace wss
} // namespace net
} // namespace muduo
