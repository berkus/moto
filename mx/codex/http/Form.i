Extension: Form

Include: "httpd.h"
Include: "module.h"
Include: "state.h"
Include: "forms.h"

Interface:

/**
  Convenience function that returns a String equivalent to: <br><br>
  &lt; input type="checkbox" name="$(<i>name</i>)" value="$(<i>value</i>)" &gt;
  <br><br>
  The attribute CHECKED will be added if the state variable named <i>name</i> is set
  and equals the value <i>value</i> OR the state variable named <i>name</i> is
  not set and <i>checked</i> is set to true
**/
 
tracked String formCheckbox(String name,String value,boolean checked) =>
   char* form_checkbox(char* name, char* value, char checked);

/**
  Convenience function that returns a String equivalent to: <br><br>
  &lt; input type="checkbox" name="$(<i>name</i>)" value="$(<i>value</i>)" &gt;
  <br><br>
  The attribute CHECKED will be added if the state variable named <i>name</i> is set
  and contains the value <i>value</i> OR the state variable named <i>name</i> is
  not set and <i>checked</i> is set to true
**/

tracked String formMultiCheckbox(String name,String value, boolean checked) =>
   char* form_multicheckbox(char* name, char* value, char checked);

/**
  Convenience function that returns a String equivalent to: <br><br>
  &lt;textarea name="$(<i>name</i>)" rows="$(<i>rows</i>)" cols="$(<i>cols</i>)"&gt;<br>
     $(getValue(<i>name</i>,<i>value</i>))<br>
  &lt;/textarea&gt;
**/

tracked String formTextarea(String name, String value, int rows, int cols) =>
   char* form_textarea( char* name, char* value, int rows, int cols);

/** Convenience function that returns a String equivalent to: formTextField(<i>name</i>,null) **/
tracked String formTextfield(String name) =>
   char* form_textfieldNameOnly( char* name);

/** Convenience function that returns a String equivalent to: formTextField(<i>name</i>,<i>value</i>,0) **/
tracked String formTextfield(String name, String value) =>
   char* form_textfieldNameAndValue( char* name,char* value);

/** Convenience function that returns a String equivalent to: formTextField(<i>name</i>,null,0) **/
tracked String formTextfield(String name, int length) =>
   char* form_textfieldNameAndLength( char* name,int length);

/** Convenience function that returns a String equivalent to: formTextField(<i>name</i>,<i>value</i>,<i>length</i>) **/
tracked String formTextfield(String name, String value, int length) =>
   char* form_textfieldNameValueAndLength( char* name,char* value,int length);

/** Convenience function that returns a String equivalent to: <br><br>
   &lt;input type="text" name="$(<i>name</i>)" value="$(getValue(<i>name</i>,<i>value</i>))" &gt;<br><br>
   If <i>length</i> is != 0 a length="$(<i>length</i>)" attribute is added <br>
   If <i>maxlength</i> is != 0 a maxlength="$(<i>maxlength</i>)" attribute is added <br>
**/
tracked String formTextfield(String name, String value, int length,int maxlength) =>
   char* form_textfield( char* name, char* value, int length, int maxlength);

/**
  Convenience function that returns a String equivalent to: <br><br>
  &lt; input type="radio" name="$(<i>name</i>)" value="$(<i>value</i>)" &gt;
  <br><br>
  The attribute CHECKED will be added if the state variable named <i>name</i> is set
  and equals the value <i>value</i> OR the state variable named <i>name</i> is
  not set and <i>checked</i> is set to true
**/

tracked String formRadio(String name,String value,boolean checked) =>
   char* form_radio(char* name, char* value, char checked);

/**
  Convenience function that returns a String equivalent to: <br><br>
  &lt; option value="$(<i>value</i>)" &gt;
  <br><br>
  The attribute SELECTED will be added if the state variable named <i>name</i> is set
  and equals the value <i>value</i> OR the state variable named <i>name</i> is
  not set and <i>selected</i> is set to true
**/

tracked String formOption(String name,String value,String selected) =>
   char* form_option(char* name, char* value, char* selected);

/**
  Convenience function that returns a String equivalent to: <br><br>
  &lt; option value="$(<i>value</i>)" &gt;
  <br><br>
  The attribute SELECTED will be added if the state variable named <i>name</i> is set
  and contains the value <i>value</i> OR the state variable named <i>name</i> is
  not set and <i>selected</i> is set to true
**/

tracked String formMultiOption(String name,String value,String selected) =>
   char* form_multioption(char* name, char* value, char* selected);

/**
  Convenience function that returns a String equivalent to: <br><br>
  &lt; input type="hidden" name="$(<i>name</i>)" value="$(<i>value</i>)" &gt;
**/

tracked String formHidden(String name,String value) =>
   char* form_hidden(char* name, char* value);
