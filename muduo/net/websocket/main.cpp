#include "WebSocketServer.h"
#include <muduo/net/EventLoop.h>
#include <muduo/net/Openssl.h>

using namespace muduo;
using namespace muduo::net;
int main(int argc, char* argv[])
{
	//int numThreads = 0;
	if (argc > 1)
	{
		//benchmark = true;
	}
	ssl::sslAttrivutesPtr ssl(new ssl::SslServerAttributes);
	ssl->certificatePath = "/home/ubuntu/SSLKey/Apache/2_www.laobiaokuaipao.xyz.crt";
	ssl->keyPath = "/home/ubuntu/SSLKey/Apache/3_www.laobiaokuaipao.xyz.key";
	ssl->certificateType = 1;
	ssl->keyType = 1;
	EventLoop loop;
	std::string name("wss");
	wss::WebSocketServer server(&loop, InetAddress(8000), name, TcpServer::kNoReusePort, ssl);
	server.start();
	loop.loop();
}
