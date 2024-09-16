#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

struct initrd_header {
	uint8_t magic;
	char name[64];
	uint32_t offset;
	uint32_t len;
};

int main(int argc, char **argv) {
	int nheaders = (argc - 1) / 2;
	struct initrd_header headers[64];
	printf("size of headers: %ld\n", sizeof(struct initrd_header));
	uint32_t off = sizeof(struct initrd_header) * 64 + sizeof(int);
	size_t i;
	for (i = 0; i < nheaders; i++) {
		printf("writing file %s->%s at 0x%x\n", argv[i * 2 + 1], argv[i * 2 + 2], off);
		strcpy(headers[i].name, argv[i * 2 + 2]);
		headers[i].offset = off;
		FILE* stream =fopen(argv[i * 2 + 1], "r");
		if (!stream) {
			printf("Error: file not found: %s\n", argv[i * 2 + 1]);
			return 1;
		}
		fseek(stream, 0, SEEK_END);
		headers[i].len = ftell(stream);
		off += headers[i].len;
		fclose(stream);
		headers[i].magic = 0xBF;
	}

	FILE* wstream = fopen("./initrd.img", "w");
	uint8_t* data = (uint8_t*)malloc(off);
	fwrite(&nheaders, sizeof(uint32_t), 1, wstream);
	fwrite(headers, sizeof(struct initrd_header), 64, wstream);

	for (i = 0; i < nheaders; i++) {
		FILE* stream = fopen(argv[i * 2 + 1], "r");
		uint8_t* buf = (uint8_t*)malloc(headers[i].len);
		fread(buf, 1, headers[i].len, stream);
		fwrite(buf, 1, headers[i].len, wstream);
		fclose(stream);
		free(buf);
	}

	fclose(wstream);
	free(data);
	return 0;
}
