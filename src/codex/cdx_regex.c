#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "memsw.h"
#include "excpfn.h"
#include "exception.h"
#include "mxstring.h"
#include "mxarr.h"
#include "mman.h"
#include "runtime.h"
#include "cdx_regex.h"

Regex* regex_create(char* rx){
	return tnfa_create(rx);
}
void regex_free(Regex* rx){
	tnfa_free(rx);
}

Match* regex_match(Regex* rx, char* input){
	Match* match = (Match*)emalloc(sizeof(Match));
	match->tnfa = rx;
	match->input = input;
	match->tvf = tnfa_match(match->tnfa,input,1,1);
	return match;
}

Match* regex_search(Regex* rx, char* input){
	Match* match = (Match*)emalloc(sizeof(Match));
	match->tnfa = rx;
	match->input = input;
	match->tvf = tnfa_match(match->tnfa,input,0,0);
	return match;
}
Match* regex_matchPrefix(Regex* rx,char* input){
	Match* match = (Match*)emalloc(sizeof(Match));
	match->tnfa = rx;
	match->input = input;
	match->tvf = tnfa_match(match->tnfa,input,1,0);
	return match;
}
Match* regex_matchSuffix(Regex* rx,char* input){
	Match* match = (Match*)emalloc(sizeof(Match));
	match->tnfa = rx;
	match->input = input;
	match->tvf = tnfa_match(match->tnfa,input,0,1);
	return match;
}


char regex_matches(Regex* rx, char* input){
	TagValueFunction* tvf = tnfa_match(rx,input,1,1);
	if(tvf[0]!=-1){
		free(tvf);
		return 1; 
	} else {
		free(tvf);
		return 0;
	}
}

char regex_matchesPrefix(Regex* rx, char* input){
	TagValueFunction* tvf = tnfa_match(rx,input,1,0);
	if(tvf[0]!=-1){
		free(tvf);
		return 1; 
	} else {
		free(tvf);
		return 0;
	}
}

char regex_matchesSuffix(Regex* rx, char* input){
	TagValueFunction* tvf = tnfa_match(rx,input,0,1);
	if(tvf[0]!=-1){
		free(tvf);
		return 1; 
	} else {
		free(tvf);
		return 0;
	}
}

char regex_matchesSubstring(Regex* rx, char* input){
	TagValueFunction* tvf = tnfa_match(rx,input,0,0);
	if(tvf[0]!=-1){
		free(tvf);
		return 1; 
	} else {
		free(tvf);
		return 0;
	}
}

char* regex_toTNFAString(Regex* rx){
	return tnfa_toString(rx);
}

void match_free(Match* match){
	free(match->tvf);
	free(match);
}

char* match_subMatch(Match* m, int i){
	if(i>m->tnfa->tagcount / 2)
		{ THROW_D("ArrayBoundsException"); }
	if(m->tvf[i*2] == -1) /* subexpression not matched */
		return NULL;
	return mxstr_substring(m->input,m->tvf[i*2],m->tvf[i*2 +1]);
}

char* match_preMatch(Match* m){
	if(m->tvf[0] == -1) /* expression not matched */
		return m->input;
	return mxstr_substring(m->input,0,m->tvf[0]);
}

char* match_postMatch(Match* m){
	if(m->tvf[1] == -1) /* expression not matched */
		return m->input;
	return mxstr_substr(m->input,m->tvf[1]);
}

int match_startIndex(Match* m,int i){
	if(i>m->tnfa->tagcount / 2)
		{ THROW_D("ArrayBoundsException"); }
	return m->tvf[i*2];
}

int match_endIndex(Match* m,int i){
	if(i>m->tnfa->tagcount / 2)
		{ THROW_D("ArrayBoundsException"); }
	return m->tvf[i*2+1];
}

char* match_toString(Match* m){
	StringBuffer *sb = buf_createDefault();
	int i; char* s;
	buf_putc(sb,'{');
	/* buf_puts(sb,tvf_toString(m->tvf,m->tnfa));*/
	for (i=0;i<m->tnfa->tagcount / 2;i++){
		if(i>0)buf_putc(sb,',');
		s=match_subMatch(m,i);
		buf_putc(sb,'"');
		buf_puts(sb,s);
		buf_putc(sb,'"');
		if (s!=NULL)free(s);
	}
	buf_putc(sb,'}');
	s=buf_toString(sb);
	buf_free(sb);
	return s;
}

typedef struct stringle{
	int submatch;
	char* substr;
	struct stringle* next;
} Stringle;

char* regex_sub(char* s, Regex* rx, char* sub){
	StringBuffer* sb = buf_createDefault();
	Match* match = NULL;
	char* csub = strchr(sub,'$');
	char* result;
	Stringle *first=NULL,*last=NULL,*curr;
	
	/* Step 1, tokenize the substitution */

	while(1) {
		csub = strchr(sub,'$');
		curr = emalloc(sizeof(Stringle));
		
		if(csub == NULL)
			csub = sub+ strlen(sub);
		
		/* Construct the stringle for the pre-match */
		curr->submatch=-1;
		curr->substr= mxstr_substring(sub,0,csub-sub);
		curr->next = NULL;
		
		if (!first) first = curr;
		else last->next = curr;
		
		last = curr;
		
		/* If the substitution ended then break */
		if(*csub == '\0') break;
		
		/* Otherwise move past the '$' */
		csub ++;
				
		curr = emalloc(sizeof(Stringle));
		curr->submatch = atoi(csub);
		curr->substr = NULL;
		curr->next = NULL;
		
		last->next = curr;		
		last = curr;
		
		/* Move past the numbers after the $ */
		while(isdigit(*csub)) csub ++;
		
		if(*csub == '\0') break;
		sub=csub;
	}
	
	/* Step 2, replace instances of the pattern in s with the substitution */

	while(*s){
		/* Attempt to find the pattern within t->string */
		match = regex_search(rx,s);

		/* Append the pre-match to the buffer */
		if(match_startIndex(match,0) == -1)
			buf_puts(sb,s);
		else
			buf_put(sb,s,match_startIndex(match,0));
			
		/* If the pattern was not found then we are done */
		if(match_startIndex(match,0) == -1) {
			match_free(match);
			break;
		}
		
		for(curr=first;curr != NULL ;curr=curr->next){
			if(curr->submatch == -1)
				buf_puts(sb,curr->substr);
			else if(match_startIndex(match,curr->submatch) != -1)
				buf_put(sb,s + match_startIndex(match,curr->submatch), 
					match_endIndex(match,curr->submatch) - match_startIndex(match,curr->submatch));
		}
		
		s += match_endIndex(match,0);
		match_free(match);
	}

	/* Step 3, free the pattern stringle */
	for(curr=first;curr != NULL ;){
		if(curr->submatch == -1 && curr->substr != NULL) free(curr->substr);
		last=curr->next;
		free(curr);
		curr=last;
	}
	
	result = buf_toString(sb);
	buf_free(sb);
	
	return result;
}

char* regex_fsub(char* s, Regex* rx, Function* sub){
	StringBuffer* sb = buf_createDefault();
	Match* match = NULL;
	char* result;
	
	/* Step 2, replace instances of the pattern in s with the substitution */

	while(*s){
		/* Attempt to find the pattern within t->string */
		match = regex_search(rx,s);

		/* Append the pre-match to the buffer */
		if(match_startIndex(match,0) == -1)
			buf_puts(sb,s);
		else
			buf_put(sb,s,match_startIndex(match,0));
			
		/* If the pattern was not found then we are done */
		if(match_startIndex(match,0) == -1) {
			match_free(match);
			break;
		}
		
		buf_puts(sb,
			( sub->flags & F_INTERPRETED ? 
			(char*)ifunc_rcall(sub,"O", match) : 
			((char*(*)(Function*,Match*))sub->fn)(sub,match)  )
		);
		
		s += match_endIndex(match,0);
		match_free(match);
	}
	
	result = buf_toString(sb);
	buf_free(sb);
	
	return result;
}

UnArray* regex_split(char* s, Regex* rx){
	Vector* v = vec_createDefault();
	UnArray* a;
	Match* match = NULL;
	int i;
	
	while(*s){
		int submatches;
		
		/* Attempt to find the pattern within t->string */
		match = regex_search(rx,s);

		/* Append the pre-match to the vector */
		if(match_startIndex(match,0) == -1)
			vec_add(v,mman_track(rtime_getMPool(),estrdup(s)));
		else
			vec_add(v,mman_track(rtime_getMPool(),mxstr_substring(s,0,match_startIndex(match,0))));
			
		/* If the pattern was not found then we are done */
		if(match_startIndex(match,0) == -1) {
			match_free(match);
			break;
		}
		
		/* Add captured submatches to the vector */
		submatches = match->tnfa->tagcount / 2;
		for(i=1;i< submatches;i++)
			vec_add(v,mman_track(rtime_getMPool(),match_subMatch(match, i)));
		
		s += match_endIndex(match,0);
		match_free(match);
	}
	
	a = vec_toArray(v);
	vec_free(v);
	
	return a;
}

char* regex_join(UnArray* a, char* s){
	StringBuffer* sb = buf_createDefault();
	char* result;
	int i;
	
	for(i=0;i<a->meta.length;i++){
		if(i>0) buf_puts(sb,s);
		buf_puts(sb,*(char**)inline_rsub(a,i));
	}	
	result = buf_toString(sb);
	buf_free(sb);
	
	return result;
}

/*
int mxstr_prefixMatches(char* s,Regex* rx);
int mxstr_suffixMatches(char* s,Regex* rx);
int mxstr_contains(char* s, Regex* rx);
int mxstr_matches(char* s, Regex* rx);*/
