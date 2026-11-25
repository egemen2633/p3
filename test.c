#include "0h.c"
#include "1.c"
#include "3.c"
#include <unistd.h>

U4 to_int(Str x) {
   U4 out = 0;
   U4 i = x.length;
   while (i--)
      out = out << 4 | nibble_to_int(x.start[i]);
   return out;
}

Str from_int(U4 x) {
	U4 length = x ? 8 - (__builtin_clz(x) >> 2) : 1;
	U1 *out = Malloc(length);
	U1 *ptr = out + length;
	I4 i = -length;
	while (i) {
		ptr[i] = nibble_upper(x & 0xF);
		x >>= 4;
		i++;
	}
	return (Str){out, length};
}

U4 length_Str(Str x) { return x.length; }

U4 add4(U4 x, U4 y) { return x + y; }
U4 sub4(U4 x, U4 y) { return x - y; }
U4 mul4(U4 x, U4 y) { return x * y; }
U4 neg4(U4 x) { return -x; }

U4 sul4(U4 x, U4 y) { return x << (y & 31); }
U4 sdl4(U4 x, U4 y) { return x >> (y & 31); }
U4 sda4(U4 x, U4 y) { return (I4)x >> (y & 31); }

U4 and4(U4 x, U4 y) { return x & y; }
U4 or4(U4 x, U4 y) { return x | y; }
U4 xor4(U4 x, U4 y) { return x ^ y; }
U4 not4(U4 x) { return ~x; }

void print_tokens(Token *org) {
	Token *token = org;
	while ((token = container_of(token->link.next, Token, link)) != org) {
		print_Token_debug(token);
		putchar(' ');
	}
	putchar('\n');
}

void rest_of_main(Node *node) {
	RuleEval concat_str_rule;
	concat_str_rule.match = &match_concat_str;
	concat_str_rule.name = cStr("concat_str");

	RuleEval substr_rule;
	substr_rule.match = &match_substr;
	substr_rule.name = cStr("substr");

	RuleEval I_rule;
	I_rule.match = &match_I;
	I_rule.name = cStr("I");

	RuleEval uni_ind_rule;
	uni_ind_rule.match = &match_uni_ind;
	uni_ind_rule.name = cStr("uni_ind");

	RuleEval left_bin_ind_rule;
	left_bin_ind_rule.match = &match_bin_ind;
	left_bin_ind_rule.name = cStr("left_bin_ind");
	left_bin_ind_rule.ctx = 0;

	RuleEval right_bin_ind_rule;
	right_bin_ind_rule.match = &match_bin_ind;
	right_bin_ind_rule.name = cStr("right_bin_ind");
	right_bin_ind_rule.ctx = (void *)1;

	RuleEval pair_rule;
	pair_rule.match = &match_pair;
	pair_rule.name = cStr("pair");

	RuleEval left_pair_rule;
	left_pair_rule.match = &match_binary;
	left_pair_rule.name = cStr("left");
	left_pair_rule.ctx = 0;

	RuleEval right_pair_rule;
	right_pair_rule.match = &match_binary;
	right_pair_rule.name = cStr("right");
	right_pair_rule.ctx = (void *)1;

	RuleEval length_rule;
	length_rule.match = &match_str_to_int;
	length_rule.name = cStr("length");
	length_rule.ctx = &length_Str;

	RuleEval to_int_rule;
	to_int_rule.match = &match_str_to_int;
	to_int_rule.name = cStr("$");
	to_int_rule.ctx = &to_int;

	RuleEval add4_rule;
	add4_rule.match = &match_bin4;
	add4_rule.name = cStr("add4");
	add4_rule.ctx = &add4;

	RuleEval sub4_rule;
	sub4_rule.match = &match_bin4;
	sub4_rule.name = cStr("sub4");
	sub4_rule.ctx = &sub4;

	RuleEval mul4_rule;
	mul4_rule.match = &match_bin4;
	mul4_rule.name = cStr("mul4");
	mul4_rule.ctx = &mul4;

	RuleEval neg4_rule;
	neg4_rule.match = &match_uni4;
	neg4_rule.name = cStr("neg4");
	neg4_rule.ctx = &neg4;

	RuleEval sul4_rule;
	sul4_rule.match = &match_bin4;
	sul4_rule.name = cStr("sul4");
	sul4_rule.ctx = &sul4;

	RuleEval sdl4_rule;
	sdl4_rule.match = &match_bin4;
	sdl4_rule.name = cStr("sdl4");
	sdl4_rule.ctx = &sdl4;

	RuleEval sda4_rule;
	sda4_rule.match = &match_bin4;
	sda4_rule.name = cStr("sda4");
	sda4_rule.ctx = &sda4;

	RuleEval and4_rule;
	and4_rule.match = &match_bin4;
	and4_rule.name = cStr("and4");
	and4_rule.ctx = &and4;

	RuleEval or4_rule;
	or4_rule.match = &match_bin4;
	or4_rule.name = cStr("or4");
	or4_rule.ctx = &or4;

	RuleEval xor4_rule;
	xor4_rule.match = &match_bin4;
	xor4_rule.name = cStr("xor4");
	xor4_rule.ctx = &xor4;

	RuleEval not4_rule;
	not4_rule.match = &match_uni4;
	not4_rule.name = cStr("not4");
	not4_rule.ctx = &not4;

	RuleEval from_int_rule;
	from_int_rule.match = &match_int_to_str;
	from_int_rule.name = cStr("from_int");
	from_int_rule.ctx = &from_int;


	concat_str_rule.link.next = &substr_rule.link;
	substr_rule.link.next = &I_rule.link;
	I_rule.link.next = &left_bin_ind_rule.link;
	// uni_ind_rule.link.next = &left_bin_ind_rule.link;
	left_bin_ind_rule.link.next = &right_bin_ind_rule.link;
	right_bin_ind_rule.link.next = &pair_rule.link;
	pair_rule.link.next = &left_pair_rule.link;
	left_pair_rule.link.next = &right_pair_rule.link;
	right_pair_rule.link.next = &length_rule.link;
	length_rule.link.next = &to_int_rule.link;
	to_int_rule.link.next = &add4_rule.link;

	add4_rule.link.next = &sub4_rule.link;
	sub4_rule.link.next = &mul4_rule.link;
	mul4_rule.link.next = &neg4_rule.link;
	neg4_rule.link.next = &sul4_rule.link;
	sul4_rule.link.next = &sdl4_rule.link;
	sdl4_rule.link.next = &sda4_rule.link;
	sda4_rule.link.next = &and4_rule.link;
	and4_rule.link.next = &or4_rule.link;
	or4_rule.link.next = &xor4_rule.link;
	xor4_rule.link.next = &not4_rule.link;
	not4_rule.link.next = &from_int_rule.link;

	from_int_rule.link.next = NULL;

	print_Node(node, 1);
	putchar('\n');
	while (step_eval(node, &concat_str_rule)) {
		print_Node(node, 1);
		putchar('\n');
	}

	release_Node(node);
}

// clang test.c -o test 0.c -D_GNU_SOURCE
int main() {
	RuleLexer str_rule = {
		.match = &match_str,
		.print = &print_str_token,
		.name = cStr("str")
	};
	RuleLexer space_rule = {
		.match = &match_spaces,
		.name = cStr("space")
	};
	RuleLexer char_rule = {
		.match = &match_char,
		.print = &print_char_token,
		.name = cStr("char")
	};
	RuleLexer null_rule = {
		.name = cStr("null")
	};

	space_rule.link.next = &str_rule.link;
	str_rule.link.next = &char_rule.link;
	char_rule.link.next = &space_rule.link;

	MFF4 mff = {.start = 0, .end = 0};
	alloc_MFF(&mff);

	Token org;
	org.rule = &null_rule;
	org.link.next = &org.link;
	org.link.prev = &org.link;

	FILE *fp = fopen("test.txt", "rb");

	RuleLexer *rule = &space_rule;
	pull: {
		U4 plus = PAGE_SIZE - (mff.end - mff.start);
		U4 read = fread(mff.buffer + mff.end, 1, plus, fp);
		if (read == 0) goto end;
		mff.end += read;
	} consume: {
		U4 consumed = run_list_once_Lexer(str_MFF(&mff), &rule, &org);
		consume_MFF(&mff, consumed);
		if (consumed && mff.end != mff.start) goto consume;
		goto pull;
	} end:

	fclose(fp);
	free_MFF(&mff);

	// print_tokens(&org);

	convert_atoms(&org);
	// find_vars(&org);
	// print_tokens(&org);
	// putchar('\n');

	full_parse(&org);
	// print_tokens(&org);

	Token *expr_token = container_of(org.link.next, Token, link);
	Node *expr = expr_token->data.generic._0;
	Free(expr_token);

	printf("Free/Malloc = %u/%u\n\n", total_Free, total_Malloc);
	rest_of_main(expr);
	printf("\nFree/Malloc = %u/%u\n", total_Free, total_Malloc);
}
