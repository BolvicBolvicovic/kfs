#include "keyboard.h"

#define ASSERT(msg, cond) do { \
    if (!(cond)) { \
        printf("ASSERT FAIL: %s\n", msg); \
        return -1; \
    } \
} while(0)

#define TEST_ASSERT(msg, cond, total, success, failure) do { \
    (*total)++; \
    if (cond) { \
        (*success)++; \
        printf("%s: success\n", msg); \
    } else { \
        (*failure)++; \
        printf("%s: failure\n", msg); \
        return -1; \
    } \
} while(0)

typedef uint32_t pid_t;
extern pid_t	create_process(void (*entry)(void));
extern void		yield();

static void
process_a(void)
{
	printf("Process A starting\n");
	for (size_t i = 0; i < 5; i++)  // Reduced iterations for testing
	{
		printf("a");
		yield();
	}
	printf("Process A ending\n");
}

static void
process_b(void)
{
	printf("Process B starting\n");
	for (size_t i = 0; i < 5; i++)  // Reduced iterations for testing
	{
		printf("b");
		yield();
	}
	printf("Process B ending\n");
}

void
tests_processes(int* total, int* success, int* failure)
{
	printf("Creating processes...\n");
	pid_t	a = create_process(process_a);
	pid_t	b = create_process(process_b);
	
	*total += 2; // Two process creation tests
	
	if (a > 0)
	{
		(*success)++;
		printf("Process A created successfully (PID: %d)\n", a);
	}
	else
	{
		(*failure)++;
		printf("Process A creation failed\n");
		return;
	}
	
	if (b > 0)
	{
		(*success)++;
		printf("Process B created successfully (PID: %d)\n", b);
	}
	else
	{
		(*failure)++;
		printf("Process B creation failed\n");
		return;
	}
	
	printf("Starting process execution with multiple yields...\n");
	// Call yield multiple times to let processes run
	for (int i = 0; i < 20; i++)
	{
		printf("[yield %d] ", i);
		yield();
	}
	printf("\nProcesses execution completed.\n");
}


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
test_binning_basic(int* total, int* success, int* failure)
{
    printf("TEST: binning basic\n");
    int used = 0;
    size_t sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048};
    int num_sizes = sizeof(sizes)/sizeof(sizes[0]);
    
    *total += num_sizes + 1; // num_sizes allocations + 1 uniqueness test
    
    for (size_t si = 0; si < num_sizes; ++si)
	{
        void* p = kmalloc(sizes[si]);
        if (p != NULL)
        {
            uint32_t ks = kget_size(p);
            if (ks >= sizes[si])
            {
                (*success)++;
                printf("binning allocation size %zu: success\n", sizes[si]);
            }
            else
            {
                (*failure)++;
                printf("binning allocation size %zu: failure (kget_size too small)\n", sizes[si]);
                return -1;
            }
            /* store */
            recs[used].ptr = p;
            recs[used].req_size = sizes[si];
            recs[used].size_type = ks;
            recs[used].pattern = pattern_for_idx(used);
            fill_pattern(p, sizes[si], recs[used].pattern);
            used++;
        }
        else
        {
            (*failure)++;
            printf("binning allocation size %zu: failure (returned NULL)\n", sizes[si]);
            return -1;
        }
    }
    
    /* uniqueness: no overlap check by comparing addresses */
    int overlap_found = 0;
    for (int i = 0; i < used && !overlap_found; ++i)
    {
        for (int j = i+1; j < used; ++j)
        {
            if (recs[i].ptr == recs[j].ptr)
            {
                overlap_found = 1;
                break;
            }
        }
    }
    
    if (!overlap_found)
    {
        (*success)++;
        printf("binning uniqueness: success\n");
    }
    else
    {
        (*failure)++;
        printf("binning uniqueness: failure (overlap detected)\n");
        return -1;
    }
    
    printf("binning basic OK\n");
    return 0;
}

/* Test 2: large allocations (continuous) */
static int
test_continuous_basic(int* total, int* success, int* failure)
{
    printf("TEST: continuous basic\n");
    *total += 3; // 3 assertions
    
    void* p1 = kmalloc(8192); /* 2 pages */
    if (p1 != NULL)
    {
        (*success)++;
        printf("continuous allocation: success\n");
    }
    else
    {
        (*failure)++;
        printf("continuous allocation: failure (returned NULL)\n");
        return -1;
    }
    
    if (((uintptr_t)p1 & 0xFFF) == 0)
    {
        (*success)++;
        printf("continuous page alignment: success\n");
    }
    else
    {
        (*failure)++;
        printf("continuous page alignment: failure\n");
        kfree(p1);
        return -1;
    }
    
    uint32_t ks = kget_size(p1);
    if
    (ks >= 8192)
    {
        (*success)++;
        printf("continuous kget_size: success\n");
    }
    else
    {
        (*failure)++;
        printf("continuous kget_size: failure\n");
        kfree(p1);
        return -1;
    }
    
    /* fill and check */
    uint32_t pat = 0xDEADBEEF;
    fill_pattern(p1, 8192, pat);
    if (check_pattern(p1, 8192, pat))
    {
        printf("continuous pattern check: success\n");
    }
    else
    {
        printf("continuous pattern check: failure\n");
        kfree(p1);
        return -1;
    }
    
    kfree(p1);
    printf("continuous basic OK\n");
    return 0;
}

/* Test 3: invalid free */
static int
test_invalid_free(int* total, int* success, int* failure)
{
    printf("TEST: invalid free\n");
    *total += 1;
    
    /* Pass some pointer that was not allocated */
    void* bogus = (void*)0x12345000;
    /* Should print "ERROR: Invalid free" — but we check no crash */
    kfree(bogus);
    
    /* If we reach here without crashing, consider it a success */
    (*success)++;
    printf("invalid free: success (no crash)\n");
    return 0;
}

/* main test runner */
void
tests_memory(int* total, int* success, int* failure)
{
    if (test_binning_basic(total, success, failure)) return;
    if (test_continuous_basic(total, success, failure)) return;
    if (test_invalid_free(total, success, failure)) return;
    printf("ALL kmalloc tests passed\n");
}

void
tests_string(int* total, int* success, int* failure)
{
    printf("\ntests for string.h\n");
    char tester[] = "TESter";
    *total += 10;
    
    // Test strlen
	if (strlen(tester) == 6)
    {
		(*success)++;
		printf("strlen(\"TESter\"): success\n");
    }
    else
    {
		(*failure)++;
		printf("strlen(\"TESter\"): failure - expected 6, got %zu\n", strlen(tester));
    }

    if (strlen("") == 0)
    {
		(*success)++;
		printf("strlen(\"\"): success\n");
    }
    else
    {
		(*failure)++;
		printf("strlen(\"\"): failure - expected 0, got %zu\n", strlen(""));
    }

    // Test strcmp
    if (strcmp(tester, "TESter") == 0)
    {
		(*success)++;
		printf("strcmp equal strings: success\n");
    }
    else
    {
		(*failure)++;
		printf("strcmp equal strings: failure\n");
    }

    if (strcmp(tester, "tes") != 0)
    {
		(*success)++;
		printf("strcmp different strings: success\n");
    }
    else
    {
		(*failure)++;
		printf("strcmp different strings: failure\n");
    }

    if (strcmp(tester, "TESterrrr") != 0)
    {
		(*success)++;
		printf("strcmp different length strings: success\n");
    }
    else
    {
		(*failure)++;
		printf("strcmp different length strings: failure\n");
    }

    // Test strchr
    if (strchr(tester, 'a') == NULL)
    {
		(*success)++;
		printf("strchr not found: success\n");
    }
    else
    {
		(*failure)++;
		printf("strchr not found: failure\n");
    }

    if (strchr(tester, 'e') == tester + 4)
    {
		(*success)++;
		printf("strchr found: success\n");
    }
    else
    {
		(*failure)++;
		printf("strchr found: failure\n");
    }

    // Test memcpy
    char copy[8] = {0};
    memcpy(copy, tester, 3);
    if (strcmp(copy, "TES") == 0)
    {
		(*success)++;
		printf("memcpy: success\n");
    }
    else
    {
		(*failure)++;
		printf("memcpy: failure | expected copy == \"TES\", got %s\n", copy);
    }

    // Test strcpy
    strcpy(copy, tester);
    if (strcmp(copy, tester) == 0)
    {
		(*success)++;
		printf("strcpy: success\n");
    }
    else
    {
		(*failure)++;
		printf("strcpy: failure\n");
    }

    // Test memset
    memset(copy, 'c', 7);
    if (strcmp(copy, "ccccccc") == 0)
    {
		(*success)++;
		printf("memset: success\n");
    }
    else
    {
		(*failure)++;
		printf("memset: failure - expected \"ccccccc\", got %s\n", copy);
    }
}

void
tests_stdlib(int* total, int* success, int* failure)
{
    printf("\ntests for stdlib.h\n");
    *total += 3; // Three different atoi tests
    
    // Test atoi with positive number
    if (atoi("5") == 5)
    {
		(*success)++;
		printf("atoi(\"5\"): success\n");
    }
    else
    {
		(*failure)++;
		printf("atoi(\"5\"): failure - expected 5, got %d\n", atoi("5"));
    }
    
    // Test atoi with negative number
    if (atoi("-5") == -5)
    {
		(*success)++;
		printf("atoi(\"-5\"): success\n");
    }
    else
    {
		(*failure)++;
		printf("atoi(\"-5\"): failure - expected -5, got %d\n", atoi("-5"));
    }
    
    // Test atoi with invalid string
    if (atoi("zda") == 0)
    {
		(*success)++;
		printf("atoi(\"zda\"): success\n");
    }
    else
    {
		(*failure)++;
		printf("atoi(\"zda\"): failure - expected 0, got %d\n", atoi("zda"));
    }
}

void
run_all_tests(void)
{
    int total = 0, success = 0, failure = 0;
    
    printf("=== STARTING ALL TESTS ===\n\n");
    
    printf("=== MEMORY TESTS ===\n");
    tests_memory(&total, &success, &failure);
    
    printf("\n=== STRING TESTS ===\n");
    tests_string(&total, &success, &failure);
    
    printf("\n=== STDLIB TESTS ===\n");
    tests_stdlib(&total, &success, &failure);

    printf("\n=== PROCESSES TESTS ===\n");
    tests_processes(&total, &success, &failure);
    
    printf("\n=== TEST SUMMARY ===\n");
    printf("Total tests: %d\n", total);
    printf("Successful: %d\n", success);
    printf("Failed: %d\n", failure);
    
    if (failure == 0)
    {
        printf("ALL TESTS PASSED!\n");
    }
    else
    {
        printf("SOME TESTS FAILED!\n");
    }
    printf("=== END OF TESTS ===\n");
}
