#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t mid = (uint32_t)(challenge >> 16);

    uint32_t step0 = (low * 5) ^ (high + 0x37c5);
    uint32_t step1 = ((mid ^ 0x765d) + high * 4);
    uint32_t step2 = (low + 0xa78d) ^ step1;

    uint32_t result = step2 ^ step0;

    uint32_t values[8] = {0};
    values[0] = result;
    size_t count = 1;

    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};

    values[0] = ((uint32_t)(challenge >> 21) & 0x1F) + 5;

    int ecx = 19;
    for (int d = 1; d < 6; ++d) {
        uint32_t part = (uint32_t)(challenge >> ecx) & 0x1F;
        values[d] = (part ^ d) + 4 * values[d - 1];
        ecx += 5;
    }
    
    size_t count = 6;
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t val0 = (uint32_t)(challenge & 7);
    uint32_t x = (uint32_t)(challenge >> 32) ^ (uint32_t)(challenge >> 3);
    uint32_t val1 = 0;

    switch (val0) {
        case 0: val1 = (x ^ 0x1769u) + (x >> 1); break;   
        case 1: val1 = (x * 4u) ^ 0x1c93u; break;        
        case 2: val1 = (x * 5u) + 0xf7d6u; break;         
        case 3: val1 = (x * 5u) + 0x273d0u; break;       
        case 4: val1 = (x * 5u) + 0x19a3cu; break;       
        case 5: val1 = (x * 2u) ^ 0xb925u; break;        
        case 6: val1 = (x ^ 0xa8e7u) + (x >> 2); break;  
        case 7: val1 = (x * 5u) + 0x3f8bu; break;        
    }

    values[0] = val0;
    values[1] = val1;
    return write_answer(out, cap, values, 2);
}

static uint32_t p4_walk(uint32_t target, uint32_t low, uint32_t high, uint32_t context) {
    uint32_t mid = low + ((high - low) >> 1);
    if (mid == target) {
        return (mid + 0x44) ^ context;
    }
    uint32_t ebx = mid + context;
    if (target < mid) {
        uint32_t ret = p4_walk(target, low, mid - 1, context);
        return ((ret * 3) ^ ebx) ^ 0x8;
    } else {
        uint32_t ret = p4_walk(target, mid + 1, high, context);
        return ((ret * 3) ^ ebx) ^ 0x1d;
    }
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    
    uint32_t target = (uint32_t)(challenge & 0x7f);
    uint32_t context = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    
    uint32_t ans = p4_walk(target, 0, 0x7f, context);
    
    values[0] = ans;
    return write_answer(out, cap, values, 1);
}


struct p5_node { int next; uint32_t b; uint32_t c; };

static const struct p5_node p5_nodes[12] = {
    /* 0  @0x405ba0 */ {  3, 2603u, 32071u },
    /* 1  @0x405bb0 */ {  9, 2876u, 18921u },
    /* 2  @0x405bc0 */ { 11, 1837u,   727u },
    /* 3  @0x405bd0 */ {  4, 1337u, 21163u },
    /* 4  @0x405be0 */ {  2, 1055u, 29023u },
    /* 5  @0x405bf0 */ { -1,  349u, 31708u },
    /* 6  @0x405c00 */ {  5, 2384u, 22690u },
    /* 7  @0x405c10 */ {  6, 2124u, 42497u },
    /* 8  @0x405c20 */ { 10,  814u, 52826u },
    /* 9  @0x405c30 */ {  7,   23u, 12156u },
    /* 10 @0x405c40 */ {  0,  594u, 19442u },
    /* 11 @0x405c50 */ {  1, 1576u,  6611u },
};

static const int p5_table[8] = { 8, 10, 0, 3, 4, 2, 11, 1 };

static uint32_t p5_traverse(uint64_t challenge, uint32_t *slot) {
    int n = p5_table[(uint32_t)(challenge & 7u)];
    uint32_t eax = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    eax ^= 0x57ebu;
    eax = eax * 3u + p5_nodes[n].c;
    *slot = p5_nodes[n].b;
    eax ^= p5_nodes[n].b;

    uint32_t steps = (uint32_t)((challenge >> 3) & 3u);
    for (uint32_t i = 0; i < steps; i++) {
        n = p5_nodes[n].next;
        eax = eax * 3u + p5_nodes[n].c;
        eax ^= p5_nodes[n].b;
        *slot = p5_nodes[n].b;
    }
    return eax;
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};

    values[0] = p5_traverse(challenge, &values[1]);
    size_t count = 2;

    return write_answer(out, cap, values, count);
}


static uint32_t p6_rol32(uint32_t x, unsigned n) {
    return (x << n) | (x >> (32u - n));
}
static uint32_t p6_ror32(uint32_t x, unsigned n) {
    return (x >> n) | (x << (32u - n));
}

static uint32_t p6_op_0(uint32_t x) { return p6_rol32(x, 5) ^ 0xba82u; }
static uint32_t p6_op_1(uint32_t x) { return p6_ror32(x, 2); }
static uint32_t p6_op_2(uint32_t x) { return (x + 0x550eu) ^ (x >> 7); }
static uint32_t p6_op_3(uint32_t x) { return ((x << 5) + 0x2db8u) ^ (7u * x); }
static uint32_t p6_op_4(uint32_t x) { return (x ^ 0x99d5u) + (x << 5); }
static uint32_t p6_op_5(uint32_t x) { return ((x >> 7) ^ x) + 0x1f0fu; }

typedef uint32_t (*p6_fn)(uint32_t);

static const p6_fn p6_table[8] = {
    p6_op_4, p6_op_5, p6_op_2, p6_op_4,
    p6_op_1, p6_op_3, p6_op_3, p6_op_0,
};

static uint32_t p6_dispatch(uint64_t challenge) {
    uint32_t arg = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    uint32_t r1 = p6_table[(uint32_t)(challenge & 7u)](arg);
    uint32_t r2 = p6_table[(uint32_t)((challenge >> 3) & 7u)](r1 ^ 0xfa14u);
    return r2 + 0x7a39u;
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};

    values[0] = p6_dispatch(challenge);
    size_t count = 1;

    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t a = (uint32_t)(context & 3u);
    uint32_t esi = (uint32_t)(challenge >> 32) ^ (uint32_t)challenge;
    uint32_t r8d = (uint32_t)(context >> 32) ^ (uint32_t)context;
    uint32_t r9d = a * 0x8321u ^ (uint32_t)(context >> 16);

    uint32_t edi = esi ^ r8d;
    uint32_t mix = (edi * 3u + 0x8321u) ^ 0xe0e7u;

    uint32_t b = mix ^ r9d;
    uint32_t edx = r8d + a + 0xe0e7u + (esi >> 1);
    uint32_t c = b ^ edx;

    int n = snprintf(out, cap,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "OPEN%" PRIu32 " %" PRIu32 " %" PRIu32,
        a, b, c);
    return n < 0 || (size_t)n >= cap;
}
