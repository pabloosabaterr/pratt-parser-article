/*
 * Minimal Pratt parser for my blog post.
 *
 * Operands are words (letters/digits) or single characters.
 * Assumes well-formed input: no error handling, no freeing.
 * Prints the result as an S-expression: 1 + 2 * 3  ->  (+ 1 (* 2 3))
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
	char *lex;
	struct node *lhs;
	struct node *rhs;
	struct node *els; /* only used by if */
};

static const char *src; /* start of the current token */
static int len;         /* length of the current token */

static void next(void)
{
	src += len;
	while (*src == ' ')
		src++;

	len = 0;
	while (isalnum((unsigned char)src[len]))
		len++;
	if (len == 0 && *src)
		len = 1;
}

static bool is(const char *word)
{
	return !strncmp(src, word, len) && word[len] == '\0';
}

/* Returns the current token as a string and moves to the next one. */
static char *take(void)
{
	char *lex = strndup(src, len);
	next();
	return lex;
}

static struct node *make_node(char *lex, struct node *lhs, struct node *rhs)
{
	struct node *n = calloc(1, sizeof *n);
	n->lex = lex;
	n->lhs = lhs;
	n->rhs = rhs;
	return n;
}

enum op_type {
	COMMA,
	ADD,
	SUB,
	MUL,
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

/*
 * A left binding power of -1 means the token can never come after an
 * expression: min_bp is never below 0, so the loop always stops on it.
 */
static const struct binding_powers binding_table[] = {
	[COMMA] = { 0, 1 },
	[ADD]   = { 1, 2 },
	[SUB]   = { 1, 2 },
	[MUL]   = { 3, 4 },
	[NOT]   = { -1, 5 },
	[POW]   = { 7, 6 },
	[INDEX] = { 9, 0 },
	[CALL]  = { 9, 0 },
	[DOT]   = { 9, 10 },
};

static enum op_type get_type(char c)
{
	switch (c) {
	case ',': return COMMA;
	case '+': return ADD;
	case '-': return SUB;
	case '*': return MUL;
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

	if (*src == '(') {
		next();
		lhs = parse_expr(0);
		next(); /* skip ')' */
	} else if (is("if")) {
		lhs = make_node(take(), NULL, NULL);
		lhs->lhs = parse_expr(0);
		lhs->rhs = parse_expr(0);
		if (is("else")) {
			next();
			lhs->els = parse_expr(0);
		}
	} else if (get_type(*src) == NOT) {
		char *lex = take();
		lhs = make_node(lex, NULL, parse_expr(binding_table[NOT].right));
	} else {
		lhs = make_node(take(), NULL, NULL);
	}

	for (;;) {
		enum op_type op = get_type(*src);
		if (op == NOT_OP)
			break;

		struct binding_powers bp = binding_table[op];
		if (bp.left < min_bp)
			break;

		char *lex = take();

		if (op == INDEX || op == CALL) {
			lhs = make_node(lex, lhs, parse_expr(0));
			next(); /* skip ']' or ')' */
			continue;
		}

		lhs = make_node(lex, lhs, parse_expr(bp.right));
	}

	return lhs;
}

static void print_sexpr(const struct node *n)
{
	if (!n->lhs && !n->rhs) {
		fputs(n->lex, stdout);
		return;
	}
	printf("(%s ", n->lex);
	if (n->lhs) {
		print_sexpr(n->lhs);
		putchar(' ');
	}
	print_sexpr(n->rhs);
	if (n->els) {
		putchar(' ');
		print_sexpr(n->els);
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
		len = 0;
		next();
		print_sexpr(parse_expr(0));
		putchar('\n');
	}
	return 0;
}
