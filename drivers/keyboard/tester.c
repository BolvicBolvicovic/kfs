#include "keyboard.h"

#define ASSERT(msg, cond) do { \
    if (!(cond)) { \
        printf("ASSERT FAIL: %s\n", msg); \
        return -1; \
    } \
} while(0)

#define MAX_TEST_PTRS 1024

extern void*	kmalloc(size_t s);

typedef struct
{
    void* ptr;
    size_t req_size;
    size_t size_type; /* what kget_size should say */
    uint32_t pattern;
} alloc_rec_t;

static alloc_rec_t recs[MAX_TEST_PTRS];

static uint32_t
pattern_for_idx(int idx)
{
    /* produce distinct patterns */
    uint32_t p = 0xA5A50000 ^ ((uint32_t)idx * 0x9E3779B1);
    return p | ((p >> 16) & 0xFFFF);
}

static void
fill_pattern(void* p, size_t size, uint32_t pattern)
{
    uint32_t *w = (uint32_t*)p;
    size_t n = size / 4;
    for (size_t i = 0; i < n; ++i) w[i] = pattern ^ (uint32_t)i;
    /* tail bytes */
    uint8_t *b = (uint8_t*)p + n*4;
    for (size_t i = (n*4); i < size; ++i) b[i - n*4] = (uint8_t)pattern;
}

static int
check_pattern(void* p, size_t size, uint32_t pattern)
{
    uint32_t *w = (uint32_t*)p;
    size_t n = size / 4;
    for (size_t i = 0; i < n; ++i) if (w[i] != (pattern ^ (uint32_t)i)) return 0;
    uint8_t *b = (uint8_t*)p + n*4;
    for (size_t i = (n*4); i < size; ++i) if (b[i - n*4] != (uint8_t)pattern) return 0;
    return 1;
}

/* Test 1: small allocations + kget_size + uniqueness */
static int
test_binning_basic(void)
{
    printf("TEST: bining basic\n");
    int used = 0;
    size_t sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048};
    for (size_t si = 0; si < sizeof(sizes)/sizeof(sizes[0]); ++si)
	{
        void* p = kmalloc(sizes[si]);
        ASSERT("bining returned NULL", p != NULL);
        uint32_t ks = kget_size(p);
        ASSERT("kget_size smaller than requested", ks >= sizes[si]);
        /* store */
        recs[used].ptr = p;
        recs[used].req_size = sizes[si];
        recs[used].size_type = ks;
        recs[used].pattern = pattern_for_idx(used);
        fill_pattern(p, sizes[si], recs[used].pattern);
        used++;
    }
    /* uniqueness: no overlap check by comparing addresses */
    for (int i = 0; i < used; ++i) for (int j = i+1; j < used; ++j)
        ASSERT("bining overlap", recs[i].ptr != recs[j].ptr);
    printf("bining basic OK\n");
    return 0;
}

/* Test 2: large allocations (continuous) */
static int
test_continuous_basic(void)
{
    printf("TEST: continuous basic\n");
    void* p1 = kmalloc(8192); /* 2 pages */
    ASSERT("continuous p1 null", p1 != NULL);
    ASSERT("continuous page aligned", ((uintptr_t)p1 & 0xFFF) == 0);
    uint32_t ks = kget_size(p1);
    ASSERT("kget_size multi-page", ks >= 8192);
    /* fill and check */
    uint32_t pat = 0xDEADBEEF;
    fill_pattern(p1, 8192, pat);
    ASSERT("pattern ok", check_pattern(p1, 8192, pat));
    kfree(p1);
    printf("continuous basic OK\n");
    return 0;
}

/* Test 3: invalid free */
static int
test_invalid_free(void)
{
    printf("TEST: invalid free\n");
    /* Pass some pointer that was not allocated */
    void* bogus = (void*)0x12345000;
    /* Should print "ERROR: Invalid free" — but we check no crash */
    kfree(bogus);
    printf("invalid free done (check log)\n");
    return 0;
}

/* main test runner */
void
tests_memory(int* total, int* success, int* failure)
{
    if (test_binning_basic()) return;
    if (test_continuous_basic()) return;
    if (test_invalid_free()) return;
    printf("ALL kmalloc tests passed\n");
}

void
tests_string(int* total, int* success, int* failure)
{
    printf("\ntests for string.h\n");
    char tester[] = "TESter";
    *total += 10;
    
	if (strlen(tester) == 6)
	{
		*success += 1;
		printf("strlen : success\n");
    }
	else
	{
		*failure += 1;
		printf("strlen : failure\n");
    }

    if (strlen("") == 0)
	{
		*success += 1;
		printf("strlen : success\n");
    }
	else
	{
		*failure += 1;
		printf("strlen : failure\n");
    }

    if (strcmp(tester, "TESter") == 0)
	{
		*success += 1;
		printf("strcmp : success\n");
    }
	else
	{
		*failure += 1;
		printf("strcmp : failure\n");
    }

    if (strcmp(tester, "tes") != 0)
	{
		*success += 1;
		printf("strcmp : success\n");
    }
	else
	{
		*failure += 1;
		printf("strcmp : failure\n");
    }

    if (strcmp(tester, "TESterrrr") != 0)
	{
		*success += 1;
		printf("strcmp : success\n");
    }
	else
	{
		*failure += 1;
		printf("strcmp : failure\n");
    }

    if (strchr(tester, 'a') == 0)
	{
		*success += 1;
		printf("strchr : success\n");
    }
	else
	{
		*failure += 1;
		printf("strchr : failure\n");
    }

    if (strchr(tester, 'e') == tester + 4)
	{
		*success += 1;
		printf("strchr : success\n");
    }
	else
	{
		*failure += 1;
		printf("strchr : failure\n");
    }

    char copy[8] = {0};
    memcpy(copy, tester, 3);
    if (strcmp(copy, "TES") == 0)
	{
		*success += 1;
		printf("memcpy : success\n");
    }
	else
	{
		*failure += 1;
		printf("memcpy : failure | expected copy == \"TES\", got %s\n", copy);
    }

    strcpy(copy, tester);
    if (strcmp(copy, tester) == 0)
	{
		*success += 1;
		printf("strcpy : success\n");
    }
	else
	{
		*failure += 1;
		printf("strcpy : failure\n");
    }

    memset(copy, 'c', 7);
    if (strcmp(copy, "ccccccc") == 0)
	{
		*success += 1;
		printf("memset : success\n");
    }
	else
	{
		*failure += 1;
		printf("memset : failure\n");
    }
}

void
tests_stdlib(int* total, int* success, int* failure)
{
    printf("\ntests for stdlib.h\n");
    *total += 1;

    if (atoi("5") == 5 && atoi("-5") == -5 && atoi("zda") == 0)
	{
		*success += 1;
		printf("atoi   : success\n");
    }
	else
	{
		*failure += 1;
		printf("atoi   : failure atoi(5) == %d | atoi(-5) == %d | atoi('zda') == %d\n", atoi("5"), atoi("-5"), atoi("zda"));
    }
}
