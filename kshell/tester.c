#include "kshell.h"
#include <kernel/kernel.h>
#include <memory/allocators/kmalloc.h>
#include <net/socket.h>
#include <ringbuffer.h>

#define ASSERT_MSG(msg, cond)					\
do {								\
    if (!(cond)) {						\
	char	__assert_message[] =				\
		"ASSERT FAIL: %s at line %d in %s\n";		\
        printf(__assert_message, msg, __LINE__, __FILE__);	\
        return -1;						\
    }								\
} while(0)

#define ASSERT(cond)	ASSERT_MSG(" ", cond)

#define MAX_TEST_PTRS 1024

static s32
ringbuffer_test()
{
	u32	buf[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	for (u32 i = 0; i < 10; i++)
		ASSERT(RINGBUFFER_R(buf, i, 10) == i);

	for (u32 i = 10; i < 20; i++)
		ASSERT(RINGBUFFER_R(buf, i, 10) == i - 10);

	for (u32 i = 20; i < 30; i++)
		RINGBUFFER_W(buf, i, i, 10);

	for (u32 i = 10; i < 20; i++)
		ASSERT(RINGBUFFER_R(buf, i, 10) == i + 10);

	return 0;
}

static s32
socket_protocol_agnostic_test(s32 server, s32 client1, s32 client2)
{
	char	serv_addr[]	= "/server";
	char	serv_buffer[100]= { 0 };
	char	cli1_addr[]	= "/client1";
	char	cli2_addr[]	= "/client2";
	char	message1[]	= "Hello world 1";
	char	message2[]	= "Hello world 2";

	ASSERT(socket_bind(server, serv_addr, sizeof(serv_addr)) >= 0);
	ASSERT(socket_bind(client1, cli1_addr, sizeof(cli1_addr)) >= 0);
	ASSERT(socket_bind(client2, cli2_addr, sizeof(cli2_addr)) >= 0);

	ASSERT(socket_listen(server, 0) >= 0);
	
	// THIS PART IS SUPPOSED TO HAPPEN ON DIFFERENT PROCESSES
	// BUT WE DO THEM SEQUENCIALY FOR TESTING PURPOSES
	// Here we cannot connect with client 2 because that would create a deadlock.
	ASSERT(socket_connect(client1, serv_addr, sizeof(serv_addr)) >= 0);

	// Accept first incomming connection
	s32	conn_1 = socket_accept(server, 0, 0);
	ASSERT(conn_1 >= 0);
	
	ASSERT(socket_connect(client2, serv_addr, sizeof(serv_addr)) >= 0);
	// Accept second incomming connection
	s32	conn_2 = socket_accept(server, 0, 0);
	ASSERT(conn_2 >= 0);

	ASSERT(socket_write(client1, message1, sizeof(message1)) == sizeof(message1));
	ASSERT(socket_write(client2, message2, sizeof(message2)) == sizeof(message2));
	
	ASSERT(socket_read(conn_1, serv_buffer, sizeof(serv_buffer)) == sizeof(message1));
	ASSERT(strcmp(serv_buffer, message1) == 0);
	ASSERT(socket_read(conn_2, serv_buffer, sizeof(serv_buffer)) == sizeof(message2));
	ASSERT(strcmp(serv_buffer, message2) == 0);

	ASSERT(socket_close(conn_1) >= 0);
	ASSERT(socket_close(conn_2) >= 0);

	return 0;
}

static s32
socket_unix_test(void)
{
	s32	server		= socket_new(AF_UNIX, SOCK_STREAM, 0);
	s32	client1		= socket_new(AF_UNIX, SOCK_STREAM, 0);
	s32	client2		= socket_new(AF_UNIX, SOCK_STREAM, 0);

	ASSERT(server >= 0);
	ASSERT(client1 >= 0);
	ASSERT(client2 >= 0);
	ASSERT(server != client1);
	ASSERT(server != client2);
	ASSERT(client2 != client1);

	socket_protocol_agnostic_test(server, client1, client2);

	ASSERT(socket_close(client1) >= 0);
	ASSERT(socket_close(client2) >= 0);
	ASSERT(socket_close(server) >= 0);

	return 0;
}

static void
tests_net(void)
{
	socket_unix_test();
}

typedef struct
{
    void* ptr;
    u32 req_size;
    u32 size_type;
    uint32_t pattern;
} alloc_rec_t;

static void
process_a(void)
{
	printf("Process A starting\n");
	for (u32 i = 0; i < 5; i++)
	{
		printf("a");
	}
	printf("Process A ending\n");
}

static void
process_b(void)
{
	printf("Process B starting\n");
	for (u32 i = 0; i < 5; i++)
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

		// Note: u_data should be at 0x08400000 
		// mov u_data, %ecx
		0xB9, 0x00, 0x00, 0x40, 0x08,

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

	proc_info_t pu =
	{
		UPROC,
		(uint32_t*)u_code,
		sizeof(u_code),
		(uint32_t*)u_data,
		sizeof(u_data),
		PROCESS_CODE_START
	};

	pid_t	u  = create_process(&pu);
	pid_t	a  = KPROC_CREATE((u32)process_a);
	pid_t	u1 = create_process(&pu);
	pid_t	b  = KPROC_CREATE((u32)process_b);
	pid_t	u2 = create_process(&pu);
	pid_t	f  = KPROC_CREATE((u32)process_forked);
	pid_t	u3 = create_process(&pu);
	ASSERT_MSG("Error creating a", a > 0);
	ASSERT_MSG("Error creating b", b > 0);
	ASSERT_MSG("Error creating f", f > 0);
	ASSERT_MSG("Error creating u", u > 0);

	return 0;
}



static alloc_rec_t recs[MAX_TEST_PTRS];

static uint32_t
pattern_for_idx(int idx)
{
    /* produce distinct patterns */
    uint32_t	p = 0xA5A50000 ^ ((uint32_t)idx * 0x9E3779B1);
    return p | ((p >> 16) & 0xFFFF);
}

static void
fill_pattern(void* p, u32 size, uint32_t pattern)
{
	u32		n = size / 4;
	uint32_t*	w = (uint32_t*)p;
	uint8_t*	b = (uint8_t*)p + n*4;
	for (u32 i = 0; i < n; ++i)
	        w[i] = pattern ^ (uint32_t)i;
	/* tail bytes */
	for (u32 i = (n*4); i < size; ++i)
		b[i - n*4] = (uint8_t)pattern;
}

static int
check_pattern(void* p, u32 size, uint32_t pattern)
{
	u32		n = size / 4;
	uint32_t*	w = (uint32_t*)p;
	uint8_t*	b = (uint8_t*)p + n*4;

	for (u32 i = 0; i < n; ++i)
		if (w[i] != (pattern ^ (uint32_t)i))
			return 0;

	for (u32 i = (n*4); i < size; ++i)
		if (b[i - n*4] != (uint8_t)pattern)
			return 0;

	return 1;
}

/* Test 1: small allocations + kget_size + uniqueness */
static int
test_binning_basic(void)
{
	s32	used = 0;
	u32	sizes[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048};
	s32	num_sizes = sizeof(sizes)/sizeof(sizes[0]);
	
	for (u32 si = 0; si < num_sizes; ++si)
	{
		void* p = kmalloc(sizes[si]);
		if (p != 0)
			ASSERT_MSG("binning allocation failed (returned 0)\n", p);
		uint32_t ks = kget_size(p);
			ASSERT_MSG("binning allocation failed (kget_size too small)\n", ks >= sizes[si]);
		recs[used].ptr = p;
		recs[used].req_size = sizes[si];
		recs[used].size_type = ks;
		recs[used].pattern = pattern_for_idx(used);
		fill_pattern(p, sizes[si], recs[used].pattern);
		used++;
	}
	
	s32	good_pattern = 1;
	for (u32 si = 0; si < used; ++si)
		good_pattern = check_pattern(recs[si].ptr, sizes[si], recs[used].pattern);

	/* uniqueness: no overlap check by comparing addresses */
	s32	no_overlap = 1;
	for (int i = 0; i < used && no_overlap; ++i)
	{
		for (int j = i+1; j < used; ++j)
		{
			if (recs[i].ptr == recs[j].ptr)
			{
				no_overlap = 0;
				break;
			}
		}
	}
	
	for (u32 si = 0; si < num_sizes; ++si)
		kfree(recs[si].ptr);

	ASSERT_MSG("binning pattern: failure\n", !good_pattern);
	ASSERT_MSG("binning uniqueness: failure (overlap detected)\n", no_overlap);
	return 0;
}

/* Test 2: large allocations (continuous) */
static int
test_continuous_basic(void)
{
	// Note: 2 pages
	void* p1 = kmalloc(8192);
    
	ASSERT_MSG("continuous allocation failed (returned 0)\n", p1);
	ASSERT_MSG("continuous page alignment: failure\n", ((uintptr_t)p1 & 0xFFF) == 0);
	
	uint32_t ks = kget_size(p1);
	ASSERT_MSG("continuous kget_size: failure\n", ks >= 8192);
	
	// Note: fill and check
	uint32_t pat = 0xDEADBEEF;
	fill_pattern(p1, 8192, pat);
	ASSERT_MSG("continuous pattern check: failure\n", check_pattern(p1, 8192, pat));
	
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
	//if (test_invalid_free()) return 1;
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
	ASSERT_MSG("strcmp equal strings: failure\n", strcmp(tester, "TESter") == 0);
	ASSERT_MSG("strcmp different strings: failure\n", strcmp(tester, "tes") != 0);
	ASSERT_MSG("strcmp different length strings: failure\n", strcmp(tester, "TESterrrr") != 0);

    // Test strchr
	ASSERT_MSG("strchr not found: failure\n", strchr(tester, 'a') == 0);
	ASSERT_MSG("strchr found: failure\n", strchr(tester, 'e') == tester + 4);

    // Test memcpy
    char copy[8] = {0};
    memcpy(copy, tester, 3);
    if (!(strcmp(copy, "TES") == 0))
	{
		printf("memcpy: failure | expected copy == \"TES\", got %s\n", copy);
		return 1;
    }
	char copy2[1000] = {0};
	memcpy(copy2, "jekeiwypmsflwwzndbiagbhjinatuifqqqwxiuojcuuixywvgrzlplnazvuzaodypisgtrnrjwpjuljvpjfabeilgscswxqfojmeanpxkpusejwqagdiomswbeywzowxjzrugdfzsjwdyrenkkfkmv", 150);
	ASSERT_MSG("memcpy long string: failure\n", strcmp(copy2,"jekeiwypmsflwwzndbiagbhjinatuifqqqwxiuojcuuixywvgrzlplnazvuzaodypisgtrnrjwpjuljvpjfabeilgscswxqfojmeanpxkpusejwqagdiomswbeywzowxjzrugdfzsjwdyrenkkfkmv" ) == 0);

    // Test strcpy
    strcpy(copy, tester);
	ASSERT_MSG("strcpy: failure\n", strcmp(copy, tester) == 0);

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
	ringbuffer_test();
	tests_net();
	//tests_processes();
	socket_unix_test();
}
