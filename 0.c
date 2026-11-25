#include "0h.c"
#include <stdio.h>

U4 total_Malloc = 0;
U4 total_Free = 0;

void alloc_MFF(MFF4 *mff) {
	int fd = memfd_create("", 0);
	assert(fd != -1);
	assert(ftruncate(fd, PAGE_SIZE) != -1);
	assert((mff->buffer = mmap(NULL, PAGE_SIZE * 2, PROT_NONE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) != MAP_FAILED);
	assert(mmap(mff->buffer, PAGE_SIZE, PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_FIXED, fd, 0) != MAP_FAILED);
	assert(mmap(mff->buffer + PAGE_SIZE, PAGE_SIZE, PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_FIXED, fd, 0) != MAP_FAILED);
	close(fd);
}

void print_Token_debug(Token *x) {
	print_Str(x->rule->name);
	fputs("[\033[31m", stdout);
	print_Token(x);
	fputs("\033[0m]", stdout);
}

U4 run_list_once_Lexer(Str str, RuleLexer **rules, Token *last) {
	U4 n = 0;
	RuleLexer *first = *rules;
	RuleLexer *rule = first;
	RuleLexer *next;
	do {
		next = container_of(rule->link.next, RuleLexer, link);
		if ((n = rule->match(str, rule, last))) {
			*rules = next;
			return n;
		}
		rule = next;
	} while (rule != first);
	return 0;
}

extern inline bool is_digit(U1 c);
extern inline bool is_lower(U1 c);
extern inline bool is_upper(U1 c);
extern inline U1 to_lower(U1 c);
extern inline U1 to_upper(U1 c);
extern inline bool is_alpha(U1 c);
extern inline bool is_alnum(U1 c);
extern inline bool is_space(U1 c);
extern inline bool is_printable(U1 c);

void char_repr(U1 c) {
	if (is_printable(c)) {
		if (c == '\\')
			putchar('\\');
		putchar(c);
		return;
	}

	fwrite((U1[3]){
		'\\',
		nibble_upper(c & 0xF),
		nibble_upper(c >> 4),
	}, 1, 3, stdout);
}

void print2_Str(Str x) {
	for (U4 i = 0; i != x.length; i++) {
		U1 c = x.start[i];
		if (!is_alnum(c) && c != '_')
			putchar('~');
		char_repr(c);
	}
}

void print1_Str(Str x) {
	for (U4 i = 0; i != x.length; i++)
		char_repr(x.start[i]);
}

/*
void print_StrEx(StrEx x) {
	if (x.flags & PURE_FLAGS_STREX) {
		print_Str(*Str_of_StrEx(&x));
		return;
	}

	putchar('"');
	U1 *b = x.start + x.length;
	I4 i = -x.length;
	while (i)
		char_repr(b[i++]);
	putchar('"');
}
*/
