#include <muduo/net/websocket/WebSocketConnection.h>
#include <muduo/net/Endian.h>
#include <muduo/net/Channel.h>
#include <muduo/net/websocket/WebSocketContext.h>
#include <muduo/net/EventLoop.h>

namespace muduo::net
{
namespace wss
{
WebSocketConnection::WebSocketConnection(EventLoop *loop,
										 const string &name,
										 int sockfd,
										 const InetAddress &localAddr,
										 const InetAddress &peerAddr,
										 ssl::sslAttrivutesPtr sslAttr)
	:TcpConnection(loop, name, sockfd, localAddr, peerAddr, sslAttr->ctx),
	sslAttr_(sslAttr)
{}
WebSocketConnection::~WebSocketConnection()
{
	LOG_DEBUG << "WebSocketConnection::dtor[" << name_ << "] at " << this
		<< " fd=" << channel_->fd();
}

bool WebSocketConnection::connected() const
{
	return std::any_cast<WebSocketContext>(context_).getState() >= WebSocketContext::WebSocketParseState::kConnectionEstablished;
}

bool WebSocketConnection::disconnected() const
{
	return std::any_cast<WebSocketContext>(context_).getState() < WebSocketContext::WebSocketParseState::kConnectionEstablished;
}

void WebSocketConnection::send(const void *data, int len)
{
	uint8_t payloadExternBytes, payload;
	if(len > 32767)
	{
		payloadExternBytes = 8;
		payload = 127;
	}
	else if(len >= 126)
	{
		payloadExternBytes = 2;
		payload = 126;
	}
	else
	{
		payloadExternBytes = 0;
		payload = static_cast<uint8_t>(len);
	}

	Buffer buf;
	buf.appendInt8(static_cast<int8_t>(0X80 | frame_));
	buf.appendInt8(payload);
	if(payloadExternBytes == 2)
	{
		buf.appendInt16(/*sockets::hostToNetwork16(*/ static_cast<uint16_t>(len) /*)*/);
	}
	else if(payloadExternBytes == 8)
	{
		buf.appendInt64(len);
	}
	buf.append(data, len);
	LOG_DEBUG << buf.peek();
	TcpConnection::send(&buf);
}

void WebSocketConnection::send(const StringPiece &message)
{
	send(message.data(), static_cast<int>(message.length()));
}

void WebSocketConnection::send(Buffer *message)
{
	send(message->peek(), static_cast<int>(message->readableBytes()));
}

size_t WebSocketConnection::sendToChannel(const void *message, size_t len)
{
	return ssl::sslSend(channel_->ssl(), message, static_cast<int>(len));
}

void WebSocketConnection::handleRead(Timestamp receiveTime)
{
	loop_->assertInLoopThread();
	int savedErrno = 0;
	LOG_DEBUG << sslAttr_.get();
	if(channel_->getSslAccpeted())
	{
		ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno, channel_->ssl());

		if(n > 0)
		{
			WebSocketContext context = std::any_cast<WebSocketContext>(context_);
			if(context.getState() < WebSocketContext::WebSocketParseState::kConnectionEstablished)
			{
				if(context.manageHandshake(&inputBuffer_, receiveTime))
				{
					string key = context.webSocketHandshakeAccept();
					outputBuffer_.append("HTTP/1.1 101 Switching Protocols\r\n"
										 "Upgrade: WebSocket\r\n"
										 "Sec-WebSocket-Version: 13\r\n"
										 "Connection: Upgrade\r\n"
										 "Sec-WebSocket-Accept: " +
										 key + "\r\n");
					if(!channel_->isWriting())
					{
						channel_->enableWriting();
					}
					inputBuffer_.retrieveAll();
					connectionCallback_(shared_from_this());
				}
			}
			else
			{
				preaseMessage(&inputBuffer_, receiveTime);
			}
		}
		else if(n == 0)
		{
			handleClose();
		}
		else
		{
			errno = savedErrno;
			LOG_SYSERR << "TcpConnection::handleRead";
			handleError();
		}
	}
	else
	{
		if(ssl::sslAccept(channel_->ssl()) != 1)
		{
			if(SSL_get_error(channel_->ssl(), 0) != SSL_ERROR_WANT_READ)
			{
				shutdown();
				LOG_ERROR << channel_->fd() << " open ssl error";
			}
			// next readable try to handshape
			LOG_ERROR << channel_->fd() << "openssl error is SSL_ERROR_WANT_READ ";
		}
		else
		{
			channel_->setSslAccpeted(true);
			LOG_INFO << channel_->fd() << " open ssl channeled";
		}
	}
}

void WebSocketConnection::connectEstablished()
{
	loop_->assertInLoopThread();
	assert(state_ == kConnecting);
	setState(kConnected);
	channel_->tie(shared_from_this());
	channel_->enableReading();
}

bool WebSocketConnection::preaseMessage(Buffer *buf, Timestamp receiveTime)
{
	if(receiveHeader_.preaseDown && buf->readableBytes() < 2)
		return false;
	if(receiveHeader_.preaseDown)
	{
		fetchFIN(buf);
		if(!fecthOpcode(buf))
			return false;
		fetchMask(buf);
		if(!fetchPayloadLength(buf))
			return false;
		receiveHeader_.preaseDown = false;
	}
	LOG_INFO << "fin " << receiveHeader_.fin << " opcode " << receiveHeader_.opcode
		<< " maske " << receiveHeader_.mask << " payload "
		<< receiveHeader_.payload << "Key" << receiveHeader_.maskKey;
	fetchPayload(buf);
	LOG_INFO << "fin " << receiveHeader_.fin << " opcode " << receiveHeader_.opcode
		<< " maske " << receiveHeader_.mask << " payload "
		<< receiveHeader_.payload << "Key" << receiveHeader_.maskKey;

	if(receiveHeader_.preaseDown && receiveHeader_.fin)
		messageCallback_(shared_from_this(), &recivedBuf_, receiveTime);
	if(receiveHeader_.preaseDown)
		memset(receiveHeader_.maskKey, 0, 4);
	return receiveHeader_.preaseDown;
}

void WebSocketConnection::shutdown()
{
	TcpConnection::shutdown();
}

void WebSocketConnection::forceClose()
{
	TcpConnection::forceClose();
}
void WebSocketConnection::forceCloseWithDelay(double secondes)
{
	TcpConnection::forceCloseWithDelay(secondes);
}

void WebSocketConnection::connectDestroyed()
{
	TcpConnection::connectDestroyed();
}

void WebSocketConnection::fetchFIN(Buffer *buf)
{
	const char *data = buf->peek();
	receiveHeader_.fin = data[0] & 0x80;
}

bool WebSocketConnection::fecthOpcode(Buffer *buf)
{
	receiveHeader_.opcode = *(buf->peek()) & 0x0F;
	if(receiveHeader_.opcode != Opcode::TEXT_FRAME &&
	   receiveHeader_.opcode != Opcode::BINARY_FRAME)
	{
		LOG_ERROR << "The opcode is" << receiveHeader_.opcode
			<< " will be retrieve all";
		buf->retrieveAll();
		return false;
	}
	return true;
}

void WebSocketConnection::fetchMask(Buffer *buf)
{
	const char *data = buf->peek();
	receiveHeader_.mask = data[1] & 0x80;
}

bool WebSocketConnection::fetchPayloadLength(Buffer *buf)
{
	const char *data = buf->peek();
	receiveHeader_.payload = data[1] & 0x7F;
	if(receiveHeader_.payload < 126)
	{
		buf->retrieve(2);
		return true;
	}
	if(receiveHeader_.payload == 126 && buf->readableBytes() >= 4)
	{
		//buf->retrieve(2);
		uint16_t length = 0;
		memcpy(&length, &data[2], 2);
		buf->retrieve(4);
		receiveHeader_.payload = sockets::networkToHost16(length);
		return true;
	}
	else if(receiveHeader_.payload == 127 && buf->readableBytes() >= 10)
	{
		//buf->retrieve(2);
		uint64_t length = 0;
		memcpy(&length, &data[2], 8);
		buf->retrieve(10);
		receiveHeader_.payload = sockets::networkToHost64(length);
		return true;
	}

	return false;
}

void WebSocketConnection::fetchMaskingKey(Buffer *buf)
{
	if(receiveHeader_.mask)
	{
		const char *data = buf->peek();
		for(size_t i = 0; i < 4; i++)
		{
			receiveHeader_.maskKey[i] = data[i];
		}
		buf->retrieve(4);
	}
}
void WebSocketConnection::fetchPayload(Buffer *buf)
{
	bool down = false;
	size_t readable = buf->readableBytes();
	if(receiveHeader_.mask == 0)
	{
		if(receiveHeader_.payload <= readable)
		{
			recivedBuf_.append(buf->retrieveAsString(receiveHeader_.payload));
			down = true;
		}
	}
	else
	{
		if(std::equal(receiveHeader_.maskKey, receiveHeader_.maskKey + 4, "\0\0\0\0"))
		{
			if(buf->readableBytes() >= 4)
				fetchMaskingKey(buf);
			else
				return;
		}
		string message;
		if(receiveHeader_.payload <= readable)
		{
			message = buf->retrieveAsString(receiveHeader_.payload);
			down = true;
			for(size_t i = 0; i < message.size(); i++)
			{
				int j = i % 4;
				char val = message[i] ^ receiveHeader_.maskKey[j];
				recivedBuf_.append(&val, 1);
			}
		}
		receiveHeader_.preaseDown = down;
	}
}

} // namespace wss
} // namespace muduo::net