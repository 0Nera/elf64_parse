#include <stdio.h>
#include <stdint.h>

int test_int = 100;
extern void *_init;
extern void *_start;
void *lolkek_228;

int main(int argc, char **argv) {
    printf("virt | main     | 0x%lX\n", (uint64_t)&main);
    printf("virt | _init    | 0x%lX\n", (uint64_t)&_init);
    printf("virt | _start   | 0x%lX\n", (uint64_t)&_start);
    printf("virt | test_int | 0x%lX\n", (uint64_t)&test_int);
    printf("phys | main     | 0x%lX\n", (uint64_t)&main - (uint64_t)&_init + 0x1000);
    printf("phys | _start   | 0x%lX\n", (uint64_t)&_start - (uint64_t)&_init + 0x1000);
    printf("phys | test_int | 0x%lX\n", (uint64_t)&test_int - (uint64_t)&_init + 0x1000);
 
    return 0;
}