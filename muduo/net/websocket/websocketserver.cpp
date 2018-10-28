#include "websocketserver.h"
#include "websocketcontext.h"

namespace wss
{

	WebSocketServer::WebSocketServer(EventLoop* loop, const InetAddress & addr, const string& name, TcpServer::Option option, ssl::sslAttrivutesPtr sslAttr)
		:tcpServer_(loop, addr, name, option, sslAttr)
	{
		tcpServer_.setConnectionCallback(bind(&WebSocketServer::onConnection, this, std::placeholders::_1));
		tcpServer_.setMessageCallback(bind(&WebSocketServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

	WebSocketServer::~WebSocketServer()
	{
	}
	void WebSocketServer::start()
	{
		LOG_INFO << tcpServer_.name() << "websocketserver started";
		tcpServer_.start();
	}

	void WebSocketServer::onConnection(const TcpConnectionPtr & connection)
	{
		if (connection->connected())
		{
			connection->setContext(WebSocketContext());
			LOG_INFO << "Tcp connection connected" << connection->getTcpInfoString();
		}
	}

	void WebSocketServer::onMessage(const TcpConnectionPtr & connection, Buffer * buf, Timestamp reciveTime)
	{
		WebSocketContext* context = std::any_cast<WebSocketContext>(connection->getMutableContext());
		if (context->getState() <= WebSocketContext::WebSocketParseState::kConnectionEstablished)
		{
			if (context->manageHandshake(buf, reciveTime))
			{
				onHandshake(connection, context);
			}
		}
		LOG_INFO << buf->retrieveAllAsString();
	}

	void WebSocketServer::onHandshake(const TcpConnectionPtr&  connection, const WebSocketContext* context)
	{
		Buffer buf;
		string key = context->webSocketHandshakeAccept();
		buf.append("HTTP/1.1 101 Switching Protocols\r\n"\
			"Upgrade: WebSocket\r\n"\
			"Sec-WebSocket-Version: 13\r\n"\
			"Connection: Upgrade\r\n"\
			"Sec-WebSocket-Accept: " + key + "\r\n"
		);
		connection->send(&buf);
	}
}
