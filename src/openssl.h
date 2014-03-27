#include <openssl/pkcs7.h>
#include <openssl/x509.h>

PKCS7 *_PKCS7_Sign(X509 *signcert, EVP_PKEY *pkey, STACK_OF(X509) *certs,
          BIO *data, int flags);
int _SMIME_write_PKCS7(BIO *bio, PKCS7 *p7, BIO *data, int flags);
