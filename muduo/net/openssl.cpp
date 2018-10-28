#include <muduo/net/openssl.h>
#include "openssl.h"

namespace muduo
{
	namespace net
	{
		ssl::SslServerAttributes::SslServerAttributes()
			: keyPath(), keyType(0), certificatePath(), certificateType(0)
		{
		}
		ssl::SslServerAttributes::~SslServerAttributes()
		{
			if (ctx)
				SSL_CTX_free(ctx);
		}
		bool ssl::initSslServer(ssl::sslAttrivutesPtr attrivutes)
		{
			bool ok = true;

			::SSL_library_init();
			::OpenSSL_add_all_algorithms();
			::SSL_load_error_strings();
			::ERR_load_BIO_strings();
			if ((attrivutes->ctx = ::SSL_CTX_new(SSLv23_server_method())) == nullptr)
			{
				LOG_FATAL << "The openssl`s ctx creat fatal" << " erro info" << ERR_error_string(ERR_get_error(), NULL);
				ok = false;
			}
			if (::SSL_CTX_use_certificate_file(attrivutes->ctx, attrivutes->certificatePath.c_str(), attrivutes->certificateType) <= 0)
			{
				LOG_FATAL << "The openssl`s load the user certificate fatal" << attrivutes->certificatePath << " erro info" << ERR_error_string(ERR_get_error(), NULL);
				ok = false;
			}
			if (::SSL_CTX_use_PrivateKey_file(attrivutes->ctx, attrivutes->keyPath.c_str(), attrivutes->keyType) <= 0)
			{
				LOG_FATAL << "The openssl`s load the private key fatal" << " erro info" << ERR_error_string(ERR_get_error(), NULL);
				ok = false;
			}
			if (::SSL_CTX_check_private_key(attrivutes->ctx) <= 0)
			{
				LOG_FATAL << "The openssl`s check the private key fatal" << " erro info" << ERR_error_string(ERR_get_error(), NULL);
				ok = false;
			}
			return ok;
		}

		string ssl::sslSha1(const string & str)
		{
			SHA_CTX ctx;
			SHA1_Init(&ctx);
			SHA1_Update(&ctx, str.c_str(), str.length());
			unsigned char szSHA1[SHA_DIGEST_LENGTH] = { 0 };
			SHA1_Final(szSHA1, &ctx);

			string retString;
			char strTmpHex[SHA_DIGEST_LENGTH * 2 + 1] = { 0 };
			for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
			{
				sprintf(&strTmpHex[i * 2], "%02x", (unsigned int)szSHA1[i]);
			}
			LOG_INFO << str << "       sha1     " << strTmpHex;
			return string(reinterpret_cast<char*>(szSHA1), SHA_DIGEST_LENGTH);
		}

		string ssl::sslBase64Encode(const string & str)
		{

			BIO *b64, *bio;
			BUF_MEM *bufptr = nullptr;
			size_t size = 0;


			b64 = BIO_new(BIO_f_base64());
			bio = BIO_new(BIO_s_mem());
			bio = BIO_push(b64, bio);

			BIO_write(bio, str.c_str(), str.length());
			BIO_flush(bio);

			BIO_get_mem_ptr(bio, &bufptr);
			string retStr = string(bufptr->data,bufptr->length);
			BIO_free_all(bio);
			LOG_INFO << str << "       base64     " << retStr;
			return retStr;
		}

	}
}

