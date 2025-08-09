#ifndef PEPE_ENCODING_H
#define PEPE_ENCODING_H

char *Pepe_Base64Map = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

u32
Pepe_Base64GetLength(u32 strLength)
{
  return strLength * 4 / 3 + 4;
}

// 20 * 4 / 3 + 4
#define PEPE_SHA1_BASE64_LENGTH 30

Pepe_String
Pepe_Base64Encode(Pepe_String from, Pepe_Slice memory)
{
  u64 inIndex;
  u64 outIndex;
  byte *out;

  memset(memory.base, 0, memory.capacity);
  inIndex = 0;
  outIndex = 0;
  out = (byte *)memory.base;

  while (inIndex < from.length) {
    out[outIndex++] = Pepe_Base64Map[(from.base[inIndex] & 0xFC) >> 2];
    if ((inIndex + 1) == from.length) {
      out[outIndex++] = Pepe_Base64Map[ ((from.base[inIndex] & 0x3) << 4) ];
      out[outIndex++] = '=';
      out[outIndex++] = '=';
      break;
    }

    out[outIndex++] = Pepe_Base64Map[ ((from.base[inIndex] & 0x3) << 4) | ((from.base[inIndex + 1] & 0xF0) >> 4) ];

    if ((inIndex + 2) == from.length) {
      out[outIndex++] = Pepe_Base64Map[ ((from.base[inIndex + 1] & 0xF) << 2) ];
      out[outIndex++] = '=';
      break;
    }

    out[outIndex++] = Pepe_Base64Map[ ((from.base[inIndex + 1] & 0xF) << 2) | ((from.base[inIndex + 2] & 0xC0) >> 6) ];

    // fourth 6-bit
    out[outIndex++] = Pepe_Base64Map[ from.base[inIndex + 2] & 0x3F ];

    inIndex += 3;
  }
  out[outIndex] = '\0';

  memory.length = outIndex;

  return Pepe_StringInit(memory);
}


#define PEPE_SHA1Rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* PEP_SHA1_BLK0() and PEP_SHA1_BLK() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#define PEPE_SHA1_BLK0_LE(i) (block->l[i] = (PEPE_SHA1Rol(block->l[i],24)&0xFF00FF00) \
    |(PEPE_SHA1Rol(block->l[i],8)&0x00FF00FF))
#define PEPE_SHA1_BLK0_BE(i) block->l[i]

#define PEP_SHA1_BLK0(i) (Pepe_Endian.c == 0 ? PEPE_SHA1_BLK0_BE(i) : PEPE_SHA1_BLK0_LE(i))

#define PEP_SHA1_BLK(i) (block->l[i&15] = PEPE_SHA1Rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+PEP_SHA1_BLK0(i)+0x5A827999+PEPE_SHA1Rol(v,5);w=PEPE_SHA1Rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+PEP_SHA1_BLK(i)+0x5A827999+PEPE_SHA1Rol(v,5);w=PEPE_SHA1Rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+PEP_SHA1_BLK(i)+0x6ED9EBA1+PEPE_SHA1Rol(v,5);w=PEPE_SHA1Rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+PEP_SHA1_BLK(i)+0x8F1BBCDC+PEPE_SHA1Rol(v,5);w=PEPE_SHA1Rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+PEP_SHA1_BLK(i)+0xCA62C1D6+PEPE_SHA1Rol(v,5);w=PEPE_SHA1Rol(w,30);

typedef struct Pepe_SHA1Context Pepe_SHA1Context;
struct Pepe_SHA1Context {
    u32 state[5];
    u32 count[2];
    u8 buffer[64];
};

/* Hash a single 512-bit block. This is the core of the algorithm. */

void SHA1Transform(
    u32 state[5],
    u8 buffer[64]
)
{
    u32 a, b, c, d, e;

    typedef union
    {
        u8 c[64];
        u32 l[16];
    } CHAR64LONG16;

    CHAR64LONG16 block[1];      /* use array to appear as a pointer */

    memcpy(block, buffer, 64);

    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a, b, c, d, e, 0);
    R0(e, a, b, c, d, 1);
    R0(d, e, a, b, c, 2);
    R0(c, d, e, a, b, 3);
    R0(b, c, d, e, a, 4);
    R0(a, b, c, d, e, 5);
    R0(e, a, b, c, d, 6);
    R0(d, e, a, b, c, 7);
    R0(c, d, e, a, b, 8);
    R0(b, c, d, e, a, 9);
    R0(a, b, c, d, e, 10);
    R0(e, a, b, c, d, 11);
    R0(d, e, a, b, c, 12);
    R0(c, d, e, a, b, 13);
    R0(b, c, d, e, a, 14);
    R0(a, b, c, d, e, 15);
    R1(e, a, b, c, d, 16);
    R1(d, e, a, b, c, 17);
    R1(c, d, e, a, b, 18);
    R1(b, c, d, e, a, 19);
    R2(a, b, c, d, e, 20);
    R2(e, a, b, c, d, 21);
    R2(d, e, a, b, c, 22);
    R2(c, d, e, a, b, 23);
    R2(b, c, d, e, a, 24);
    R2(a, b, c, d, e, 25);
    R2(e, a, b, c, d, 26);
    R2(d, e, a, b, c, 27);
    R2(c, d, e, a, b, 28);
    R2(b, c, d, e, a, 29);
    R2(a, b, c, d, e, 30);
    R2(e, a, b, c, d, 31);
    R2(d, e, a, b, c, 32);
    R2(c, d, e, a, b, 33);
    R2(b, c, d, e, a, 34);
    R2(a, b, c, d, e, 35);
    R2(e, a, b, c, d, 36);
    R2(d, e, a, b, c, 37);
    R2(c, d, e, a, b, 38);
    R2(b, c, d, e, a, 39);
    R3(a, b, c, d, e, 40);
    R3(e, a, b, c, d, 41);
    R3(d, e, a, b, c, 42);
    R3(c, d, e, a, b, 43);
    R3(b, c, d, e, a, 44);
    R3(a, b, c, d, e, 45);
    R3(e, a, b, c, d, 46);
    R3(d, e, a, b, c, 47);
    R3(c, d, e, a, b, 48);
    R3(b, c, d, e, a, 49);
    R3(a, b, c, d, e, 50);
    R3(e, a, b, c, d, 51);
    R3(d, e, a, b, c, 52);
    R3(c, d, e, a, b, 53);
    R3(b, c, d, e, a, 54);
    R3(a, b, c, d, e, 55);
    R3(e, a, b, c, d, 56);
    R3(d, e, a, b, c, 57);
    R3(c, d, e, a, b, 58);
    R3(b, c, d, e, a, 59);
    R4(a, b, c, d, e, 60);
    R4(e, a, b, c, d, 61);
    R4(d, e, a, b, c, 62);
    R4(c, d, e, a, b, 63);
    R4(b, c, d, e, a, 64);
    R4(a, b, c, d, e, 65);
    R4(e, a, b, c, d, 66);
    R4(d, e, a, b, c, 67);
    R4(c, d, e, a, b, 68);
    R4(b, c, d, e, a, 69);
    R4(a, b, c, d, e, 70);
    R4(e, a, b, c, d, 71);
    R4(d, e, a, b, c, 72);
    R4(c, d, e, a, b, 73);
    R4(b, c, d, e, a, 74);
    R4(a, b, c, d, e, 75);
    R4(e, a, b, c, d, 76);
    R4(d, e, a, b, c, 77);
    R4(c, d, e, a, b, 78);
    R4(b, c, d, e, a, 79);
    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    /* Wipe variables */
    a = b = c = d = e = 0;
    memset(block, '\0', sizeof(block));
}


/* Pepe_SHA1Init - Initialize new context */

void Pepe_SHA1Init(
    Pepe_SHA1Context * context
)
{
    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

void Pepe_SHA1Update(
    Pepe_SHA1Context * context,
    u8 *data,
    u32 len
)
{
    u32 i;

    u32 j;

    j = context->count[0];
    if ((context->count[0] += len << 3) < j)
        context->count[1]++;
    context->count[1] += (len >> 29);
    j = (j >> 3) & 63;
    if ((j + len) > 63)
    {
        memcpy(&context->buffer[j], data, (i = 64 - j));
        SHA1Transform(context->state, context->buffer);
        for (; i + 63 < len; i += 64)
        {
            SHA1Transform(context->state, &data[i]);
        }
        j = 0;
    }
    else
        i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);
}


/* Add padding and return the message digest. */

void Pepe_SHA1Final(
    u8 digest[20],
    Pepe_SHA1Context * context
)
{
    u32 i;

    u8 finalcount[8];

    u8 c;


    for (i = 0; i < 8; i++)
    {
        finalcount[i] = (unsigned char) ((context->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);      /* Endian independent */
    }
    c = 0200;
    Pepe_SHA1Update(context, &c, 1);
    while ((context->count[0] & 504) != 448)
    {
        c = 0000;
        Pepe_SHA1Update(context, &c, 1);
    }
    Pepe_SHA1Update(context, finalcount, 8); /* Should cause a SHA1Transform() */
    for (i = 0; i < 20; i++)
    {
        digest[i] = (unsigned char)
            ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }
    /* Wipe variables */
    memset(context, '\0', sizeof(*context));
    memset(&finalcount, '\0', sizeof(finalcount));
}

void Pepe_SHA1(
    byte *hash_out,
    Pepe_String string
)
{
    u32 len;
    byte *str;
    Pepe_SHA1Context ctx;
    u32 ii;

    str = string.base;
    len = (u32)string.length;
    

    Pepe_SHA1Init(&ctx);
    for (ii=0; ii<len; ii+=1)
        Pepe_SHA1Update(&ctx, str + ii, 1);
    Pepe_SHA1Final(hash_out, &ctx);
}

#endif
