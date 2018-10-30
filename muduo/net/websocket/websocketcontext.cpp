#include <iostream>
#include <muduo/net/websocket/WebSocketContext.h>

namespace muduo::net
{
namespace wss
{

WebSocketContext::WebSocketContext(WebSocketPtr &websocket)
	: webSocketConn_(websocket),
	  state_(WebSocketParseState::kExpectRequestLine) {}

WebSocketContext::~WebSocketContext() {}
bool WebSocketContext::manageHandshake(Buffer *buf, Timestamp reciveTime)
{
	bool ok = true, hasMore = true;
	while (hasMore)
	{
		const char *crlf = buf->findCRLF();
		if (state_ == WebSocketParseState::kExpectRequestLine)
		{
			if (crlf)
			{
				ok = processRequestLine(buf->peek(), crlf);
				if (ok)
				{
					buf->retrieveUntil(crlf + 2);
					state_ = WebSocketParseState::kExpectUpgradeLine;
				}
				else
				{
					hasMore = false;
				}
			}
			else
			{
				hasMore = false;
				ok = false;
			}
		}
		else if (state_ == WebSocketParseState::kExpectUpgradeLine)
		{
			if (crlf)
			{
				ok = processUpgradeLine(buf->peek(), crlf);

				buf->retrieveUntil(crlf + 2);
				if (ok)
				{
					state_ = WebSocketParseState::kExpectConnectionLine;
				}
				else
				{
					continue;
				}
			}
			else
			{
				hasMore = false;
				ok = false;
			}
		}
		else if (state_ == WebSocketParseState::kExpectConnectionLine)
		{
			if (crlf)
			{
				ok = processConnectionLine(buf->peek(), crlf);

				buf->retrieveUntil(crlf + 2);
				if (ok)
				{
					state_ = WebSocketParseState::kExpectOriginLine;
				}
				else
				{
					continue;
				}
			}
			else
			{
				hasMore = false;
				ok = false;
			}
		}
		else if (state_ == WebSocketParseState::kExpectOriginLine)
		{
			if (crlf)
			{
				ok = processOriginLine(buf->peek(), crlf);

				buf->retrieveUntil(crlf + 2);
				if (ok)
				{
					state_ = WebSocketParseState::kExpectSecWebSocketKey;
				}
				else
				{
					continue;
				}
			}
			else
			{
				hasMore = false;
				ok = false;
			}
		}
		else if (state_ == WebSocketParseState::kExpectSecWebSocketVersion)
		{
			if (crlf)
			{
				ok = processWebSocketVeisionLine(buf->peek(), crlf);
				buf->retrieveUntil(crlf + 2);
				if (ok)
				{
					state_ = WebSocketParseState::kExpectSecWebSocketKey;
				}
				else
				{
					continue;
				}
			}
			else
			{
				hasMore = false;
				ok = false;
			}
		}
		else if (state_ == WebSocketParseState::kExpectSecWebSocketKey)
		{
			if (crlf)
			{
				ok = processWebSocketKeyLine(buf->peek(), crlf);

				buf->retrieveUntil(crlf + 2);
				if (ok)
				{
					state_ = WebSocketParseState::kConnectionEstablished;
					hasMore = false;
				}
				else
				{
					continue;
				}
			}
			else
			{
				hasMore = false;
				ok = false;
			}
		}
	}

	return ok;
}
string WebSocketContext::webSocketHandshakeAccept() const
{
	string sha1 = ssl::sslSha1(strSecWebSocketKey_ + wssMgic);

	return ssl::sslBase64Encode(sha1);
}
bool WebSocketContext::processRequestLine(const char *begin, const char *end)
{
	bool succeed = false;
	const char *start = begin;
	const char *space = std::find(start, end, ' ');
	if (space != end)
	{
		succeed = std::equal(start, space, "GET");
	}
	if (succeed)
	{
		space = std::find(space + 1, end, ' ');
		if (space != end)
		{
			succeed = std::equal(space + 1, end, "HTTP/1.1");
		}
		else
		{
			succeed = false;
		}
	}

	return succeed;
}
bool WebSocketContext::processUpgradeLine(const char *begin, const char *end)
{
	bool succeed = false;
	const char *start = begin;
	const char *space = std::find(start, end, ' ');
	if (space != end)
	{
		succeed = std::equal(start, space, "Upgrade:");
	}
	if (succeed)
	{
		succeed = std::equal(space + 1, end, "websocket");
	}

	return succeed;
}
bool WebSocketContext::processConnectionLine(const char *begin,
											 const char *end)
{
	return true;
}
bool WebSocketContext::processOriginLine(const char *begin, const char *end)
{
	return true;
}
bool WebSocketContext::processWebSocketKeyLine(const char *begin,
											   const char *end)
{
	bool succeed = false;
	const char *start = begin;
	const char *space = std::find(start, end, ' ');
	if (space != end)
	{
		succeed = std::equal(start, space, "Sec-WebSocket-Key:");
	}
	if (succeed)
	{
		strSecWebSocketKey_ = string(space + 1, end);
	}
	return succeed;
}
bool WebSocketContext::processWebSocketVeisionLine(const char *begin,
												   const char *end)
{
	return true;
}
} // namespace wss
} // namespace muduo::net
