#pragma once
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Openssl.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/websocket/WebSocketContext.h>
#include <muduo/net/websocket/WebSocketTypes.h>
#include <string>

namespace muduo
{
namespace net
{
namespace wss
{
using namespace std;
class WebSocketServer
{
public:
	WebSocketServer(EventLoop *loop, const InetAddress &addr, const string &name,
									TcpServer::Option option = TcpServer::kNoReusePort,
									ssl::sslAttrivutesPtr sslAttr = ssl::sslAttrivutesPtr());
	~WebSocketServer();
	void start();
	void setOnMessageCallBack(WebSocketMessageCallback callback)
	{
		onMessageCallback_ = callback;
	}

private:
	// noncopyable
	WebSocketServer(const WebSocketServer &) = delete;
	WebSocketServer &operator=(const WebSocketServer &) = delete;

	// callback funcation
	void onConnection(const TcpConnectionPtr &connection);
	void onMessage(const TcpConnectionPtr &connection, Buffer *buf,
								 Timestamp reciveTime);
	void onHandshake(const TcpConnectionPtr &connection,
									 const WebSocketContext *context);
	static void defaultOnMessageCallback(WebSocketPtr websocket, Buffer *buf, Timestamp receiveTime);

	TcpServer tcpServer_;
	WebSocketMessageCallback onMessageCallback_;
};
} // namespace wss
} // namespace net
} // namespace muduo
