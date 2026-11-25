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

U4 length_Str(Str x) {
   return x.length;
}

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
	to_int_rule.link.next = NULL;

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
