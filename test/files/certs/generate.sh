#!/bin/bash

BASE=$(dirname `readlink -f $0`)
pushd "$BASE"

CA_KEY=ca.key.pem
CA_CERT=ca.cert.pem

INTERMEDIATE_KEY=intermediate.key.pem
INTERMEDIATE_CERT=intermediate.cert.pem

CERT_KEY=cert.key.pem
CERT_CERT=cert.cert.pem

SUBJ="/C=US/O=Alice/CN="
if [ "$OS" == "Windows_NT" ]; then SUBJ="//C=US\O=Alice\CN="; fi

echo Generating root folder structure
for root in root-ca root-intermediate
do
    rm -rf $root
    for i in certs crl newcerts private
    do
        mkdir -p $root/$i
    done
    echo 01 > $root/serial
    touch $root/index.txt
done

echo Generating CA
openssl genrsa -out $CA_KEY 4096
openssl req -config ca_root.cnf \
    -key $CA_KEY \
    -new -x509 -days 7305 -sha256 -extensions v3_ca \
    -out $CA_CERT \
    -subj "${SUBJ}Root CA"



echo Generating intermediate
openssl genrsa -out $INTERMEDIATE_KEY 4096
openssl req -config ca_intermediate.cnf \
    -new -sha256 \
    -key $INTERMEDIATE_KEY \
    -out intermediate.csr \
    -subj "${SUBJ}Intermediate Certificate"
openssl ca -config ca_root.cnf \
    -extensions v3_intermediate_ca \
    -days 3653 -notext -md sha256 \
    -in intermediate.csr \
    -batch \
    -out $INTERMEDIATE_CERT


echo Generating cert
openssl genrsa -out $CERT_KEY 2048
openssl req -new -sha256 \
    -key $CERT_KEY \
    -out cert.csr \
    -subj "${SUBJ}example.com"
openssl ca -config ca_intermediate.cnf \
    -extensions server_cert \
    -days 375 -notext -md sha256 \
    -batch \
    -in cert.csr \
    -out $CERT_CERT

popd