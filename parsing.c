#include <stdio.h>
#include <stdlib.h>
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

int main(int argc, char** argv) {

  /* mpc_parser_t* Number = mpc_new("number"); */
  /* mpc_parser_t* NumOperator = mpc_new("num_operator"); */
  /* mpc_parser_t* LitOperator = mpc_new("lit_operator"); */
  /* mpc_parser_t* Operator = mpc_new("operator"); */
  /* mpc_parser_t* Expr = mpc_new("expr"); */
  /* mpc_parser_t* Nthlisp = mpc_new("nthlisp"); */
  mpc_parser_t* Adjective = mpc_new("adjective");
  mpc_parser_t* Noun = mpc_new("noun");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Nthlisp = mpc_new("nthlisp");
  mpca_lang(MPCA_LANG_DEFAULT,
            /* "                                                           \ */
            /*   number   : /-?[0-9]+/'.'?/[0-9]+/ ;                       \ */
            /*   num_operator : '+' | '-' | '*' | '/' | '%' ;              \ */
            /*   lit_operator : /add/ | /sub/ | /div/ | /mul/ ;            \ */
            /*   operator: <num_operator> | <lit_operator> ;               \ */
            /*   expr     : <number> | '(' <operator> <expr>+ ')' ;        \ */
            /*   nthlisp  : /^/ <operator> <expr>+ /$/ ;                   \ */
            /* ", */
            /* "                                                           \ */
            /*   number   : /-?[0-9]+/;                                    \ */
            /*   num_operator : '+' | '-' | '*' | '/' | '%' ;              \ */
            /*   lit_operator : /add/ | /sub/ | /div/ | /mul/ ;            \ */
            /*   operator: <num_operator> | <lit_operator> ;               \ */
            /*   expr     : <number> | '(' <number> <operator> <expr> ')' ; \ */
            /*   nthlisp  : /^/ <number> <operator> <expr> /$/ ;         \ */
            /* ", */
            "                                                       \
              noun : /lisp/ | /language/ | /c/ | /book/ | /build/ ; \
              adjective : /wow/ | /many/ | /so/ | /such/ ;          \
              expr : <adjective> <noun> ;                           \
              nthlisp : <expr>+ ;                                   \
            ",
            Noun, Adjective, Expr, Nthlisp);

  puts("Nthlisp Version 0.0.0.3");
  puts("Press Ctrl+c to exit\n");

  while (1) {
    char* input = readline("nthlisp> ");
    add_history(input);
    /* printf("%s\n", input); */

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Nthlisp, &r)) {
     /* On Success Print the AST */
      mpc_ast_print(r.output);
      mpc_ast_delete(r.output);
    } else {
      /* Otherwise Print the Error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    free(input);
  }
  mpc_cleanup(6, Adjective, Noun, Expr, Nthlisp);
            //Number, Operator, Expr, NumOperator, LitOperator, Nthlisp);
  return 0;
}
