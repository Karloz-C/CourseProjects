#include<cstdio>
#include<ctime>
#include "sm3.h"

uint8_t msg[]="abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd";
uint8_t dst[32];

clock_t start, end;

int main()
{
	init();
	SM3_CTX ctx;
	start = clock();
	FOR(i,0,1000000)
	sm3(&ctx, msg, 64, dst);
	end = clock();
	for (int i = 0; i < 32; ++i)printf("%02x", dst[i]);
	printf("\nruntime:%fs", (double)(end - start) / CLOCKS_PER_SEC);
}