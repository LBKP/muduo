#pragma once
#include <string>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/openssl.h>
#include <muduo/net/websocket/websocketcontext.h>


namespace wss
{
	using namespace std;
	using namespace muduo;
	using namespace muduo::net;
	class WebSocketServer
	{
	public:
		WebSocketServer(EventLoop* loop, const InetAddress & addr, const string& name, TcpServer::Option option = TcpServer::kNoReusePort, ssl::sslAttrivutesPtr sslAttr = ssl::sslAttrivutesPtr());
		~WebSocketServer();
		void start();
	private:
		//noncopyable
		WebSocketServer(const WebSocketServer&) = delete;
		WebSocketServer& operator=(const WebSocketServer&) = delete;

		//callback funcation
		void onConnection(const TcpConnectionPtr& connection);
		void onMessage(const TcpConnectionPtr& connection, Buffer* buf, Timestamp reciveTime);
		void onHandshake(const TcpConnectionPtr& connection, const WebSocketContext* context);


		TcpServer tcpServer_;


	};
}

