#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpc/mpc.h"

#ifdef _WIN32

#include <string.h>

static char buffer[2048];

char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

void add_history(char* unused) {}


#else

#include <editline/readline.h>
#include <editline/history.h>

#endif

#define DEBUG 0

enum { LVAL_NUM, LVAL_ERR };
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Declare New lval Struct */
typedef struct {
  int type;
  long num;
  int err;
} lval;

lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

/* Create a new error type lval */
lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

/* Print an "lval" */
void lval_print(lval v) {
  switch (v.type) {
    /* In the case the type is a number print it, then 'break' out of the switch. */
    case LVAL_NUM: printf("%li", v.num); break;

    /* In the case the type is an error */
    case LVAL_ERR:
      /* Check What exact type of error it is and print it */
      if (v.err == LERR_DIV_ZERO) { printf("Error: Division By Zero!"); }
      if (v.err == LERR_BAD_OP)   { printf("Error: Invalid Operator!"); }
      if (v.err == LERR_BAD_NUM)  { printf("Error: Invalid Number!"); }
    break;
  }
}

/* Print an "lval" followed by a newline */
void lval_println(lval v) { lval_print(v); putchar('\n'); }


long min(int x, int y) {
  if (x < y) {
    return x;
  } else {
    return y;
  }
}

long max(int x, int y) {
  if (x > y) {
    return x;
  } else {
    return y;
  }
}

lval eval_op(lval x, char* op, lval y) {

  if (x.type == LVAL_ERR) { return x; }
  if (y.type == LVAL_ERR) { return x; }

  if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
  if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
  if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
  if (strcmp(op, "/") == 0) {
    return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
  }
  if (strcmp(op, "%") == 0) { return lval_num( x.num % y.num); }
  if (strcmp(op, "^") == 0) { return lval_num(pow(x.num, y.num)); }
  if (strcmp(op, "min") == 0) { return lval_num(min(x.num, y.num)); }
  if (strcmp(op, "max") == 0) { return lval_num(max(x.num, y.num)); }
  return lval_err(LERR_BAD_OP);
}

int number_of_nodes(mpc_ast_t* t) {
  if (t->children_num == 0) { return 1; }
  if (t->children_num >= 1) {
    int total = 1;
    for (int i = 0; i < t->children_num; i++) {
      total = total + number_of_nodes(t->children[i]);
    }
    return total;
  }
}

int leaves_num(mpc_ast_t* t) {
  /* Compute number of leaves of a tree */
  int count = 0;
  count += t->children_num;
  for(int j = 0; j < t->children_num; j++){
    count += leaves_num(t->children[j]);
  }
  return count;
}

int branches_num(mpc_ast_t* t) {
  /* Compute number of branches of a tree */
  int count = 0;
  if (t->children_num == 0) {
    return 1;
  }
  for(int j = 0; j < t->children_num; j++){
    count += branches_num(t->children[j]);
  }
  return count;
}

lval eval(mpc_ast_t* t) {

  /* If tagged as number return it directly, otherwise expression. */
  if (strstr(t->tag, "number")) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  /* The operator is always second child. */
  char* op = t->children[1]->contents;

  /* We store the third child in `x` */
  lval x = eval(t->children[2]);

  /* Iterate the remaining children, combining using our operator */
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  return x;
}

int main(int argc, char** argv) {

  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Nthlisp = mpc_new("nthlisp");

  mpca_lang(MPCA_LANG_DEFAULT,
            "                                                                \
              number   : /-?[0-9]+/ ;                                        \
              operator : '+' | '-' | '*' | '/' | '%' | '^' | /max/ | /min/ ; \
              expr     : <number> | '(' <operator> <expr>+ ')' ;             \
              nthlisp  : /^/ <operator> <expr>+ /$/ ;                        \
            ",
            Number,
            Operator,
            Expr,
            Nthlisp);

  puts("Nthlisp Version 0.0.0.3");
  puts("Press Ctrl+c to exit\n");

  while (1) {
    char* input = readline("nthlisp> ");
    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Nthlisp, &r)) {
     /* On Success Print the AST */
      lval result = eval(r.output);
      lval_println(result);
      if (DEBUG) { mpc_ast_print(r.output); }
      mpc_ast_delete(r.output);

    } else {
      /* Otherwise Print the Error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    free(input);
  }
  mpc_cleanup(6,
              Number,
              Operator,
              Expr,
              Nthlisp);
  return 0;
}
