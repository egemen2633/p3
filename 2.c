#include "0h.c"
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


U4 to_int(Str x) {
   U4 out = 0;
   U4 i = x.length;
   while (i--)
      out = out << 4 | nibble_to_int(x.start[i]);
   return out;
}

U4 length_from_int(U4 x) {
	return x ? 8 - (__builtin_clz(x) >> 2) : 1;
}

U4 _from_int(U4 x, U4 length, U1 *out) {
	U1 *ptr = out + length;
	I4 i = -length;
	while (i) {
		ptr[i] = nibble_upper(x & 0xF);
		x >>= 4;
		i++;
	}
	return length;
}

Str from_int(U4 x) {
	U4 length = length_from_int(x);
	U1 *out = Malloc(length);
	_from_int(x, length, out);
	return (Str){out, length};
}


#define BIN_TYPE_FLAG 0b10
#define ATOM_TYPE_FLAG 0
#define IND_TYPE_FLAG 1
#define APP_BIN_FLAG 0
#define PAIR_BIN_FLAG 1
#define TYPE_FLAG_NODE 0b11
#define NAMED_FLAG_NODE 4
#define ONE_TIME_FLAG_NODE 8

typedef struct Node Node;

struct Node {
	RefC refc;
	U4 flags;
	union {
		Str atom;
		struct {
			Node *left;
			Node *right;
		} bin;
		Node *ind;
	} data;
};

Destructor destructor_Node;

void release_Node(Node *x) {
	release_Handle(&x->refc, &destructor_Node);
}

void destructor_Node(Handle h) {
	Node *x = container_of(h, Node, refc);

	if (x->flags & BIN_TYPE_FLAG) {
		release_Node(x->data.bin.left);
		release_Node(x->data.bin.right);
	} else if (x->flags & IND_TYPE_FLAG)
		release_Node(x->data.ind);
	else
		Free(x->data.atom.start);

	Free(x);
}

void print_Node(Node *x, U4 outer_n_side) {
	if (x->flags & ONE_TIME_FLAG_NODE) fputs("\033[31m", stdout);
	switch (x->flags & TYPE_FLAG_NODE) {
	case ATOM_TYPE_FLAG: {
		print2_Str(x->data.atom);
		break;
	}
	case IND_TYPE_FLAG: {
		if (x->flags & NAMED_FLAG_NODE) {
			fputs("...", stdout);
			break;
		}
		putchar('[');
		print_Node(x->data.ind, 1);
		putchar(']');
		break;
	}
	case APP_BIN_FLAG | BIN_TYPE_FLAG: {
		if (outer_n_side == 2) putchar('(');
		print_Node(x->data.bin.left, 0);
		putchar(' ');
		print_Node(x->data.bin.right, 2);
		if (outer_n_side == 2) putchar(')');
		break;
	}
	case PAIR_BIN_FLAG | BIN_TYPE_FLAG: {
		if (outer_n_side != 1) putchar('(');
		print_Node(x->data.bin.left, 1);
		putchar(',');
		print_Node(x->data.bin.right, 3);
		if (outer_n_side != 1) putchar(')');
		break;
	}
	}
	if (x->flags & ONE_TIME_FLAG_NODE) {
		fputs("\033[0m", stdout);
		x->flags ^= ONE_TIME_FLAG_NODE;
	}
}

Node *copy_Node(Node *x) {
	x->refc++;
	return x;
}

Node *new_atom_Node(/*owner transfer*/ Str str) {
	Node *x = Malloc(sizeof(Node));
	x->refc = 1;
	x->flags = ATOM_TYPE_FLAG;
	x->data.atom = str;
	return x;
}

Node *new_bin_Node(Node *left, Node *right) {
	Node *x = Malloc(sizeof(Node));
	x->refc = 1;
	x->flags = BIN_TYPE_FLAG;
	x->data.bin.left = left;
	x->data.bin.right = right;
	return x;
}

void print_Token_Node(Token *x) {
	print_Node(x->data.generic._0, 1);
}

RuleLexer node_rule = {
	.name = cStr("node"),
	.print = &print_Token_Node
};

typedef struct Def Def;

struct Def {
	Str name;
	Node *to;
	Def *next;
};

void print_Token_Def(Token *x) {
	Def *def = x->data.generic._0;
	print2_Str(def->name);
	fputs(" = ", stdout);
	if (def->to->data.ind)
		print_Node(def->to->data.ind, 1);
	else
	 	fputs("NULL", stdout);
}

RuleLexer def_rule = {
	.name = cStr("def"),
	.print = &print_Token_Def
};

void convert_atoms(Token *org) {
	Token *token = org;
	Token *token_first_def = container_of(token->link.next, Token, link);
	assert(!cmp_Str(token_first_def->rule->name, cStr("def")));
	Def *first_def = token_first_def->data.generic._0;
out:
	token = container_of(token->link.next, Token, link);
	if (token == org) return;
	// print_Token_debug(token);
	// putchar('\n');

	if (!cmp_Str(token->rule->name, cStr("def"))) goto in;
	goto out;
in:
	token = container_of(token->link.next, Token, link);
	if (token == org) return;
	// putchar('\t');
	// print_Token_debug(token);
	// putchar('\n');

	if (!cmp_Str(token->rule->name, cStr("char"))) {
		if (token->data.bytes[0] == ';') goto out;
		goto in;
	}
	if (cmp_Str(token->rule->name, cStr("str"))) goto in;

	token->rule = &node_rule;
{
	Def *def = first_def;
	do {
		if (!cmp_Str(def->name, token->data.str)) {
			Free(token->data.str.start);
			token->data.generic._0 = copy_Node(def->to);
			goto in;
		}
		def = def->next;
	} while (def);
}
	token->data.generic._0 = new_atom_Node(token->data.str);
	goto in;
}

/*
void find_names(Token *org) {
	Token *token = org;
	while ((token = container_of(token->link.next, Token, link)) != org) {
		if (cmp_Str(token->rule->name, cStr("char")) || token->data.bytes[0] != '%') continue;
		token = container_of(token->link.next, Token, link);
		Free(pop_DL(token->link.prev));
		Node *node = token->data.generic._0;
		node->flags |= NAME_FLAG_NODE;
	}
}
*/

bool is_char_token(Token *t) {
	return !cmp_Str(t->rule->name, cStr("char"));
}

bool is_node_token(Token *t) {
	return !cmp_Str(t->rule->name, cStr("node"));
}

bool is_null_token(Token *t) {
	return !cmp_Str(t->rule->name, cStr("null"));
}

void app_parse(Token *t0, Token *t1) {
	Node *new = new_bin_Node(t0->data.generic._0, t1->data.generic._0);
	t0->data.generic._0 = new;
	Free(pop_DL(&t1->link));
}

void pair_parse(Token *t0, Token *comma, Token *t1) {
	Free(pop_DL(&comma->link));
	Node *new = new_bin_Node(t0->data.generic._0, t1->data.generic._0);
	new->flags |= PAIR_BIN_FLAG;
	t0->data.generic._0 = new;
	Free(pop_DL(&t1->link));
}

void full_parse(Token *left) {
	Token *t0 = container_of(left->link.next, Token, link);
	Token *t1 = container_of(t0->link.next, Token, link);

	if (is_char_token(t0)) {
		assert(t0->data.bytes[0] == '('); // (|(
		full_parse(t0);
		full_parse(left);
		return;
	}

	assert(is_node_token(t0)); // (|a

	// if (is_null_token(t1)) return; // (|a|#

	if (is_char_token(t1)) {
		if (t1->data.bytes[0] == ';') return; // (|a|,
		if (t1->data.bytes[0] == ',') goto continue_0; // (|a|,
		if (t1->data.bytes[0] == ')') { // (|a|)
			Free(pop_DL(&left->link));
			Free(pop_DL(&t1->link));
			return; // a
		}
		assert(t1->data.bytes[0] == '('); // (|a|(
		full_parse(t1);
		full_parse(left);
		return;
	}

	assert(is_node_token(t1)); // (|a|a

	app_parse(t0, t1);
	full_parse(left);
	return;

continue_0: { // (|a|,
	Token *t2 = container_of(t1->link.next, Token, link);
	if (is_char_token(t2)) {
		assert(t2->data.bytes[0] == '('); // (|a|,|(
		full_parse(t2);
		full_parse(left);
		return;
	}

	assert(is_node_token(t2)); // (|a|,|a
label_1: {
	Token *t3 = container_of(t2->link.next, Token, link);

	if (is_node_token(t3)) { // (|a|,|a|a
		app_parse(t2, t3);
		goto label_1; // (|a|,|a a
	}

	assert(is_char_token(t3));

	if (t3->data.bytes[0] == ';') {
		pair_parse(t0, t1, t2);
		return;
	}

	if (t3->data.bytes[0] == '(') { // (|a|,|a|(
		full_parse(t3);
		full_parse(left);
		return;
	}

	if (t3->data.bytes[0] == ',') { // (|a|,|a|,
		pair_parse(t0, t1, t2);
		t1 = t3;
		goto continue_0; // (|a,a|,
	}

	assert(t3->data.bytes[0] == ')'); // (|a|,|a|)
	pair_parse(t0, t1, t2);
	Free(pop_DL(&left->link));
	Free(pop_DL(&t3->link));
	return; // a,a
}
}
}

void full_process(Token *org) {
	Token *x = org;
	while ((x = container_of(x->link.next, Token, link)) != org) {
		if (cmp_Str(x->rule->name, cStr("def"))) continue;
		full_parse(x);
		Def *def = x->data.generic._0;
		Token *node_token = container_of(pop_DL(x->link.next), Token, link);
		def->to->data.ind = node_token->data.generic._0;
		Free(node_token);
		Free(pop_DL(x->link.next)); // ;
	}
}

/*
void create_defs(Token *org) {
	Token *x = org;
	while ((x = container_of(x->link.next, Token, link)) != org) {
		if (!(is_char_token(x) && x->data.bytes[0] == '='))
			continue;
		Def *def = Malloc(sizeof(Def));
		def->name = container_of(x->link.prev, Token, link)->data.str;
		def->to = container_of(x->link.next, Token, link)->data.generic._0;
		x->data.generic._0 = def;
		x->rule = &def_rule;
		Free(pop_DL(x->link.prev));
		Free(pop_DL(x->link.next));
		Free(pop_DL(x->link.next));
	}
}
*/

void pre_defs(Token *org) {
	Token *x = org;
	Def *prev = NULL;
	while ((x = container_of(x->link.next, Token, link)) != org) {
		if (!(is_char_token(x) && x->data.bytes[0] == '='))
			continue;
		Def *def = Malloc(sizeof(Def));
		if (prev) prev->next = def;
		prev = def;
		def->name = container_of(x->link.prev, Token, link)->data.str;
		Node *ind = Malloc(sizeof(Node));
		ind->flags = IND_TYPE_FLAG | NAMED_FLAG_NODE;
		ind->refc = 1;
		ind->data.ind = NULL;
		def->to = ind;
		x->data.generic._0 = def;
		x->rule = &def_rule;
		Free(pop_DL(x->link.prev));
	}
	prev->next = NULL;
}
