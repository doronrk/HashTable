#include<stdio.h>
#include<stdlib.h>

struct dic
{
	long total_slots;
	long used_slots;
	long mask;
	struct dic_entry *table;
};

struct dic_entry
{
	struct py_obj *key;
	struct py_obj *value;
};

struct py_obj
{
	int len;
	long hash;
	char *str;
};

// INITIALIZERS //
struct dic *new_dic(long total_slots, long used_slots);
struct dic_entry *new_table(long total_slots);
struct dic_entry *new_entry(struct py_obj *key, struct py_obj *value);
struct py_obj *new_py_obj(char *str, int len);
//////////////////

//	INSERTION	//
void insert_raw(struct dic *d, char *k, int k_length, char *v, int v_length);
void insert_entry(struct dic *d, struct dic_entry *entry_to_insert);
void insert_at_index(struct dic *d, struct dic_entry *entry_to_insert, long index, long col_count);
void handle_collision(struct dic *d, struct dic_entry *entry_to_insert, long index, long col_count);
//////////////////

//	RESIZING	//
void handle_resize(struct dic *d);
void copy_dics(struct dic *d_old, struct dic *d_new);
//////////////////

//	HASH 		//
static long string_hash(struct py_obj *key);
//////////////////

//	PRINTING	//
void print_array(char a[], int len);
void print_dic(struct dic *d);
//////////////////

main()
{
	printf("begin program\n");
	struct dic *d = new_dic(8, 0);
	insert_raw(d, "a", 1, "a", 1);
	insert_raw(d, "b", 1, "b", 1);
	insert_raw(d, "c", 1, "c", 1);
	insert_raw(d, "d", 1, "d", 1);
	insert_raw(d, "e", 1, "e", 1);
	insert_raw(d, "f", 1, "f", 1);
	insert_raw(d, "g", 1, "g", 1);
	insert_raw(d, "z", 1, "z", 1);
	print_dic(d);
	return 0;
}

///////////////////// INTIALIZERS ////////////////////
//////////////////////////////////////////////////////

// returns pointer to a new dictionary and initializes its fields
struct dic *new_dic(long total_slots, long used_slots) {
	//printf("new_dic called\n");
	struct dic *d = malloc(sizeof(struct dic));
	d->total_slots = total_slots;
	d->used_slots = used_slots;
	d->mask = total_slots - 1;
	d->table = new_table(total_slots);
	return d;
}

// returns pointer to a new table and sets the key/value of each entry to NULL
struct dic_entry *new_table(long total_slots) {
	//printf("new_table called\n");
	long i;
	struct dic_entry *new_table = malloc(sizeof(struct dic_entry));
	for (i = 0; i < total_slots; i++) {
		new_table[i] = *new_entry(NULL, NULL);
	}
	return new_table;
}

// returns pointer to a new dic_entry and with given key and value pointer
struct dic_entry *new_entry(struct py_obj *key, struct py_obj *value) {
	//printf("new_entry called\n");
	struct dic_entry *new_entry = malloc(sizeof(struct dic_entry));
	new_entry->key = key;
	new_entry->value = value;
	return new_entry;
}

// returns pointer to a new py_obj
struct py_obj *new_py_obj(char *str, int len) {
	//printf("new_py_obj called\n");
	struct py_obj *po = malloc(sizeof(struct py_obj));
	po->len = len;
	po->hash = -1; // hash of -1 means uncached/yet to be hashed
	po->str = str;
	return po;
}


///////////////////// INSERTION///////////////////////
//////////////////////////////////////////////////////

// creates py_obj key/value entry and calls insert_entry
void insert_raw(struct dic *d, char *k, int k_length, char *v, int v_length)
{
	//printf("insert_raw called\n");
	struct py_obj *key = new_py_obj(k, k_length);
	struct py_obj *value = new_py_obj(v, v_length);
	struct dic_entry *entry_to_insert = new_entry(key, value);
	insert_entry(d, entry_to_insert);
}

// inserts dic_entry into table of dic
void insert_entry(struct dic *d, struct dic_entry *entry_to_insert) {
	//printf("insert_entry called\n");
	long index = string_hash(entry_to_insert->key)&d->mask;
	insert_at_index(d, entry_to_insert, index, 0);
	handle_resize(d);
}

// checks status of entry at given index, sends collisions to handle_collision
// increments # of used_slots if appropriate
void insert_at_index(struct dic *d, struct dic_entry *entry_to_insert, long index, long col_count)
{
	//printf("insert_at_index called\n");
	if (d->table[index].key == NULL) {
		d->table[index] = *entry_to_insert;
		d->used_slots++;
	}
	else if (d->table[index].key->str == entry_to_insert->key->str) {
		d->table[index] = *entry_to_insert;
	}
	else {
		handle_collision(d, entry_to_insert, index, col_count + 1);
	}
}

void handle_collision(struct dic *d, struct dic_entry *entry_to_insert, long index, long col_count) {
	//printf("handle_collision called\n");
	int incr, new_index;
	incr = col_count*col_count;
	//incr = 1;
	new_index = (index + incr);
	//index += incr;  ???
	insert_at_index(d, entry_to_insert, new_index, col_count);
}

// determines whether used slots exceeds 2/3 of total slots
// determines resize ratio
// creates new dictionary and copies table using copy_dics
// rewrites old dictionary pointers with new dictionary pointers
// frees new dictionary 
void handle_resize(struct dic *d) {
	//printf("handle_resize called\n");
	long maxsize = (d->total_slots * 2)/3;
	if (d->used_slots > maxsize) {
		long new_min, new_total_slots; // bad to initalize these variables inside conditional???
		new_total_slots = d->total_slots;
		if (d->used_slots < 50000) {
			new_min = 4*d->used_slots; // bad to initalize these variables inside conditional???
		}
		else {
			new_min = 2*d->used_slots;
		}
		while (new_total_slots < new_min) {
			new_total_slots = new_total_slots << 1;
		}
		struct dic *dic_new = new_dic(new_total_slots, d->used_slots);
		copy_dics(d, dic_new);
		d->total_slots = new_total_slots;
		d->mask = new_total_slots - 1;
		d->table = dic_new->table;
		//free(d_new); // is this doing what I think its doing??
	}
}

// copies non-null entries from old dic to new dic
void copy_dics(struct dic *d_old, struct dic *d_new) {
	//printf("copy_dics called\n");
	long i;
	for (i = 0; i < d_old->total_slots; i++) {
		struct dic_entry de = d_old->table[i];
		if (de.key != NULL) {
			insert_entry(d_new, &de);
		}
	}
}

// why static?
static long string_hash(struct py_obj *key) {
	//printf("string_hash called\n");
	if (key->hash != -1) {
		return key->hash;
	}
	int len;
	long x;
	unsigned char *p;
	p = key->str;
	x = *p << 7;
	len = key->len;
	while (--len >= 0){
		x = (1000003*x) ^ *p++;
	}
	x ^= len;
	if (x == -1) {
		x = -2;
	}
	key->hash = x;
	return x;
}
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////





///////////////////// PRINTERS ///////////////////////
//////////////////////////////////////////////////////
void print_array(char a[], int len) {
	// printf("print_array called\n");
	int i;
	for (i = 0; i < len; i++) {
		printf("%c", a[i]);
	}
}

void print_dic(struct dic *d) {
	//printf("print_dic called\n");
	int i;
	for (i = 0; i < d->total_slots; i++)
	{
		struct dic_entry de = d->table[i];
		if (de.key == NULL) {
			printf("empty index at %d\n",i);
		}
		else {
			printf("{");
			print_array(de.key->str, de.key->len);
			printf(": ");
			print_array(de.value->str, de.value->len);
			printf("}\n");
		}
	}
	printf("\n");
	printf("total_slots = %ld\n", d->total_slots);
	printf("used_slots = %ld\n", d->used_slots);
	printf("mask = %ld\n", d->mask);
}
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
