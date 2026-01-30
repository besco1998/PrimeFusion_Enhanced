#include <cbor.h>
#include <stdio.h>

int main() {
    unsigned char buffer[128];
    // Check for existence of streaming API functions
    size_t n = cbor_encode_uint8(10, buffer, 128);
    printf("Encoded %zu bytes\n", n);
    return 0;
}
