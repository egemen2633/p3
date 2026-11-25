#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

typedef uint8_t U1;
typedef uint32_t U4;
typedef int32_t I4;

extern U4 total_Malloc;
extern U4 total_Free;

static inline void *Malloc(size_t size) {
	total_Malloc++;
	return malloc(size);
}

static inline void Free(void *block) {
	total_Free++;
	free(block);
}

#define container_of(x, T, m) ((T *)((U1 *)(x) - offsetof(T, m)))
#define PAGE_SIZE 0x1000


inline bool is_digit(U1 c) { return (U1)(c - '0') < 10; }
inline bool is_lower(U1 c) { return (U1)(c - 'a') < 26; }
inline bool is_upper(U1 c) { return (U1)(c - 'A') < 26; }
inline U1 to_lower(U1 c) { return c | 0x20; }
inline U1 to_upper(U1 c) { return c & ~0x20; }
inline bool is_alpha(U1 c) { return is_upper(to_upper(c)); }
inline bool is_alnum(U1 c) { return is_alpha(c) || is_digit(c); }
inline bool is_space(U1 c) { return c == ' ' || c == '\n' || c == '\t'; }
inline bool is_printable(U1 c) { return (U1)(c - ' ') <= '~' - ' '; }

static inline U1 nibble_upper(U1 x) {
	static U1 chars[16] = "0123456789ABCDEF";
	return chars[x];
}

static inline U1 nibble_to_int(U1 c) {
	if (is_digit(c)) return c - '0';
	if (is_upper(c)) return c - 'A' + 10;
	if (is_lower(c)) return c - 'a' + 10;
	abort();
}

void char_repr(U1 c);

// static inline U1 get_bit(U1 *x, U1 i) { return (x[i >> 3] >> (i & 7)) & 1; }


typedef struct Str Str;

struct Str {
	U1 *start;
	U4 length;
};

#define cStr(x) ((Str){.start = (U1 *)x, .length = sizeof(x) - 1})

static inline void print_Str(Str x) { fwrite(x.start, 1, x.length, stdout); }
void print1_Str(Str x);
void print2_Str(Str x);

static inline I4 cmp_Str(Str x, Str y) {
	I4 dl = x.length - y.length;
	return dl ? dl : memcmp(x.start, y.start, x.length);
}

/*
static inline Str copy_Str_libc(Str x) {
	U1 *start = Malloc(x.length);
	memcpy(start, x.start, x.length);
	return (Str){.start = start, .length = x.length};
}
*/

/*
#define PURE_FLAGS_STREX 1

typedef struct StrEx StrEx;

struct StrEx {
	U1 *start;
	U4 length;
	U4 flags;
};

static inline Str *Str_of_StrEx(StrEx *x) { return (Str *)x; }

void print_StrEx(StrEx x);
*/

typedef struct MFF4 MFF4;

struct MFF4 {
	U1 *buffer;
	U4 start;
	U4 end;
};

void alloc_MFF(MFF4 *mff);

static inline void free_MFF(MFF4 *mff) { munmap(mff->buffer, 0x2000); }

static inline void consume_MFF(MFF4 *mff, U4 count) {
	mff->start += count;
	if (mff->start < PAGE_SIZE) return;
	mff->start -= PAGE_SIZE;
	mff->end -= PAGE_SIZE;
}

static inline Str str_MFF(MFF4 *mff) {
	return (Str){.start = mff->buffer + mff->start, .length = mff->end - mff->start};
}

static inline U4 read_MFF(MFF4 *mff, FILE *fp) {
	U4 read = fread(mff->buffer + mff->end, 1, PAGE_SIZE - (mff->end - mff->start), fp);
	mff->end += read;
	return read;
}

typedef struct SL SL;

struct SL {
	SL *next;
};

static inline SL *pop_next_SL(SL *prev) {
	SL *x = prev->next;
	prev->next = x->next;
	return x;
}

static inline void insert_after_SL(SL *prev, SL *x) {
	x->next = prev->next;
	prev->next = x;
}


typedef struct DL DL;

struct DL {
	DL *next;
	DL *prev;
};

static inline DL *pop_DL(DL *x) {
	x->prev->next = x->next;
	x->next->prev = x->prev;
	return x;
}

static inline void insert_DL(DL *prev, DL *x, DL *next) {
	x->prev = prev;
	x->next = next;
	prev->next = x;
	next->prev = x;
}

static inline void insert_after_DL(DL *prev, DL *x) {
	insert_DL(prev, x, prev->next);
}

static inline void insert_before_DL(DL *next, DL *x) {
	insert_DL(next->prev, x, next);
}


typedef struct RuleLexer RuleLexer;
typedef struct Token Token;
typedef U4 MatchRule(Str, RuleLexer *, Token *);
typedef void PrintToken(Token *);

struct RuleLexer {
	SL link;
	MatchRule *match;
	PrintToken *print;
	Str name;
	void *ctx;
};

struct Token {
	DL link;
	RuleLexer *rule;
	union {
		struct {
			void *_0;
			U4 _1;
			U4 _2;
		} generic;
		Str str;
		U1 bytes[16];
	} data;
};

static inline void print_Token(Token *x) { x->rule->print(x); }

static inline Token *create_n_push_Token(RuleLexer *rule, Token *org) {
	Token *new = Malloc(sizeof(Token));
	new->rule = rule;

	insert_before_DL(&org->link, &new->link);

	return new;
}

void print_Token_debug(Token *x);

U4 run_list_once_Lexer(Str str, RuleLexer **rules, Token *last);

typedef U4 RefC;
typedef RefC *Handle;
typedef void Destructor(Handle);

static inline Handle copy_Handle(Handle x) {
	(*x)++;
	return x;
}

static inline void release_Handle(Handle x, Destructor *destructor) {
	if (!--(*x))
		destructor(x);
}
