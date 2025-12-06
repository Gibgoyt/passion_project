#include "boringssl_prefix_symbols.h"
#include <openssl/ssl.h>

int main() {
    SSL_CTX *ctx = SSL_CTX_new(TLS_method());
    return 0;
}