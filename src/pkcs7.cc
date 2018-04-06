#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif // #ifndef BUILDING_NODE_EXTENSION
#include <node.h>
#include <node_buffer.h>

#include <openssl/pkcs7.h>
#include <openssl/x509.h>
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

using v8::FunctionTemplate;
using v8::Local;
using v8::Object;
using v8::String;

NAN_METHOD(Sign) {
  HandleScope scope;

  long flags = PKCS7_BINARY | PKCS7_DETACHED | PKCS7_NOOLDMIMETYPE;
  bool hasIntermediate = info.Length() == 4 && !info[3]->IsNull() && !info[3]->IsUndefined();

  if (info.Length() < 3) {
    return ThrowTypeError("Wrong number of arguments (should be at least 3)");
  }

  if (!info[0]->IsObject() || !info[1]->IsObject() || !info[2]->IsObject()) {
    return ThrowTypeError("All parameters should be Buffers");
  }

  if (hasIntermediate && !info[3]->IsObject()) {
    return ThrowTypeError("intermediate certificate should be a Buffer");
  }

  // Get cert
  Local<Object> certObj = info[0]->ToObject();
  BIO *bio1 = BIO_new_mem_buf(node::Buffer::Data(certObj), node::Buffer::Length(certObj));
  X509 *cert = PEM_read_bio_X509(bio1, NULL, NULL, NULL);
  if (cert == NULL) {
      return ThrowTypeError(ERR_error_string(ERR_peek_error(), NULL));
  }
  BIO_free(bio1);

  // Get private key
  Local<Object> pKeyObj = info[1]->ToObject();
  BIO *bio2 = BIO_new_mem_buf(node::Buffer::Data(pKeyObj), node::Buffer::Length(pKeyObj));
  EVP_PKEY *pKey = PEM_read_bio_PrivateKey(bio2, NULL, NULL, NULL);
  if (pKey == NULL) {
      return ThrowTypeError(ERR_error_string(ERR_peek_error(), NULL));
  }
  BIO_free(bio2);

  // In data
  Local<Object> dataObj = info[2]->ToObject();
  BIO *in = BIO_new_mem_buf(node::Buffer::Data(dataObj), node::Buffer::Length(dataObj));
  if (in == NULL) {
      return ThrowTypeError("Failed allocating memory for the data");
  }

  // Allocate memory for output
  BIO *out = BIO_new(BIO_s_mem());
  PKCS7 *p7;

  if (!hasIntermediate) {
    // Sign
    p7 = PKCS7_sign(cert, pKey, NULL, in, flags);
  } else {
    // Sign with extra cert

    Local<Object> intermediateObj = info[3]->ToObject();
    BIO *bio4 = BIO_new_mem_buf(node::Buffer::Data(intermediateObj), node::Buffer::Length(intermediateObj));
    X509 *intermediate = PEM_read_bio_X509(bio4, NULL, NULL, NULL);
    if (intermediate == NULL) {
        return ThrowTypeError(ERR_error_string(ERR_peek_error(), NULL));
    }
    BIO_free(bio4);

    STACK_OF(X509) *sk = sk_X509_new_null();
    sk_X509_push(sk, intermediate);

    p7 = PKCS7_sign(cert, pKey, sk, in, flags);
  }

  if (p7 == NULL) {
      return ThrowTypeError(ERR_error_string(ERR_peek_error(), NULL));
  }

  (void)BIO_reset(in);

  // Finalize
  SMIME_write_PKCS7(out, p7, in, flags);

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

NAN_METHOD(Verify) {
  HandleScope scope;

  bool hasIntermediate = info.Length() > 3 && !info[3]->IsNull() && !info[3]->IsUndefined();
  bool hasRoot = info.Length() > 4 && !info[4]->IsNull() && !info[4]->IsUndefined();

  if (info.Length() < 3) {
    return ThrowTypeError("Wrong number of arguments (should be at least 3)");
  }

  if (!info[0]->IsObject() || !info[1]->IsObject() || !info[2]->IsObject()) {
    return ThrowTypeError("All parameters should be Buffers");
  }

  if (hasIntermediate && !info[3]->IsObject()) {
    return ThrowTypeError("Intermediate certificate should be a Buffer");
  }

  if (hasRoot && !info[4]->IsObject()) {
    return ThrowTypeError("Root certificate should be a Buffer");
  }

  OpenSSL_add_all_algorithms();

  // Load content
  Local<Object> inObj = info[0]->ToObject();
  BIO *in = BIO_new_mem_buf(node::Buffer::Data(inObj), node::Buffer::Length(inObj));
  if (in == NULL) {
      return ThrowTypeError("Failed allocating memory for the data");
  }

  // Load signature
  Local<Object> sigObject = info[1]->ToObject();
  const char *sigData = node::Buffer::Data(sigObject);
  BIO *bio1 = BIO_new_mem_buf(sigData, node::Buffer::Length(sigObject));
  PKCS7 *p7Sig = d2i_PKCS7_bio(bio1, NULL);
  if (p7Sig == NULL) {
      return ThrowTypeError(ERR_error_string(ERR_peek_error(), NULL));
  }
  BIO_free(bio1);

  X509_STORE* store = X509_STORE_new();

  // Load certificate
  Local<Object> certObj = info[2]->ToObject();
  BIO *bio2 = BIO_new_mem_buf(node::Buffer::Data(certObj), node::Buffer::Length(certObj));
  X509 *cert = PEM_read_bio_X509(bio2, NULL, NULL, NULL);
  if (cert == NULL) {
      return ThrowTypeError(ERR_error_string(ERR_peek_error(), NULL));
  }
  X509_STORE_add_cert(store, cert);
  BIO_free(bio2);

  if (hasIntermediate) {
    Local<Object> intermObj = info[3]->ToObject();
    BIO *bio3 = BIO_new_mem_buf(node::Buffer::Data(intermObj), node::Buffer::Length(intermObj));
    X509 *intermCert = PEM_read_bio_X509(bio3, NULL, NULL, NULL);
    if (intermCert == NULL) {
        return ThrowTypeError(ERR_error_string(ERR_peek_error(), NULL));
    }
    X509_STORE_add_cert(store, intermCert);
    BIO_free(bio3);
  }

  if (hasRoot) {
    Local<Object> rootObj = info[4]->ToObject();
    BIO *bio4 = BIO_new_mem_buf(node::Buffer::Data(rootObj), node::Buffer::Length(rootObj));
    X509 *rootCert = PEM_read_bio_X509(bio4, NULL, NULL, NULL);
    if (rootCert == NULL) {
        return ThrowTypeError(ERR_error_string(ERR_peek_error(), NULL));
    }
    X509_STORE_add_cert(store, rootCert);
    BIO_free(bio4);
  }

  int verifyResult = PKCS7_verify(p7Sig, NULL, store, in, NULL, PKCS7_NOCHAIN);
  info.GetReturnValue().Set(verifyResult);

  (void)BIO_reset(in);
  BIO_free(in);
  PKCS7_free(p7Sig);
}

NAN_MODULE_INIT(Init) {
  Set(target, New<String>("sign").ToLocalChecked(),
      GetFunction(New<FunctionTemplate>(Sign)).ToLocalChecked());
  Set(target, New<String>("verify").ToLocalChecked(),
      GetFunction(New<FunctionTemplate>(Verify)).ToLocalChecked());
}

NODE_MODULE(pkcs7, Init)
