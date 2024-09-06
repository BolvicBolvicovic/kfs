#include "keyboard.h"

void    tests_string(int* total, int* success, int* failure) {
    printf("\ntests for string.h\n");
    char tester[] = "TESter";
    *total += 10;
    if (strlen(tester) == 6) {
	*success += 1;
	printf("strlen : success\n");
    } else {
	*failure += 1;
	printf("strlen : failure\n");
    }
    if (strlen("") == 0) {
	*success += 1;
	printf("strlen : success\n");
    } else {
	*failure += 1;
	printf("strlen : failure\n");
    }
    if (strcmp(tester, "TESter") == 0) {
	*success += 1;
	printf("strcmp : success\n");
    } else {
	*failure += 1;
	printf("strcmp : failure\n");
    }
    if (strcmp(tester, "tes") != 0) {
	*success += 1;
	printf("strcmp : success\n");
    } else {
	*failure += 1;
	printf("strcmp : failure\n");
    }
    if (strcmp(tester, "TESterrrr") != 0) {
	*success += 1;
	printf("strcmp : success\n");
    } else {
	*failure += 1;
	printf("strcmp : failure\n");
    }
    if (strchr(tester, 'a') == 0) {
	*success += 1;
	printf("strchr : success\n");
    } else {
	*failure += 1;
	printf("strchr : failure\n");
    }
    if (strchr(tester, 'e') == tester + 4) {
	*success += 1;
	printf("strchr : success\n");
    } else {
	*failure += 1;
	printf("strchr : failure\n");
    }
    char copy[8] = {0};
    memcpy(copy, tester, 3);
    if (strcmp(copy, "TES") == 0) {
	*success += 1;
	printf("memcpy : success\n");
    } else {
	*failure += 1;
	printf("memcpy : failure | expected copy == \"TES\", got %s\n", copy);
    }
    strcpy(copy, tester);
    if (strcmp(copy, tester) == 0) {
	*success += 1;
	printf("strcpy : success\n");
    } else {
	*failure += 1;
	printf("strcpy : failure\n");
    }
    memset(copy, 'c', 7);
    if (strcmp(copy, "ccccccc") == 0) {
	*success += 1;
	printf("memset : success\n");
    } else {
	*failure += 1;
	printf("memset : failure\n");
    }
}

void tests_stdlib(int* total, int* success, int* failure) {
    printf("\ntests for stdlib.h\n");
    *total += 1;
    if (atoi("5") == 5 && atoi("-5") == -5 && atoi("zda") == 0) {
	*success += 1;
	printf("atoi   : success\n");
    } else {
	*failure += 1;
	printf("atoi   : failure atoi(5) == %d | atoi(-5) == %d | atoi('zda') == %d\n", atoi("5"), atoi("-5"), atoi("zda"));
    }
}
