#include <muduo/net/websocket/WebSocketConnection.h>
#include <muduo/net/Endian.h>

namespace muduo::net
{
namespace wss
{
WebSocketConnection::WebSocketConnection(const TcpConnectionPtr &conn)
	: connection_(conn) {}
WebSocketConnection::~WebSocketConnection() {}

bool WebSocketConnection::connected() const
{
	auto conn = connection_.lock();
	if (conn)
		return conn->connected();
}

bool WebSocketConnection::disconnected() const
{
	auto conn = connection_.lock();
	if (conn)
		return conn->disconnected();
}

void WebSocketConnection::send(const void * message, int64_t len, Opcode frame)
{
	uint8_t payloadExternBytes, payload;
	if (len > 32767)
	{
		payloadExternBytes = 8;
		payload = 127;
	}
	else if (len >= 126)
	{
		payloadExternBytes = 2;
		payload = 126;
	}
	else
	{
		payloadExternBytes = 0;
		payload = len;
	}

	Buffer buf;
	buf.append(&static_cast<char>(0X80 | frame), 1);
	buf.append(&static_cast<char>(len), 1);
	if (payloadExternBytes == 2)
	{	
		buf.append(&static_cast<char>(sockets::hostToNetwork16(static_cast<uint16_t>(len))), 2);
	}
	else if (payloadExternBytes == 8)
	{
		buf.append(&static_cast<char>(sockets::hostToNetwork64(static_cast<uint64_t>(len))), 8);
	}
	buf.append(message, len);
	LOG_DEBUG<<buf.
	auto conn = connection_.lock();
	if (conn)
		return conn->send(&buf);
}

void WebSocketConnection::send(const StringPiece & message, Opcode frame)
{
}

void WebSocketConnection::send(Buffer * message, Opcode frame)
{
}

bool WebSocketConnection::preaseMessage(Buffer *buf, Timestamp receiveTime)
{
	if (receiveHeader_.preaseDown && buf->readableBytes() < 2)
		return false;
	if (receiveHeader_.preaseDown)
	{
		fetchFIN(buf);
		if (!fecthOpcode(buf))
			return false;
		fetchMask(buf);
		if (!fetchPayloadLength(buf))
			return false;
		fetchPayload(buf);
	}
	LOG_DEBUG << "fin " << receiveHeader_.fin << " opcode " << receiveHeader_.opcode
		<< " maske " << receiveHeader_.mask << " payload "
		<< receiveHeader_.payload;

	receiveHeader_.preaseDown = true;
	onMessageCallback_(shared_from_this(), &recivedBuf_, receiveTime);
	return true;
}

void WebSocketConnection::shutdown()
{
	auto conn = connection_.lock();
	if (conn)
		return conn->shutdown();
}

void WebSocketConnection::forceClose()
{
	auto conn = connection_.lock();
	if (conn)
		return conn->forceClose();
}
void WebSocketConnection::forceCloseWithDelay(double secondes)
{
	auto conn = connection_.lock();
	if (conn)
		return conn->forceCloseWithDelay(secondes);
}

void WebSocketConnection::fetchFIN(Buffer *buf)
{
	const char *data = buf->peek();
	receiveHeader_.fin = data[0] & 0x80;
}

bool WebSocketConnection::fecthOpcode(Buffer *buf)
{
	receiveHeader_.opcode = *(buf->peek()) & 0x0F;
	buf->retrieve(1);
	if (receiveHeader_.opcode != WebSocketHeader::Opcode::TEXT_FRAME &&
		receiveHeader_.opcode != WebSocketHeader::Opcode::BINARY_FRAME)
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
	receiveHeader_.mask = data[0] & 0x80;
}


bool WebSocketConnection::fetchPayloadLength(Buffer *buf)
{
	const char *data = buf->peek();
	receiveHeader_.payload = data[0] & 0x7F;
	data++;
	buf->retrieve(1);
	if (receiveHeader_.payload == 126)
	{
		uint16_t length = 0;
		memcpy(&length, data, 2);
		buf->retrieve(2);
		receiveHeader_.payload = sockets::networkToHost16(length);
	}
	else if (receiveHeader_.payload == 127)
	{
		uint64_t length = 0;
		memcpy(&length, data, 8);
		buf->retrieve(8);
		receiveHeader_.payload = sockets::networkToHost64(length);
	}
	return true;
}

void WebSocketConnection::fetchMaskingKey(Buffer *buf)
{
	if (receiveHeader_.mask)
	{
		const char *data = buf->peek();
		for (size_t i = 0; i < 4; i++)
		{
			receiveHeader_.maskKey[i] = data[i];
		}
		buf->retrieve(4);
	}
}
void WebSocketConnection::fetchPayload(Buffer *buf)
{
	if (receiveHeader_.mask == 0)
	{
		recivedBuf_.append(buf->retrieveAsString(receiveHeader_.payload));
	}
	else
	{
		fetchMaskingKey(buf);
		string message = buf->retrieveAsString(receiveHeader_.payload);
		for (size_t i = 0; i < message.size(); i++)
		{
			int j = i % 4;
			char val = message[i] ^ receiveHeader_.maskKey[j];
			recivedBuf_.append(&val, 1);
		}
	}
}

} // namespace wss
} // namespace muduo::net