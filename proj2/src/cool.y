/*
 *  cool.y
 *              Parser definition for the COOL language.
 *
 */
%{
#include <iostream>
#include "cool-tree.h"
#include "stringtab.h"
#include "utilities.h"

/* Add your own C declarations here */


/************************************************************************/
/*                DONT CHANGE ANYTHING IN THIS SECTION                  */

extern int yylex();           /* the entry point to the lexer  */
extern int curr_lineno;
extern char *curr_filename;
Program ast_root;            /* the result of the parse  */
Classes parse_results;       /* for use in semantic analysis */
int omerrs = 0;              /* number of errors in lexing and parsing */

/*
   The parser will always call the yyerror function when it encounters a parse
   error. The given yyerror implementation (see below) justs prints out the
   location in the file where the error was found. You should not change the
   error message of yyerror, since it will be used for grading puproses.
*/
void yyerror(const char *s);

/*
   The VERBOSE_ERRORS flag can be used in order to provide more detailed error
   messages. You can use the flag like this:

     if (VERBOSE_ERRORS)
       fprintf(stderr, "semicolon missing from end of declaration of class\n");

   By default the flag is set to 0. If you want to set it to 1 and see your
   verbose error messages, invoke your parser with the -v flag.

   You should try to provide accurate and detailed error messages. A small part
   of your grade will be for good quality error messages.
*/
extern int VERBOSE_ERRORS;

%}

/* A union of all the types that can be the result of parsing actions. */
%union {
  Boolean boolean;
  Symbol symbol;
  Program program;
  Class_ class_;
  Classes classes;
  Feature feature;
  Features features;
  Formal formal;
  Formals formals;
  Case case_;
  Cases cases;
  Expression expression;
  Expressions expressions;
  char *error_msg;
}

/* 
   Declare the terminals; a few have types for associated lexemes.
   The token ERROR is never used in the parser; thus, it is a parse
   error when the lexer returns it.

   The integer following token declaration is the numeric constant used
   to represent that token internally.  Typically, Bison generates these
   on its own, but we give explicit numbers to prevent version parity
   problems (bison 1.25 and earlier start at 258, later versions -- at
   257)
*/
%token CLASS 258 ELSE 259 FI 260 IF 261 IN 262 
%token INHERITS 263 LET 264 LOOP 265 POOL 266 THEN 267 WHILE 268
%token CASE 269 ESAC 270 OF 271 DARROW 272 NEW 273 ISVOID 274
%token <symbol>  STR_CONST 275 INT_CONST 276 
%token <boolean> BOOL_CONST 277
%token <symbol>  TYPEID 278 OBJECTID 279 
%token ASSIGN 280 NOT 281 LE 282 ERROR 283

/*  DON'T CHANGE ANYTHING ABOVE THIS LINE, OR YOUR PARSER WONT WORK       */
/**************************************************************************/
 
   /* Complete the nonterminal list below, giving a type for the semantic
      value of each non terminal. (See section 3.6 in the bison 
      documentation for details). */

/* Declare types for the grammar's non-terminals. */
%type <program> program
%type <classes> class_list
%type <class_> class

/* You will want to change the following line. */
/* Feature non-terminals */
%type <feature>  feature auxi_feature
%type <features> feature_list n_feature_list
/* Formal non-terminals */ 
%type <formals> formal_list n_formal_list
/* Case non-terminals */
%type <cases> case_list
/* Expr non-terminals */
%type <expression>  expr expr_let
%type <expression>  expr_with_no
%type <expressions> expr_list expr_block

/* Precedence declarations go here. Same as manual 11.1 precedence. */  
%right LET_PREC // Used for fix the confilct of let expression.
%right ASSIGN
%left NOT
%nonassoc LE '<' '='
%left '+' '-'
%left '*' '/'
%left ISVOID
%left '~'
%left '@'
%left '.'


%%
/* 
   Save the root of the abstract syntax tree in a global variable.
*/
program : class_list { ast_root = program($1); }
        ;

class_list
        : class            /* single class */
                { $$ = single_Classes($1); parse_results = $$; }
        | class_list class /* several classes */
                { $$ = append_Classes($1,single_Classes($2)); parse_results = $$; }
        ;


/* If no parent is specified, the class inherits from the Object class. */
class  : CLASS TYPEID '{' feature_list '}' ';'
                { $$ = class_($2,idtable.add_string("Object"),$4,
                              stringtable.add_string(curr_filename)); }
        | CLASS TYPEID INHERITS TYPEID '{' feature_list '}' ';'
                { $$ = class_($2,$4,$6,stringtable.add_string(curr_filename)); }
        | error ';'
        { }      /* Error recovery, when one class finished. */
        ;


/* Feature list may be empty, but no empty features in list. */
feature_list: %empty     /* empty */
        {  $$ = nil_Features(); }
        | n_feature_list[nfl]
        { $$ = $nfl; }
        ;

/* For feature;+, the feature list is non-empty */
n_feature_list: feature[f] ';'
        { $$ = single_Features($f); }
        | n_feature_list[nfl] feature[f] ';' 
        { $$ = append_Features($nfl, single_Features($f)); }
        ;

/* For feature, use the following defined auxi_feature */        
feature: OBJECTID[obj] '(' formal_list[fl] ')' ':' TYPEID[ty] '{' expr[ep] '}'
        { $$ = method($obj, $fl, $ty, $ep); }
        | auxi_feature[af]
        { $$ = $af; }
        | error
        { /* Recover from errors at the end of feautures. */}
        ;

/* For feature ID:TYPE[<- expr] */
auxi_feature: OBJECTID[obj] ':' TYPEID[ty]  
        { $$ = attr($obj, $ty, no_expr()); }
        | OBJECTID[obj] ':' TYPEID[ty] ASSIGN expr[ep]
        { $$ = attr($obj, $ty, $ep); }
        ;

/* Define the formal list and formal, used in feature (formal,*) */
formal_list: %empty     /* Empty */
        { $$ = nil_Formals(); }
        | n_formal_list[nfl] 
        { $$ = $nfl; }
        ;

/* Define the non-empty formal list. Only formal list are used so formal is 
   not defined. */
n_formal_list: OBJECTID[obj] ':' TYPEID[ty] ',' n_formal_list[nfl]
        { $$ = append_Formals(single_Formals(formal($obj, $ty)), $nfl); }
        | OBJECTID[obj] ':' TYPEID[ty]
        { $$ = single_Formals(formal($obj, $ty)); }
        ;

/* Case list need to be specialy treated */
case_list: case_list[cl] OBJECTID[obj] ':' TYPEID[ty] DARROW expr[ep] ';'
        { $$ = append_Cases($cl, single_Cases(branch($obj, $ty, $ep))); }
        | OBJECTID[obj] ':' TYPEID[ty] DARROW expr[ep] ';'
        { $$ = single_Cases(branch($obj, $ty, $ep)); }
        ;

/* Define the block expression */
expr_block: expr_block[eb] expr[ep] ';'
        { $$ = append_Expressions($eb, single_Expressions($ep)); }
        | expr[ep] ';'
        { $$ = single_Expressions($ep); }
        | error ';'
        { yyerrok; /* Error recovery when meet ; */ }
        ;

/* Define the expr list for the dispatcher. */
expr_list: expr_list[el] ',' expr[ep] 
        { $$ = append_Expressions($el, single_Expressions($ep)); }
        | expr[ep]
        { $$ = single_Expressions($ep); } 
        | %empty
        { $$ = nil_Expressions(); }

/* Define expr with empty expression, may used in while */
expr_with_no: expr[ep]
        { $$ = $ep; }
        | %empty
        { $$ = no_expr(); }
        

/* Finally, we handle the let expression. */
expr_let: OBJECTID[obj] ':' TYPEID[ty] IN expr[ep] %prec LET_PREC
        { $$ = let($obj, $ty, no_expr(), $ep); }
        | OBJECTID[obj] ':' TYPEID[ty] ASSIGN expr[ep] ',' expr_let[el]
        { $$ = let($obj, $ty, $ep, $el); }
        | OBJECTID[obj] ':' TYPEID[ty] ',' expr_let[el]
        { $$ = let($obj, $ty, no_expr(), $el); }
        | OBJECTID[obj] ':' TYPEID[ty] ASSIGN expr[ep1] IN expr[ep2] %prec LET_PREC
        { $$ = let($obj, $ty, $ep1, $ep2); }  
        | error IN expr %prec LET_PREC 
        { yyerrok; /* Error recovery, when meet IN */ }
        | error ',' expr_let 
        { yyerrok; /* Error recovery, when meet , */ }
        ;

/* Define the expression. */
expr: OBJECTID[obj] ASSIGN expr[ep]
        { $$ = assign($obj, $ep); }

        /* The code for three form of dispatcher */
        | expr[ep] '.' OBJECTID[obj] '(' expr_list[el] ')'          /* For dispatcher. */
        { $$ = dispatch($ep, $obj, $el); }
        | OBJECTID[obj] '(' expr_list[el] ')'
        { $$ = dispatch(object(idtable.add_string("self")), $obj, $el); }
        | expr[ep] '@' TYPEID[ty] '.' OBJECTID[obj] '(' expr_list[el] ')'
        { $$ = static_dispatch($ep, $ty, $obj, $el); }

        /* If and while expression */
        | IF expr[ep1] THEN expr[ep2] ELSE expr[ep3] FI
        { $$ = cond($ep1, $ep2, $ep3); }
        | WHILE expr[ep] LOOP expr_with_no[ewn] POOL 
        { $$ = loop($ep, $ewn); }
        
        /* Block, case, let, new, isvoid expression */
        | '{' expr_block[eb] '}'
        { $$ = block($eb); }
        | LET expr_let[el]
        { $$ = $el; }
        | CASE expr[ep] OF case_list[cl] ESAC
        { $$ = typcase($ep, $cl); }
        | NEW TYPEID[ty]
        { $$ = new_($ty); }
        | ISVOID expr[ep]
        { $$ = isvoid($ep); }

        /* Mathmatical expression, add, sub, ... */
        | expr[ep1] '+' expr[ep2]
        { $$ = plus($ep1, $ep2); }
        | expr[ep1] '-' expr[ep2]
        { $$ = sub($ep1, $ep2); }
        | expr[ep1] '*' expr[ep2]
        { $$ = mul($ep1, $ep2); }
        | expr[ep1] '/' expr[ep2]
        { $$ = divide($ep1, $ep2); }
        | '~' expr[ep]
        { $$ = neg($ep); }
        | expr[ep1] '<' expr[ep2]
        { $$ = lt($ep1, $ep2); }
        | expr[ep1] LE expr[ep2]
        { $$ = leq($ep1, $ep2); }
        | expr[ep1] '=' expr[ep2]
        { $$ = eq($ep1, $ep2); }
        | NOT expr[ep]
        { $$ = comp($ep); }

        /* Int, string expression. */
        | '(' expr[ep] ')'
        { $$ = $ep; }
        | OBJECTID[obj]
        { $$ = object($obj); }
        | INT_CONST[ic]
        { $$ = int_const($ic); }
        | STR_CONST[sc]
        { $$ = string_const($sc); }
        | BOOL_CONST[bc]
        { $$ = bool_const($bc); }
        ;

/* end of grammar */
%%

/* This function is called automatically when Bison detects a parse error. */
void yyerror(const char *s)
{
  cerr << "\"" << curr_filename << "\", line " << curr_lineno << ": " \
    << s << " at or near ";
  print_cool_token(yychar);
  cerr << endl;
  omerrs++;

  if(omerrs>20) {
      if (VERBOSE_ERRORS)
         fprintf(stderr, "More than 20 errors\n");
      exit(1);
  }
}

