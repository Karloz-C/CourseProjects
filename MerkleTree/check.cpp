#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include<cstdio>
#include<ctime>
#include<string>
#include<cstring>
#include<openssl/sha.h>
#include<iostream>

#define reg register int
#define FOR(i,a,b) for(reg i=a;i<b;++i)

using namespace std;

typedef unsigned int uint;
typedef unsigned char uchar;


inline int hex2int(char c)
{
	if (c >= '0' && c <= '9')return c - '0';
	else if (c >= 'a' && c <= 'f')return 10 + c - 'a';
	else return -1;
}

inline void hex2str(string src, char* dst)
{
	for (int i = 0; i < src.size(); i += 2)dst[i / 2] = hex2int(src[i]) * 16 + hex2int(src[i + 1]);
}

//#define test 

#ifdef test
int main()
{
	uchar t1[32], t2[32];
	uchar t[65];
	t[0] = 1;
	string s1 = "775921a4e428d5768955ee440c81d193960641c3a5b2c1f71916fcebe285e2c5";
	string s2 = "775921a4e428d5768955ee440c81d193960641c3a5b2c1f71916fcebe285e2c5";
	hex2str(s1, (char*)t1); hex2str(s2, (char*)t2);
	FOR(i, 0, 32)
	{
		t[i + 1] = t1[i];
		t[i + 32 + 1] = t2[i];
	}
	//FOR(i, 0, 65)printf("%d ", t[i]);
	uchar d[32];
	SHA256(t, 65, d);
	FOR(i, 0, 32)printf("%02x", d[i]);
}
#endif // test