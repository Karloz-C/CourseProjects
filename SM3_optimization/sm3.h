#pragma once
#include<immintrin.h>
#include<cstring>
#include <cstring>
#include <stdint.h>


struct SM3_CTX
{
	uint32_t sw[8];
	uint64_t nblocks;
	uint8_t block[64];
	size_t used;
};

uint32_t IV[8] = { 0x7380166F ,0x4914B2B9 , 0x172442D7 ,0xDA8A0600,0xA96F30BC,0x163138AA,0xE38DEE4D,0xB0FB0E4E };
//T_j << (j mod 32)
static uint32_t T_j[64];

#define reg register int
#define FOR(i,a,b) for(reg i = a; i < b; ++i)

#define ROTL(x,n)  (x << n) | (x >> (32 - n))
#define ROL32(a,n) ((a << n) | ((a & 0xffffffff) >> (32 - n)))
#define P0(x) x ^ ROL32(x, 9) ^ ROL32(x,17)
#define P1(x) x ^ ROL32(x,15) ^ ROL32(x,23))


#define FF0(x,y,z)  (x ^ y ^ z)
#define FF1(x,y,z)  ((x & y) | (x & z) | (y & z))

#define GG0(x,y,z)  (x ^ y ^ z)
#define GG1(x,y,z)  ((x & y) | ((~x) & z))

#define RoundFunc(A, B, C, D, E, F, G, H, tag)				\
	SS1 = ROL32(ROL32(A, 12) + E + T_j[j], 7);		\
	SS2 = SS1 ^ ROL32(A, 12);				\
	TT1 = FF##tag(A, B, C) + D + SS2 + (W[j] ^ W[j + 4]);	\
	TT2 = GG##tag(E, F, G) + H + SS1 + W[j];			\
	B = ROL32(B, 9);					\
	H = TT1;						\
	F = ROL32(F, 19);					\
	D = P0(TT2);						\
	j++

#define unRoll(A, B, C, D, E, F, G, H, tag)				\
	RoundFunc(A, B, C, D, E, F, G, H, tag);				\
	RoundFunc(H, A, B, C, D, E, F, G, tag);				\
	RoundFunc(G, H, A, B, C, D, E, F, tag);				\
	RoundFunc(F, G, H, A, B, C, D, E, tag);				\
	RoundFunc(E, F, G, H, A, B, C, D, tag);				\
	RoundFunc(D, E, F, G, H, A, B, C, tag);				\
	RoundFunc(C, D, E, F, G, H, A, B, tag);				\
	RoundFunc(B, C, D, E, F, G, H, A, tag)


# define rotl(X,i) _mm_xor_si128(_mm_slli_epi32(X, i), _mm_srli_epi32(X, 32 - i))


//按照大端序存储32位字
inline void bigEndian32(uint8_t *a, int n)
{
	a[0] = uint8_t(n >> 24);
	a[1] = uint8_t(n >> 16);
	a[2] = uint8_t(n >> 8);
	a[3] = uint8_t(n);
}


//压缩函数
inline void CF(uint32_t sw[8], const uint8_t* msg, int blocks)
{
	uint32_t A, B, C, D, E, F, G, H;
	uint32_t W[68];
	uint32_t SS1, SS2, TT1, TT2;
	int j;

	__m128i X, T, R;
	//考虑到消息扩展函数中使用了W[j-3]，因此当计算W[j]时，至多能同时处理三组，一旦超过三组，则会出现未知量，不能完成正确计算
	//因此使用128位simd，可以处理4个32位字，实际上只使用了三个。使用更宽的寄存器没有意义。
	__m128i M = _mm_setr_epi32(0, 0, 0, 0xffffffff);

	//大端序，对于每个32位字，高字节放在低地址
	__m128i V = _mm_setr_epi8(3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12);

	while (blocks--) 
	{
		A = sw[0];
		B = sw[1];
		C = sw[2];
		D = sw[3];
		E = sw[4];
		F = sw[5];
		G = sw[6];
		H = sw[7];


		for (j = 0; j < 16; j += 4) 
		{
			X = _mm_loadu_si128((__m128i*)(msg + j * 4));
			X = _mm_shuffle_epi8(X, V);
			_mm_storeu_si128((__m128i*)(W + j), X);
		}

		for (j = 16; j < 68; j += 4) 
		{
			X = _mm_loadu_si128((__m128i*)(W + j - 3));
			X = _mm_andnot_si128(M, X);

			X = rotl(X, 15);
			T = _mm_loadu_si128((__m128i*)(W + j - 9));
			X = _mm_xor_si128(X, T);
			T = _mm_loadu_si128((__m128i*)(W + j - 16));
			X = _mm_xor_si128(X, T);

			T = rotl(X, (23 - 15));
			T = _mm_xor_si128(T, X);
			T = rotl(T, 15);
			X = _mm_xor_si128(X, T);

			T = _mm_loadu_si128((__m128i*)(W + j - 13));
			T = rotl(T, 7);
			X = _mm_xor_si128(X, T);
			T = _mm_loadu_si128((__m128i*)(W + j - 6));
			X = _mm_xor_si128(X, T);

			R = _mm_shuffle_epi32(X, 0);
			R = _mm_and_si128(R, M);
			T = rotl(R, 15);
			T = _mm_xor_si128(T, R);
			T = rotl(T, 9);
			R = _mm_xor_si128(R, T);
			R = rotl(R, 6);
			X = _mm_xor_si128(X, R);

			_mm_storeu_si128((__m128i*)(W + j), X);
		}

		j = 0;
		unRoll(A, B, C, D, E, F, G, H, 0);
		unRoll(A, B, C, D, E, F, G, H, 0);
		unRoll(A, B, C, D, E, F, G, H, 1);
		unRoll(A, B, C, D, E, F, G, H, 1);
		unRoll(A, B, C, D, E, F, G, H, 1);
		unRoll(A, B, C, D, E, F, G, H, 1);
		unRoll(A, B, C, D, E, F, G, H, 1);
		unRoll(A, B, C, D, E, F, G, H, 1);

		sw[0] ^= A;
		sw[1] ^= B;
		sw[2] ^= C;
		sw[3] ^= D;
		sw[4] ^= E;
		sw[5] ^= F;
		sw[6] ^= G;
		sw[7] ^= H;

		msg += 64;
	}
}


inline void init()
{
	uint32_t T0 = 0x79cc4519, T1 = 0x7a879d8a;
	FOR(j, 0, 16)T_j[j] = ROL32(T0, j);
	FOR(j, 16, 64)T_j[j] = ROL32(T1, j % 32);
}


void sm3(SM3_CTX* ctx, const uint8_t* data, size_t data_len, uint8_t* dst)
{
	//init
	memset(ctx, 0, sizeof(*ctx));
	FOR(i, 0, 8)ctx->sw[i] = IV[i];

	size_t blocks = data_len / 64;
	
	CF(ctx->sw, data, blocks);
	ctx->nblocks += blocks;
	data += 64 * blocks;
	data_len -= 64 * blocks;

	ctx->used = data_len;
	if (data_len) memcpy(ctx->block, data, data_len);


	int i;

	//填充规则，填充1bit的1和k个0，满足L + 1 + k = 448 (mod 512)
	//0x80=10000000b 即先填充一个字节，后面的部分根据具体情况考虑
	ctx->used &= 0x3f;
	ctx->block[ctx->used] = 0x80;

	//最后需要填充一个64bit串，若剩余空间不小于9B(64bit长度，加上0x80占用一个，共9B)，则可以在当前块完成填充
	if (ctx->used <= 55)memset(ctx->block + ctx->used + 1, 0, 64 - ctx->used - 9);

	//剩余不足，需要新的块。
	else
	{
		memset(ctx->block + ctx->used + 1, 0, 64 - ctx->used - 1);
		CF(ctx->sw, ctx->block, 1);
		memset(ctx->block, 0, 56);
	}

	//填充64bit的长度信息，大端序
	bigEndian32(ctx->block + 56, ctx->nblocks >> 23);
	bigEndian32(ctx->block + 60, (ctx->nblocks << 9) + (ctx->used << 3));

	//处理最后一块
	CF(ctx->sw, ctx->block, 1);

	FOR(i, 0, 8) bigEndian32(dst + i * 4, ctx->sw[i]);
}