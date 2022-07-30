#include "mul.c"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>

int test_1_limb_add() {
    limb_t ret, res, a, b;

    a = 0x03;
    b = 0x40;

    ret = add(&res, &a, &b, 1);

    assert(ret == 0);
    assert(res == 0x43);

    return 0;
}

int test_1_limb_mul() {
    limb_t ret[2];
    limb_t *tmp;
    limb_t a, b;
    a = 0x02;
    b = 0x05;

    tmp = malloc(sizeof(limb_t) * mul_limb_numb(1));

    mul(ret, &a, &b, 1, tmp);

    assert(ret[0] == 0);
    assert(ret[1] == 0x2 * 0x5);

    free(tmp);
    return 0;
}

int test_1_limb_mul_2_limb_res() {
    limb_t ret[2];
    limb_t *tmp;
    limb_t a, b;
    a = (limb_t)0x1 << (LIMB_BIT_SIZE-1);
    b = 0x2;

    tmp = malloc(sizeof(limb_t) * mul_limb_numb(1));

    mul(ret, &a, &b, 1, tmp);

    assert(ret[0] == 0x1);
    assert(ret[1] == 0x0);

    free(tmp);
    return 0;
}

int test_1_limb_mul_max_result() {
    limb_t ret[2];
    limb_t *tmp;
    limb_t a, b;
    a = -1;
    b = -1;

    tmp = malloc(sizeof(limb_t) * mul_limb_numb(1));

    mul(ret, &a, &b, 1, tmp);

    assert(ret[0] == -2);
    assert(ret[1] == 1);

    free(tmp);
    return 0;
}

int test_2_limb_mul() {
    limb_t ret[4];
    limb_t *tmp;
    limb_t a[2], b[2];
    a[0] = 0x0;
    a[1] = 0x2;
    b[0] = 0x0;
    b[1] = 0x5;

    tmp = malloc(sizeof(limb_t) * mul_limb_numb(2));

    mul(ret, a, b, 2, tmp);

    assert(ret[0] == 0);
    assert(ret[1] == 0);
    assert(ret[2] == 0);
    assert(ret[3] == 10);

    free(tmp);
    return 0;
}

int test_2_limb_mul_rand() {
    limb_t ret[4];
    limb_t *tmp;
    limb_t a[2], b[2];
    a[0] = 0xda392f6b22b65eafUL;
    a[1] = 0x7a12069f7ec00bbfUL;
    b[0] = 0x6de18e60a931614cUL;
    b[1] = 0xda56c84306050ad0UL;

    tmp = malloc(sizeof(limb_t) * mul_limb_numb(2));

    mul(ret, a, b, 2, tmp);

    assert(ret[0] == 0x5daa9ed168ea3639UL);
    assert(ret[1] == 0x17992fd9d977b294UL);
    assert(ret[2] == 0x1c4aafb01997e8d6UL);
    assert(ret[3] == 0x8e97e18e313a0130UL);

    free(tmp);
    return 0;
}

int test_2_limb_mul_2_limb_res() {
    limb_t ret[4];
    limb_t *tmp;
    limb_t a[2], b[2];
    a[0] = 0;
    a[1] = (limb_t)0x1 << (LIMB_BIT_SIZE-1);
    b[0] = 0;
    b[1] = 0x2;

    tmp = malloc(sizeof(limb_t) * mul_limb_numb(2));

    mul(ret, a, b, 2, tmp);

    assert(ret[0] == 0x0);
    assert(ret[1] == 0x0);
    assert(ret[2] == 0x1);
    assert(ret[3] == 0x0);

    free(tmp);
    return 0;
}

int test_2_limb_mul_2_limb_max_res() {
    limb_t ret[4];
    limb_t *tmp;
    limb_t a[2], b[2];
    a[0] = 0;
    a[1] = -1;
    b[0] = 0;
    b[1] = -1;

    tmp = malloc(sizeof(limb_t) * mul_limb_numb(2));

    mul(ret, a, b, 2, tmp);

    assert(ret[0] == 0x0);
    assert(ret[1] == 0x0);
    assert(ret[2] == -2);
    assert(ret[3] == 1);

    free(tmp);
    return 0;
}

int test_2_limb_mul_3_limb_res() {
    limb_t ret[4];
    limb_t *tmp;
    limb_t a[2], b[2];
    a[0] = 1;
    a[1] = 0;
    b[0] = 1;
    b[1] = 0;

    tmp = malloc(sizeof(limb_t) * mul_limb_numb(2));

    mul(ret, a, b, 2, tmp);

    assert(ret[0] == 0);
    assert(ret[1] == 1);
    assert(ret[2] == 0);
    assert(ret[3] == 0);

    free(tmp);
    return 0;
}

int test_2_limb_mul_4_limb_max_res() {
    limb_t ret[4];
    limb_t *tmp;
    limb_t a[2], b[2];
    a[0] = -1;
    a[1] = -1;
    b[0] = -1;
    b[1] = -1;

    tmp = malloc(sizeof(limb_t) * mul_limb_numb(2));

    mul(ret, a, b, 2, tmp);

    assert(ret[0] == -1);
    assert(ret[1] == -2);
    assert(ret[2] == 0);
    assert(ret[3] == 1);

    free(tmp);
    return 0;
}

int test_3_limb_mul_rand() {
    limb_t ret[6];
    limb_t *tmp;
    limb_t a[3], b[3];
    a[0] = 0xda392f6b22b65eafUL;
    a[1] = 0x7a12069f7ec00bbfUL;
    a[2] = 0x6331e7f4a737a85fUL;
    b[0] = 0x6de18e60a931614cUL;
    b[1] = 0xda56c84306050ad0UL;
    b[2] = 0xd2245e0461b76d36UL;

    tmp = malloc(sizeof(limb_t) * mul_limb_numb(3));

    mul(ret, a, b, 3, tmp);

    assert(ret[0] == 0x5daa9ed168ea3639UL);
    assert(ret[1] == 0x17992fd9d977b294UL);
    assert(ret[2] == 0xfa003e908ee5ef00UL);
    assert(ret[3] == 0xcd4c8d04868454ccUL);
    assert(ret[4] == 0x4196e7432916ed86UL);
    assert(ret[5] == 0x57973bdf5356f70aUL);

    free(tmp);
    return 0;
}


int test_4_limb_mul() {
    limb_t ret[8];
    limb_t *tmp;
    limb_t a[4], b[4];

    // random numbers, nothing specific about them
    a[0] = 0xefd0606aa4812a22UL;
    a[1] = 0xbafe80b783c74577UL;
    a[2] = 0x4ed173157dedb092UL;
    a[3] = 0x744cd5a499380cf4UL;

    b[0] = 0x833d3ac5ec82c171UL;
    b[1] = 0x9b8b8e1985f7ae1fUL;
    b[2] = 0xef91eba64bb6aa23UL;
    b[3] = 0x8fea83bc5c44a50cUL;

    tmp = malloc(sizeof(limb_t) * mul_limb_numb(4));

    mul(ret, a, b, 4, tmp);

    assert(ret[0] == 0x7af0fd0c2a583c96UL);
    assert(ret[1] == 0x76178cc3f4cebbe0UL);
    assert(ret[2] == 0xb5827651448c0c09UL);
    assert(ret[3] == 0x3fe552bb92d4f66fUL);
    assert(ret[4] == 0x17524119f7546b66UL);
    assert(ret[5] == 0x2b030ece3bfe7ebeUL);
    assert(ret[6] == 0x456c3383732b761fUL);
    assert(ret[7] == 0x36e851336fc9df70UL);

    free(tmp);
    return 0;
}


int main(int argc, char** argv) {
    test_1_limb_add();
    test_1_limb_mul();
    test_1_limb_mul_2_limb_res();
    test_1_limb_mul_max_result();

    test_2_limb_mul();
    test_2_limb_mul_rand();
    test_2_limb_mul_2_limb_res();
    test_2_limb_mul_2_limb_max_res();
    test_2_limb_mul_3_limb_res();
    test_2_limb_mul_4_limb_max_res();

    test_3_limb_mul_rand();

    test_4_limb_mul();
}
