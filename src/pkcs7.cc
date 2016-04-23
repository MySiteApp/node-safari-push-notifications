#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif // #ifndef BUILDING_NODE_EXTENSION
#include <node.h>
#include <node_buffer.h>
#include "openssl.h"

#include <openssl/pem.h>
#include <openssl/err.h>

#include <string.h>

#include <nan.h>

using Nan::CopyBuffer;
using Nan::GetFunction;
using Nan::HandleScope;
using Nan::New;
using Nan::ThrowTypeError;
using Nan::Set;

using v8::String;
using v8::FunctionTemplate;

NAN_METHOD(Sign) {
  HandleScope scope;

  long flags = PKCS7_BINARY | PKCS7_DETACHED | PKCS7_NOOLDMIMETYPE;

  if (info.Length() < 3) {
    return ThrowTypeError("Wrong number of arguments (should be 3)");
  }

  if (!info[0]->IsObject() || !info[1]->IsObject() || !info[2]->IsObject()) {
    return ThrowTypeError("All parameters should be Buffers");
  }

  if (info.Length() == 4 && !info[3]->IsObject()) {
    return ThrowTypeError("intermediate certificate should be Buffer");
  }

  // Get cert
  char* certData = node::Buffer::Data(info[0]->ToObject());
  BIO *bio1 = BIO_new_mem_buf(certData, -1);
  X509 *cert = PEM_read_bio_X509(bio1, NULL, NULL, NULL);
  if (cert == NULL) {
      return ThrowTypeError(ERR_error_string(ERR_peek_error(), NULL));
  }
  BIO_free(bio1);

  // Get private key
  char* pKeyData = node::Buffer::Data(info[1]->ToObject());
  BIO *bio2 = BIO_new_mem_buf(pKeyData, -1);
  EVP_PKEY *pKey = PEM_read_bio_PrivateKey(bio2, NULL, NULL, NULL);
  if (pKey == NULL) {
      return ThrowTypeError(ERR_error_string(ERR_peek_error(), NULL));
  }
  BIO_free(bio2);

  // In data
  const char* inData = node::Buffer::Data(info[2]->ToObject());
  BIO *in = BIO_new_mem_buf((void*)inData, -1);
  if (in == NULL) {
      return ThrowTypeError("Failed allocating memory for the data");
  }

  // Allocate memory for output
  BIO *out = BIO_new(BIO_s_mem());
  PKCS7 *p7;

  if (info.Length() == 3) {
    // Sign
    p7 = _PKCS7_Sign(cert, pKey, NULL, in, flags);
  } else {
    // Sign with extra cert

    char* intermediateCert = node::Buffer::Data(info[3]->ToObject());
    BIO *bio4 = BIO_new_mem_buf(intermediateCert, -1);
    X509 *intermediate = PEM_read_bio_X509(bio4, NULL, NULL, NULL);
    if (intermediate == NULL) {
        return ThrowTypeError(ERR_error_string(ERR_peek_error(), NULL));
    }
    BIO_free(bio4);

    STACK_OF(X509) *sk = sk_X509_new_null();
    sk_X509_push(sk, intermediate);

    p7 = _PKCS7_Sign(cert, pKey, sk, in, flags);
  }

  if (p7 == NULL) {
      return ThrowTypeError(ERR_error_string(ERR_peek_error(), NULL));
  }

  (void)BIO_reset(in);

  // Finalize
  _SMIME_write_PKCS7(out, p7, in, flags);

  // Free
  BIO_free(in);
  PKCS7_free(p7);
  EVP_PKEY_free(pKey);
  X509_free(cert);

  // Let's return the BIO as Buffer
  BUF_MEM *bptr;
  BIO_get_mem_ptr(out, &bptr);
  (void)BIO_set_close(out, BIO_NOCLOSE); /* So BIO_free() leaves BUF_MEM alone */
  (void)BIO_free(out);

  info.GetReturnValue().Set(CopyBuffer(bptr->data, bptr->length).ToLocalChecked());
  BUF_MEM_free(bptr);
}

NAN_MODULE_INIT(Init) {
  Set(target, New<String>("sign").ToLocalChecked(),
      GetFunction(New<FunctionTemplate>(Sign)).ToLocalChecked());
}

NODE_MODULE(pkcs7, Init)
