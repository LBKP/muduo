#pragma once
#include <muduo/base/Types.h>
#include <muduo/base/Logging.h>
extern  "C"
{
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>
}

#include <memory>

namespace muduo
{
	namespace net
	{
		class ssl
		{
		public:
			struct SslServerAttributes
			{
				string keyPath;
				int keyType;
				string certificatePath;
				int certificateType;
				SSL_CTX *ctx;
				SslServerAttributes();
				~SslServerAttributes();
			};
			typedef std::shared_ptr<SslServerAttributes> sslAttrivutesPtr;

			static bool initSslServer(sslAttrivutesPtr attrivutes);
			static int sslAccept(SSL* ssl) { return ::SSL_accept(ssl); };
			static SSL* sslNew(SSL_CTX* ctx) { return ::SSL_new(ctx); };
			static int sslSetfd(SSL* ssl, int fd) { return ::SSL_set_fd(ssl, fd); };
			static void sslFree(SSL* ssl) { ::SSL_free(ssl); };
			static int sslRead(SSL* ssl, void* buf, int num) { return ::SSL_read(ssl, buf, num); };
			static int sslSend(SSL* ssl, const void* buf, int num) { return ::SSL_write(ssl, buf, num); }
			static string sslSha1(const string& str);
			static string sslBase64Encode(const string& str);

		};
	} // namespace net
} // namespace muduo