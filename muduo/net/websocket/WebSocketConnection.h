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
	: noncopyable,
	  public std::enable_shared_from_this<WebSocketConnection>
{
  public:
		WebSocketConnection(EventLoop *loop,
												const string &name,
												int sockfd,
												const InetAddress &localAddr,
												const InetAddress &peerAddr,
												ssl::sslAttrivutesPtr sslAttr = ssl::sslAttrivutesPtr());
		virtual ~WebSocketConnection();
		virtual bool connected() const;
		virtual bool disconnected() const;

		// void send
		virtual void send(const void *message, int64_t len, Opcode frame);
		virtual void send(const StringPiece &message, Opcode frame);
		// void send buffer
		virtual void send(Buffer *message, Opcode frame);
		virtual size_t sendToChannel(const void *message, size_t len);
		// prease message
		bool preaseMessage(Buffer *buf, Timestamp receiveTime);

		void shutdown();
		void forceClose();
		void forceCloseWithDelay(double secondes);
		void setMessageCallBack(WebSocketMessageCallback cb)
		{
			onMessageCallback_ = cb;
	}

  private:
		virtual void handleRead(Timestamp receiveTime);
		void fetchFIN(Buffer *buf);
		bool fecthOpcode(Buffer *buf);
		void fetchMask(Buffer *buf);
		void fetchMaskingKey(Buffer *buf);
		bool fetchPayloadLength(Buffer *buf);
		void fetchPayload(Buffer *buf);
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
					: fin(false), opcode(CONTINUATION_FRAME), mask(false), payload(0),
						preaseDown(true)
			{
				memset(maskKey, 0, 4);
			}
			~WebSocketHeader() {}
	};

  private:
	Buffer recivedBuf_;

	//openssl optional
	ssl::sslAttrivutesPtr sslAttr_;
};
} // namespace wss
} // namespace muduo::net