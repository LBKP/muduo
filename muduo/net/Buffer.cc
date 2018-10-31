// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <muduo/net/Buffer.h>

#include <muduo/net/SocketsOps.h>

#include <errno.h>
#include <sys/uio.h>

using namespace muduo;
using namespace muduo::net;

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

ssize_t Buffer::readFd(int fd, int* savedErrno, SSL* ssl = nullptr)
{
	// saved an ioctl()/FIONREAD call to tell how much to read
	char extrabuf[65536];
	struct iovec vec[2];
	const size_t writable = writableBytes();
	vec[0].iov_base = begin() + writerIndex_;
	vec[0].iov_len = writable;
	vec[1].iov_base = extrabuf;
	vec[1].iov_len = sizeof extrabuf;
	// when there is enough space in this buffer, don't read into extrabuf.
	// when extrabuf is used, we read 128k-1 bytes at most.
	const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
	ssize_t n = 0;
	if (ssl)
	{
		char buf[16384];
		int ires = 0;
		while (true)
		{
			ires = ssl::sslRead(ssl, buf + n, 16384);
			if (ires > 0)
			{
				n += ires;
				if (n >= 16384)
					break;
			}
			if (SSL_get_error(ssl, 0) == SSL_ERROR_NONE)
			{
				continue;
			}
			else
			{
				break;
			}
		}
		if (n > 0)
		{
			memcpy(vec[0].iov_base, buf, n > implicit_cast<ssize_t>(vec[0].iov_len) ? vec[0].iov_len : n);
			if (n > implicit_cast<ssize_t>(vec[0].iov_len))
				memcpy(vec[1].iov_base, buf + vec[0].iov_len, n - vec[0].iov_len);
		}
	}
	else
	{
		n = sockets::readv(fd, vec, iovcnt);
	}
	if (n < 0)
	{
		*savedErrno = errno;
	}
	else if (implicit_cast<size_t>(n) <= writable)
	{
		writerIndex_ += n;
	}
	else
	{
		writerIndex_ = buffer_.size();
		append(extrabuf, n - writable);
	}
	// if (n == writable + sizeof extrabuf)
	// {
	//   goto line_30;
	// }
	return n;
}

