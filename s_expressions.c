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

#define DEBUG 1

enum { LVAL_INT, LVAL_DEC, LVAL_ERR };
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Declare New lval Struct */

typedef union _num_u {
  long inte;
  double dec;
  int err;
} num_u;

typedef struct {
  int type;
  num_u val;
} lval;

lval* lval_int(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_INT;
  v->val.inte = x;
  return v;
}

lval lval_dec(double x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_DEC;
  v->val.dec = x;
  return v;
}

/* Create a new error type lval */
lval lval_err(char* m) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->val.err = malloc(strlen(m) + 1);
  strcpy(v->val.err, m);
  return v;
}

lval* lval_sym(char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->val.sym = malloc(strlen(s) + 1);
  strcpy(v->val.sym, s);
  return v;
}

lval* lval_sexpre(void) {
  lval* v = malloca(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

void lval_del(lval* v) {

  switch (v->type) {
    case LVAL_NUM: break;
    case LVAL_ERR: free(v->val.err); break;
    case LVAL_SYM: free(v->val.sym); break;

    case LVAL_SEXPR:
      for (int i = 0; i < v->count; i++) {
        lval_del(v->cell[i]);
      }
      free(v->cell);
      break;
  }

  free(v);
}

/* Print an "lval" */
void lval_print(lval v) {
  switch (v.type) {
    /* In the case the type is a number print it, then 'break' out of the switch. */
    case LVAL_INT:
      printf("%li", v.val.inte);
      break;
    case LVAL_DEC:
      printf("%f", v.val.dec);
      break;
    /* In the case the type is an error */
    case LVAL_ERR:
      /* Check What exact type of error it is and print it */
      if (v.val.err == LERR_DIV_ZERO) { printf("Error: Division By Zero!"); }
      if (v.val.err == LERR_BAD_OP)   { printf("Error: Invalid Operator!"); }
      if (v.val.err == LERR_BAD_NUM)  { printf("Error: Invalid Number!"); }
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
  double d_x, d_y;
  long e_x, e_y;
  int is_double = 0;
  if (x.type == LVAL_DEC) {
    d_x = x.val.dec;
    is_double = 1;
  } else {
    e_x = x.val.inte;
  }
  if (y.type == LVAL_DEC) {
    d_y = y.val.dec;
    if (!is_double) {
      d_x = (double) e_x;
      is_double = 1;
    }
  } else {
    if (is_double) {
      d_y = (double) y.val.inte;
    } else {
      e_y = y.val.inte;
    }
  }

  if (is_double) {
    if (strcmp(op, "+") == 0) { return lval_dec(d_x + d_y); }
    if (strcmp(op, "-") == 0) { return lval_dec(d_x - d_y); }
    if (strcmp(op, "*") == 0) { return lval_dec(d_x * d_y); }
    if (strcmp(op, "/") == 0) {
      return d_y == 0 ?
          lval_err(LERR_DIV_ZERO) : lval_dec(d_x / d_y);
    }
    if (strcmp(op, "%") == 0) { return lval_dec( fmod(d_x, d_y)); }
    if (strcmp(op, "^") == 0) { return lval_dec(pow(d_x, d_y)); }
    if (strcmp(op, "min") == 0) { return lval_dec(min(d_x, d_y)); }
    if (strcmp(op, "max") == 0) { return lval_dec(max(d_x, d_y)); }
  } else {
    if (strcmp(op, "+") == 0) { return lval_int(e_x + e_y); }
    if (strcmp(op, "-") == 0) { return lval_int(e_x - e_y); }
    if (strcmp(op, "*") == 0) { return lval_int(e_x * e_y); }
    if (strcmp(op, "/") == 0) {
      return e_y == 0 ?
          lval_err(LERR_DIV_ZERO) : lval_int(e_x / e_y);
    }
    if (strcmp(op, "%") == 0) { return lval_int( e_x % e_y); }
    if (strcmp(op, "^") == 0) { return lval_int(pow(e_x, e_y)); }
    if (strcmp(op, "min") == 0) { return lval_int(min(e_x, e_y)); }
    if (strcmp(op, "max") == 0) { return lval_int(max(e_x, e_y)); }
  }
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
  if (strstr(t->tag, "integer")) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_int(x) : lval_err(LERR_BAD_NUM);
  }
  if (strstr(t->tag, "decimal")) {
    char* end;
    errno = 0;
    double x = strtod(t->contents, &end);
    return errno != ERANGE ? lval_dec(x) : lval_err(LERR_BAD_NUM);
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
  mpc_parser_t* Integer = mpc_new("integer");
  mpc_parser_t* Decimal = mpc_new("decimal");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Nthlisp = mpc_new("nthlisp");

  mpca_lang(MPCA_LANG_DEFAULT,
            "                                                                \
              integer  : /-?[0-9]+/ ;                                        \
              decimal  : /-?[0-9]+\\.[0-9]*/ ;                               \
              number   : <decimal> | <integer> ;                             \
              operator : '+' | '-' | '*' | '/' | '%' | '^' | /max/ | /min/ ; \
              expr     : <number> | '(' <operator> <expr>+ ')' ;             \
              nthlisp  : /^/ <operator> <expr>+ /$/ ;                        \
            ",
            Integer,
            Decimal,
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
              Integer,
              Decimal,
              Number,
              Operator,
              Expr,
              Nthlisp);
  return 0;
}
