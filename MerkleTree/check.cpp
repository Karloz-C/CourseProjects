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


int main()
{
	uchar t1[32], t2[32];
	uchar t[66];
	t[0] = 1; t[65] = 0;
	string s1 = "775921a4e428d5768955ee440c81d193960641c3a5b2c1f71916fcebe285e2c5";
	string s2 = "9eacac1f1d19d078906cb14930cb9e864bf512aac3f8e26153666d5eaef7b387";
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

//e8fb69d07523527f600e1fd288b9be6dd210431e2cfd0f2dfc7576b513ec9e77
//775921a4e428d5768955ee440c81d193960641c3a5b2c1f71916fcebe285e2c5
//leaf0 5feceb66ffc86f38d952786c6d696c79c2dbc239dd4e91b46729d73a27fb57e9
//leaf1 6b86b273ff34fce19d6b804eff5a3f5747ada4eaa22f1d49c01e52ddb7875b4b
//9eacac1f1d19d078906cb14930cb9e864bf512aac3f8e26153666d5eaef7b387
//leaf2 d4735e3a265e16eee03f59718b9b5d03019c07d8b6c51f90da3a666eec13ab35
//leaf3 4e07408562bedb8b60ce05c1decfe3ad16b72230967de01f640b7e4729b49fce