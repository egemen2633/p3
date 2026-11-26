#include "0h.c"
#include "1.c"
#include "3.c"
#include <unistd.h>

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
		putchar('\n');
	}
	putchar('\n');
}

void eval(Node *node, RuleEval *rule) {
	do {
		print_Node(node->data.ind, 1);
		putchar('\n');
	} while (step_eval(node, rule));
	putchar('\n');
}

void rest_of_main(Token *org) {
	RuleEval concat_str_rule    = {.match = &match_concat_str, .name = cStr("concat_str")};
	RuleEval substr_rule        = {.match = &match_substr,     .name = cStr("substr")};
	RuleEval I_rule             = {.match = &match_I,          .name = cStr("I")};
	RuleEval K_rule 				 = {.match = &match_K,			  .name = cStr("K")};
	RuleEval S_rule 				 = {.match = &match_S, 			  .name = cStr("S")};
	RuleEval uni_ind_rule       = {.match = &match_uni_ind,    .name = cStr("uni_ind")};
	RuleEval left_bin_ind_rule  = {.match = &match_bin_ind,    .name = cStr("left_bin_ind"),  .ctx = (void *)0};
	RuleEval right_bin_ind_rule = {.match = &match_bin_ind,    .name = cStr("right_bin_ind"), .ctx = (void *)1};
	RuleEval pair_rule          = {.match = &match_pair,       .name = cStr("pair")};
	RuleEval left_pair_rule     = {.match = &match_binary,     .name = cStr("left"),          .ctx = (void *)0};
	RuleEval right_pair_rule    = {.match = &match_binary,     .name = cStr("right"),         .ctx = (void *)1};
	RuleEval length_rule        = {.match = &match_str_to_int, .name = cStr("length"),        .ctx = &length_Str};
	RuleEval to_int_rule        = {.match = &match_str_to_int, .name = cStr("$"),             .ctx = &to_int};
	RuleEval add4_rule          = {.match = &match_bin4,       .name = cStr("add4"),          .ctx = &add4};
	RuleEval sub4_rule          = {.match = &match_bin4,       .name = cStr("sub4"),          .ctx = &sub4};
	RuleEval mul4_rule          = {.match = &match_bin4,       .name = cStr("mul4"),          .ctx = &mul4};
	RuleEval neg4_rule          = {.match = &match_uni4,       .name = cStr("-"),             .ctx = &neg4};
	RuleEval sul4_rule          = {.match = &match_bin4,       .name = cStr("sul4"),          .ctx = &sul4};
	RuleEval sdl4_rule          = {.match = &match_bin4,       .name = cStr("sdl4"),          .ctx = &sdl4};
	RuleEval sda4_rule          = {.match = &match_bin4,       .name = cStr("sda4"),          .ctx = &sda4};
	RuleEval and4_rule          = {.match = &match_bin4,       .name = cStr("and4"),          .ctx = &and4};
	RuleEval or4_rule           = {.match = &match_bin4,       .name = cStr("or4"),           .ctx = &or4};
	RuleEval xor4_rule          = {.match = &match_bin4,       .name = cStr("xor4"),          .ctx = &xor4};
	RuleEval not4_rule          = {.match = &match_uni4,       .name = cStr("not4"),          .ctx = &not4};
	RuleEval from_int_rule      = {.match = &match_int_to_str, .name = cStr("from_int"),      .ctx = &from_int};
	RuleEval is_equal_rule 		 = {.match = &match_is_equal,   .name = cStr("is_equal")};
	// RuleEval def_rules 		 = {.match = &match_defs,		  .name = cStr("defs"),				.ctx = org};

	// RuleEval exec_cancel_rule = {.match = &match_exec_cancel, .name = cStr("exec_cancel")};
	// RuleEval exec_atom_cancel_rule = {.match = &match_exec_atom_cancel, .name = cStr("exec_atom_cancel")};
	// RuleEval exec_rule = {.match = &match_exec, .name = cStr("exec")};


{
#define ADD(r) insert_after_SL(&concat_str_rule.link, &(r).link)

	// ADD(def_rules);
	ADD(right_bin_ind_rule); ADD(left_bin_ind_rule);
	ADD(S_rule); ADD(K_rule); ADD(I_rule);
	ADD(right_pair_rule); ADD(left_pair_rule); ADD(pair_rule);
	ADD(is_equal_rule);
	ADD(not4_rule); ADD(xor4_rule); ADD(or4_rule); ADD(and4_rule);
	ADD(sda4_rule); ADD(sdl4_rule); ADD(sul4_rule);
	ADD(neg4_rule); ADD(mul4_rule); ADD(sub4_rule); ADD(add4_rule);
	ADD(from_int_rule); ADD(to_int_rule);
	ADD(length_rule); ADD(substr_rule);

#undef ADD
}

{
	Token *token = container_of(org->link.next, Token, link);
	assert(!cmp_Str(token->rule->name, cStr("def")));
	Def *def = token->data.generic._0;
	fputs("\033[33m*", stdout);
	print2_Str(def->name);
	puts("*\033[0m");
	eval(def->to, &concat_str_rule);
}

	// print_tokens(org);

{
	Token *token = org, *next = container_of(token->link.next, Token, link);
	while ((token = next) != org) {
		assert(!cmp_Str(token->rule->name, cStr("def")));
		Def *def = token->data.generic._0;
		Free(def->name.start);
		release_Node(def->to->data.ind);
		def->to->data.ind = NULL;
		release_Node(def->to);
		Free(def);
		next = container_of(token->link.next, Token, link);
		Free(token);
	}
}
	printf("Free/Malloc = %u/%u\n", total_Free, total_Malloc);
}

// clang test.c -o test 0.c -D_GNU_SOURCE
int main(int argc, char **argv) {
	assert(argc == 2);
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

	FILE *fp = fopen(argv[1], "rb");

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

	// find_names(&org);

	pre_defs(&org);
	convert_atoms(&org);
	// print_tokens(&org);
	full_process(&org);
	print_tokens(&org);

	rest_of_main(&org);
}
