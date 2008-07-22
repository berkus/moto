Extension: HashSet

Include: "hashset.h"
Include: "enumeration.h"

Interface:

tracked HashSet HashSet::HashSet() =>
	HashSet *hset_createDefault();

void HashSet::~HashSet() =>
	void hset_free(HashSet *set);

void HashSet::add(Object obj) =>
	void hset_add(HashSet *set, void *obj);

void HashSet::clear() =>
	void hset_clear(HashSet *set);

boolean HashSet::contains(Object obj) =>
	int hset_contains(HashSet *set, void *obj);

tracked Enumeration HashSet::elements() =>
	Enumeration *hset_elements(HashSet *set);

Object HashSet::get(Object obj) =>
	void *hset_get(HashSet *set, void *obj);

boolean HashSet::remove(Object obj) =>
	int hset_remove(HashSet *set, void *obj);

int HashSet::size() =>
	int hset_size(HashSet *set);

