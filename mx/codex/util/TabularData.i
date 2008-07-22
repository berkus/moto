Extension: TabularData

Include: "tabulardata.h"

Interface:

/** Constructs a new TabularData object of <i>rows</i> rows and <i>columns</i> 
columns. The column names are specified in the '|' delimited string <i>colnames</i> .
The column types are specified in the '|' delimited string <i>coltypes</i>. e.g. <br><br>
$do(cardData = new TabularData(<br>
&nbsp;&nbsp;&nbsp;cacherset.getInt(0)+1,<br>
&nbsp;&nbsp;&nbsp;10,<br>
&nbsp;&nbsp;&nbsp;"shortname,name,card,print,color,type,cost,rarity,price,diff",<br>
&nbsp;&nbsp;&nbsp;"String,String,int,int,char,String,String,char,float,float")<br>
)
**/

tracked TabularData TabularData::TabularData(
   int rows, int columns, String colnames, String coltypes
) => TabularData* tdata_create(
   int rows, int columns, char* colnames, char* coltypes
);

void TabularData::~TabularData() => 
	void tdata_free(TabularData* tdata);

/** Optimizes the storage for this tabular data object by moving the contents of all
String fields into one big buffer **/
void TabularData::optimize() =>
	void tdata_optimize(TabularData* tdata);

/** Set the int at row <i>row</i> col <i>col</i> to <i>value</i> **/
void TabularData::setInt(int row, int col, int value) =>
	void tdata_setInt(TabularData* this,int row, int col, int value);

/** Set the float at row <i>row</i> col <i>col</i> to <i>value</i> **/
void TabularData::setFloat(int row, int col, float value) =>
	void tdata_setFloat(TabularData* this,int row, int col, float value);

/** Set the char at row <i>row</i> col <i>col</i> to <i>value</i> **/
void TabularData::setChar(int row, int col, char value) =>
	void tdata_setChar(TabularData* this,int row, int col, char value);

/** Set the String at row <i>row</i> col <i>col</i> to <i>value</i> **/
void TabularData::setString(int row, int col, String value) =>
	void tdata_setString(TabularData* this,int row, int col, char* value);

/** Set the double at row <i>row</i> col <i>col</i> to <i>value</i> **/
void TabularData::setDouble(int row, int col, double value) =>
	void tdata_setDouble(TabularData* this, int row, int col,double value);

/** Returns the int at row <i>row</i> col <i>col</i>. If row < 0 or row >= the
number of rows in this TabularData object an Exception will be thrown. If col < 0 or col >= the
number of columns in this TabularData object or the type of column <i>col</i> 
is not an int type an Exception will be thrown **/

int TabularData::getInt(int row, int col) =>
	int tdata_getInt(TabularData* this, int row, int col);

/** Returns the float at row <i>row</i> col <i>col</i>. If row < 0 or row >= the
number of rows in this TabularData object an Exception will be thrown. If col < 0 or col >= the
number of columns in this TabularData object or the type of column <i>col</i> 
is not a float type an Exception will be thrown **/

float TabularData::getFloat(int row, int col) =>
	float tdata_getFloat(TabularData* this, int row, int col);

/** Returns the char at row <i>row</i> col <i>col</i>. If row < 0 or row >= the
number of rows in this TabularData object an Exception will be thrown. If col < 0 or col >= the
number of columns in this TabularData object or the type of column <i>col</i> 
is not a char type an Exception will be thrown **/
char TabularData::getChar(int row, int col) =>
	char tdata_getChar(TabularData* this, int row, int col);

/** Returns the String at row <i>row</i> col <i>col</i>. If row < 0 or row >= the
number of rows in this TabularData object an Exception will be thrown. If col < 0 or col >= the
number of columns in this TabularData object or the type of column <i>col</i> 
is not a String type an Exception will be thrown **/

String TabularData::getString(int row, int col) =>
	char* tdata_getString(TabularData* this, int row, int col);

/** Returns the double at row <i>row</i> col <i>col</i>. If row < 0 or row >= the
number of rows in this TabularData object an Exception will be thrown. If col < 0 or col >= the
number of columns in this TabularData object or the type of column <i>col</i> 
is not a double type an Exception will be thrown **/

double TabularData::getDouble(int row, int col) =>
	double tdata_getDouble(TabularData* this, int row, int col);

/** Returns the int at row <i>row</i> col <i>col</i>. If row < 0 or row >= the
number of rows in this TabularData object an Exception will be thrown.
If the column <i>col</i> is not present in this TabularData object or 
the type of column <i>col</i> is not an int type an Exception will be thrown **/

int TabularData::getInt(int row, String col) =>
        int tdata_getIntByColumnName(TabularData* this, int row, char* col);

/** Returns the float at row <i>row</i> col <i>col</i>. If row < 0 or row >= the
number of rows in this TabularData object an Exception will be thrown. 
If the column <i>col</i> is not present in this TabularData object or 
the type of column <i>col</i> is not a float type an Exception will be thrown **/

float TabularData::getFloat(int row, String col) =>
        float tdata_getFloatByColumnName(TabularData* this, int row, char* col);

/** Returns the char at row <i>row</i> col <i>col</i>. If row < 0 or row >= the
number of rows in this TabularData object an Exception will be thrown. 
If the column <i>col</i> is not present in this TabularData object or 
the type of column <i>col</i> is not a char type an Exception will be thrown **/

char TabularData::getChar(int row, String col) =>
        char tdata_getCharByColumnName(TabularData* this, int row, char* col);

/** Returns the String at row <i>row</i> col <i>col</i>. If row < 0 or row >= the
number of rows in this TabularData object an Exception will be thrown. 
If the column <i>col</i> is not present in this TabularData object or 
the type of column <i>col</i> is not a String type an Exception will be thrown **/

String TabularData::getString(int row, String col) =>
        char* tdata_getStringByColumnName(TabularData* this, int row, char* col);

/** Returns the double at row <i>row</i> col <i>col</i>. If row < 0 or row >= the
number of rows in this TabularData object an Exception will be thrown. 
If the column <i>col</i> is not present in this TabularData object or 
the type of column <i>col</i> is not a double type an Exception will be thrown **/

double TabularData::getDouble(int row, String col) =>
	double tdata_getDoubleByColumnName(TabularData* this, int row, char* col);

/** Returns the number of rows in this TabularData Object **/
int TabularData::getRows() =>
	int tdata_getRows(TabularData* this);

/** Returns the number of columns in this TabularData Object **/
int TabularData::getColumns() =>
	int tdata_getColumns(TabularData* this);

/** Returns the column name of column <i>col</i> **/
String TabularData::getColumnName(int col) =>
	char* tdata_getColumnName(TabularData* this,int col);

/** Returns the column type of column <i>col</i> **/
String TabularData::getColumnType(int col) =>
	char* tdata_getColumnType(TabularData* this,int col);

/** Returns the column type of column <i>col</i> **/
String TabularData::getColumnType(String col) =>
	char* tdata_getColumnTypeByColumnName(TabularData* this,char* col);
