#include "sm3.h"
#include<cstdio>
#include<iostream>
#include<ctime>

typedef unsigned char uchar;
typedef unsigned long long ll;

const int length = 32;
const ll upperbound = pow(2, length >> 1);


inline bool cmp(uchar s1[], uchar s2[], int len)
{
	FOR(i, 0, len)if (s1[i] != s2[i])return 1;
	return 0;
}


inline void randBN(uchar* dst, int len)//len��byte
{
	FOR(i, 0, len)dst[i] = rand() % 256;
}


inline void printhex(uchar* c, int len)
{
	FOR(i, 0, len)printf("%02x", c[i]);
	puts("");
}


int main()
{
	uchar msg1[33], msg2[33];
	uchar h1[33], temph[33], h2[33];
	SM3_CTX ctx;

	bool flag = 0;
	while (!flag)
	{
		randBN(msg1, 32); randBN(msg2, 32);

		FOR(i, 0, upperbound)
		{
			sm3(&ctx, msg1, 32, h1);

			sm3(&ctx, msg2, 32, temph);
			sm3(&ctx, temph, 32, h2);
			if (!cmp(h1, h2, 2))
			{
				printf("msg1:"); printhex(msg1, 32);
				printf("msg2:"); printhex(temph, 32);
				printf("hash1:"); printhex(h1, 32);
				printf("hash2:"); printhex(h2, 32);
				flag = 1;
				break;
			}
			FOR(i, 0, 32)
			{
				msg1[i] = h1[i];
				msg2[i] = h2[i];
			}
		}
	}
	
	return 0;
}