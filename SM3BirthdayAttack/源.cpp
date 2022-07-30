#include "sm3.h"
#include<cstdio>
#include<ctime>
#include<iostream>
#include<cmath>
#include<algorithm>

#define reg register int
#define FOR(i,a,b) for(reg i=a;i<b;++i)

typedef unsigned char uchar;
typedef unsigned long long ll;

using namespace std;

const int length = 32;
const ll upperbound = pow(2, length >> 1);

uchar dst[65], h[33];

struct setNode
{
	uchar msg[65];
	int c32;
	ll c64;
	setNode() { memset(msg,0,sizeof(msg)); c32 = 0; c64 = 0; };
	bool operator < (const setNode other)const
	{
		return this->c64 < other.c64;
	}
};

setNode *rec = new setNode[upperbound];

inline void long2bytes(uint64_t num, uchar* dst)
{
	FOR(i, 0, 8)
	{
		dst[7 - i] = num & 0xff;
		num >>= 8;
	}
}


inline uint64_t bytes2long(uchar* src)
{
	uint64_t res = 0;
	FOR(i, 0, 8)res = (res << 8) + src[i];
	return res;
}


inline uint32_t bytes2int(uchar* src)
{
	uint32_t res = 0;
	FOR(i, 0, 4)res = (res << 8) + src[i];
	return res;
}



inline void randBN(uchar* dst, int len)//len¸öbyte
{
	FOR(i, 0, len)dst[i] = rand() % 256;
}

inline void printhex(uchar* c,int len)
{
	FOR(i, 0, len)printf("%02x", c[i]);
	puts("");
}


int main()
{
	srand(time(0));
	bool flag = 0;
	while (!flag)
	{
		SM3_CTX ctx;
		memset(rec, 0, sizeof(rec));

		FOR(i, 0, upperbound)
		{
			randBN(rec[i].msg, 32);
			sm3(&ctx, rec[i].msg, 32, h);
			rec[i].c64 = bytes2long(h);
			rec[i].c32 = bytes2int(h);
		}

		sort(rec, rec + upperbound);

		FOR(i, 0, upperbound)if (rec[i].c32 != 0 && rec[i].c32 == rec[i + 1].c32)
		{
			printf("msg1:"); printhex(rec[i].msg, 32);
			printf("msg2:"); printhex(rec[i + 1].msg, 32);

			uchar h1[33], h2[33];
			memset(h1, 0, sizeof(h1)); memset(h2, 0, sizeof(h2));
			sm3(&ctx, rec[i].msg, 32, h1); sm3(&ctx, rec[i + 1].msg, 32, h2);

			printf("hash1:"); printhex(h1, 32);
			printf("hash2:"); printhex(h2, 32);
			puts("");

			flag = 1;
			break;
		}
	}

	return 0;
}
