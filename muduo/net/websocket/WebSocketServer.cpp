#include <muduo/net/websocket/WebSocketServer.h>
#include <muduo/net/websocket/WebSocketConnection.h>

namespace muduo
{
namespace net
{
namespace wss
{

WebSocketServer::WebSocketServer(EventLoop *loop, const InetAddress &addr,
								 const string &name, TcpServer::Option option,
								 ssl::sslAttrivutesPtr sslAttr)
	: TcpServer(loop, addr, name, option),
	sslAttributes_(sslAttr),
	frame_(Opcode::TEXT_FRAME)
{
	setMessageCallback(WebSocketServer::defaultOnMessageCallback);
	if(sslAttributes_)
	{
		ssl::initSslServer(sslAttributes_);
	}
}

WebSocketServer::~WebSocketServer()
{
}

TcpConnectionPtr
WebSocketServer::createConnectiong(EventLoop *ioLoop, 
								   const string &nameArg,
                                   int sockfd, 
								   const InetAddress &localAddr,
                                   const InetAddress &peerAddr) 
{
  LOG_INFO << "WebSocketServer::newConnection [" << name_
           << "] - new connection [" << nameArg << "] from "
           << peerAddr.toIpPort();
  TcpConnectionPtr conn(new WebSocketConnection(
	  ioLoop, nameArg, sockfd, localAddr, peerAddr, sslAttributes_));
  WebSocketPtr webConn = std::dynamic_pointer_cast<WebSocketConnection>(conn);
  webConn->setOpcode(frame_);
  return conn;
}

void WebSocketServer::defaultOnMessageCallback(TcpConnectionPtr websocket,
											   Buffer *buf,
											   Timestamp receiveTime)
{
	buf->appendInt8(0);
	LOG_INFO << "recived message " << buf->peek();
	websocket->send(buf);
}
} // namespace wss
} // namespace net
} // namespace muduo
