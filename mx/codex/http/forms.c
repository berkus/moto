#include "forms.h"

#include "state.h"
#include "module.h"
#include "memsw.h"
#include "stringbuffer.h"
#include "tokenizer.h"

char* form_textfieldNameOnly( char* name)
{ return form_textfield(name,NULL,0,0); }

char* form_textfieldNameAndValue( char* name,char* value)
{ return form_textfield(name,value,0,0); }

char* form_textfieldNameAndLength( char* name,int length)
{ return form_textfield(name,NULL,length,0); }

char* form_textfieldNameValueAndLength( char* name,char* value,int length)
{ return form_textfield(name,value,length,0); }

char* form_textfield(
   char* name, 
   char* value, 
   int length, 
   int maxlength
){
   State *s =getState();
   StringBuffer* sb = buf_createDefault();
   char* cv,*r;

   buf_puts(sb,"<input type=\"text\" name=\"");
   buf_puts(sb,name);
   buf_puts(sb,"\" ");

   cv = state_getValue(s,name) ;
   if (cv == NULL)
      cv = value;  
   
   if (cv != NULL){
      buf_puts(sb,"value=\"");
      buf_puts(sb,cv);
      buf_puts(sb,"\" ");
   }

   if (length > 0){
      buf_puts(sb,"size=\"");
      buf_puti(sb,length);
      buf_puts(sb,"\" ");
   }

   if (maxlength > 0){
      buf_puts(sb,"maxlength=\"");
      buf_puti(sb,maxlength);
      buf_puts(sb,"\" ");
   }

   buf_putc(sb,'>');
   r=buf_toString(sb);

   buf_free(sb);
   return r;
}

char* form_textarea(
   char* name,
   char* value,
   int rows,
   int cols
){
   State *s =getState();
   StringBuffer* sb = buf_createDefault();
   char* cv,*r;

   buf_puts(sb,"<textarea name=\"");
   buf_puts(sb,name);
   buf_puts(sb,"\" ");

   cv = state_getValue(s,name) ;
   if (cv == NULL)
      cv = value; 

   if (rows > 0){
      buf_puts(sb,"rows=\"");
      buf_puti(sb,rows);
      buf_puts(sb,"\" ");
   }

   if (cols > 0){
      buf_puts(sb,"cols=\"");
      buf_puti(sb,cols);
      buf_puts(sb,"\" ");
   }

   buf_putc(sb,'>');

   if (cv != NULL)
      buf_puts(sb,cv);

   buf_puts(sb,"</textarea>");

   r=buf_toString(sb);

   buf_free(sb);
   return r;
}

char* form_multicheckbox(char* name, char* value, char checked){
   State* s = getState();
   StringBuffer* sb = buf_createDefault();
   char* cv,*r;

   buf_puts(sb,"<input type=\"checkbox\" name=\"");
   buf_puts(sb,name);
   buf_puts(sb,"\" ");

   if (value != NULL){
      buf_puts(sb,"value=\"");
      buf_puts(sb,value);
      buf_puts(sb,"\" ");
   }

   cv = state_getValue(s,name) ;
   if (cv != NULL){
      if(value == NULL){
         buf_puts(sb,"CHECKED ");
      } else if (strcmp(value,cv)==0){
         buf_puts(sb,"CHECKED ");
      } else {
         Tokenizer* t = tok_createCTok(cv,'|');
         char* substr;
         while((substr = tok_next(t)) != NULL){
            if (strcmp (substr,value)==0){
               buf_puts(sb,"CHECKED ");
               free(substr);
               break;
            }
            free(substr);
         }
         tok_free(t);
      }
   } else if (checked) {
      buf_puts(sb,"CHECKED ");
   }

   buf_putc(sb,'>');
   r=buf_toString(sb);

   buf_free(sb);   
   return r;
}

char* form_checkbox(char* name, char* value, char checked){
   State* s=getState();
   StringBuffer* sb = buf_createDefault();
   char* cv,*r;

   buf_puts(sb,"<input type=\"checkbox\" name=\"");
   buf_puts(sb,name);
   buf_puts(sb,"\" ");

   if (value != NULL){
      buf_puts(sb,"value=\"");
      buf_puts(sb,value);
      buf_puts(sb,"\" ");
   }

   cv = state_getValue(s,name) ;
   if (cv != NULL){
      if(value == NULL){
         buf_puts(sb,"CHECKED ");
      } else if (strcmp(value,cv)==0){
         buf_puts(sb,"CHECKED ");
      } 
   } else if (checked) {
      buf_puts(sb,"CHECKED ");
   }

   buf_putc(sb,'>');
   r=buf_toString(sb);

   buf_free(sb);
   return r;
}

char* form_hidden(char* name, char* value){
   State* s=getState();
   StringBuffer* sb = buf_createDefault();
   char* cv,*r;

   buf_puts(sb,"<input type=\"hidden\" name=\"");
   buf_puts(sb,name);
   buf_puts(sb,"\" value=\"");

   cv = state_getValue(s,name) ;
   if (cv != NULL)
      buf_puts(sb,cv);
   else
      buf_puts(sb,value);
   buf_puts(sb,"\">");

   r=buf_toString(sb);

   buf_free(sb);
   return r;
}

char* form_radio(char* name, char* value, char checked){
   State* s=getState();
   StringBuffer* sb = buf_createDefault();
   char* cv,*r;

   buf_puts(sb,"<input type=\"radio\" name=\"");
   buf_puts(sb,name);

   buf_puts(sb,"\" value=\"");
   buf_puts(sb,value);
   buf_puts(sb,"\" ");

   cv = state_getValue(s,name) ;
   if (cv != NULL){
      if(value == NULL){
         buf_puts(sb,"CHECKED ");
      } else if (strcmp(value,cv)==0){
         buf_puts(sb,"CHECKED ");
      } 
   } else if (checked) {
      buf_puts(sb,"CHECKED ");
   }

   buf_putc(sb,'>');
   r=buf_toString(sb);

   buf_free(sb);
   return r;
}

char* form_multioption(char* name, char* value, char* selected){
   State*s=getState();
   StringBuffer* sb = buf_createDefault();
   char* cv,*r;

   buf_puts(sb,"<option value=\"");
   buf_puts(sb,value);
   buf_puts(sb,"\" ");

   cv = state_getValue(s,name) ;
   if (cv != NULL){
      if(value == NULL){
         buf_puts(sb,"SELECTED ");
      } else if (strcmp(value,cv)==0){
         buf_puts(sb,"SELECTED ");
      } else {
         Tokenizer* t = tok_createCTok(cv,'|');
         char* substr;
         while((substr = tok_next(t)) != NULL){
            if (strcmp (substr,value)==0){
               buf_puts(sb,"SELECTED ");
               free(substr);
               break;
            }
            free(substr);
         }
         tok_free(t);
      }
   } else if (selected) { 
      Tokenizer* t = tok_createCTok(selected,'|');
      char* substr;
      while((substr = tok_next(t)) != NULL){
         if (strcmp (substr,value)==0){
            buf_puts(sb,"SELECTED ");
            free(substr);
            break;
         }
         free(substr);
      }
      tok_free(t);
   }

   buf_putc(sb,'>');
   r=buf_toString(sb);

   buf_free(sb);
   return r;
}

char* form_option(char* name, char* value, char* selected){
   State* s=getState();
   StringBuffer* sb = buf_createDefault();
   char* cv,*r;

   buf_puts(sb,"<option value=\"");
   buf_puts(sb,value);
   buf_puts(sb,"\" ");

   cv = state_getValue(s,name) ;
   if (cv != NULL){
      if(value == NULL){
         buf_puts(sb,"SELECTED ");
      } else if (strcmp(value,cv)==0){
         buf_puts(sb,"SELECTED ");
      } 
   } else if (selected != NULL && strcmp(value,selected)==0) {
      buf_puts(sb,"SELECTED ");
   }

   buf_putc(sb,'>');
   r=buf_toString(sb);

   buf_free(sb);
   return r;
}
