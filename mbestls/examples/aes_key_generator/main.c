#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#define ARRAY_SIZE(array)   ((sizeof(array))/(sizeof(array[0])))

int main(int argc, char const *argv[])
{
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    uint8_t key[16];

    char * pers = "aes generate key";
    int ret;

    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (uint8_t *)pers, strlen(pers))) != 0) {
        printf("failed\nmbedtls_ctr_drbg_init returned -0x%04x\n", -ret);
        goto exit;
    }

    if((ret = mbedtls_ctr_drbg_random(&ctr_drbg, key, ARRAY_SIZE(key))) != 0 ) {
        printf( " failed\n ! mbedtls_ctr_drbg_random returned -0x%04x\n", -ret );
        goto exit;
    }

    printf("Generated AES key is: ");
    for (size_t i = 0; i < ARRAY_SIZE(key); i++) {
        printf("%02X", key[i]);
    }
    printf("\n");

exit:
    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    return 0;
}
