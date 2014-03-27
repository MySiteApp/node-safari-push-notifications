/**
 * Taken from openssl-0.9.8/crypto/pkcs7/pk7_mime.c & pk7_smime.c
 **/

 /* ====================================================================
 * Copyright (c) 1999-2005 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

 #include <openssl/pem.h>
 #include <openssl/err.h>
 #include <openssl/rand.h>
 #include <openssl/pkcs7.h>
 #include <openssl/x509.h>

static int B64_write_PKCS7(BIO *bio, PKCS7 *p7)
{
    BIO *b64;
    if(!(b64 = BIO_new(BIO_f_base64()))) {
        PKCS7err(PKCS7_F_B64_WRITE_PKCS7,ERR_R_MALLOC_FAILURE);
        return 0;
    }
    bio = BIO_push(b64, bio);
    i2d_PKCS7_bio(bio, p7);
    (void)BIO_flush(bio);
    bio = BIO_pop(bio);
    BIO_free(b64);
    return 1;
}

static int pkcs7_output_data(BIO *out, BIO *data, PKCS7 *p7, int flags)
    {
    BIO *tmpbio, *p7bio;

    if (!(flags & PKCS7_STREAM))
        {
        SMIME_crlf_copy(data, out, flags);
        return 1;
        }

    /* Partial sign operation */

    /* Initialize sign operation */
    p7bio = PKCS7_dataInit(p7, out);

    /* Copy data across, computing digests etc */
    SMIME_crlf_copy(data, p7bio, flags);

    /* Must be detached */
    PKCS7_set_detached(p7, 1);

    /* Finalize signatures */
    PKCS7_dataFinal(p7, p7bio);

    /* Now remove any digests prepended to the BIO */

    while (p7bio != out)
        {
        tmpbio = BIO_pop(p7bio);
        BIO_free(p7bio);
        p7bio = tmpbio;
        }

    return 1;

    }

PKCS7 *_PKCS7_Sign(X509 *signcert, EVP_PKEY *pkey, STACK_OF(X509) *certs,
          BIO *data, int flags)
{
    PKCS7 *p7;
    PKCS7_SIGNER_INFO *si;
    BIO *p7bio;
    STACK_OF(X509_ALGOR) *smcap;
    int i;

    if(!X509_check_private_key(signcert, pkey)) {
        PKCS7err(PKCS7_F_PKCS7_SIGN,PKCS7_R_PRIVATE_KEY_DOES_NOT_MATCH_CERTIFICATE);
                return NULL;
    }

    if(!(p7 = PKCS7_new())) {
        PKCS7err(PKCS7_F_PKCS7_SIGN,ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    PKCS7_set_type(p7, NID_pkcs7_signed);

    PKCS7_content_new(p7, NID_pkcs7_data);

        if (!(si = PKCS7_add_signature(p7,signcert,pkey,EVP_sha1()))) {
        PKCS7err(PKCS7_F_PKCS7_SIGN,PKCS7_R_PKCS7_ADD_SIGNATURE_ERROR);
        PKCS7_free(p7);
        return NULL;
    }

    if(!(flags & PKCS7_NOCERTS)) {
        PKCS7_add_certificate(p7, signcert);
        if(certs) for(i = 0; i < sk_X509_num(certs); i++)
            PKCS7_add_certificate(p7, sk_X509_value(certs, i));
    }

    if(!(flags & PKCS7_NOATTR)) {
        PKCS7_add_signed_attribute(si, NID_pkcs9_contentType,
                V_ASN1_OBJECT, OBJ_nid2obj(NID_pkcs7_data));
        /* Add SMIMECapabilities */
        if(!(flags & PKCS7_NOSMIMECAP))
        {
        if(!(smcap = sk_X509_ALGOR_new_null())) {
            PKCS7err(PKCS7_F_PKCS7_SIGN,ERR_R_MALLOC_FAILURE);
            PKCS7_free(p7);
            return NULL;
        }
#ifndef OPENSSL_NO_DES
        PKCS7_simple_smimecap (smcap, NID_des_ede3_cbc, -1);
#endif
#ifndef OPENSSL_NO_RC2
        PKCS7_simple_smimecap (smcap, NID_rc2_cbc, 128);
        PKCS7_simple_smimecap (smcap, NID_rc2_cbc, 64);
#endif
#ifndef OPENSSL_NO_DES
        PKCS7_simple_smimecap (smcap, NID_des_cbc, -1);
#endif
#ifndef OPENSSL_NO_RC2
        PKCS7_simple_smimecap (smcap, NID_rc2_cbc, 40);
#endif
        PKCS7_add_attrib_smimecap (si, smcap);
        sk_X509_ALGOR_pop_free(smcap, X509_ALGOR_free);
        }
    }

    if (flags & PKCS7_STREAM)
        return p7;

    if (!(p7bio = PKCS7_dataInit(p7, NULL))) {
        PKCS7err(PKCS7_F_PKCS7_SIGN,ERR_R_MALLOC_FAILURE);
        PKCS7_free(p7);
        return NULL;
    }

    SMIME_crlf_copy(data, p7bio, flags);

    if(flags & PKCS7_DETACHED)PKCS7_set_detached(p7, 1);

        if (!PKCS7_dataFinal(p7,p7bio)) {
        PKCS7err(PKCS7_F_PKCS7_SIGN,PKCS7_R_PKCS7_DATASIGN);
        PKCS7_free(p7);
        BIO_free_all(p7bio);
        return NULL;
    }

    BIO_free_all(p7bio);
    return p7;
}

int _SMIME_write_PKCS7(BIO *bio, PKCS7 *p7, BIO *data, int flags)
{
    char bound[33], c;
    int i;
    char *mime_prefix, *mime_eol, *msg_type=NULL;
    if (flags & PKCS7_NOOLDMIMETYPE)
        mime_prefix = (char*)"application/pkcs7-";
    else
        mime_prefix = (char*)"application/x-pkcs7-";

    if (flags & PKCS7_CRLFEOL)
        mime_eol = (char*)"\r\n";
    else
        mime_eol = (char*)"\n";
    if((flags & PKCS7_DETACHED) && data) {
    /* We want multipart/signed */
        /* Generate a random boundary */
        RAND_pseudo_bytes((unsigned char *)bound, 32);
        for(i = 0; i < 32; i++) {
            c = bound[i] & 0xf;
            if(c < 10) c += '0';
            else c += 'A' - 10;
            bound[i] = c;
        }
        bound[32] = 0;
        BIO_printf(bio, "MIME-Version: 1.0%s", mime_eol);
        BIO_printf(bio, "Content-Type: multipart/signed;");
        BIO_printf(bio, " protocol=\"%ssignature\";", mime_prefix);
        BIO_printf(bio, " micalg=\"sha1\"; boundary=\"----%s\"%s%s",
                        bound, mime_eol, mime_eol);
        BIO_printf(bio, "This is an S/MIME signed message%s%s",
                        mime_eol, mime_eol);
        /* Now write out the first part */
        BIO_printf(bio, "------%s%s", bound, mime_eol);
        pkcs7_output_data(bio, data, p7, flags);
        BIO_printf(bio, "%s------%s%s", mime_eol, bound, mime_eol);

        /* Headers for signature */

        BIO_printf(bio, "Content-Type: %ssignature;", mime_prefix);
        BIO_printf(bio, " name=\"smime.p7s\"%s", mime_eol);
        BIO_printf(bio, "Content-Transfer-Encoding: base64%s",
                                mime_eol);
        BIO_printf(bio, "Content-Disposition: attachment;");
        BIO_printf(bio, " filename=\"smime.p7s\"%s%s",
                            mime_eol, mime_eol);
        B64_write_PKCS7(bio, p7);
        BIO_printf(bio,"%s------%s--%s%s", mime_eol, bound,
                            mime_eol, mime_eol);
        return 1;
    }

    /* Determine smime-type header */

    if (PKCS7_type_is_enveloped(p7))
        msg_type = (char*)"enveloped-data";
    else if (PKCS7_type_is_signed(p7))
        {
        /* If we have any signers it is signed-data othewise
         * certs-only.
         */
        STACK_OF(PKCS7_SIGNER_INFO) *sinfos;
        sinfos = PKCS7_get_signer_info(p7);
        if (sk_PKCS7_SIGNER_INFO_num(sinfos) > 0)
            msg_type = (char*)"signed-data";
        else
            msg_type = (char*)"certs-only";
        }
    /* MIME headers */
    BIO_printf(bio, "MIME-Version: 1.0%s", mime_eol);
    BIO_printf(bio, "Content-Disposition: attachment;");
    BIO_printf(bio, " filename=\"smime.p7m\"%s", mime_eol);
    BIO_printf(bio, "Content-Type: %smime;", mime_prefix);
    if (msg_type)
        BIO_printf(bio, " smime-type=%s;", msg_type);
    BIO_printf(bio, " name=\"smime.p7m\"%s", mime_eol);
    BIO_printf(bio, "Content-Transfer-Encoding: base64%s%s",
                        mime_eol, mime_eol);
    B64_write_PKCS7(bio, p7);
    BIO_printf(bio, "%s", mime_eol);
    return 1;
}
