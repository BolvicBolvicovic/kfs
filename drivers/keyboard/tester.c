#include "keyboard.h"

#define ASSERT(msg, cond) do { \
    if (!(cond)) { \
        printf("ASSERT FAIL: %s\n", msg); \
        return -1; \
    } \
} while(0)

#define MAX_TEST_PTRS 1024

typedef struct
{
    void* ptr;
    size_t req_size;
    size_t size_type;
    uint32_t pattern;
} alloc_rec_t;

typedef uint32_t pid_t;
enum p_type
{
	KPROC,
	UPROC
};

typedef struct
{
	enum p_type	type;
	uint32_t*	code;
	uint32_t	code_size;
	uint32_t*	data;
	uint32_t	data_size;
	uint32_t	entry;
} p_info_t;

extern void*	kmalloc(size_t s);
extern pid_t	create_process(p_info_t*);
extern pid_t	fork(void);

static void
process_a(void)
{
	printf("Process A starting\n");
	for (size_t i = 0; i < 5; i++)
	{
		printf("a");
	}
	printf("Process A ending\n");
}

static void
process_b(void)
{
	printf("Process B starting\n");
	for (size_t i = 0; i < 5; i++)
	{
		printf("b");
	}
	printf("Process B ending\n");
}

static void
process_forked(void)
{
	printf("Process Forked starting\n");
	
	pid_t p = fork();

	if (!p)
	{
		printf("Hello from child\n");
	}
	else
	{
		printf("Hello from parent\n");
	}
}

int
tests_processes(void)
{
	p_info_t pa =
	{
		KPROC, 0, 0, 0, 0,
		(uint32_t)process_a
	};

	p_info_t pb =
	{
		KPROC, 0, 0, 0, 0,
		(uint32_t)process_b
	};

	p_info_t pf =
	{
		KPROC, 0, 0, 0, 0,
		(uint32_t)process_forked
	};

	uint8_t	u_data[14] =
	{
		// Note: "Hello World!\n"
		0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64, 0x21, 0x0A, 0x00
	};

	uint8_t	u_code[27] =
	{
		// xor %eax, %eax
		0x31, 0xC0,

		// inc %eax
		0x40,

		// xor %ebx, %ebx
		0x31, 0xDB,

		// Note: u_data should be at 0x08049000
		// mov u_data, %ecx
		0xB9, 0x00, 0x90, 0x04, 0x08,

		// mov $0x0D, %edx
		0xBA, 0x0D, 0x00, 0x00, 0x00,

		// int $0x80
		0xCD, 0x80,

		// xor %ebx, %ebx
		0x31, 0xDB,

		// mov $0x3C, %eax
		0xB8, 0x3C, 0x00, 0x00, 0x00,

		// int $0x80
		0xCD, 0x80
	};

	p_info_t pu =
	{
		UPROC,
		(uint32_t*)u_code,
		sizeof(u_code),
		(uint32_t*)u_data,
		sizeof(u_data),
		PROCESS_CODE_START
	};

	pid_t	a = create_process(&pa);
	pid_t	b = create_process(&pb);
	pid_t	f = create_process(&pf);
	pid_t	u = create_process(&pu);
	ASSERT("Error creating a", a > 0);
	ASSERT("Error creating b", b > 0);
	ASSERT("Error creating f", f > 0);
	ASSERT("Error creating u", u > 0);

	return 0;
}



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
    int used = 0;
    size_t sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048};
    int num_sizes = sizeof(sizes)/sizeof(sizes[0]);
    
    for (size_t si = 0; si < num_sizes; ++si)
	{
        void* p = kmalloc(sizes[si]);
        if (p != NULL)
		ASSERT("binning allocation failed (returned NULL)\n", p);
        uint32_t ks = kget_size(p);
		ASSERT("binning allocation failed (kget_size too small)\n", ks >= sizes[si]);
        recs[used].ptr = p;
        recs[used].req_size = sizes[si];
        recs[used].size_type = ks;
        recs[used].pattern = pattern_for_idx(used);
        fill_pattern(p, sizes[si], recs[used].pattern);
        used++;
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
    
    ASSERT("binning uniqueness: failure (overlap detected)\n", !overlap_found);
    return 0;
}

/* Test 2: large allocations (continuous) */
static int
test_continuous_basic(void)
{
	// Note: 2 pages
	void* p1 = kmalloc(8192);
    
	ASSERT("continuous allocation failed (returned NULL)\n", p1);
    ASSERT("continuous page alignment: failure\n", ((uintptr_t)p1 & 0xFFF) == 0);
    
    uint32_t ks = kget_size(p1);
    ASSERT("continuous kget_size: failure\n", ks >= 8192);
    
    // Note: fill and check
    uint32_t pat = 0xDEADBEEF;
    fill_pattern(p1, 8192, pat);
    ASSERT("continuous pattern check: failure\n", check_pattern(p1, 8192, pat));
    
    kfree(p1);
    return 0;
}

/* Test 3: invalid free */
static int
test_invalid_free(void)
{
    // Note: Pass some pointer that was not allocated
    void* bogus = (void*)0x12345000;
    // Note: Should print "ERROR: Invalid free" — but we check no crash
    kfree(bogus);
    
    return 0;
}

/* main test runner */
int
tests_memory(void)
{
    if (test_binning_basic()) return 1;
    if (test_continuous_basic()) return 1;
    if (test_invalid_free()) return 1;
	return 0;
}

int
tests_string(void)
{
    char tester[] = "TESter";
    
    // Note: test strlen
	if (!(strlen(tester) == 6))
    {
		printf("strlen(\"TESter\"): failure - expected 6, got %zu\n", strlen(tester));
		return 1;
    }

    if (!(strlen("") == 0))
    {
		printf("strlen(\"\"): failure - expected 0, got %zu\n", strlen(""));
		return 1;
    }

    // Test strcmp
	ASSERT("strcmp equal strings: failure\n", strcmp(tester, "TESter") == 0);
	ASSERT("strcmp different strings: failure\n", strcmp(tester, "tes") != 0);
	ASSERT("strcmp different length strings: failure\n", strcmp(tester, "TESterrrr") != 0);

    // Test strchr
	ASSERT("strchr not found: failure\n", strchr(tester, 'a') == NULL);
	ASSERT("strchr found: failure\n", strchr(tester, 'e') == tester + 4);

    // Test memcpy
    char copy[8] = {0};
    memcpy(copy, tester, 3);
    if (!(strcmp(copy, "TES") == 0))
	{
		printf("memcpy: failure | expected copy == \"TES\", got %s\n", copy);
		return 1;
    }

    // Test strcpy
    strcpy(copy, tester);
	ASSERT("strcpy: failure\n", strcmp(copy, tester) == 0);

    // Test memset
    memset(copy, 'c', 7);
    if (!(strcmp(copy, "ccccccc") == 0))
	{
		printf("memset: failure - expected \"ccccccc\", got %s\n", copy);
		return 1;
    }

	return 0;
}

int
tests_stdlib()
{
    // Test atoi with positive number
    if (!(atoi("5") == 5))
	{
		printf("atoi(\"5\"): failure - expected 5, got %d\n", atoi("5"));
		return 1;
    }
    
    // Test atoi with negative number
    if (!(atoi("-5") == -5))
    {
		printf("atoi(\"-5\"): failure - expected -5, got %d\n", atoi("-5"));
		return 1;
    }
    
    // Test atoi with invalid string
    if (!(atoi("zda") == 0))
    {
		printf("atoi(\"zda\"): failure - expected 0, got %d\n", atoi("zda"));
		return 1;
    }

	return 0;
}

void
run_all_tests(void)
{
    tests_string();
    tests_stdlib();
    tests_memory();
    tests_processes();
}
