#include "0h.c"
#include <stdio.h>

typedef struct Iter Iter;

struct Iter {
	U4 i;
	U4 e;
};

bool next2_Iter(Str str, Iter *iter, U1 *out) {
	if (iter->i == str.length) return false;
	*out = str.start[iter->i++];
	if (*out != '\\') return true;
	if (iter->i == str.length) {
		iter->i--;
		return false;
	}
	*out = str.start[iter->i++];
	if (*out == '\\') {
		iter->e++;
		return true;
	}
	if (iter->i == str.length) {
		iter->i -= 2;
		return false;
	}
	iter->e += 2;
	*out = nibble_to_int(*out) | (nibble_to_int(str.start[iter->i++]) << 4);
	return true;
}

bool next1_Iter(Str str, Iter *iter, U1 *out) {
	Iter pre = *iter;
	bool y = next2_Iter(str, iter, out);
	fputs("|\033[32m", stdout);
	if (y)
		char_repr(*out);
	else
		puts("<EOF>");
	fputs("\033[0m", stdout);
	return y;
}

#define next_Iter next2_Iter

U4 match_spaces(Str str, RuleLexer *rule, Token *org) {
	// puts("match_spaces(...);");
	Iter iter = {0, 0};
	U1 c;
	U4 end;
	do
		end = iter.i;
	while (next_Iter(str, &iter, &c) && is_space(c));
	return end;
}

U4 match_str(Str str, RuleLexer *rule, Token *org) {
	// puts("match_str(...);");
	I4 n;
	Iter iter = {0, 0};
	U1 c;
	I4 consumed;
{
	U4 e1 = 0;
loop:
	consumed = iter.i;
	if (!next_Iter(str, &iter, &c))
		return 0;
	if (is_alnum(c) || c == '_') goto loop;
	if (c == '~') {
		e1++;
		if (!next_Iter(str, &iter, &c))
			return 0;
		goto loop;
	}
	n = (iter.i - iter.e) - e1 - 1;
	if (!n) return 0;
}

	iter = (Iter){0, 0};
	Token *token = create_n_push_Token(rule, org);
	token->data.str.length = n;
	token->data.str.start = Malloc(n);

	U1 *ptr = token->data.str.start + n; 
	n = -n;
	while (n) {
		next_Iter(str, &iter, &c);
		if (c == '~')
			next_Iter(str, &iter, &c);
		ptr[n++] = c;
	}

	return consumed;
}

void print_str_token(Token *x) {
	print1_Str(x->data.str);
}

U4 match_char(Str str, RuleLexer *rule, Token *org) {
	// puts("match_char(...);");
	Iter iter = {0, 0};
	U1 c;
	if (!next_Iter(str, &iter, &c) || is_space(c) || is_alnum(c) || c == '~' || c == '_')
		return 0;

	Token *token = create_n_push_Token(rule, org);
	token->data.bytes[0] = c;

	return iter.i;
}

void print_char_token(Token *x) {
	char_repr(x->data.bytes[0]);
}
