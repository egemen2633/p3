#include "0h.c"
#include "2.c"
#include <stdbool.h>

typedef struct RuleEval RuleEval;
typedef bool MatchEval(Node *, RuleEval *);

struct RuleEval {
	SL link;
	MatchEval *match;
	Str name;
	void *ctx;
};


Node left_node = {
   .refc = 1,
   .flags = ATOM_TYPE_FLAG,
   .data.atom = cStr("left")
};

Node right_node = {
   .refc = 1,
   .flags = ATOM_TYPE_FLAG,
   .data.atom = cStr("right")
};


U4 is_atom_Node(Node *x) { return (x->flags & TYPE_FLAG_NODE) == ATOM_TYPE_FLAG && !(x->flags & NAME_FLAG_NODE); }
U4 is_name_Node(Node *x) { return (x->flags & TYPE_FLAG_NODE) == ATOM_TYPE_FLAG && (x->flags & NAME_FLAG_NODE); }
U4 is_ind_Node(Node *x)  { return (x->flags & TYPE_FLAG_NODE) == IND_TYPE_FLAG; }
U4 is_app_Node(Node *x)  { return (x->flags & TYPE_FLAG_NODE) == (APP_BIN_FLAG | BIN_TYPE_FLAG); }
U4 is_pair_Node(Node *x) { return (x->flags & TYPE_FLAG_NODE) == (PAIR_BIN_FLAG | BIN_TYPE_FLAG); }
U4 is_bin_Node(Node *x)  { return x->flags & BIN_TYPE_FLAG; }


bool is_arity_1(Node *node, Str name) {
   if (!is_app_Node(node)) return false;
   Node *nl = node->data.bin.left;
   return is_atom_Node(nl) && !cmp_Str(nl->data.atom, name);
}

bool is_arity_2(Node *node, Str name) {
   if (!is_app_Node(node)) return false;
   return is_arity_1(node->data.bin.left, name);
}

Node *arg_0_arity_1(Node *node) { return node->data.bin.right; }
Node *arg_0_arity_2(Node *node) { return node->data.bin.left->data.bin.right; }
Node *arg_1_arity_2(Node *node) { return node->data.bin.right; }

bool match_concat_str(Node *node, RuleEval *rule) {
   if (!is_arity_2(node, rule->name)) return false;
   Node *arg_0 = arg_0_arity_2(node);
   if (!is_atom_Node(arg_0)) return false;
   Node *arg_1 = arg_1_arity_2(node);
   if (!is_atom_Node(arg_1)) return false;

   U4 length = arg_0->data.atom.length + arg_1->data.atom.length;
   U1 *buffer = Malloc(length);
   memcpy(buffer, arg_0->data.atom.start, arg_0->data.atom.length);
   memcpy(buffer + arg_0->data.atom.length, arg_1->data.atom.start, arg_1->data.atom.length);

   release_Node(node->data.bin.left);
   release_Node(node->data.bin.right);
   node->flags = ATOM_TYPE_FLAG;
   node->data.atom.start = buffer;
   node->data.atom.length = length;
   return true;
}

bool match_substr(Node *node, RuleEval *rule) {
   if (!is_arity_2(node, rule->name)) return false;
   Node *arg_0 = arg_0_arity_2(node);
   if (!is_pair_Node(arg_0)) return false;
   Node *arg_1 = arg_1_arity_2(node);
   if (!is_atom_Node(arg_1)) return false;

   Node *nl = arg_0->data.bin.left;
   if (!is_atom_Node(nl) || nl->data.atom.length != 4) return false;
   Node *nr = arg_0->data.bin.right;
   if (!is_atom_Node(nr) || nr->data.atom.length != 4) return false;

   U4 left = *(U4 *)nl->data.atom.start;
   U4 right = *(U4 *)nr->data.atom.start;

   U4 length = right - left;
   U1 *buffer = Malloc(length);
   memcpy(buffer, arg_1->data.atom.start + left, length);

   release_Node(node->data.bin.left);
   release_Node(node->data.bin.right);
   node->flags = ATOM_TYPE_FLAG;
   node->data.atom.start = buffer;
   node->data.atom.length = length;
   return true;
}

typedef U4 unary_U4_func(U4);
typedef U4 binary_U4_func(U4, U4);

bool match_bin4(Node *node, RuleEval *rule) {
   if (!is_arity_2(node, rule->name)) return false;
   Node *arg_0 = arg_0_arity_2(node);
   if (!is_atom_Node(arg_0) || arg_0->data.atom.length != 4) return false;
   Node *arg_1 = arg_1_arity_2(node);
   if (!is_atom_Node(arg_1) || arg_1->data.atom.length != 4) return false;

   U4 *buffer = Malloc(4);
   *buffer = ((binary_U4_func *)rule->ctx)(*(U4 *)arg_0->data.atom.start, *(U4 *)arg_1->data.atom.start);

   release_Node(node->data.bin.left);
   release_Node(node->data.bin.right);
   node->flags = ATOM_TYPE_FLAG;
   node->data.atom.start = (U1 *)buffer;
   node->data.atom.length = 4;
   return true;
}

bool match_uni4(Node *node, RuleEval *rule) {
   if (!is_arity_1(node, rule->name)) return false;
   Node *arg_0 = arg_0_arity_1(node);
   if (!is_atom_Node(arg_0) || arg_0->data.atom.length != 4) return false;

   U4 *buffer = Malloc(4);
   *buffer = ((unary_U4_func *)rule->ctx)(*(U4 *)arg_0->data.atom.start);

   release_Node(node->data.bin.left);
   release_Node(node->data.bin.right);
   node->flags = ATOM_TYPE_FLAG;
   node->data.atom.start = (U1 *)buffer;
   node->data.atom.length = 4;
   return true;
}

// I x -> [x]
bool match_I(Node *node, RuleEval *rule) {
   if (!is_arity_1(node, rule->name)) return false;
   Node *arg_0 = arg_0_arity_1(node);

   release_Node(node->data.bin.left);
   // arg_0->refc++;
   // release_Node(node->data.bin.right);
   node->flags = IND_TYPE_FLAG;
   node->data.ind = arg_0;
   return true;
}

bool ind_reduce(Node **nlp) {
   Node *nl = *nlp;
   if (!is_ind_Node(nl))
      return false;

   *nlp = nl->data.ind;
   nl->data.ind->refc++;
   release_Node(nl);
   return true;
}

// [x] y -> x y
bool match_bin_ind(Node *node, RuleEval *rule) {
   if (!is_bin_Node(node)) return false;
   return ind_reduce(rule->ctx ? &node->data.bin.right : &node->data.bin.left);
}

bool match_uni_ind(Node *node, RuleEval *rule) {
   if (!is_ind_Node(node)) return false;
   return ind_reduce(&node->data.ind);
}

// pair x y -> x,y
bool match_pair(Node *node, RuleEval *rule) {
   if (!is_arity_2(node, rule->name)) return false;
   Node *arg_0 = arg_0_arity_2(node);
   // Node *arg_1 = arg_1_arity_2(node);

   arg_0->refc++;
   release_Node(node->data.bin.left);
   // arg_1->refc++;
   // release_Node(node->data.bin.right);

   node->data.bin.left = arg_0;
   // node->data.bin.right = arg_1;

   node->flags = PAIR_BIN_FLAG | BIN_TYPE_FLAG;
   return true;
}

// left (x,y) -> x
bool match_binary(Node *node, RuleEval *rule) {
   if (!is_arity_1(node, rule->name)) return false;
   Node *pair = arg_0_arity_1(node);
   if (!is_pair_Node(pair)) return false;
   Node *x = rule->ctx ? pair->data.bin.right : pair->data.bin.left;
   x->refc++;
   release_Node(node->data.bin.left);
   release_Node(node->data.bin.right);
   node->flags = IND_TYPE_FLAG;
   node->data.ind = x;
   return true;
}

typedef U4 str_to_int_func(Str);

bool match_str_to_int(Node *node, RuleEval *rule) {
   if (!is_arity_1(node, rule->name)) return false;
   Node *arg_0 = arg_0_arity_1(node);
   if (!is_atom_Node(arg_0)) return false;

   U4 *buffer = Malloc(4);
   *buffer = ((str_to_int_func *)rule->ctx)(arg_0->data.atom);

   release_Node(node->data.bin.left);
   release_Node(node->data.bin.right);
   node->flags = ATOM_TYPE_FLAG;
   node->data.atom.start = (U1 *)buffer;
   node->data.atom.length = 4;
   return true;
}

typedef Str int_to_str_func(U4);

bool match_int_to_str(Node *node, RuleEval *rule) {
   if (!is_arity_1(node, rule->name)) return false;
   Node *arg_0 = arg_0_arity_1(node);
   if (!is_atom_Node(arg_0) || arg_0->data.atom.length != 4) return false;

   Str str = ((int_to_str_func *)rule->ctx)(*(U4 *)arg_0->data.atom.start);
   release_Node(node->data.bin.left);
   release_Node(node->data.bin.right);
   node->flags = ATOM_TYPE_FLAG;
   node->data.atom = str;
   return true;
}

bool match_defs(Node *node, RuleEval *rule) {
   if (!is_name_Node(node)) return false;
   Token *org = rule->ctx;

   Token *token = org;
	while ((token = container_of(token->link.next, Token, link)) != org) {
		assert(!cmp_Str(token->rule->name, cStr("def")));
		Def *def = token->data.generic._0;
      if (!cmp_Str(node->data.atom, def->name)) {
         Free(node->data.atom.start);
         node->flags = IND_TYPE_FLAG;
         node->data.ind = copy_Node(def->to);
         return true;
      }
	}

   return false;
}

bool match_is_equal(Node *node, RuleEval *rule) {
   if (!is_arity_2(node, rule->name)) return false;
   Node *arg_0 = arg_0_arity_2(node);
   if (!is_atom_Node(arg_0)) return false;
   Node *arg_1 = arg_1_arity_2(node);
   if (!is_atom_Node(arg_1)) return false;

   U4 cmp = cmp_Str(arg_0->data.atom, arg_1->data.atom);
   release_Node(node->data.bin.left);
   release_Node(node->data.bin.right);
   node->flags = IND_TYPE_FLAG;
   node->data.ind = copy_Node(cmp ? &right_node : &left_node);
   return true;
}


bool local_eval(Node *node, RuleEval *first) {
   while (first) {
      if (first->match(node, first)) {
         node->flags |= ONE_TIME_FLAG_NODE;
         return true;
      }
      first = container_of(first->link.next, RuleEval, link);
   }
   return false;
}

bool step_eval(Node *node, RuleEval *first) {
   if (local_eval(node, first))
      return true;
   if (node->flags & BIN_TYPE_FLAG)
      return step_eval(node->data.bin.left, first)
         || step_eval(node->data.bin.right, first);
   if ((node->flags & 1) == IND_TYPE_FLAG)
      return step_eval(node->data.ind, first);
   return false;
}



bool match_exec_cancel(Node *node, RuleEval *rule) {
   if (!is_app_Node(node)) return false;
   Node *nl = node->data.bin.left;
   Node *nr = node->data.bin.right;
   if (!is_atom_Node(nl)) return false;
   if (!is_app_Node(nr)) return false;
   if (cmp_Str(cStr("exec"), nl->data.atom)) return false;
   Node *nrl = nr->data.bin.left;
   Node *nrr = nr->data.bin.right;
   if (!is_atom_Node(nrl)) return false;
   if (cmp_Str(cStr("quote"), nrl->data.atom)) return false;

   nrr = copy_Node(nrr);
   release_Node(node->data.bin.left);
   release_Node(node->data.bin.right);
   node->data.ind = nrr;
   node->flags = IND_TYPE_FLAG;
   return true;
}


bool match_exec_atom_cancel(Node *node, RuleEval *rule) {
   if (!is_app_Node(node)) return false;
   Node *nl = node->data.bin.left;
   Node *nr = node->data.bin.right;
   if (!is_atom_Node(nl)) return false;
   if (!is_atom_Node(nr)) return false;
   if (cmp_Str(cStr("exec"), nl->data.atom)) return false;

   release_Node(node->data.bin.left);
   // release_Node(node->data.bin.right);
   node->data.ind = node->data.bin.right;
   node->flags = IND_TYPE_FLAG;
   return true;
}

// K a x -> [a]
bool match_K(Node *node, RuleEval *rule) {
   if (!is_arity_2(node, rule->name)) return false;

   Node *a = copy_Node(arg_0_arity_2(node));

   release_Node(node->data.bin.left);
   release_Node(node->data.bin.right);

   node->flags = IND_TYPE_FLAG;
   node->data.ind = a;
   return true;
}



bool is_arity_3(Node *node, Str name) {
   if (!is_app_Node(node)) return false;
   return is_arity_2(node->data.bin.left, name);
}

Node *arg_0_arity_3(Node *node) {
   return arg_0_arity_2(node->data.bin.left);
}

Node *arg_1_arity_3(Node *node) {
   return node->data.bin.left->data.bin.right;
}

Node *arg_2_arity_3(Node *node) {
   return node->data.bin.right;
}


// S a b x -> (a x) (b x)
bool match_S(Node *node, RuleEval *rule) {
   if (!is_arity_3(node, rule->name)) return false;

   Node *a = copy_Node(arg_0_arity_3(node));
   Node *b = copy_Node(arg_1_arity_3(node));
   Node *x = arg_2_arity_3(node);

   Node *ax = new_bin_Node(a, copy_Node(x));
   Node *bx = new_bin_Node(b, x);

   release_Node(node->data.bin.left);

   node->flags = BIN_TYPE_FLAG | APP_BIN_FLAG;
   node->data.bin.left = ax;
   node->data.bin.right = bx;
   return true;
}
