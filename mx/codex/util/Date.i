Extension: Date

Include: <ctype.h>
Include: <time.h>
Include: "cdate.h"

Interface:

/** Constructs a new Date object. The value of this date object
will reflect the current date and time **/
tracked Date Date::Date() =>
	Date* date_createDefault();

/** Constructs a Date object from a string representation of
a date in the format specified by <i>format</i>. The formatting 
conventions are those of the C function strptime(). All MySQL 
formats are also recognized excepting the %[xX] and %[vV].  **/
tracked Date Date::Date(String format, String date) =>
	Date* date_create(char* format, char* date);

void Date::~Date() => void date_free(Date* this);

/** Returns a string representing this Date object with the format
specified by <i>format</i>. The formatting conventions are those of
the C function strftime() **/
tracked String Date::format(String format) =>
	char* date_format(Date* this, char* format);

/** Calls Date.format(format) **/
tracked String Date::toString(String format) =>
  char* date_format(Date* this, char* format);

/** Returns the number of seconds after the minute **/
int Date::getSeconds() => int date_getSeconds(Date* this);

/** Returns the number of minutes after the hour **/
int Date::getMinutes() =>int date_getMinutes(Date* this);

/** Returns the number of hours past midnight (0-23) **/
int Date::getHour() =>int date_getHour(Date* this);

/** Returns the day of the month (1-31) **/
int Date::getDayOfMonth() =>int date_getMday(Date* this);

/** Returns the number of months since January (0-11) **/
int Date::getMonth() =>int date_getMon(Date* this);

/** Returns the number of years since 1900 **/
int Date::getYear() =>int date_getYear(Date* this);

/** Returns the number of days since Sunday (0-6) **/
int Date::getDayOfWeek() =>int date_getWday(Date* this);

/** Returns the number of days since Jan 1 (0-365) **/
int Date::getDayOfYear() =>int date_getYday(Date* this);

/** Returns the number of seconds since the Epoch 
(00:00:00 UTC, January 1, 1970) **/
int time() => int secondsSinceTheEpoch();

