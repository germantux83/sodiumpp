//  sodiumpp.cpp
//
// Copyright (c) 2014, Ruben De Visscher
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "sodiumpp.h"
extern "C" {
#include <sodium.h>
}
using std::string;

string sodiumpp::crypto_auth(const string &m,const string &k)
{
    if (k.size() != crypto_auth_KEYBYTES) throw "incorrect key length";
    unsigned char a[crypto_auth_BYTES];
    ::crypto_auth(a,(const unsigned char *) m.c_str(),m.size(),(const unsigned char *) k.c_str());
    return string((char *) a,crypto_auth_BYTES);
}

void sodiumpp::crypto_auth_verify(const string &a,const string &m,const string &k)
{
    if (k.size() != crypto_auth_KEYBYTES) throw "incorrect key length";
    if (a.size() != crypto_auth_BYTES) throw "incorrect authenticator length";
    if (::crypto_auth_verify(
                           (const unsigned char *) a.c_str(),
                           (const unsigned char *) m.c_str(),m.size(),
                           (const unsigned char *) k.c_str()) == 0) return;
    throw "invalid authenticator";
}

string sodiumpp::crypto_box(const string &m,const string &n,const string &pk,const string &sk)
{
    if (pk.size() != crypto_box_PUBLICKEYBYTES) throw "incorrect public-key length";
    if (sk.size() != crypto_box_SECRETKEYBYTES) throw "incorrect secret-key length";
    if (n.size() != crypto_box_NONCEBYTES) throw "incorrect nonce length";
    size_t mlen = m.size() + crypto_box_ZEROBYTES;
	std::string mpad( mlen, 0 );
    for (size_t i = crypto_box_ZEROBYTES;i < mlen;++i) mpad[i] = m[i - crypto_box_ZEROBYTES];
	std::string cpad( mlen, 0 );
	::crypto_box( ( unsigned char* ) &cpad.front(), ( unsigned char* ) &mpad.front(), mlen,
               (const unsigned char *) n.c_str(),
               (const unsigned char *) pk.c_str(),
               (const unsigned char *) sk.c_str()
               );
	return cpad.substr( crypto_box_BOXZEROBYTES );
}

string sodiumpp::crypto_box_keypair(string *sk)
{
	std::string pk(crypto_box_PUBLICKEYBYTES, 0);
	sk->resize( crypto_box_SECRETKEYBYTES );
	::crypto_box_keypair( ( unsigned char* ) &pk.front(), ( unsigned char* ) &sk->front() );
    return pk;
}

string sodiumpp::crypto_box_beforenm(const string &pk, const string &sk) {
    if (pk.size() != crypto_box_PUBLICKEYBYTES) throw "incorrect public-key length";
    if (sk.size() != crypto_box_SECRETKEYBYTES) throw "incorrect secret-key length";
    std::string k(crypto_box_BEFORENMBYTES, 0);
    ::crypto_box_beforenm((unsigned char *)&k[0], (const unsigned char *)&pk[0], (const unsigned char *)&sk[0]);
    return k;
}

string sodiumpp::crypto_box_afternm(const string &m,const string &n,const string &k) {
    if (k.size() != crypto_box_BEFORENMBYTES) throw "incorrect nm-key length";
    if (n.size() != crypto_box_NONCEBYTES) throw "incorrect nonce length";
	size_t mlen = m.size() + crypto_box_BOXZEROBYTES;
	std::string mpad( mlen, 0 );
	for( size_t i = crypto_box_BOXZEROBYTES; i < mlen; ++i ) mpad[i] = m[i - crypto_box_BOXZEROBYTES];
	std::string cpad( mlen, 0 );
	::crypto_box_afternm( ( unsigned char* ) &cpad.front(), ( unsigned char* ) &mpad.front(), mlen,
                 (const unsigned char *) n.c_str(),
                 (const unsigned char *) k.c_str()
                 );
    return cpad.substr( crypto_box_BOXZEROBYTES );
}

string sodiumpp::crypto_box_open_afternm(const string &c,const string &n,const string &k)
{
    if (k.size() != crypto_box_BEFORENMBYTES) throw "incorrect nm-key length";
    if (n.size() != crypto_box_NONCEBYTES) throw "incorrect nonce length";
    size_t clen = c.size() + crypto_box_BOXZEROBYTES;
    std::string cpad(clen, 0);
    for (size_t i = crypto_box_BOXZEROBYTES;i < clen;++i) cpad[i] = c[i - crypto_box_BOXZEROBYTES];
    std::string mpad(clen, 0);
    if (::crypto_box_open_afternm((unsigned char*)&mpad.front(),(unsigned char*)&cpad.front(),clen,
                                  (const unsigned char *) n.c_str(),
                                  (const unsigned char *) k.c_str()
                                  ) != 0)
        throw "ciphertext fails verification";
    if (clen < crypto_box_ZEROBYTES)
        throw "ciphertext too short"; // should have been caught by _open
    return mpad.substr( crypto_box_ZEROBYTES );
}

string sodiumpp::crypto_box_open(const string &c,const string &n,const string &pk,const string &sk)
{
    if (pk.size() != crypto_box_PUBLICKEYBYTES) throw "incorrect public-key length";
    if (sk.size() != crypto_box_SECRETKEYBYTES) throw "incorrect secret-key length";
    if (n.size() != crypto_box_NONCEBYTES) throw "incorrect nonce length";
	size_t clen = c.size() + crypto_box_BOXZEROBYTES;
	std::string cpad( clen, 0 );
	for( size_t i = crypto_box_BOXZEROBYTES; i < clen; ++i ) cpad[i] = c[i - crypto_box_BOXZEROBYTES];
	std::string mpad( clen, 0 );
	if( ::crypto_box_open( ( unsigned char* ) &mpad.front(), ( unsigned char* ) &cpad.front(), clen,
                        (const unsigned char *) n.c_str(),
                        (const unsigned char *) pk.c_str(),
                        (const unsigned char *) sk.c_str()
                        ) != 0)
        throw "ciphertext fails verification";
    if (clen < crypto_box_ZEROBYTES)
        throw "ciphertext too short"; // should have been caught by _open
    return mpad.substr( crypto_box_ZEROBYTES );
}

string sodiumpp::crypto_hash(const string &m)
{
    unsigned char h[crypto_hash_BYTES];
    ::crypto_hash(h,(const unsigned char *) m.c_str(),m.size());
    return string((char *) h,sizeof h);
}

string sodiumpp::crypto_onetimeauth(const string &m,const string &k)
{
    if (k.size() != crypto_onetimeauth_KEYBYTES) throw "incorrect key length";
    unsigned char a[crypto_onetimeauth_BYTES];
    ::crypto_onetimeauth(a,(const unsigned char *) m.c_str(),m.size(),(const unsigned char *) k.c_str());
    return string((char *) a,crypto_onetimeauth_BYTES);
}

void sodiumpp::crypto_onetimeauth_verify(const string &a,const string &m,const string &k)
{
    if (k.size() != crypto_onetimeauth_KEYBYTES) throw "incorrect key length";
    if (a.size() != crypto_onetimeauth_BYTES) throw "incorrect authenticator length";
    if (::crypto_onetimeauth_verify(
                                  (const unsigned char *) a.c_str(),
                                  (const unsigned char *) m.c_str(),m.size(),
                                  (const unsigned char *) k.c_str()) == 0) return;
    throw "invalid authenticator";
}

string sodiumpp::crypto_scalarmult_base(const string &n)
{
    unsigned char q[crypto_scalarmult_BYTES];
    if (n.size() != crypto_scalarmult_SCALARBYTES) throw "incorrect scalar length";
    ::crypto_scalarmult_base(q,(const unsigned char *) n.c_str());
    return string((char *) q,sizeof q);
}

string sodiumpp::crypto_scalarmult(const string &n,const string &p)
{
    unsigned char q[crypto_scalarmult_BYTES];
    if (n.size() != crypto_scalarmult_SCALARBYTES) throw "incorrect scalar length";
    if (p.size() != crypto_scalarmult_BYTES) throw "incorrect element length";
    ::crypto_scalarmult(q,(const unsigned char *) n.c_str(),(const unsigned char *) p.c_str());
    return string((char *) q,sizeof q);
}

string sodiumpp::crypto_secretbox(const string &m,const string &n,const string &k)
{
    if (k.size() != crypto_secretbox_KEYBYTES) throw "incorrect key length";
    if (n.size() != crypto_secretbox_NONCEBYTES) throw "incorrect nonce length";
	size_t mlen = m.size() + crypto_secretbox_ZEROBYTES;
	std::string mpad( mlen, 0 );
	for( size_t i = crypto_secretbox_ZEROBYTES; i < mlen; ++i ) mpad[i] = m[i - crypto_secretbox_ZEROBYTES];
	std::string cpad( mlen, 0 );
    ::crypto_secretbox((unsigned char*)&cpad.front(),(const unsigned char*)mpad.c_str(),mlen,(const unsigned char *) n.c_str(),(const unsigned char *) k.c_str());
    return cpad.substr( crypto_secretbox_BOXZEROBYTES );
}

string sodiumpp::crypto_secretbox_open(const string &c,const string &n,const string &k)
{
    if (k.size() != crypto_secretbox_KEYBYTES) throw "incorrect key length";
    if (n.size() != crypto_secretbox_NONCEBYTES) throw "incorrect nonce length";
    size_t clen = c.size() + crypto_secretbox_BOXZEROBYTES;
	std::string cpad( clen, 0 );
	for( size_t i = crypto_secretbox_BOXZEROBYTES; i < clen; ++i ) cpad[i] = c[i - crypto_secretbox_BOXZEROBYTES];
	std::string mpad( clen, 0 );
    if (::crypto_secretbox_open((unsigned char*)&mpad.front(),(const unsigned char*)cpad.c_str(),clen,(const unsigned char *) n.c_str(),(const unsigned char *) k.c_str()) != 0)
        throw "ciphertext fails verification";
    if (clen < crypto_secretbox_ZEROBYTES)
        throw "ciphertext too short"; // should have been caught by _open
    return mpad.substr( crypto_secretbox_ZEROBYTES );
}

string sodiumpp::crypto_sign_keypair(string *sk_string)
{
    unsigned char pk[crypto_sign_PUBLICKEYBYTES];
    unsigned char sk[crypto_sign_SECRETKEYBYTES];
    ::crypto_sign_keypair(pk,sk);
    *sk_string = string((char *) sk,sizeof sk);
    return string((char *) pk,sizeof pk);
}

string sodiumpp::crypto_sign_open(const string &sm_string, const string &pk_string)
{
    if (pk_string.size() != crypto_sign_PUBLICKEYBYTES) throw "incorrect public-key length";
    size_t smlen = sm_string.size();
    std::string m(sm_string);
    unsigned long long mlen;
    if (::crypto_sign_open(
                         ( unsigned char* ) &m.front(),
                         &mlen,
						 ( const unsigned char* ) sm_string.c_str(),
                         smlen,
                         (const unsigned char *) pk_string.c_str()
                         ) != 0)
        throw "ciphertext fails verification";
	return m;
}

string sodiumpp::crypto_sign(const string &m_string, const string &sk_string)
{
    if (sk_string.size() != crypto_sign_SECRETKEYBYTES) throw "incorrect secret-key length";
    size_t mlen = m_string.size();
    std::string m(mlen+crypto_sign_BYTES, 0);
    unsigned long long smlen;
    for (size_t i = 0;i < mlen;++i) m[i] = m_string[i];
    ::crypto_sign(
                (unsigned char*)&m.front(),
                &smlen,
                (const unsigned char*)m_string.c_str(),
                mlen,
                (const unsigned char *) sk_string.c_str()
                );
    return m;
}

string sodiumpp::crypto_stream(size_t clen,const string &n,const string &k)
{
    if (n.size() != crypto_stream_NONCEBYTES) throw "incorrect nonce length";
    if (k.size() != crypto_stream_KEYBYTES) throw "incorrect key length";
    std::string c(clen, 0);
    ::crypto_stream((unsigned char*)&c.front(),clen,(const unsigned char *) n.c_str(),(const unsigned char *) k.c_str());
	return c;
}

string sodiumpp::crypto_stream_xor(const string &m,const string &n,const string &k)
{
    if (n.size() != crypto_stream_NONCEBYTES) throw "incorrect nonce length";
    if (k.size() != crypto_stream_KEYBYTES) throw "incorrect key length";
    size_t mlen = m.size();
    std::string c(mlen, 0);
    ::crypto_stream_xor((unsigned char*)&c.front(),
                      (const unsigned char *) m.c_str(),mlen,
                      (const unsigned char *) n.c_str(),
                      (const unsigned char *) k.c_str()
                      );
	return c;
}

string sodiumpp::bin2hex(const string& bytes) {
    std::string hex(bytes.size()*2, 0);
    sodium_bin2hex(&hex[0], hex.size(), reinterpret_cast<const unsigned char *>(&bytes[0]), bytes.size());
    return hex;
}

void sodiumpp::memzero(string& bytes) {
    sodium_memzero((unsigned char *)&bytes[0], bytes.size());
}

std::ostream& sodiumpp::operator<<(std::ostream& stream, const sodiumpp::public_key& pk) {
    return stream << "public_key(\"" << sodiumpp::bin2hex(pk.get()) << "\")";
}

std::ostream& sodiumpp::operator<<(std::ostream& stream, const sodiumpp::secret_key& sk) {
    return stream << sk.pk << ", secret_key(\"" << sodiumpp::bin2hex(sk.get()) << "\")";
}

