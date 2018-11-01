#pragma once
#include <memory>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/websocket/WebSocketTypes.h>

namespace muduo::net
{
class Buffer;
namespace wss
{
class WebSocketConnection
	: noncopyable,
	  public std::enable_shared_from_this<WebSocketConnection>
{
  public:
	WebSocketConnection(const TcpConnectionPtr &conn);
	~WebSocketConnection();
	bool connected() const;
	bool disconnected() const;

	// void send
	void send(const void *message, int64_t len, Opcode frame);
	void send(const StringPiece &message, Opcode frame);
	// void send buffer
	void send(Buffer *message, Opcode frame);

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
			  preaseDown(true) {}
			memset(maskKey, 0, 4);
		~WebSocketHeader() {}
	};

  private:
	std::weak_ptr<TcpConnection> connection_;
	Buffer recivedBuf_;
	WebSocketMessageCallback onMessageCallback_;
	WebSocketHeader receiveHeader_;
};
} // namespace wss
} // namespace muduo::net