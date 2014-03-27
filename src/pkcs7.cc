#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif // #ifndef BUILDING_NODE_EXTENSION
#include <node.h>
#include <node_buffer.h>
#include "openssl.h"

#include <openssl/pem.h>
#include <openssl/err.h>

#include <string.h>

using namespace v8;

Handle<Value> Sign(const Arguments& args) {
  HandleScope scope;

  long flags = PKCS7_BINARY | PKCS7_DETACHED | PKCS7_NOOLDMIMETYPE;

  if (args.Length() != 3) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments (should be 3)")));
    return scope.Close(Undefined());
  }

  if (!args[0]->IsObject() || !args[1]->IsObject() || !args[2]->IsObject()) {
    ThrowException(Exception::TypeError(String::New("All parameters should be Buffers")));
    return scope.Close(Undefined());
  }

  // Get cert
  char* certData = node::Buffer::Data(args[0]->ToObject());
  BIO *bio1 = BIO_new_mem_buf(certData, -1);
  X509 *cert = PEM_read_bio_X509(bio1, NULL, NULL, NULL);
  if (cert == NULL) {
      ThrowException(Exception::TypeError(String::New(ERR_error_string(ERR_peek_error(), NULL))));
      return scope.Close(Undefined());
  }
  BIO_free(bio1);

  // Get private key
  char* pKeyData = node::Buffer::Data(args[1]->ToObject());
  BIO *bio2 = BIO_new_mem_buf(pKeyData, -1);
  EVP_PKEY *pKey = PEM_read_bio_PrivateKey(bio2, NULL, NULL, NULL);
  if (pKey == NULL) {
      ThrowException(Exception::TypeError(String::New(ERR_error_string(ERR_peek_error(), NULL))));
      return scope.Close(Undefined());
  }
  BIO_free(bio2);

  // In data
  const char* inData = node::Buffer::Data(args[2]->ToObject());
  BIO *in = BIO_new_mem_buf((void*)inData, -1);
  if (in == NULL) {
      ThrowException(Exception::TypeError(String::New("Failed allocating memory for the data")));
      return scope.Close(Undefined());
  }

  // Allocate memory for output
  BIO *out = BIO_new(BIO_s_mem());

  // Sign
  PKCS7 *p7 = _PKCS7_Sign(cert, pKey, NULL, in, flags);
  if (p7 == NULL) {
      ThrowException(Exception::TypeError(String::New(ERR_error_string(ERR_peek_error(), NULL))));
      return scope.Close(Undefined());
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

  // @see http://luismreis.github.io/node-bindings-guide/docs/returning.html
  node::Buffer *slowBuffer = node::Buffer::New(bptr->length);
  memcpy(node::Buffer::Data(slowBuffer), bptr->data, bptr->length);

  Local<Object> globalObj = Context::GetCurrent()->Global();
  Local<Function> bufferConstructor = Local<Function>::Cast(globalObj->Get(String::New("Buffer")));
  Handle<Value> constructorArgs[3] = { slowBuffer->handle_, v8::Integer::New(bptr->length), v8::Integer::New(0) };
  Local<Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);
  BUF_MEM_free(bptr);
  return scope.Close(actualBuffer);
}

void Init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("sign"),
      FunctionTemplate::New(Sign)->GetFunction());
}

NODE_MODULE(pkcs7, Init)
