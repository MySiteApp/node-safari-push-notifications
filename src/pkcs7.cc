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

using namespace v8;

NAN_METHOD(Sign) {
  NanScope();

  long flags = PKCS7_BINARY | PKCS7_DETACHED | PKCS7_NOOLDMIMETYPE;

  if (args.Length() != 3) {
    return NanThrowError(Exception::TypeError(NanNew("Wrong number of arguments (should be 3)")));
  }

  if (!args[0]->IsObject() || !args[1]->IsObject() || !args[2]->IsObject()) {
    return NanThrowError(Exception::TypeError(NanNew("All parameters should be Buffers")));
  }

  // Get cert
  char* certData = node::Buffer::Data(args[0]->ToObject());
  BIO *bio1 = BIO_new_mem_buf(certData, -1);
  X509 *cert = PEM_read_bio_X509(bio1, NULL, NULL, NULL);
  if (cert == NULL) {
      return NanThrowError(Exception::TypeError(NanNew(ERR_error_string(ERR_peek_error(), NULL))));
  }
  BIO_free(bio1);

  // Get private key
  char* pKeyData = node::Buffer::Data(args[1]->ToObject());
  BIO *bio2 = BIO_new_mem_buf(pKeyData, -1);
  EVP_PKEY *pKey = PEM_read_bio_PrivateKey(bio2, NULL, NULL, NULL);
  if (pKey == NULL) {
      return NanThrowError(Exception::TypeError(NanNew(ERR_error_string(ERR_peek_error(), NULL))));
      //return NanReturnValue(NanUndefined());
  }
  BIO_free(bio2);

  // In data
  const char* inData = node::Buffer::Data(args[2]->ToObject());
  BIO *in = BIO_new_mem_buf((void*)inData, -1);
  if (in == NULL) {
      return NanThrowError(Exception::TypeError(NanNew("Failed allocating memory for the data")));
      //return NanReturnValue(NanUndefined());
  }

  // Allocate memory for output
  BIO *out = BIO_new(BIO_s_mem());

  // Sign
  PKCS7 *p7 = _PKCS7_Sign(cert, pKey, NULL, in, flags);
  if (p7 == NULL) {
      return NanThrowError(Exception::TypeError(NanNew(ERR_error_string(ERR_peek_error(), NULL))));
      //return NanReturnValue(NanUndefined());
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

  v8::Local<v8::Object> slowBuffer = NanNewBufferHandle(bptr->data, bptr->length);

  //memcpy(node::Buffer::Data(slowBuffer), bptr->data, bptr->length);

  Local<Object> globalObj = NanGetCurrentContext()->Global();
  Local<Function> bufferConstructor = Local<Function>::Cast(globalObj->Get(NanNew("Buffer")));
  Handle<Value> constructorArgs[3] = { slowBuffer, NanNew<Integer>(static_cast<unsigned int>(bptr->length)), NanNew<Integer>(0) };
  Local<Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);
  BUF_MEM_free(bptr);
  NanReturnValue(actualBuffer);
}

void Init(Handle<Object> exports) {
  exports->Set(NanNew("sign"),
      NanNew<FunctionTemplate>(Sign)->GetFunction());
}

NODE_MODULE(pkcs7, Init)
