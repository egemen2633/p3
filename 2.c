#include "0h.c"
#include <linux/limits.h>

#define BIN_TYPE_FLAG 0b10
#define ATOM_TYPE_FLAG 0
#define IND_TYPE_FLAG 1
#define APP_BIN_FLAG 0
#define PAIR_BIN_FLAG 1
#define TYPE_FLAG_NODE 0b11
// #define VAR_FLAG_NODE 2

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
	switch (x->flags & TYPE_FLAG_NODE) {
	case ATOM_TYPE_FLAG: {
		print2_Str(x->data.atom);
		return;
	}
	case IND_TYPE_FLAG: {
		putchar('[');
		print_Node(x->data.ind, 1);
		putchar(']');
		return;
	}
	case APP_BIN_FLAG | BIN_TYPE_FLAG: {
		if (outer_n_side == 2) putchar('(');
		print_Node(x->data.bin.left, 0);
		putchar(' ');
		print_Node(x->data.bin.right, 2);
		if (outer_n_side == 2) putchar(')');
		return;
	}
	case PAIR_BIN_FLAG | BIN_TYPE_FLAG: {
		if (outer_n_side != 1) putchar('(');
		print_Node(x->data.bin.left, 1);
		putchar(',');
		print_Node(x->data.bin.right, 3);
		if (outer_n_side != 1) putchar(')');
		return;
	}
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
	print_Node(x->data.generic._0, false);
}

RuleLexer node_rule = {
	.name = cStr("node"),
	.print = &print_Token_Node
};

void convert_atoms(Token *org) {
	Token *token = org;
	while ((token = container_of(token->link.next, Token, link)) != org) {
		if (cmp_Str(token->rule->name, cStr("str"))) continue;
		token->rule = &node_rule;
		token->data.generic._0 = new_atom_Node(token->data.str);
	}
}

/*
void find_vars(Token *org) {
	Token *token = org;
	while ((token = container_of(token->link.next, Token, link)) != org) {
		if (cmp_Str(token->rule->name, cStr("char")) || token->data.bytes[0] != '%') continue;
		token = container_of(token->link.next, Token, link);
		Free(pop_DL(token->link.prev));
		Node *node = token->data.generic._0;
		// node->flags |= VAR_FLAG_NODE;
	}
}
*/

/*
bool step_parse(Token *left) {
	Token *one = container_of(left->link.next, Token, link);
	Token *two = container_of(one->link.next, Token, link);
	if (!cmp_Str(one->rule->name, cStr("node"))) {
		if (!cmp_Str(two->rule->name, cStr("node"))) {
			one->data.generic._0 = new_bin_Node(one->data.generic._0, two->data.generic._0);
			Free(pop_DL(&two->link));
			return true;
		}

		if (!cmp_Str(two->rule->name, cStr("char"))) {
			if (two->data.bytes[0] == ')') {
				Free(pop_DL(&left->link));
				Free(pop_DL(&two->link));
				return true;
			}
			if (two->data.bytes[0] == '(')	
				return step_parse(two);
		}
	}

	if (!cmp_Str(one->rule->name, cStr("char"))) {
		if (one->data.bytes[0] == '(')
			return step_parse(one);
	}

	return false;
}
*/

void full_parse(Token *left) {
	Token *one = container_of(left->link.next, Token, link);
	Token *two = container_of(one->link.next, Token, link);
	if (!cmp_Str(one->rule->name, cStr("node"))) {
	label_0:
		if (!cmp_Str(two->rule->name, cStr("node"))) {
			one->data.generic._0 = new_bin_Node(one->data.generic._0, two->data.generic._0);
			Free(pop_DL(&two->link));
			return full_parse(left);
		}

		if (!cmp_Str(two->rule->name, cStr("char"))) {
// (|node|char
if (two->data.bytes[0] == ')') {
	Free(pop_DL(&left->link));
	Free(pop_DL(&two->link));
	return;
}
if (two->data.bytes[0] == '(') {
	full_parse(two);
	two = container_of(one->link.next, Token, link);
	goto label_0;
}
if (two->data.bytes[0] == ',') {
	Token *three = container_of(two->link.next, Token, link);
	if (!cmp_Str(three->rule->name, cStr("char")) && three->data.bytes[0] == '(') {
		full_parse(three);
		three = container_of(two->link.next, Token, link);
	}
	if (!cmp_Str(three->rule->name, cStr("node"))) {
		Node *new = new_bin_Node(one->data.generic._0, three->data.generic._0);
		new->flags |= PAIR_BIN_FLAG;
		one->data.generic._0 = new;
		Free(pop_DL(&two->link));
		Free(pop_DL(&three->link));
		return full_parse(left);
	}
}
		}
	}

	if (!cmp_Str(one->rule->name, cStr("char"))) {
		if (one->data.bytes[0] == '(') {
			full_parse(one);
			return full_parse(left);
		}
	}

	return;
}
