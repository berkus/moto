Extension: Sharedmem

Include: "sharedmem.h"

Interface:

int 	getTotalMemory() => int shared_getTotalMemory();
int 	getFreeMemory() => int shared_getFreeMemory();
int 	getANodes() => int shared_getANodes();
int 	getFNodes() => int shared_getFNodes();
