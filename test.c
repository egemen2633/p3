#include "0h.c"
#include "1.c"
#include "2.c"

void print_tokens(Token *org) {
	Token *token = org;
	while ((token = container_of(token->link.next, Token, link)) != org) {
		print_Token_debug(token);
		putchar(' ');
	}
	putchar('\n');
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
	print_tokens(&org);
	putchar('\n');

	full_parse(&org);
	print_tokens(&org);

	Token *expr = container_of(org.link.next, Token, link);
	release_Node(expr->data.generic._0);
	Free(expr);

	printf("Free/Malloc = %u/%u\n", total_Free, total_Malloc);
}
