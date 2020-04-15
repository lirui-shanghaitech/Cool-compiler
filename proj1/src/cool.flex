/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
  if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
    YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;

extern YYSTYPE cool_yylval;

int bracket_counter = 0;
long int string_counter = 0;

/*
 *  Add Your own definitions here
 */
%}

  //Define some exclusive start condition.
%x INLINE_COMMENT
%x MULTILINE_COMMENT
%x STRING

%option noyywrap

/*
 * Define names for regular expressions here.
 */

DIGIT       [0-9]
DIGITS      [0-9]+

%%

 /*
  * Define regular expressions for the tokens of COOL here. Make sure, you
  * handle correctly special cases, like:
  *   - Nested comments
  *   - String constants: They use C like systax and can contain escape
  *     sequences. Escape sequence \c is accepted for all characters c. Except
  *     for \n \t \b \f, the result is c.
  *   - Keywords: They are case-insensitive except for the values true and
  *     false, which must begin with a lower-case letter.
  *   - Multiple-character operators (like <-): The scanner should produce a
  *     single token for every such operator.
  *   - Line counting: You should keep the global variable curr_lineno updated
  *     with the correct line number
  */

 /*-------------------------------------------------------------------------------*/
 /* First handle the inline comment which start with "--".*/
<INITIAL>"--" BEGIN INLINE_COMMENT;
<INLINE_COMMENT>{
  [^\n]*  {/*Eat up the comments*/ }
  \n      {curr_lineno = curr_lineno + 1; BEGIN INITIAL;}
}
 /*-------------------------------------------------------------------------------*/




 /*-------------------------------------------------------------------------------*/
 /* Then handle the multiline comment which start with "(*", since the nested 
 comment is allowed, we maintain a bracket_counter to track the comments */
<INITIAL,MULTILINE_COMMENT>"(*" {bracket_counter++; BEGIN MULTILINE_COMMENT;}
<MULTILINE_COMMENT>{
  <<EOF>> {
    yylval.error_msg = "EOF in comment";
    BEGIN INITIAL;
    return ERROR;
  }
  
  [^*\(\)\n]* {/*Eat up anything that is not a '*()' */}
  "("[^*\n]* {/*Eat up anything not followed by '*' */}
  "(*" bracket_counter++;

  ")" {/*Handle the situation such as (*)))*) */}

  "*"[^*\)\n]* {/*Eat up anything that is not followed by ")", bug fix add * avoid (** *) */}
  "*)" {
    bracket_counter--;
    if (bracket_counter == 0){
      BEGIN INITIAL;
    }
  }

  \n {curr_lineno++;}
}

 /*Help the multiline_comment detect the unmatched comments.*/
<INITIAL>"*)" {
  yylval.error_msg = "Unmatched *)";
  return ERROR;
}
 /*-------------------------------------------------------------------------------*/





 /*-------------------------------------------------------------------------------*/
 /*Handle the string, return the error when needed*/
<INITIAL>\" {BEGIN STRING; string_buf_ptr = string_buf; string_counter = 0;}
<STRING>{
  \" {
      BEGIN INITIAL;
      *string_buf_ptr = '\0'; /*End of the string, set the pointer*/
      if (string_counter >= MAX_STR_CONST) {
        /*If the string too long return an error*/
        yylval.error_msg = "String constant too long";
        /*Need to empty the buffer*/
        for (int i = 0; i< MAX_STR_CONST; i++)
          string_buf[0] = 0;
        return ERROR;
      } else {
        /*Add it to the string table*/
        yylval.symbol = stringtable.add_string(string_buf);
        for (int i = 0; i< MAX_STR_CONST; i++)
          string_buf[0] = 0;
        return STR_CONST;
      }
  }
  \n {
    yylval.error_msg = "Unterminated string constant";
    curr_lineno++;
    BEGIN INITIAL;
    for (int i = 0; i< MAX_STR_CONST; i++)
      string_buf[0] = 0;
    return ERROR;
  } 
  <<EOF>> {
    yylval.error_msg = "EOF in string constant";
    BEGIN INITIAL;
    for (int i = 0; i< MAX_STR_CONST; i++)
      string_buf[0] = 0;
    return ERROR;
  }

  \\n {
    string_counter++; 
    if (string_counter <= MAX_STR_CONST)
      *string_buf_ptr++ = '\n';
  }
  \\b {
    string_counter++;
    if (string_counter <= MAX_STR_CONST)
      *string_buf_ptr++ = '\b';
  }
  \\t {
    string_counter++;
    if (string_counter <=MAX_STR_CONST)
      *string_buf_ptr++ = '\t';
  }
  \\f {
    string_counter++;
    if (string_counter <= MAX_STR_CONST)
      *string_buf_ptr++ = '\f';
  }

  \\\n {
    curr_lineno++;
    string_counter++;
    if (string_counter <= MAX_STR_CONST)
      *string_buf_ptr++ = yytext[1];
  }
  \\(.) {
    string_counter++;
    if (string_counter <= MAX_STR_CONST)
      *string_buf_ptr++ = yytext[1];
  }

  [^\\\n\"]+ {
    char* ptr = yytext;
    while(*ptr){
      string_counter++;
      if (string_counter <= MAX_STR_CONST){
        *string_buf_ptr = *ptr;
        string_buf_ptr++;
      }
      ptr++;
    }
  }
}
 /*-------------------------------------------------------------------------------*/



 /*-------------------------------------------------------------------------------*/
 /*Handle int and bool constant, here true and false must start with lowercase*/
<INITIAL>{
  [0-9]+ {
    yylval.symbol = inttable.add_string(yytext);
    return INT_CONST;
  }
  t(?i:rue) {
    yylval.boolean = 1;
    return BOOL_CONST; 
  }
  f(?i:alse) {
    yylval.boolean = 0;
    return BOOL_CONST;
  }
}
 /*-------------------------------------------------------------------------------*/




 /*-------------------------------------------------------------------------------*/
 /*Handle the keywords except the ture and false keywords, case insensitive*/
<INITIAL>{
  (?i:class)             return CLASS;
  (?i:else)              return ELSE;
  (?i:fi)                return FI;
  (?i:if)                return IF;
  (?i:in)                return IN;
  (?i:inherits)          return INHERITS;
  (?i:let)               return LET;
  (?i:loop)              return LOOP;
  (?i:pool)              return POOL;
  (?i:then)              return THEN;
  (?i:while)             return WHILE;
  (?i:case)              return CASE;
  (?i:esac)              return ESAC;
  (?i:of)                return OF;
  (?i:new)               return NEW;
  (?i:isvoid)            return ISVOID;
  (?i:not)               return NOT;
}
 /*-------------------------------------------------------------------------------*/





 /*-------------------------------------------------------------------------------*/
 /*Handle the operators, no > and >= in cool*/
<INITIAL>{
   /*Bug fix: put <= => <- in the front to avoid the situation such as <<=*/
  "<=" return LE;
  "<-" return ASSIGN;
  "=>" return DARROW;
}

<INITIAL>{
  "+" return(int('+'));
  "-" return(int('-'));
  "*" return(int('*'));
  "/" return(int('/'));
  "~" return(int('~'));
  "<" return(int('<'));
  "=" return(int('='));
  "." return(int('.'));
  "@" return(int('@'));
  ":" return(int(':'));
  "," return(int(','));
  ";" return(int(';'));
  "{" return(int('{'));
  "}" return(int('}'));
  "(" return(int('('));
  ")" return(int(')'));
}
 /*-------------------------------------------------------------------------------*/




 /*-------------------------------------------------------------------------------*/
 /*Handle the identifier and typeid and whitespace*/
 <INITIAL>{
   [A-Z][A-Za-z0-9_]* {
     yylval.symbol = idtable.add_string(yytext);
     return TYPEID;
   }
   [a-z][A-Za-z0-9_]* {
     yylval.symbol = idtable.add_string(yytext);
     return OBJECTID;
   }
   [ \f\r\t\v]+ {/*White space defined in the coolaid*/}
 }
 /*-------------------------------------------------------------------------------*/


 /*Track the line number.*/
<INITIAL>\n {curr_lineno++;}
 /*Some unkown error handling*/
<INITIAL>[^\n] {
  yylval.error_msg = yytext;
  return ERROR;
}

%%
