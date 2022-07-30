# Meow hash

**Project: Find a key with hash value “sdu_cst_20220610” under a message composed of your name followed by your student ID. For example, “San Zhan 202000460001”.**

------

#### 代码说明

根据计算hash的过程，将整个函数以及其中的组件逆向转化成求解key的函数。

逆向包括**逆运算**和**逆序**两个部分。

首先，对于计算顺序，还原的过程应与原函数相反，确保数据的时序。

然后对于各种独立的运算，应当修改为逆运算。比如：加法改为减法，解密改为加密。由于异或操作自身可逆，不需要改动。

原函数如下，分析其过程。

在最开始，可以明确rax和rcx分别保存了输入的信息和key。最开始的时候将key的每16字节（128bit）存放在128位SIMD寄存器中，参与了一系列运算。

```c++
MeowHash(void *Seed128Init, meow_umm Len, void *SourceInit)
{
    meow_u128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7; // NOTE(casey): xmm0-xmm7 are the hash accumulation lanes
    meow_u128 xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15; // NOTE(casey): xmm8-xmm15 hold values to be appended (residual, length)
    
    meow_u8 *rax = (meow_u8 *)SourceInit;
    meow_u8 *rcx = (meow_u8 *)Seed128Init;
    
    //
	// NOTE(casey): Seed the eight hash registers
    //
    
    movdqu(xmm0, rcx + 0x00);
    movdqu(xmm1, rcx + 0x10);
    movdqu(xmm2, rcx + 0x20);
    movdqu(xmm3, rcx + 0x30);
    
    movdqu(xmm4, rcx + 0x40);
    movdqu(xmm5, rcx + 0x50);
    movdqu(xmm6, rcx + 0x60);
    movdqu(xmm7, rcx + 0x70);
    
    MEOW_DUMP_STATE("Seed", xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, 0);
    
    //
    // NOTE(casey): Hash all full 256-byte blocks
    //
    
    meow_umm BlockCount = (Len >> 8);
    if(BlockCount > MEOW_PREFETCH_LIMIT)
    {
        // NOTE(casey): For large input, modern Intel x64's can't hit full speed without prefetching, so we use this loop
        while(BlockCount--)
        {
            prefetcht0(rax + MEOW_PREFETCH + 0x00);
            prefetcht0(rax + MEOW_PREFETCH + 0x40);
            prefetcht0(rax + MEOW_PREFETCH + 0x80);
            prefetcht0(rax + MEOW_PREFETCH + 0xc0);
            
            MEOW_MIX(xmm0,xmm4,xmm6,xmm1,xmm2, rax + 0x00);
            MEOW_MIX(xmm1,xmm5,xmm7,xmm2,xmm3, rax + 0x20);
            MEOW_MIX(xmm2,xmm6,xmm0,xmm3,xmm4, rax + 0x40);
            MEOW_MIX(xmm3,xmm7,xmm1,xmm4,xmm5, rax + 0x60);
            MEOW_MIX(xmm4,xmm0,xmm2,xmm5,xmm6, rax + 0x80);
            MEOW_MIX(xmm5,xmm1,xmm3,xmm6,xmm7, rax + 0xa0);
            MEOW_MIX(xmm6,xmm2,xmm4,xmm7,xmm0, rax + 0xc0);
            MEOW_MIX(xmm7,xmm3,xmm5,xmm0,xmm1, rax + 0xe0);
            
            rax += 0x100;
        }
    }
    else
    {
        // NOTE(casey): For small input, modern Intel x64's can't hit full speed _with_ prefetching (because of port pressure), so we use this loop.
        while(BlockCount--)
        {
            MEOW_MIX(xmm0,xmm4,xmm6,xmm1,xmm2, rax + 0x00);
            MEOW_MIX(xmm1,xmm5,xmm7,xmm2,xmm3, rax + 0x20);
            MEOW_MIX(xmm2,xmm6,xmm0,xmm3,xmm4, rax + 0x40);
            MEOW_MIX(xmm3,xmm7,xmm1,xmm4,xmm5, rax + 0x60);
            MEOW_MIX(xmm4,xmm0,xmm2,xmm5,xmm6, rax + 0x80);
            MEOW_MIX(xmm5,xmm1,xmm3,xmm6,xmm7, rax + 0xa0);
            MEOW_MIX(xmm6,xmm2,xmm4,xmm7,xmm0, rax + 0xc0);
            MEOW_MIX(xmm7,xmm3,xmm5,xmm0,xmm1, rax + 0xe0);
            
            rax += 0x100;
        }
    }
    
    MEOW_DUMP_STATE("PostBlocks", xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, 0);
    
    //
    // NOTE(casey): Load any less-than-32-byte residual
    //
    
    pxor_clear(xmm9, xmm9);
    pxor_clear(xmm11, xmm11);
    
    //
    // TODO(casey): I need to put more thought into how the end-of-buffer stuff is actually working out here,
    // because I _think_ it may be possible to remove the first branch (on Len8) and let the mask zero out the
    // result, but it would take a little thought to make sure it couldn't read off the end of the buffer due
    // to the & 0xf on the align computation.
    //
    
    // NOTE(casey): First, we have to load the part that is _not_ 16-byte aligned
    meow_u8 *Last = (meow_u8 *)SourceInit + (Len & ~0xf);
    int unsigned Len8 = (Len & 0xf);
    if(Len8)
    {
        // NOTE(casey): Load the mask early
        movdqu(xmm8, &MeowMaskLen[0x10 - Len8]);
        
        meow_u8 *LastOk = (meow_u8*)((((meow_umm)(((meow_u8 *)SourceInit)+Len - 1)) | (MEOW_PAGESIZE - 1)) - 16);
        int Align = (Last > LastOk) ? ((int)(meow_umm)Last) & 0xf : 0;
        movdqu(xmm10, &MeowShiftAdjust[Align]);
        movdqu(xmm9, Last - Align);
        pshufb(xmm9, xmm10);
        
        // NOTE(jeffr): and off the extra bytes
        pand(xmm9, xmm8);
    }
    
    // NOTE(casey): Next, we have to load the part that _is_ 16-byte aligned
    if(Len & 0x10)
    {
        xmm11 = xmm9;
        movdqu(xmm9, Last - 0x10);
    }
    
    //
    // NOTE(casey): Construct the residual and length injests
    //
    
    xmm8 = xmm9;
    xmm10 = xmm9;
    palignr(xmm8, xmm11, 15);
    palignr(xmm10, xmm11, 1);
    
    // NOTE(casey): We have room for a 128-bit nonce and a 64-bit none here, but
    // the decision was made to leave them zero'd so as not to confuse people
    // about hwo to use them or what security implications they had.
    pxor_clear(xmm12, xmm12);
    pxor_clear(xmm13, xmm13);
    pxor_clear(xmm14, xmm14);
    movq(xmm15, Len);
    palignr(xmm12, xmm15, 15);
    palignr(xmm14, xmm15, 1);
    
    MEOW_DUMP_STATE("Residuals", xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15, 0);
    
    // NOTE(casey): To maintain the mix-down pattern, we always Meow Mix the less-than-32-byte residual, even if it was empty
    MEOW_MIX_REG(xmm0, xmm4, xmm6, xmm1, xmm2,  xmm8, xmm9, xmm10, xmm11);
    
    // NOTE(casey): Append the length, to avoid problems with our 32-byte padding
    MEOW_MIX_REG(xmm1, xmm5, xmm7, xmm2, xmm3,  xmm12, xmm13, xmm14, xmm15);
    
    MEOW_DUMP_STATE("PostAppend", xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, 0);
    
    //
    // NOTE(casey): Hash all full 32-byte blocks
    //
    int unsigned LaneCount = (Len >> 5) & 0x7;
    if(LaneCount == 0) goto MixDown; MEOW_MIX(xmm2,xmm6,xmm0,xmm3,xmm4, rax + 0x00); --LaneCount;
    if(LaneCount == 0) goto MixDown; MEOW_MIX(xmm3,xmm7,xmm1,xmm4,xmm5, rax + 0x20); --LaneCount;
    if(LaneCount == 0) goto MixDown; MEOW_MIX(xmm4,xmm0,xmm2,xmm5,xmm6, rax + 0x40); --LaneCount;
    if(LaneCount == 0) goto MixDown; MEOW_MIX(xmm5,xmm1,xmm3,xmm6,xmm7, rax + 0x60); --LaneCount;
    if(LaneCount == 0) goto MixDown; MEOW_MIX(xmm6,xmm2,xmm4,xmm7,xmm0, rax + 0x80); --LaneCount;
    if(LaneCount == 0) goto MixDown; MEOW_MIX(xmm7,xmm3,xmm5,xmm0,xmm1, rax + 0xa0); --LaneCount;
    if(LaneCount == 0) goto MixDown; MEOW_MIX(xmm0,xmm4,xmm6,xmm1,xmm2, rax + 0xc0); --LaneCount;
    
    //
    // NOTE(casey): Mix the eight lanes down to one 128-bit hash
    //
    
    MixDown:
    
    MEOW_DUMP_STATE("PostLanes", xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, 0);
    
    MEOW_SHUFFLE(xmm0, xmm1, xmm2, xmm4, xmm5, xmm6);
    MEOW_SHUFFLE(xmm1, xmm2, xmm3, xmm5, xmm6, xmm7);
    MEOW_SHUFFLE(xmm2, xmm3, xmm4, xmm6, xmm7, xmm0);
    MEOW_SHUFFLE(xmm3, xmm4, xmm5, xmm7, xmm0, xmm1);
    MEOW_SHUFFLE(xmm4, xmm5, xmm6, xmm0, xmm1, xmm2);
    MEOW_SHUFFLE(xmm5, xmm6, xmm7, xmm1, xmm2, xmm3);
    MEOW_SHUFFLE(xmm6, xmm7, xmm0, xmm2, xmm3, xmm4);
    MEOW_SHUFFLE(xmm7, xmm0, xmm1, xmm3, xmm4, xmm5);
    MEOW_SHUFFLE(xmm0, xmm1, xmm2, xmm4, xmm5, xmm6);
    MEOW_SHUFFLE(xmm1, xmm2, xmm3, xmm5, xmm6, xmm7);
    MEOW_SHUFFLE(xmm2, xmm3, xmm4, xmm6, xmm7, xmm0);
    MEOW_SHUFFLE(xmm3, xmm4, xmm5, xmm7, xmm0, xmm1);
    
    MEOW_DUMP_STATE("PostMix", xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, 0);
    
    paddq(xmm0, xmm2);
    paddq(xmm1, xmm3);
    paddq(xmm4, xmm6);
    paddq(xmm5, xmm7);
    pxor(xmm0, xmm1);
    pxor(xmm4, xmm5);
    paddq(xmm0, xmm4);
    
    MEOW_DUMP_STATE("PostFold", xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, 0);
    
    return(xmm0);
}
```

函数的最后，返回了xmm0，而返回值正是hash函数的计算结果。这说明最终的hash值保存在xmm0中。

那么当逆向还原的时候，xmm0就应当被赋值为已知的hash值。从最后一行逆向往上看，可以看到，有多个寄存器参与了一系列运算，而此时我们只知道xmm0的值，不能确定其它寄存器中的状态。

事实上，到了最后的运算（即上面代码中175行以后的部分），xmm0~xmm15中的数据都是完全由密钥key和输入数据决定的。在正向执行的情况下，输入数据已知且不会改变。这说明：一种key可以唯一决定一种状态。反之，由于整个运算过程都是可逆的，任意给出一种状态，一定可以确定一个密钥。

因此，这里可以随意设置寄存器状态，赋予任意的值，最后得到一种密钥。方便起见，全部置为0。

一路反着还原回去，最终得到的xmm0~xmm7中保存的就是key的值。

逆向函数如下：

```c++
static void MeowHash_re(void* hash, meow_umm Len, void* SourceInit, unsigned char* rcx)
{
    meow_u128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7; // NOTE(casey): xmm0-xmm7 are the hash accumulation lanes
    meow_u128 xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15; // NOTE(casey): xmm8-xmm15 hold values to be appended (residual, length)

    meow_u8* rax = (meow_u8*)SourceInit + (Len >> 8) * 32;

    movdqu(xmm0, hash);

    pxor_clear(xmm1, xmm1);
    pxor_clear(xmm2, xmm2);
    pxor_clear(xmm3, xmm3);
    pxor_clear(xmm4, xmm4);
    pxor_clear(xmm5, xmm5);
    pxor_clear(xmm6, xmm6);
    pxor_clear(xmm7, xmm7);


    psubq(xmm0, xmm4);
    pxor(xmm4, xmm5);
    pxor(xmm0, xmm1);
    psubq(xmm5, xmm7);
    psubq(xmm4, xmm6);
    psubq(xmm1, xmm3);
    psubq(xmm0, xmm2);


    MEOW_SHUFFLE_re(xmm3, xmm4, xmm5, xmm7, xmm0, xmm1);
    MEOW_SHUFFLE_re(xmm2, xmm3, xmm4, xmm6, xmm7, xmm0);
    MEOW_SHUFFLE_re(xmm1, xmm2, xmm3, xmm5, xmm6, xmm7);
    MEOW_SHUFFLE_re(xmm0, xmm1, xmm2, xmm4, xmm5, xmm6);
    MEOW_SHUFFLE_re(xmm7, xmm0, xmm1, xmm3, xmm4, xmm5);
    MEOW_SHUFFLE_re(xmm6, xmm7, xmm0, xmm2, xmm3, xmm4);
    MEOW_SHUFFLE_re(xmm5, xmm6, xmm7, xmm1, xmm2, xmm3);
    MEOW_SHUFFLE_re(xmm4, xmm5, xmm6, xmm0, xmm1, xmm2);
    MEOW_SHUFFLE_re(xmm3, xmm4, xmm5, xmm7, xmm0, xmm1);
    MEOW_SHUFFLE_re(xmm2, xmm3, xmm4, xmm6, xmm7, xmm0);
    MEOW_SHUFFLE_re(xmm1, xmm2, xmm3, xmm5, xmm6, xmm7);
    MEOW_SHUFFLE_re(xmm0, xmm1, xmm2, xmm4, xmm5, xmm6);



    int unsigned LaneCount = (Len >> 5) & 0x7;
    switch (LaneCount)
    {
        case 7:goto label7;
        case 6:goto label6;
        case 5:goto label5;
        case 4:goto label4;
        case 3:goto label3;
        case 2:goto label2;
        case 1:goto label1;
          
        default:
            goto label0;
    }

    label7:
    MEOW_MIX_re(xmm0, xmm4, xmm6, xmm1, xmm2, rax + 0xc0); 
    label6:
    MEOW_MIX_re(xmm7, xmm3, xmm5, xmm0, xmm1, rax + 0xa0); 
    label5:
    MEOW_MIX_re(xmm6, xmm2, xmm4, xmm7, xmm0, rax + 0x80); 
    label4:
    MEOW_MIX_re(xmm5, xmm1, xmm3, xmm6, xmm7, rax + 0x60); 
    label3:
    MEOW_MIX_re(xmm4, xmm0, xmm2, xmm5, xmm6, rax + 0x40); 
    label2:
    MEOW_MIX_re(xmm3, xmm7, xmm1, xmm4, xmm5, rax + 0x20); 
    label1:
    MEOW_MIX_re(xmm2, xmm6, xmm0, xmm3, xmm4, rax + 0x00); 


    label0:
    pxor_clear(xmm9, xmm9);
    pxor_clear(xmm11, xmm11);


    meow_u8* Last = (meow_u8*)SourceInit + (Len & ~0xf);
    int unsigned Len8 = (Len & 0xf);
    if (Len8)
    {
        // NOTE(casey): Load the mask early
        movdqu(xmm8, &MeowMaskLen[0x10 - Len8]);

        meow_u8* LastOk = (meow_u8*)((((meow_umm)(((meow_u8*)SourceInit) + Len - 1)) | (MEOW_PAGESIZE - 1)) - 16);
        int Align = (Last > LastOk) ? ((int)(meow_umm)Last) & 0xf : 0;
        movdqu(xmm10, &MeowShiftAdjust[Align]);
        movdqu(xmm9, Last - Align);
        pshufb(xmm9, xmm10);

        // NOTE(jeffr): and off the extra bytes
        pand(xmm9, xmm8);
    }

    // NOTE(casey): Next, we have to load the part that _is_ 16-byte aligned
    if (Len & 0x10)
    {
        xmm11 = xmm9;
        movdqu(xmm9, Last - 0x10);
    }

    xmm8 = xmm9;
    xmm10 = xmm9;
    palignr(xmm8, xmm11, 15);
    palignr(xmm10, xmm11, 1);


    pxor_clear(xmm12, xmm12);
    pxor_clear(xmm13, xmm13);
    pxor_clear(xmm14, xmm14);
    movq(xmm15, Len);
    palignr(xmm12, xmm15, 15);
    palignr(xmm14, xmm15, 1);


    MEOW_MIX_REG_re(xmm1, xmm5, xmm7, xmm2, xmm3, xmm12, xmm13, xmm14, xmm15);
    MEOW_MIX_REG_re(xmm0, xmm4, xmm6, xmm1, xmm2, xmm8, xmm9, xmm10, xmm11);


    meow_umm BlockCount = (Len >> 8);
    if (BlockCount > MEOW_PREFETCH_LIMIT)
    {
        // NOTE(casey): For large input, modern Intel x64's can't hit full speed without prefetching, so we use this loop
        while (BlockCount--)
        {
            prefetcht0(rax + MEOW_PREFETCH + 0x00);
            prefetcht0(rax + MEOW_PREFETCH + 0x40);
            prefetcht0(rax + MEOW_PREFETCH + 0x80);
            prefetcht0(rax + MEOW_PREFETCH + 0xc0);

            MEOW_MIX_re(xmm7, xmm3, xmm5, xmm0, xmm1, rax + 0xe0);
            MEOW_MIX_re(xmm6, xmm2, xmm4, xmm7, xmm0, rax + 0xc0);
            MEOW_MIX_re(xmm5, xmm1, xmm3, xmm6, xmm7, rax + 0xa0);
            MEOW_MIX_re(xmm4, xmm0, xmm2, xmm5, xmm6, rax + 0x80);
            MEOW_MIX_re(xmm3, xmm7, xmm1, xmm4, xmm5, rax + 0x60);
            MEOW_MIX_re(xmm2, xmm6, xmm0, xmm3, xmm4, rax + 0x40);
            MEOW_MIX_re(xmm1, xmm5, xmm7, xmm2, xmm3, rax + 0x20);
            MEOW_MIX_re(xmm0, xmm4, xmm6, xmm1, xmm2, rax + 0x00);

            rax -= 0x100;
        }
    }
    else
    {
        // NOTE(casey): For small input, modern Intel x64's can't hit full speed _with_ prefetching (because of port pressure), so we use this loop.
        while (BlockCount--)
        {
            MEOW_MIX_re(xmm7, xmm3, xmm5, xmm0, xmm1, rax + 0xe0);
            MEOW_MIX_re(xmm6, xmm2, xmm4, xmm7, xmm0, rax + 0xc0);
            MEOW_MIX_re(xmm5, xmm1, xmm3, xmm6, xmm7, rax + 0xa0);
            MEOW_MIX_re(xmm4, xmm0, xmm2, xmm5, xmm6, rax + 0x80);
            MEOW_MIX_re(xmm3, xmm7, xmm1, xmm4, xmm5, rax + 0x60);
            MEOW_MIX_re(xmm2, xmm6, xmm0, xmm3, xmm4, rax + 0x40);
            MEOW_MIX_re(xmm1, xmm5, xmm7, xmm2, xmm3, rax + 0x20);
            MEOW_MIX_re(xmm0, xmm4, xmm6, xmm1, xmm2, rax + 0x00);

            rax -= 0x100;
        }
    }

    _mm_storeu_si128((__m128i*)(rcx + 0x00), xmm0);
    _mm_storeu_si128((__m128i*)(rcx + 0x10), xmm1);
    _mm_storeu_si128((__m128i*)(rcx + 0x20), xmm2);
    _mm_storeu_si128((__m128i*)(rcx + 0x30), xmm3);
    _mm_storeu_si128((__m128i*)(rcx + 0x40), xmm4);
    _mm_storeu_si128((__m128i*)(rcx + 0x50), xmm5);
    _mm_storeu_si128((__m128i*)(rcx + 0x60), xmm6);
    _mm_storeu_si128((__m128i*)(rcx + 0x70), xmm7);
}
```

------

上面描述的只是hash函数整体的总结构，并没有涉及到其中的组件。而这些组件是本项目的重点所在，也是最为困难的地方。下面将详细介绍。

最重要的两个部分是**MEOW_MIX_REG**函数和**MEOW_SHUFFLE**函数。其定义如下：

```C
#define MEOW_MIX_REG(r1, r2, r3, r4, r5,  i1, i2, i3, i4) \
aesdec(r1, r2); \
INSTRUCTION_REORDER_BARRIER; \
paddq(r3, i1); \
pxor(r2, i2); \
aesdec(r2, r4); \
INSTRUCTION_REORDER_BARRIER; \
paddq(r5, i3); \
pxor(r4, i4)
```

```c
#define MEOW_SHUFFLE(r1, r2, r3, r4, r5, r6) \
aesdec(r1, r4); \
paddq(r2, r5); \
pxor(r4, r6); \
aesdec(r4, r2); \
paddq(r5, r6); \
pxor(r2, r3)
```

可以看到，其中包括的AES解密，加法，异或运算都是可逆的运算。因此整个函数也是可逆的，只需要将运算的顺序颠倒，并且修改为对应的逆运算即可实现。

对应的逆向过程如下：

```c
#define MEOW_MIX_REG_re(r1, r2, r3, r4, r5,  i1, i2, i3, i4)\
pxor(r4, i4);\
psubq(r5, i3); \
aesdec_inv(r2, r4); \
INSTRUCTION_REORDER_BARRIER; \
pxor(r2, i2); \
psubq(r3, i1); \
aesdec_inv(r1, r2); \
INSTRUCTION_REORDER_BARRIER
```

```c
#define MEOW_SHUFFLE_re(r1, r2, r3, r4, r5, r6) \
pxor(r2, r3);\
psubq(r5, r6); \
aesdec_inv(r4, r2); \
pxor(r4, r6); \
psubq(r2, r5); \
aesdec_inv(r1, r4)
```

其中，加法和异或很简单，改为减法和异或即可。而AES部分出现了困难的问题。

原因是：meow_hash实现中的AES一轮解密基于AES-NI指令集，而该指令集采用了等价解密的设计。这导致一个严重的问题，就是**AES一轮加密并不是一轮解密的逆过程**。这使得该函数的逆向还原出现了困难。

为了解决该问题，我研究了Intel对于AES-NI指令集设计的白皮书，最终找到了正确方法。

首先观察_mm_aesdec_si128函数。

![image-20220727220942925](./aesdec.png)

按顺序执行了逆行移位、逆S盒、逆列混合、异或密钥四个步骤。

既然函数整体找不到一个逆过程，则可以分开处理。也就是说，我们可以分别实现上述四个步骤的逆过程，再反向执行回去。

白皮书中提供了实现这些过程的方法：

![image-20220727221210818](./method.png)

对应的实现如下：

```c
__m128i Shiftrows(__m128i A)
{
    __m128i ISOLATE_SROWS_MASK = _mm_set_epi32(0x0B06010C, 0x07020D08, 0x030E0904, 0x0F0A0500);
    A = _mm_shuffle_epi8(A, ISOLATE_SROWS_MASK);
    return A;
}


__m128i Mixcolumns(__m128i A)
{
    __m128i ZERO = _mm_setzero_si128();
    A = _mm_aesdeclast_si128(A, ZERO);
    A = _mm_aesenc_si128(A, ZERO);
    return A;
}


__m128i SubBytes(__m128i &A)
{
    __m128i ISOLATE_SBOX_MASK = _mm_set_epi32(0x0306090C, 0x0F020508, 0x0B0E0104, 0x070A0D00);
    A = _mm_shuffle_epi8(A, ISOLATE_SBOX_MASK);
    __m128i ZERO = _mm_setzero_si128();
    A = _mm_aesenclast_si128(A, ZERO);
    return A;
}


void aesdec_inv(__m128i &A, __m128i B)
{
    A = _mm_xor_si128(A, B);
    A = Mixcolumns(A);
    A = SubBytes(A);
    A = Shiftrows(A);
}
```

这样，就实现了AES一轮解密整体的逆向。结合前面的总结构，最终实现了hash函数的逆向。

#### 运行结果

最终测试函数如下：

将逆向还原的密钥放入key数组中，并再次使用这个key计算hash，输出最终的hash值。如果还原正确，那么输出的hash值应为给定的**"sdu_cst_20220610"**。

```c++
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
```

结果见下图，完全正确。

![image-20220727215441410](./result.png)

------

#### 运行指导

在本项目文件夹下，打开meow_hash.sln项目文件，进入visual studio执行main函数即可。

在github上下载项目后，打开进入可能被默认设置为64位模式，如果允许出现问题，可能与此有关。应当设置为32位模式。也即将下图中红框出修改为x86。

![image-20220730171052404](./runguide.png)
