#pragma once
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <muduo/base/Types.h>
#include <muduo/net/Openssl.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/websocket/WebSocketContext.h>
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

private:
  	/// Not thread safe, but in loop
 	virtual void newConnection(int sockfd, const InetAddress &peerAddr);

	virtual TcpConnectionPtr createConnectiong(const string &nameArg,
											   int sockfd,
											   const InetAddress &localAddr,
											   const InetAddress &peerAddr);
	// callback funcation
  	void onMessage(const TcpConnectionPtr &connection, Buffer *buf,
				 Timestamp reciveTime);
  	void onHandshake(const TcpConnectionPtr &connection,
				   const WebSocketContext *context);
  	static void defaultOnMessageCallback(WebSocketPtr websocket, Buffer *buf, Timestamp receiveTime);
private:
  	//openssl is open
  	bool openSsl_;
  	ssl::sslAttrivutesPtr sslAttributes_;
};
} // namespace wss
} // namespace net
} // namespace muduo
