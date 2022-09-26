#include "mul.c"
#include <inttypes.h>
#include <byteswap.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

enum operation {Add, Sub, Mul, Mod, Rshift1, Cselect};

void help(char *name) {
    printf("Usage: %s -i file -o file -n num [-a|-s|-m|-o]\n", name);
    printf("\n");
    printf(" -i file  input file\n");
    printf(" -o file  output file to write timing data to\n");
    printf(" -n num   size of input numbers in bytes (needs to be a "
           "multiple\n");
    printf("          of word size: 32bit or 64 bit)\n");
    printf(" -a       perform addition test (default)\n");
    printf(" -s       perform subtraction test\n");
    printf(" -m       perform multiplication test\n");
    printf(" -d       perform modulo operation test\n");
}

/* Convert a big endian binary integer to a list of limb_t integers
 */
void be_buf_to_limb_t(char *buf, limb_t *out, size_t nlimb) {
    for (size_t i=0; i<nlimb; i++) {
#if LIMB_BYTE_SIZE == 8
        out[i] = bswap_64(((limb_t*)buf)[i]);
#else
        out[i] = bswap_32(((limb_t*)buf)[i]);
#endif
    }
}

int time_sub(size_t numb, int in_file, int out_file) {
    char *buf=NULL;
    char prn_buf[1024];
    limb_t *a=NULL, *b=NULL;
    limb_t *diff=NULL, *exp_diff=NULL;
    int ret = 0;
    ssize_t r_ret;
    size_t limb_count = numb / LIMB_BYTE_SIZE;

    uint32_t time_before_high = 0, time_before_low = 0, time_after_high = 0,
             time_after_low = 0;

    r_ret = write(out_file, "sub_times\n", 10);
    if (r_ret != 10) {
        fprintf(stderr, "Writing header to output file failed\n");
        ret = 1;
        goto fail;
    }

    buf = malloc(sizeof(char) * numb);
    if (buf == NULL) {
        fprintf(stderr, "malloc fail (buf)\n");
        ret = 1;
        goto fail;
    }
    a = malloc(sizeof(limb_t) * limb_count);
    if (a == NULL) {
        fprintf(stderr, "malloc fail (a)\n");
        ret = 1;
        goto fail;
    }
    b = malloc(sizeof(limb_t) * limb_count);
    if (b == NULL) {
        fprintf(stderr, "malloc fail (b)\n");
        ret = 1;
        goto fail;
    }
    diff = malloc(sizeof(limb_t) * limb_count);
    if (diff == NULL) {
        fprintf(stderr, "malloc fail (diff)\n");
        ret = 1;
        goto fail;
    }
    exp_diff = malloc(sizeof(limb_t) * limb_count);
    if (exp_diff == NULL) {
        fprintf(stderr, "malloc fail (exp_diff)\n");
        ret = 1;
        goto fail;
    }

    while ((r_ret = read(in_file, buf, numb)) > 0) {
        if (r_ret != numb) {
            fprintf(stderr, "read less data than expected (truncated file?)\n");
            ret = 1;
            goto fail;
        }
        be_buf_to_limb_t(buf, a, limb_count);

        r_ret = read(in_file, buf, numb);
        if (r_ret <= 0) {
            fprintf(stderr, "can't read (b) value from input file (truncated "
                    "file?)\n");
            ret = 1;
            goto fail;
        }
        be_buf_to_limb_t(buf, b, limb_count);

        r_ret = read(in_file, buf, numb);
        if (r_ret <= 0) {
            fprintf(stderr, "can't read (exp_diff) value from input file "
                    "(truncated file?)\n");
            ret = 1;
            goto fail;
        }
        be_buf_to_limb_t(buf, exp_diff, limb_count);

        asm volatile (
            "CPUID\n\t"
            "RDTSC\n\t"
            "mov %%edx, %0\n\t"
            "mov %%eax, %1\n\t" : "=r" (time_before_high),
            "=r" (time_before_low)::
            "%rax", "%rbx", "%rcx", "%rdx");

        sub(diff, a, b, limb_count);

        asm volatile (
            "RDTSCP\n\t"
            "mov %%edx, %0\n\t"
            "mov %%eax, %1\n\t"
            "CPUID\n\t": "=r" (time_after_high),
            "=r" (time_after_low)::
            "%rax", "%rbx", "%rcx", "%rdx");

        if (memcmp(diff, exp_diff, numb) != 0) {
            fprintf(stderr, "sub() result incorrect\n");
            printf("a: ");
            for (size_t j=0; j<limb_count; j++) {
                printf("%016" PRIx64, a[j]);
            }
            printf("\n");
            printf("b: ");
            for (size_t j=0; j<limb_count; j++) {
                printf("%016" PRIx64, b[j]);
            }
            printf("\n");
            printf("diff: ");
            for (size_t j=0; j<limb_count; j++) {
                printf("%016" PRIx64, diff[j]);
            }
            printf("\n");
            printf("exp_diff: ");
            for (size_t j=0; j<limb_count; j++) {
                printf("%016" PRIx64, exp_diff[j]);
            }
            printf("\n");
            ret = 1;
            goto fail;
        }

        r_ret = snprintf(prn_buf, 1024, "%d\n",
            (uint32_t)(((uint64_t)time_after_high<<32 | time_after_low)-
            ((uint64_t)time_before_high<<32 | time_before_low)));
        if (r_ret >= 1024) {
            fprintf(stderr, "Unexpected snprintf output\n");
            ret = 1;
            goto fail;
        }
        r_ret = write(out_file, prn_buf, r_ret);
        if (r_ret <= 0) {
            fprintf(stderr, "Write error\n");
            ret = 1;
            goto fail;
        }
    }

    if (r_ret < 0) {
        fprintf(stderr, "Reading from input file failed\n");
        ret = 1;
        goto fail;
    }

fail:
    free(buf);
    free(a);
    free(b);
    free(diff);
    free(exp_diff);
    return ret;
}


int time_add(size_t numb, int in_file, int out_file) {
    char *buf=NULL;
    char prn_buf[1024];
    limb_t *a=NULL, *b=NULL;
    limb_t *sum=NULL, *exp_sum=NULL;
    int ret = 0;
    ssize_t r_ret;
    size_t limb_count = numb / LIMB_BYTE_SIZE;

    uint32_t time_before_high = 0, time_before_low = 0, time_after_high = 0,
             time_after_low = 0;

    r_ret = write(out_file, "add_times\n", 10);
    if (r_ret != 10) {
        fprintf(stderr, "Writing header to output file failed\n");
        ret = 1;
        goto fail;
    }

    buf = malloc(sizeof(char) * numb);
    if (buf == NULL) {
        fprintf(stderr, "malloc fail (buf)\n");
        ret = 1;
        goto fail;
    }
    a = malloc(sizeof(limb_t) * limb_count);
    if (a == NULL) {
        fprintf(stderr, "malloc fail (a)\n");
        ret = 1;
        goto fail;
    }
    b = malloc(sizeof(limb_t) * limb_count);
    if (b == NULL) {
        fprintf(stderr, "malloc fail (b)\n");
        ret = 1;
        goto fail;
    }
    sum = malloc(sizeof(limb_t) * limb_count);
    if (sum == NULL) {
        fprintf(stderr, "malloc fail (sum)\n");
        ret = 1;
        goto fail;
    }
    exp_sum = malloc(sizeof(limb_t) * limb_count);
    if (exp_sum == NULL) {
        fprintf(stderr, "malloc fail (exp_sum)\n");
        ret = 1;
        goto fail;
    }

    while ((r_ret = read(in_file, buf, numb)) > 0) {
        if (r_ret != numb) {
            fprintf(stderr, "read less data than expected (truncated file?)\n");
            ret = 1;
            goto fail;
        }
        be_buf_to_limb_t(buf, a, limb_count);

        r_ret = read(in_file, buf, numb);
        if (r_ret <= 0) {
            fprintf(stderr, "can't read (b) value from input file (truncated "
                    "file?)\n");
            ret = 1;
            goto fail;
        }
        be_buf_to_limb_t(buf, b, limb_count);

        r_ret = read(in_file, buf, numb);
        if (r_ret <= 0) {
            fprintf(stderr, "can't read (exp_sum) value from input file "
                    "(truncated file?)\n");
            ret = 1;
            goto fail;
        }
        be_buf_to_limb_t(buf, exp_sum, limb_count);

        asm volatile (
            "CPUID\n\t"
            "RDTSC\n\t"
            "mov %%edx, %0\n\t"
            "mov %%eax, %1\n\t" : "=r" (time_before_high),
            "=r" (time_before_low)::
            "%rax", "%rbx", "%rcx", "%rdx");

        add(sum, a, b, limb_count);

        asm volatile (
            "RDTSCP\n\t"
            "mov %%edx, %0\n\t"
            "mov %%eax, %1\n\t"
            "CPUID\n\t": "=r" (time_after_high),
            "=r" (time_after_low)::
            "%rax", "%rbx", "%rcx", "%rdx");

        if (memcmp(sum, exp_sum, numb) != 0) {
            fprintf(stderr, "add() result incorrect\n");
            ret = 1;
            goto fail;
        }

        r_ret = snprintf(prn_buf, 1024, "%d\n",
            (uint32_t)(((uint64_t)time_after_high<<32 | time_after_low)-
            ((uint64_t)time_before_high<<32 | time_before_low)));
        if (r_ret >= 1024) {
            fprintf(stderr, "Unexpected snprintf output\n");
            ret = 1;
            goto fail;
        }
        r_ret = write(out_file, prn_buf, r_ret);
        if (r_ret <= 0) {
            fprintf(stderr, "Write error\n");
            ret = 1;
            goto fail;
        }
    }

    if (r_ret < 0) {
        fprintf(stderr, "Reading from input file failed\n");
        ret = 1;
        goto fail;
    }

fail:
    free(buf);
    free(a);
    free(b);
    free(sum);
    free(exp_sum);
    return ret;
}

int time_mul(size_t numb, int in_file, int out_file) {
    char *buf=NULL;
    char prn_buf[1024];
    limb_t *a=NULL, *b=NULL;
    limb_t *prod=NULL, *exp_prod=NULL;
    limb_t *tmp=NULL;
    int ret = 0;
    ssize_t r_ret;
    size_t limb_count = numb / LIMB_BYTE_SIZE;

    uint32_t time_before_high = 0, time_before_low = 0, time_after_high = 0,
             time_after_low = 0;

    r_ret = write(out_file, "mul_times\n", 10);
    if (r_ret != 10) {
        fprintf(stderr, "Writing header to output file failed\n");
        ret = 1;
        goto fail;
    }

    buf = malloc(sizeof(char) * numb * 2);
    if (buf == NULL) {
        fprintf(stderr, "malloc fail (buf)\n");
        ret = 1;
        goto fail;
    }
    a = malloc(sizeof(limb_t) * limb_count);
    if (a == NULL) {
        fprintf(stderr, "malloc fail (a)\n");
        ret = 1;
        goto fail;
    }
    b = malloc(sizeof(limb_t) * limb_count);
    if (b == NULL) {
        fprintf(stderr, "malloc fail (b)\n");
        ret = 1;
        goto fail;
    }
    prod = malloc(sizeof(limb_t) * limb_count * 2);
    if (prod == NULL) {
        fprintf(stderr, "malloc fail (prod)\n");
        ret = 1;
        goto fail;
    }
    exp_prod = malloc(sizeof(limb_t) * limb_count * 2);
    if (exp_prod == NULL) {
        fprintf(stderr, "malloc fail (exp_prod)\n");
        ret = 1;
        goto fail;
    }
    tmp = malloc(sizeof(limb_t) * mul_limb_numb(limb_count));
    if (tmp == NULL) {
        fprintf(stderr, "malloc fail (tmp)\n");
        ret = 1;
        goto fail;
    }

    while ((r_ret = read(in_file, buf, numb)) > 0) {
        if (r_ret != numb) {
            fprintf(stderr, "read less data than expected (truncated file?)\n");
            ret = 1;
            goto fail;
        }
        be_buf_to_limb_t(buf, a, limb_count);

        r_ret = read(in_file, buf, numb);
        if (r_ret <= 0) {
            fprintf(stderr, "can't read (b) value from input file (truncated "
                    "file?)\n");
            ret = 1;
            goto fail;
        }
        be_buf_to_limb_t(buf, b, limb_count);

        r_ret = read(in_file, buf, numb * 2);
        if (r_ret <= 0) {
            fprintf(stderr, "can't read (exp_prod) value from input file "
                    "(truncated file?)\n");
            ret = 1;
            goto fail;
        }
        be_buf_to_limb_t(buf, exp_prod, limb_count * 2);

        asm volatile (
            "CPUID\n\t"
            "RDTSC\n\t"
            "mov %%edx, %0\n\t"
            "mov %%eax, %1\n\t" : "=r" (time_before_high),
            "=r" (time_before_low)::
            "%rax", "%rbx", "%rcx", "%rdx");

        mul(prod, a, b, limb_count, tmp);

        asm volatile (
            "RDTSCP\n\t"
            "mov %%edx, %0\n\t"
            "mov %%eax, %1\n\t"
            "CPUID\n\t": "=r" (time_after_high),
            "=r" (time_after_low)::
            "%rax", "%rbx", "%rcx", "%rdx");

        if (memcmp(prod, exp_prod, numb) != 0) {
            fprintf(stderr, "mul() result incorrect\n");
            ret = 1;
            goto fail;
        }

        r_ret = snprintf(prn_buf, 1024, "%d\n",
            (uint32_t)(((uint64_t)time_after_high<<32 | time_after_low)-
            ((uint64_t)time_before_high<<32 | time_before_low)));
        if (r_ret >= 1024) {
            fprintf(stderr, "Unexpected snprintf output\n");
            ret = 1;
            goto fail;
        }
        r_ret = write(out_file, prn_buf, r_ret);
        if (r_ret <= 0) {
            fprintf(stderr, "Write error\n");
            ret = 1;
            goto fail;
        }
    }

    if (r_ret < 0) {
        fprintf(stderr, "Reading from input file failed\n");
        ret = 1;
        goto fail;
    }

fail:
    free(buf);
    free(a);
    free(b);
    free(prod);
    free(exp_prod);
    free(tmp);
    return ret;
}


int main(int argc, char** argv) {
    int ret = 0;
    int opt;
    char *in_name = NULL;
    char *out_name = NULL;
    int in_file, out_file;
    size_t numb = 0;
    enum operation oper = Add;

    while ((opt = getopt(argc, argv, "i:o:n:asmdh")) != -1) {
        switch(opt) {
            case 'i':
                in_name = optarg;
                break;
            case 'o':
                out_name = optarg;
                break;
            case 'n':
                sscanf(optarg, "%zi", &numb);
                break;
            case 'a':
                oper = Add;
                break;
            case 's':
                oper = Sub;
                break;
            case 'm':
                oper = Mul;
                break;
            case 'd':
                oper = Mod;
                break;
            case 'h':
                help(argv[0]);
                exit(0);
                break;
            default:
                fprintf(stderr, "Unknown option: %c\n", opt);
                help(argv[0]);
                exit(1);
                break;
        }
    }

    if (in_name == NULL || out_name == NULL || numb == 0) {
        fprintf(stderr, "Input file name, output file name and input size must"
                " be specified.\n");
        help(argv[0]);
        exit(1);
    }

    in_file = open(in_name, O_RDONLY);
    if (in_file == -1) {
        fprintf(stderr, "Can't open input file %s\n", in_name);
        exit(1);
    }

    out_file = open(out_name, O_WRONLY|O_TRUNC|O_CREAT, 0666);
    if (out_file == -1) {
        fprintf(stderr, "Can't open output file %s\n", out_name);
        exit(1);
    }

    switch(oper) {
        case Add:
            ret = time_add(numb, in_file, out_file);
            break;
        case Sub:
            ret = time_sub(numb, in_file, out_file);
            break;
        case Mul:
            ret = time_mul(numb, in_file, out_file);
            break;
        default:
            fprintf(stderr, "Unknown operation: %d\n", oper);
            exit(1);
    }

    if (close(out_file) == -1) {
        fprintf(stderr, "Can't close output file\n");
        exit(1);
    }
    if (close(in_file) == -1) {
        fprintf(stderr, "Can't close input file\n");
        exit(1);
    }

    exit(ret);
}
