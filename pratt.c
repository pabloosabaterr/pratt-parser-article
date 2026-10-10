/*
 * Minimal Pratt parser for my blog post.
 *
 * Operands are single characters (digit/letter).
 * Assumes well-formed input: no error handling, no freeing.
 * Prints the result as an S-expression: 1 + 2 * 3  ->  (+ 1 (* 2 3))
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
	char lex;
	struct node *lhs;
	struct node *rhs;
};

static const char *src;

static void next(void)
{
	src++;
	while (*src == ' ')
		src++;
}

static struct node *make_node(char op, struct node *lhs, struct node *rhs)
{
	struct node *n = malloc(sizeof *n);
	n->lex = op;
	n->lhs = lhs;
	n->rhs = rhs;
	return n;
}

enum op_type {
	COMMA,
	ADD,
	SUB,
	MUL,
	DIV,
	POW,
	NOT,
	INDEX,
	CALL,
	DOT,
	NOT_OP,
};

struct binding_powers {
	int left;
	int right;
};

static const struct binding_powers binding_table[] = {
	[COMMA] = { 0, 1 },
	[ADD] = { 1, 2 },
	[SUB] = { 1, 2 },
	[MUL] = { 3, 4 },
	[DIV] = { 3, 4 },
	[POW] = { 7, 6 },
	[NOT] = { 0, 6 },
	[INDEX] = { 9, 0 },
	[CALL] = { 9, 0 },
	[DOT] = { 9, 10 },
};

static enum op_type get_type(char c)
{
	switch (c) {
	case ',': return COMMA;
	case '+': return ADD;
	case '-': return SUB;
	case '*': return MUL;
	case '/': return DIV;
	case '^': return POW;
	case '!': return NOT;
	case '(': return CALL;
	case '[': return INDEX;
	case '.': return DOT;
	default: return NOT_OP;
	}
}

static struct node *parse_expr(int min_bp)
{
	struct node *lhs;
	char c = *src;

	if (c == '(') {
		next();
		lhs = parse_expr(0);
		next();
	} else if (get_type(c) == NOT) {
		next();
		struct node *operand = parse_expr(binding_table[NOT].right);
		lhs = make_node(c, operand, NULL);
	} else {
		lhs = make_node(c, NULL, NULL);
		next();
	}

	for (;;) {
		c = *src;
		enum op_type op = get_type(c);
		if (op == NOT_OP || op == NOT)
			break;

		struct binding_powers bp = binding_table[op];
		if (bp.left < min_bp)
			break;

		next();

		if (op == INDEX || op == CALL) {
			struct node *inside = parse_expr(0);
			next();
			lhs = make_node(c, lhs, inside);
			continue;
		}

		struct node *rhs = parse_expr(bp.right);
		lhs = make_node(c, lhs, rhs);
	}

	return lhs;
}

static void print_sexpr(const struct node *n)
{
	if (!n->lhs) {
		putchar(n->lex);
		return;
	}
	printf("(%c ", n->lex);
	print_sexpr(n->lhs);
	if (n->rhs) {
		putchar(' ');
		print_sexpr(n->rhs);
	}
	putchar(')');
}

int main(void)
{
	char line[256];

	while (fgets(line, sizeof line, stdin)) {
		line[strcspn(line, "\n")] = '\0';
		if (line[0] == '\0')
			continue;

		src = line;
		print_sexpr(parse_expr(0));
		putchar('\n');
	}
	return 0;
}
