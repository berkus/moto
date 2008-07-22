%{
/*=============================================================================

   Copyright (C) 2000 Kenneth Kocienda and David Hakim.
   This file is part of the Moto Programming Language.

   Moto Programming Language is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   Moto Programming Language is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Codex C Library; see the file COPYING.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   $RCSfile: moto.l,v $   
   $Revision: 1.65 $
   $Author: dhakim $
   $Date: 2003/02/28 23:16:48 $
 
==============================================================================*/
#ifdef HAVE_RCONFIG_H
#include <rconfig.h>
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "memsw.h"

#include "exception.h"
#include "excpfn.h"
#include "intstack.h"
#include "stack.h"
#include "symboltable.h"
#include "fileio.h"
#include "log.h"
#include "runtime.h"

#include "motolex.h"
#include "moto.tab.h"
#include "motoerr.h"
#include "motoutil.h"
#include "cell.h"
#include "dl.h"
#include "mototree.h"
#include "env.h"
#include "motohook.h"

#define YY_DECL int yylex(YYSTYPE *lvalp, MotoEnv *env)
#define YY_MALLOC_DECL /* stdlib.h and memsw.h are not playing happily */

int depth = 0;
static int gobblews = 0;
static int state = 0;
static MetaInfo lastOpenParen;

static IVS* createIVS(MotoEnv *env);
static SVS* createSVS(MotoEnv *env,char* s);

static void countLines(MotoEnv *env, char *s);

static int isReserved(char*);

int popEScope(MotoEnv *env);
int peekScope(MotoEnv *env, int verify);

%}

%s D EMB

I		[0-9]
A		[a-zA-Z]
K		[_]
W		[!-#%-\[\]-~]
C		[!-&\(-\[\]-~]

O		[0-7]
H		[a-fA-F0-9]
E		[Ee][+-]?{I}+
FS		(f|F|l|L)
IS		(l|L)*

S		[\f\t\n\r ]*
T		({A}|{K})({A}|{I}|{K})*({S}"["{S}"]")*

BSA	\\.
%%

"$>"(\\.|[^:])*":"{I}+":"(\\.|[^:])*":"{I}+":#>$" { 
   /* This regex matches what kind of Macro call ??? */

   char *s;
   char *leavefilename;
   char *leavelinestr;
   char *filename;
   char *linestr;
   MetaInfo *meta;

   s = yytext + 2;
   leavefilename = strtok(s, ":");
   leavelinestr = strtok(NULL, ":");
   filename = strtok(NULL, ":");
   linestr = strtok(NULL, ":");
  
   meta = (MetaInfo*)moto_malloc(env,sizeof(MetaInfo));
   meta->lineno = env->meta.lineno;
   meta->filename = env->meta.filename;
   meta->macroname = env->meta.macroname;
   meta->caller = env->meta.caller;

   env->meta.lineno = atoi(linestr);
   env->meta.filename = moto_strdup(env,filename);
   env->meta.macroname = NULL;
   env->meta.caller = meta;
}

"$>"(\\.|[^:])*":"{I}+":"(\\.|[^:])*":"{I}+":*>$" { 

   char *s;
   char *leavefilename;
   char *leavelinestr;
   char *filename;
   char *linestr;
   MetaInfo* meta;

   s = yytext + 2;
   leavefilename = strtok(s, ":");
   leavelinestr = strtok(NULL, ":");
   filename = strtok(NULL, ":");
   linestr = strtok(NULL, ":");

   meta = (MetaInfo*)moto_malloc(env,sizeof(MetaInfo));
   meta->lineno = env->meta.lineno;
   meta->filename = env->meta.filename;
   meta->macroname = env->meta.macroname;
   meta->caller = env->meta.caller;

   env->meta.lineno = atoi(linestr);
   env->meta.filename = moto_strdup(env,filename);
   env->meta.macroname = NULL;
   env->meta.caller = meta;

}

"$>"(\\.|[^:])*":"{I}+":"(\\.|[^:])*":"{I}+":"({A}|{K})({A}|{I}|{K})*(\({I}+\))?">$" {
   char *s;
   char *leavefilename;
   char *leavelinestr;
   char *filename;
   char *linestr;
   char *mname;
   MetaInfo* meta;

   s = yytext + 2;
   leavefilename = strtok(s, ":");
   leavelinestr = strtok(NULL, ":");
   filename = strtok(NULL, ":");
   linestr = strtok(NULL, ":");
   mname = strtok(NULL, ">");

   meta = (MetaInfo*)moto_malloc(env,sizeof(MetaInfo));
   meta->lineno = env->meta.lineno;
   meta->filename = env->meta.filename;
   meta->macroname = env->meta.macroname;
   meta->caller = env->meta.caller;

   env->meta.lineno = atoi(linestr);
   env->meta.filename = moto_strdup(env,filename);
   env->meta.macroname = moto_strdup(env,mname);
   env->meta.caller = meta;

}

"$<"(\\.|[^:])*":"{I}+"<$" { 

   char *s;
   char *filename;
   char *linestr;
   s = yytext + 2;
   filename = strtok(s, ":");
   linestr = strtok(NULL, ":");

   env->meta.lineno = env->meta.caller->lineno;
   env->meta.filename = env->meta.caller->filename;
   env->meta.macroname = env->meta.caller->macroname;
   env->meta.caller = env->meta.caller->caller;
}

<INITIAL>"$*" { 
	/* this is a comment. */
	int c1 = 0;
	int c2 = input();
		
	for(;;) {
	   if (c2 == EOF) {
	      unput(c2);	
	      /* goto commentend; */ break;
	   }
	   if (c1 == '*' && c2 == '$') {
              break;
	   }
	   c1 = c2;
	   c2 = input();
	   if (c2 == '\n') {
              env->meta.lineno++;
           }
	}	

	/* commentend: */ 
        /* done */
}

<INITIAL>"<?moto"|"<$"|"${" {        
	lvalp->ivs = createIVS(env);
	BEGIN EMB;
	state = EMB;
    return DOLLAROPEN;
}

<EMB>"?>"|"}$"|"$>" {
        lvalp->ivs = createIVS(env);

        BEGIN INITIAL;
        state = INITIAL;
        return CLOSEDOLLAR;
}

<EMB>"/*"([^*]|[*][^/])*"*/" 	{ /* C style comment ... ignore */ countLines(env,yytext); }
<EMB>"//"([^\r\n])* 		{ /* C++ style comment ... ignore */ }
<INITIAL>"$(" { 
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;
	gobblews = 0;	
	BEGIN D; 
	state = D;
        depth++;
        return PRINT;
}

<EMB>"print" {
        lvalp->ivs = createIVS(env);
        gobblews=0;
        return EPRINT;
}

<INITIAL>"\\$" {
	lvalp->svs=createSVS(env,"$");

	return ANYCHARS; 
}

<INITIAL>"$use(" { 
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

	gobblews = 1;	
	BEGIN D;
	state = D;
        depth++;
	return USE; 
}

<EMB>"use" {
        lvalp->ivs = createIVS(env);

	return EUSE;
}

<INITIAL>"$define(" {
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

        gobblews = 1;
        BEGIN D;
        state = D;
        depth++;
        return DEFINE;
}

<INITIAL>"$enddef"[\t ]*([\n\r])? {
        lvalp->ivs = createIVS(env);

        countLines(env, yytext);
        gobblews = 1;
        return ENDDEF;
 }

<INITIAL>"$class(" {
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

        gobblews = 1;
        BEGIN D;
        state = D;
        depth++;
        return CLASS;
}

<INITIAL>"$endclass"[\t ]*([\n\r])? {
        lvalp->ivs = createIVS(env);

        countLines(env, yytext);
        gobblews = 1;
        return ENDCLASS;
 }
 
<INITIAL>"$return(" {
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

        gobblews = 1;
        BEGIN D;
        state = D;
        depth++;
        return RETURN;
}

<INITIAL>"$return"[\t ]*([\n\r])? {
        lvalp->ivs = createIVS(env);

        countLines(env, yytext);
        gobblews = 1;
        return VRETURN;
 }


<EMB>"return" {
        lvalp->ivs = createIVS(env);
        return ERETURN;
}

<INITIAL>"$throw(" {
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

        gobblews = 1;
        BEGIN D;
        state = D;
        depth++;
        return EXCP_THROW;
}

<INITIAL>"$catch(" {
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

        gobblews = 1;
        BEGIN D;
        state = D;
        depth++;
        return EXCP_CATCH;
}

<INITIAL>"$try"[\t ]*([\n\r])? {
        lvalp->ivs = createIVS(env);

        countLines(env, yytext);
        gobblews = 1;
        return EXCP_TRY;
}

<INITIAL>"$endtry"[\t ]*([\n\r])? {
        lvalp->ivs = createIVS(env);

        countLines(env, yytext);
        gobblews = 1;
        return EXCP_ENDTRY;
}

<INITIAL>"$finally"[\t ]*([\n\r])? {
        lvalp->ivs = createIVS(env);

        countLines(env, yytext);
        gobblews = 1;
        return EXCP_FINALLY;
 }

<EMB>"catch" {
        lvalp->ivs = createIVS(env);
        return EXCP_ECATCH;
}

<EMB>"throw" {
        lvalp->ivs = createIVS(env);
        return EXCP_ETHROW;
}

<EMB>"try" {
        lvalp->ivs = createIVS(env);
        return EXCP_ETRY;
}

<EMB>"finally" {
        lvalp->ivs = createIVS(env);
        return EXCP_EFINALLY;
}

<EMB>"class" {
        lvalp->ivs = createIVS(env);
        return ECLASS;
}

<INITIAL>"$declare(" { 
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

	gobblews = 1;	
	BEGIN D;
	state = D;
        depth++;
	return DECLARE; 
}

<INITIAL>"$do(" { 
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

	gobblews = 1;	
	BEGIN D;
	state = D;
        depth++;
	return DO; 
}

<INITIAL>"$for(" { 
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

	gobblews = 1;	
	BEGIN D;
	state = D;
        depth++;
	return FOR; 
}

<EMB>"for" {
        lvalp->ivs = createIVS(env);
	return EFOR;
}

<INITIAL>"$endfor"[\t ]*([\n\r])? { 
        lvalp->ivs = createIVS(env);

        countLines(env, yytext);
        gobblews = 1;	
        return ENDFOR; 
 }

<INITIAL>"$while(" { 
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

	BEGIN D;
	state = D;
	gobblews = 1;	
        depth++;
	return WHILE; 
}

<EMB>"while" {
        lvalp->ivs = createIVS(env);

        return EWHILE;
}

<INITIAL>"$endwhile"([\t\f ]*[\n\r])? { 
        lvalp->ivs = createIVS(env);

        countLines(env, yytext);
        gobblews = 1;	
	return ENDWHILE; 
}

<INITIAL>"$if(" { 
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

	BEGIN D;
	state = D;
	gobblews = 1;	
        depth++;
	return IF; 
}

<EMB>"if" {
        lvalp->ivs = createIVS(env);
        return EIF;
}

<INITIAL>"$elseif(" { 
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

        BEGIN D;
        state = D;
        gobblews = 1;	
        depth++;
        return ELSEIF; 
}

<INITIAL>"$else"([\t\f ]*[\n\r])? { 
        lvalp->ivs = createIVS(env);

        countLines(env, yytext);
        gobblews = 1;	
        return ELSE; 
}

<EMB>"else" {
        lvalp->ivs = createIVS(env);

        gobblews = 1;
        return EELSE;
}

<INITIAL>"$endif"([\t\f ]*[\n\r])? { 
        lvalp->ivs = createIVS(env);

        countLines(env, yytext);
        gobblews = 1;	
        return ENDIF; 
}

<INITIAL>"$break"([\t\f ]*[\n\r])? { 
        lvalp->ivs = createIVS(env);

        countLines(env, yytext);
        gobblews = 1;	
        return BREAK; 
}

<EMB>"break" {
        lvalp->ivs = createIVS(env);

        gobblews = 1;
        return EBREAK;
}

<INITIAL>"$continue"([\t\f ]*[\n\r])? { 
        lvalp->ivs = createIVS(env);

        countLines(env, yytext);
	gobblews = 1;	
	return CONTINUE; 
}

<EMB>"continue" {
        lvalp->ivs = createIVS(env);

        gobblews = 1;
        return ECONTINUE;
}

<INITIAL>"$switch(" { 
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

	BEGIN D;
	state = D;
	gobblews = 1;	
        depth++;
	return SWITCH; 
}

<EMB>"switch" {
        lvalp->ivs = createIVS(env);

        return ESWITCH;
}

<INITIAL>"$case(" { 
        lvalp->ivs = createIVS(env);
        lastOpenParen = env->meta;

	BEGIN D;
	state = D;
	gobblews = 1;	
        depth++;
   	return CASE; 
}

<EMB>"case" {
        lvalp->ivs = createIVS(env);

        return ECASE;
}

<EMB>"default" {
        lvalp->ivs = createIVS(env);

        return EDEFAULT;
}

<INITIAL>"$endswitch"([\t\f ]*[\n\r])? { 
        lvalp->ivs = createIVS(env);

        countLines(env, yytext);
        gobblews = 1;	
        return ENDSWITCH; 
}

<INITIAL>"$"({A}|{I}|{K})* {
        lvalp->ivs = createIVS(env);

        moto_noSuchConstruct(&env->meta,yytext);
        countLines(env, yytext);
}

<EMB,D>"{" { lvalp->ivs = createIVS(env);
	return OPENCURLY; }
<EMB,D>"}" { lvalp->ivs = createIVS(env);
	return CLOSECURLY; }

<EMB,D>"global" 	{lvalp->ivs = createIVS(env);return GLOBAL; }
<EMB,D>"new"		{lvalp->ivs = createIVS(env);return NEW; }
<EMB,D>"delete" 	{lvalp->ivs = createIVS(env);return DELETE; }
<EMB,D>"true" 		{lvalp->svs=createSVS(env,yytext);return BOOLEAN; }
<EMB,D>"false" 	{lvalp->svs=createSVS(env,yytext);return BOOLEAN; }
<EMB,D>"null" 		{lvalp->svs=createSVS(env,yytext); return MOTONULL; }
<EMB,D>"this" 		{lvalp->svs = createSVS(env,yytext); return THIS; }
<EMB,D>"def" 		{lvalp->svs = createSVS(env,yytext); return DEF; }
<EMB,D>"&&" 		{lvalp->ivs = createIVS(env);return AND; }
<EMB,D>"||"			{lvalp->ivs = createIVS(env);return OR; }
<EMB,D>"!"			{lvalp->ivs = createIVS(env);return NOT; }
<EMB,D>"&"			{lvalp->ivs = createIVS(env);return BITWISE_AND; }
<EMB,D>"|"			{lvalp->ivs = createIVS(env);return BITWISE_OR; }
<EMB,D>"^"			{lvalp->ivs = createIVS(env);return BITWISE_XOR; }
<EMB,D>"~"			{lvalp->ivs = createIVS(env);return BITWISE_NOT; }
<EMB,D>"<<"			{lvalp->ivs = createIVS(env);return SHIFT_LEFT; }
<EMB,D>">>"			{lvalp->ivs = createIVS(env);return SHIFT_RIGHT; }    
<EMB,D>"=="			{lvalp->ivs = createIVS(env); return REL_EQ_M; }
<EMB,D>"!="			{lvalp->ivs = createIVS(env); return REL_NE_M; }
<EMB,D>">"			{lvalp->ivs = createIVS(env);return REL_GT_M; }
<EMB,D>">="			{lvalp->ivs = createIVS(env);return REL_GTE_M; }
<EMB,D>"<"			{lvalp->ivs = createIVS(env);return REL_LT_M; }
<EMB,D>"<="			{lvalp->ivs = createIVS(env);return REL_LTE_M; }
<EMB,D>"eq"			{lvalp->ivs = createIVS(env);return REL_EQ_S; }
<EMB,D>"ne"			{lvalp->ivs = createIVS(env);return REL_NE_S; }
<EMB,D>"gt"			{lvalp->ivs = createIVS(env);return REL_GT_S; }
<EMB,D>"gte"		{lvalp->ivs = createIVS(env);return REL_GTE_S; }
<EMB,D>"lt"			{lvalp->ivs = createIVS(env);return REL_LT_S; }
<EMB,D>"lte"		{lvalp->ivs = createIVS(env);return REL_LTE_S; }
<EMB,D>"="			{lvalp->ivs = createIVS(env);return ASSIGN; }
<EMB,D>"++"			{lvalp->ivs = createIVS(env);return INC; }
<EMB,D>"+="			{lvalp->ivs = createIVS(env);return ADD_ASSIGN; }
<EMB,D>"-="			{lvalp->ivs = createIVS(env);return SUB_ASSIGN; }
<EMB,D>"*="			{lvalp->ivs = createIVS(env);return MULT_ASSIGN; }
<EMB,D>"/="			{lvalp->ivs = createIVS(env);return DIV_ASSIGN; }
<EMB,D>"%="			{lvalp->ivs = createIVS(env);return MOD_ASSIGN; }
<EMB,D>"+"			{lvalp->ivs = createIVS(env);return PLUS; }
<EMB,D>"--"			{lvalp->ivs = createIVS(env);return DEC; }
<EMB,D>"-"			{lvalp->ivs = createIVS(env);return MINUS; }
<EMB,D>"*"			{lvalp->ivs = createIVS(env);return MATH_MULT; }
<EMB,D>"/"			{lvalp->ivs = createIVS(env);return MATH_DIV; }
<EMB,D>"%"			{lvalp->ivs = createIVS(env);return MATH_MOD; }
<EMB,D>";"			{lvalp->ivs = createIVS(env);return SC; }
<EMB,D>","			{lvalp->ivs = createIVS(env);return COMMA; }
<EMB,D>"."			{lvalp->ivs = createIVS(env);return DOT; }
<EMB,D>"?"			{lvalp->ivs = createIVS(env);return QUESTION; }
<EMB,D>":"			{lvalp->ivs = createIVS(env);return COLON; }
<EMB,D>"=~"			{lvalp->ivs = createIVS(env);return RX_MATCH; }
<EMB,D>"!~"			{lvalp->ivs = createIVS(env);return RX_NOT_MATCH; }
<EMB,D>"["			{lvalp->ivs = createIVS(env);return OPENBRACKET; }
<EMB,D>"]"			{lvalp->ivs = createIVS(env);return CLOSEBRACKET; }
<EMB,D>"["{S}"]"	{lvalp->ivs = createIVS(env);return OPENCLOSEBRACKET; }
<EMB,D>"("			{lvalp->ivs = createIVS(env);depth++; return OPENPAREN; }
<EMB,D>")" {
        lvalp->ivs = createIVS(env);
	depth--;
	if (depth == 0) {
        
		if (state == D){
		   BEGIN INITIAL;
		   state = INITIAL;
		} else {
		   /* FIXME ? Unmatched paren */
		}
       
	}
       
	return CLOSEPAREN;
}

<EMB,D>0[xX]{H}+{IS}? { 
	lvalp->svs=createSVS(env,yytext);
	return INTEGER; 
}

<EMB,D>0{I}+{IS}? { 
        lvalp->svs=createSVS(env,yytext);
	return INTEGER; 
}

<EMB,D>{I}+{IS}? { 
	lvalp->svs=createSVS(env,yytext);
	return INTEGER; 
}

<EMB,D>{I}+{E}{FS}? { 
	lvalp->svs=createSVS(env,yytext);
	return FPNUM; 
}

<EMB,D>{I}*"."{I}+({E})?{FS}? { 
	lvalp->svs=createSVS(env,yytext);
	return FPNUM; 
}

<EMB,D>{I}+"."{I}*({E})?{FS}? { 
	lvalp->svs=createSVS(env,yytext);
	return FPNUM; 
}

<EMB,D>'({C}|(" ")|("\\"(({O}({O}({O})?)?)|(x{H}({H})?)|([\\\'abfnrtv]))))' { 
	lvalp->svs=createSVS(env,yytext);
	countLines(env, yytext);
	return CHAR; 
}

<EMB,D>\/(\\.|[^\/\n ]|(\[(\\.|[^\]\n])+\]))+\/ {
	lvalp->svs=createSVS(env,yytext);
	countLines(env, yytext);
	return REGEX; 
}

<EMB,D>\"(\\.|[^\\"\n])*\" { 
	lvalp->svs=createSVS(env,yytext);
	countLines(env, yytext);
	return STRING; 
}

<EMB,D>\"(\\.|[^\\"\n])*$ {
	lvalp->svs=createSVS(env,yytext);
	moto_unterminatedString(&env->meta);
	countLines(env, yytext);
	return STRING;
}

<EMB,D>"@"({A}|{K})({A}|{I}|{K})* { 
	lvalp->svs=createSVS(env,yytext);
	countLines(env, yytext);
	return IDENTIFIER; 
}

<EMB,D>({A}|{K})({A}|{I}|{K})* { 
	lvalp->svs=createSVS(env,yytext);
	countLines(env, yytext);
	return NAME; 
}

<EMB,D>"new"{S}{T}{S}"("({T}|{S}|[,()]|"[]")*")"({S}"[") {

	/* FIXME: I feel so dirty :( */

	int i=0,j=0, opens = 0, matchedHOF = 0;
	char* mtext = estrdup( yytext+3);
	char word[255];
	
	for(i=0;i<strlen(yytext+3);i++){
		char c = yytext[i+3];
		
		/* Make sure there are no reserved words being used */
		if(c==',' || c=='['  || c=='('  || c==')' || c==']' || 
			c==' ' || c=='\n' || c=='\r' || c=='\t') {
			word[j] = '\0';
			if(	isReserved(word) ) {matchedHOF=0; break;}
			else if(matchedHOF == 1 && j>0) { break; }
			else j=0;    		    		     		    				
		}
		
		if(c == '(') { 
			opens++; matchedHOF=0;
		} else if(c == ')') { 
			if(opens == 0) {matchedHOF=0; break;} 
			else opens--;
			if(opens == 0) matchedHOF=1;
		} else if(c!=',' && c!='['  && c!='('  && c!=')' && c!=']' && 
					 c!=' ' && c!='\n' && c!='\r' && c!='\t')
			 {word[j] = c; j++;}
	}
	
	for(i=strlen(mtext)-1; i>=0;i--)
		unput(mtext[i]);
	free(mtext);
	
	if(matchedHOF){				
		lvalp->ivs = createIVS(env);
		return HOF_NEW;
	}else{
		lvalp->ivs = createIVS(env);
		return NEW;
	} 
}
	
<EMB,D>{T}{S}"("({T}|{S}|[,()]|"[]")*")"({S}"[]")*{S}{T} {
	int i=0,j=0, opens = 0, matchedHOF = 0;
	char word[255];
	
	for(i=0;i<strlen(yytext);i++){
		char c = yytext[i];
		
		/* Make sure there are no reserved words being used */
		if(c==',' || c=='['  || c=='('  || c==')' || c==']' || 
			c==' ' || c=='\n' || c=='\r' || c=='\t') {
			word[j] = '\0';
			if(	isReserved(word) ) {matchedHOF=0; break;}
			else if(matchedHOF == 1 && j>0) { break; }
			else j=0;    		    		     		    				
		}
		
		if(c == '(') { 
			opens++; matchedHOF=0;
		} else if(c == ')') { 
			if(opens == 0) {matchedHOF=0; break;} 
			else opens--;
			if(opens == 0) matchedHOF=1;
		} else if(c!=',' && c!='['  && c!='('  && c!=')' && c!=']' && 
					 c!=' ' && c!='\n' && c!='\r' && c!='\t')
			 {word[j] = c; j++;}
	}
    
   word[j]=0; i-=strlen(word);
   
	if(	isReserved(word) ) {matchedHOF=0;}
				 
	if(matchedHOF) {
		
		char* mtext = estrdup( yytext);
		int len = strlen(yytext), ws = 0;
		// printf("### word '%s'\n",word);
		// printf("### matched '%s'\n",mtext);
		if(i<len){
			for(j=len-1; j>=i;j--)
				unput(mtext[j]);
		}
		if(i<len) {
		//	 printf("### put back '%s'\n",mtext+i);
		}

		mtext[i]='\0';
		
		for(i=0;mtext[i+ws];i++){
			char c = mtext[i+ws];
		   if(c == ' ' || c == '\t' || c == '\r' || c == '\n') {ws++;i--;}
		   else {mtext[i]= c;}
		}
		
		mtext[i]='\0';
		
		lvalp->svs=createSVS(env,mtext);
		countLines(env, mtext);
		free(mtext);
		
		// printf("### generate '%s'\n",mtext);

		return NAME;
	} else {
		int len = strlen(yytext);
		char* mtext = estrdup( yytext);
		// printf("### matched '%s'\n",mtext);
	   for(i=0;i<len;i++) {
	   	char c = yytext[i]; 
	   	if(c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
	   		c == '[' || c == ']' || c == '(' || c == ')' || c == ',')
	   		break;
	   }
		for(j=len-1; j>=i;j--)
			unput(mtext[j]);
		// printf("### put back '%s'\n",mtext+i);
		mtext[i]=0;
		// printf("### generate '%s'\n",mtext);
		
		if(strcmp("while",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return EWHILE;}
		else if(strcmp("for",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return EFOR;}
		else if(strcmp("switch",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return ESWITCH;}
		else if(strcmp("case",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return ECASE;}
		else if(strcmp("default",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return EDEFAULT;}						
		else if(strcmp("if",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return EIF;}
		else if(strcmp("else",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return EELSE;}		
		else if(strcmp("try",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return EXCP_ETRY;}
		else if(strcmp("throw",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return EXCP_ETHROW;}
		else if(strcmp("catch",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return EXCP_ECATCH;}
		else if(strcmp("finally",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return EXCP_EFINALLY;}		
		else if(strcmp("print",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return EPRINT;}
		else if(strcmp("return",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return ERETURN;}
		else if(strcmp("use",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return EUSE;}
		else if(strcmp("break",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return EBREAK;}
		else if(strcmp("continue",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return ECONTINUE;}
		else if(strcmp("class",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return ECLASS;}
				
		else if(strcmp("eq",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return REL_EQ_S;} 	
		else if(strcmp("ne",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return REL_NE_S;}
		else if(strcmp("lt",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return REL_LT_S;}
		else if(strcmp("gt",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return REL_GT_S;}
		else if(strcmp("lte",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return REL_LTE_S;}
		else if(strcmp("gte",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return REL_GTE_S;}

		else if(strcmp("this",mtext) == 0) {lvalp->svs=createSVS(env,mtext); free(mtext); return THIS;}
		else if(strcmp("delete",mtext) == 0) {lvalp->ivs=createIVS(env); free(mtext); return DELETE;}
		else if(strcmp("new",mtext) == 0) {lvalp->ivs=createIVS(env); free(mtext); return NEW;}		
		else if(strcmp("global",mtext) == 0) {lvalp->ivs=createIVS(env); free(mtext); return GLOBAL;}	
		
		else if(strcmp("true",mtext) == 0) {lvalp->svs=createSVS(env,mtext); free(mtext); return BOOLEAN;}
		else if(strcmp("false",mtext) == 0) {lvalp->svs=createSVS(env,mtext); free(mtext); return BOOLEAN;}
		else if(strcmp("null",mtext) == 0) {lvalp->ivs = createIVS(env); free(mtext); return MOTONULL;}
						
		else {lvalp->svs=createSVS(env,mtext); free(mtext); return NAME;}
	}; 
}

<EMB,D>[ \n\r\f\t]+ { 
    /* int oldLines =env->meta.lineno; */
    countLines(env, yytext); 
    /* if(state==EMB && env->meta.lineno!=oldLines)
       gobblews = 1; */
}
 
<EMB,D>"$"{A}* { moto_noSuchConstruct(&env->meta,yytext); }

<EMB,D>. {countLines(env, yytext); moto_syntaxError(&env->meta);}

<INITIAL>(({W})+[\n\r\f ]+)+ {
	lvalp->svs=createSVS(env,yytext);
        countLines(env, yytext);
	return ANYCHARS; 
}

<INITIAL>({W})+ {
        lvalp->svs=createSVS(env,yytext);
        countLines(env, yytext);
	return ANYCHARS; 
}

<INITIAL>^[ \n\r\f\t]+ {
        /* Tabs which begin lines do not get printed */
	/* FIXME: fine ... I'll maintain this behavior for now ... but it's still dumb */
	countLines(env, yytext);
}

<INITIAL>[ \n\r\f\t]+ {
        lvalp->svs=createSVS(env,yytext);
        countLines(env, yytext);
        
        if(!gobblews)
			return WS;
		else
			gobblews=0; 
}

<INITIAL>. {
        lvalp->svs=createSVS(env,yytext);
        countLines(env, yytext);
	return ANYCHARS; 	
}

<<EOF>> {
        if (state == D) {
           moto_unmatchedParen(&lastOpenParen); 
        }
	/* if (stack_size(env->scope) > 0) {

           while(stack_size(env->scope) > 0) {
              MotoParseScope *scope = stack_peek(env->scope);
              int type = scope->type;

              switch (type) {
                 case FOR: moto_unterminatedFor(&scope->meta,'\0'); break;
                 case IF: moto_unterminatedIf(&scope->meta,'\0'); break;
                 case SWITCH: moto_unterminatedSwitch(&scope->meta,'\0'); break;
                 case WHILE: moto_unterminatedWhile(&scope->meta,'\0'); break;

                 case EFOR: moto_unterminatedFor(&scope->meta,'\1'); break;
                 case EIF: moto_unterminatedIf(&scope->meta,'\1'); break;
                 case ESWITCH: moto_unterminatedSwitch(&scope->meta,'\1'); break;
                 case EWHILE: moto_unterminatedWhile(&scope->meta,'\1'); break;
              }
              free(scope);
              stack_pop(env->scope);
           }
	} */

        yyterminate();	
        // return ENDOFFILE;
}

%% 

int 
yywrap() {
	return 1;
}

void 
moto_initYY(FILE *file) {
	yyrestart(file);
}

void 
moto_freeYYBuffer() {
	yy_delete_buffer(YY_CURRENT_BUFFER);
}

int 
moto_prepareMainBuffer(MotoEnv *env, char *text) {
	int len;
	env->tree->code = text;
	len = strlen(text);
	yy_switch_to_buffer(yy_scan_bytes(text, (yy_size_t)len));
	BEGIN INITIAL;
	state = INITIAL;
	depth = 0; 
        gobblews = 0;
	return 0;
}

static IVS*
createIVS(MotoEnv *env){
	IVS* ivs = (IVS*)moto_malloc(env,sizeof(IVS));
        ivs->meta.lineno    = env->meta.lineno;
	ivs->meta.filename  = env->meta.filename;
        ivs->meta.macroname = env->meta.macroname;
	ivs->meta.caller    = env->meta.caller;
	return ivs;
}

static SVS*
createSVS(MotoEnv *env,char* s){
        SVS* svs = (SVS*)moto_malloc(env,sizeof(SVS));
        svs->meta.lineno    = env->meta.lineno;
        svs->meta.filename  = env->meta.filename;
        svs->meta.macroname = env->meta.macroname;
        svs->meta.caller    = env->meta.caller;
	svs->sval = moto_strdup(env, s);
	return svs;
}

static void 
countLines(MotoEnv *env, char *s) {
        int d = 0;
 
        /* Set the current line to the number of lines discovered 
           up until this point */

        while (*s) { if (*s == '\n') { d++; } s++; }

        /* Add any linebreaks discovered in the current token to the
           total line count */

        env->meta.lineno += d;
}

int
popEScope(MotoEnv *env) {

        while(stack_size(env->scope) > 0){
           MotoParseScope *scope = stack_peek(env->scope);
           if (scope->type == EFOR || 
               scope->type == EWHILE || 
               scope->type == EIF || 
               scope->type == EELSE ||
               scope->type == ESWITCH  ) {

              scope = stack_pop(env->scope);
              free(scope);
           } else {
              break;
           }
        }
        return 1;
}

int 
peekScope(MotoEnv *env, int verify) {
	if (stack_size(env->scope) == 0)	{
		return 0;
	}
	else {
		MotoParseScope *scope = stack_peek(env->scope);
		if (scope->type != verify) {
			return 0;
		}
	}
	return 1;
}

static int
isReserved(char* word){
	return strcmp("while",word) == 0
		|| strcmp("for",word) == 0
		|| strcmp("switch",word) == 0
		|| strcmp("case",word) == 0   
		|| strcmp("default",word) == 0 				
		|| strcmp("if",word) == 0
		|| strcmp("try",word) == 0   
		|| strcmp("throw",word) == 0   
		|| strcmp("catch",word) == 0
		|| strcmp("print",word) == 0 
		|| strcmp("return",word) == 0
		|| strcmp("finally",word) == 0 
		|| strcmp("use",word) == 0 
		|| strcmp("break",word) == 0 
		|| strcmp("continue",word) == 0 
		|| strcmp("eq",word) == 0 	
		|| strcmp("ne",word) == 0 
		|| strcmp("lt",word) == 0 	
		|| strcmp("gt",word) == 0 	
		|| strcmp("lte",word) == 0 	
		|| strcmp("gte",word) == 0 
		|| strcmp("this",word) == 0 	
		|| strcmp("delete",word) == 0 
		|| strcmp("new",word) == 0 					
		|| strcmp("global",word) == 0 				
		|| strcmp("true",word) == 0 	
		|| strcmp("false",word) == 0 	
		|| strcmp("null",word) == 0 																																	
		|| strcmp("class",word) == 0 ;
}

/*=============================================================================
// end of file: $RCSfile: moto.l,v $
==============================================================================*/

