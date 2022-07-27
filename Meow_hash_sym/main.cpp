#include<cstdio>
#include<cstring>
#include<intrin.h>
#include "meow_hash_x64_aesni.h"

char hash[] = { 0x6a,0x13,0xb8,0x88,0x6b,0x91,0xcf,0x29,0x6a,0x13,0xb8,0x88,0x6b,0x91,0xcf,0x29 };
char msg[] = "abcdefghabcdefghabcdefghabcdefghabcdefghabcdefghabcdefghabcdefgh";
unsigned char key[128];

int main()
{
	printf("message:%s\n", msg);
	
	MeowHash_re(hash, strlen(msg), msg, key);
	printf("key:");
	for (int i = 0; i < 128; ++i)printf("%02x", key[i]);
	puts("");

	__m128i output = MeowHash(key, strlen(msg), msg);
	unsigned char ans[16];
	memset(ans, 0, sizeof(ans));
	_mm_storeu_si128((__m128i*)ans, output);

	printf("hash value:");
	for (int i = 0; i < 16; ++i)printf("%x", ans[i]);
}