#include<cstdio>
#include<cstring>
#include<intrin.h>
#include "meow_hash_x64_aesni.h"

char hash[] = "sdu_cst_20220610";
char msg[] = "Lukai Cui 202000460058";
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
	for (int i = 0; i < 16; ++i)printf("%c", ans[i]);
}