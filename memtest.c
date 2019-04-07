#include "mem.c"
#include <stdio.h>

int test_num = 1, tfail = 0, tpass = 0;
static inline void fail(void) { printf("FAIL\n"); ++tfail; }
static inline void pass(void) { printf("PASS\n"); ++tpass; }
static inline void test(const char* s) { printf("Test %2d: %-30s ", test_num++, s); }
static inline void summary(void) { printf("\nTests passed: %d\nTests failed: %d\n\n", tpass, tfail); }
static inline void reset(void) { test_num = 1; tfail = tpass = 0; printf("============================================\n"); }

// light-weight white box testing
int main(void) {

    // test 1 - without coalescing adjacent free memory chunks
    reset();
    long long offset = (long long) Mem_Init(80000);

    test("zero allocation");
    ( Mem_Alloc(0) == NULL ) ? pass() : fail();

    test("negative allocation");
    ( Mem_Alloc(-1) == NULL ) ? pass() : fail();

    test("null free");
    ( Mem_Free(NULL, 0) == 0 ) ? pass() : fail();

    // successful allocations
    int succ_allocs[] = { 4000, 8000, 16000, 32000, 1 };
    int succ_allocs_sum[] = { 0, 4000, 12000, 28000, 60000 };
    void *s_ptr[4];
    for ( int i = 0; i < 5; ++i ) {
        test("successful allocation");
        s_ptr[i] = Mem_Alloc(succ_allocs[i]);
        ( (long long) s_ptr[i] == offset + sizeof(malloc_node)*(i+1) + succ_allocs_sum[i] ) ? pass() : fail();
    }

    // failing allocations
    int fail_allocs[] = { 50000, 21841 };
    for ( int i = 0; i < 2; ++i ) {
        test("failing allocation");
        ( Mem_Alloc(fail_allocs[i]) == NULL ) ? pass() : fail();
    }
    
    // de-allocations w/o coalesce
    for ( int i = 0; i < 5; ++i ) {
        test("deallocation");
        ( Mem_Free(s_ptr[i], 0) == 0 ) ? pass() : fail();
    }
    
    // failing allocation
    test("failing allocation");
    ( Mem_Alloc(50000) == NULL ) ? pass() : fail();

    summary();
    Mem_Dump();

    
    // test 2 - with coalescing of adjacent free memory chunks
    reset();
    offset = (long long) Mem_Init(8000);

    test("1st failing allocation");
    ( Mem_Alloc(10000) == NULL ) ? pass() : fail();

    test("1st successful allocation");
    void *p0 = Mem_Alloc(2048);
    ( (long long) p0 == offset + sizeof(malloc_node) ) ? pass() : fail();
    
    test("2nd successful allocation");
    void *p1 = Mem_Alloc(1024);
    ( (long long) p1 == offset + 2*sizeof(malloc_node) + 2048 ) ? pass() : fail();
    
    test("3rd successful allocation");
    void *p2 = Mem_Alloc(1024);
    ( (long long) p2 == offset + 3*sizeof(malloc_node) + 2048 + 1024 ) ? pass() : fail();

    test("2nd failing allocation");
    ( Mem_Alloc(5000) == NULL ) ? pass() : fail();

    test("1st deallocation");
    ( Mem_Free(p2, 1) == 0 ) ? pass() : fail();

    test("4th successful allocation");
    p2 = Mem_Alloc(1024);
    ( (long long) p2 == offset + 3*sizeof(malloc_node) + 2048 + 1024 ) ? pass() : fail();

    test("2nd deallocation");
    ( Mem_Free(p1, 1) == 0 ) ? pass() : fail();

    test("5th successful allocation");
    void *p3 = Mem_Alloc(976);
    ( (long long) p3 == offset + 2*sizeof(malloc_node) + 2048 ) ? pass() : fail();

    test("6th successful allocation");
    void *p4 = Mem_Alloc(1);
    ( (long long) p4 == offset + 3*sizeof(malloc_node) + 2048 + 976 ) ? pass() : fail();

    test("3rd deallocation");
    ( Mem_Free(p4, 1) == 0 ) ? pass() : fail();

    test("7th successful allocation");
    p4 = Mem_Alloc(32);
    ( (long long) p4 == offset + 3*sizeof(malloc_node) + 2048 + 976 ) ? pass() : fail();
    
    test("8th successful allocation");
    void *p5 = Mem_Alloc(4016);
    ( (long long) p5 == offset + 5*sizeof(malloc_node) + 2048 + 976 + 32 + 1024 ) ? pass() : fail();

    test("3rd failing allocation");
    ( Mem_Alloc(1) == NULL ) ? pass() : fail();

    test("4th deallocation");
    ( Mem_Free(p5, 1) == 0 ) ? pass() : fail();

    test("9th successful allocation");
    p5 = Mem_Alloc(4017);
    ( (long long) p5 == offset + 5*sizeof(malloc_node) + 2048 + 976 + 32 + 1024 ) ? pass() : fail();

    test("5th deallocation");
    ( Mem_Free(p5, 1) == 0 ) ? pass() : fail();

    test("10th successful allocation");
    p5 = Mem_Alloc(4032);
    ( (long long) p5 == offset + 5*sizeof(malloc_node) + 2048 + 976 + 32 + 1024 ) ? pass() : fail();

    test("6th deallocation");
    ( Mem_Free(p4, 1) == 0 ) ? pass() : fail();
    test("7th deallocation");
    ( Mem_Free(p0, 1) == 0 ) ? pass() : fail();
    test("8th deallocation");
    ( Mem_Free(p5, 1) == 0 ) ? pass() : fail();
    test("9th deallocation");
    ( Mem_Free(p3, 1) == 0 ) ? pass() : fail();
    test("10th deallocation");
    ( Mem_Free(p2, 1) == 0 ) ? pass() : fail();

    test("11th successful allocation");
    p0 = Mem_Alloc(8176);
    ( (long long) p0 == offset + sizeof(malloc_node) ) ? pass() : fail();
    
    test("11th deallocation");
    ( Mem_Free(p0, 1) == 0 ) ? pass() : fail();
    
    test("Only 1 free memory chunk");
    ( (long long) first_free == offset && first_free->size >= 8000 - sizeof(free_node) ) ? pass() : fail();
    
    summary();
    Mem_Dump();

    return 0;
}
