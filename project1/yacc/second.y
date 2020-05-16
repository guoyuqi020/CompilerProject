%{
#include "../../include/IR.h"
#include "../../include/actions.h"
#include "lex.yy.c"
extern double yylval_doubletmp;
extern int yylval_inttmp;
int yyerror(char*);
YYSTYPE myroot;
%}
%token Id INTEGER FLOAT DELIM WHITESPACE

%left '+' '-'
%left '*' '/' EXACTLYDIVIDE '%'
%nonassoc '='

%%

FINAL : P {myroot=MyPBuilder($1);}

P :  P S {$$=P_action_1($1,$2);}
    |S {$$=P_action_2($1);}
    ;

S : LHS '=' RHS ';' {$$=MySBuilder($1,$3);}

LHS : TRef {$$=RHS_7($1);}


RHS: RHS '+' RHS {$$=RHS_1($1,$3);}
    |RHS '*' RHS {$$=RHS_2($1,$3);}
    |RHS '-' RHS {$$=RHS_3($1,$3);}
    |RHS '/' RHS {$$=RHS_4($1,$3);} 
    |RHS '%' RHS {$$=RHS_5($1,$3);} 
    |RHS EXACTLYDIVIDE RHS {$$=RHS_6($1,$3);}
    |TRef {$$=RHS_7($1);}
    |SRef {$$=RHS_7($1);}
    |Const {$$=RHS_8($1);}
    |'(' RHS ')' {$$=RHS_9($2);}
    ;
    
TRef : Id '<' Clist '>' '[' Alist ']' {$$=MyTRefBuilder($1,$3,$6);}

SRef : Id '<' Clist '>' {$$=MySRefBuilder($1,$3);}

Clist :  Clist ',' INTEGER {$$=Clist_action_1($1,$3);}
        |INTEGER {$$=Clist_action_2($1);}
        ;
Alist :  Alist ',' IdExpr {$$=Alist_action_1($1,$3);}
        |IdExpr {$$=Alist_action_2($1);}
        ;

IdExpr : Id {$$=IdExpr_action_0($1);}
        |IdExpr '+' IdExpr {$$=IdExpr_action_1($1,$3);}
        |IdExpr '+' INTEGER {$$=IdExpr_action_1($1,$3);}
        |IdExpr '*' INTEGER {$$=IdExpr_action_2($1,$3);}
        |IdExpr EXACTLYDIVIDE INTEGER {$$=IdExpr_action_3($1,$3);}
        |IdExpr '%' INTEGER {$$=IdExpr_action_4($1,$3);}
        |'(' IdExpr ')' {$$=IdExpr_action_5($2);}
        ;

Const :  FLOAT {$$=$1;}
        |INTEGER {$$=$1;}
        ;

%%
YYSTYPE yyparse_string(char * str)
{
        yy_switch_to_buffer(yy_scan_string(str));
        yyparse();
        return myroot;
}
int yyerror(char* s)
{
    printf("%s",s);
    return 0;
}

