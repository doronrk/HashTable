#include<stdio.h>
#include<stdlib.h>
struct entry {
	char *key;
	char *value;
};
struct dict {
	long total_slots;
	long used_slots;
	struct entry *table;
};

// Utility
int len_of(char *string);
void print_string(char *string);
void print_dict(struct dict *d);
static long string_hash(char *string);
int string_eq(char *s1, char *s2);

// Initializer
struct entry *new_table(long total_slots);
struct dict *new_dict(long total_slots, long used_slots);

//Insert
void insert_entry(struct dict *d, char *key, char *value);
void insert_helper(struct dict *d, char *key, char *value, long index, long col_count);

//Get
char *get(struct dict *d, char *key);
char *get_helper(struct dict *d, char *key, long index, long col_count);
char *get_index(struct dict *d, long index);

//Resize
void handle_resize(struct dict *d);

int main() {
	return 0;
}

char *get_index(struct dict *d, long index) {
	return d->table[index].value;
}

//Resize
void handle_resize(struct dict *d) {
	long maxsize = (d->total_slots * 2)/3;
	if (d->used_slots > maxsize) {
		long new_min, new_total_slots, i;
		new_total_slots = d->total_slots;
		if (d->used_slots < 50000) {
			new_min = 4 * (d->used_slots);
		}
		else {
			new_min = 2 * (d->used_slots);
		}
		while (new_total_slots < new_min) {
			new_total_slots = new_total_slots << 1;
		}
		struct dict *dict_new = new_dict(new_total_slots, 0);
		for (i = 0; i < d->total_slots; i++) {
			if (d->table[i].key != NULL) {
				insert_entry(dict_new, d->table[i].key, d->table[i].value);
			}
		}
		d->total_slots = dict_new->total_slots;
		d->used_slots = dict_new->used_slots;
		free(d->table);
		d->table = dict_new->table;
		free(dict_new);
	}
}

//Get
char *get(struct dict *d, char *key) {
	long hash = string_hash(key);
	return get_helper(d, key, hash, 0);
}

char *get_helper(struct dict *d, char *key, long index, long col_count) {
	index = index & (d->total_slots - 1);
	if (d->table[index].key == NULL) {
		return NULL;
	}
	else
		if (string_eq(key, d->table[index].key)) {
			return d->table[index].value;
		}
		else {
			col_count++;
			return get_helper(d, key, index + (col_count * col_count), col_count);
		}
}

// Insert
void insert_entry(struct dict *d, char *key, char *value) {
	long hash = string_hash(key);
	insert_helper(d, key, value, hash, 0);
	handle_resize(d);
}

void insert_helper(struct dict *d, char *key, char *value, long index, long col_count) {
	index = index & (d->total_slots - 1);
	if (d->table[index].key == NULL) {
		d->table[index].key = key;
		d->table[index].value = value;
		d->used_slots++;
	}
	else 
		if (string_eq(key, d->table[index].key)) {
			d->table[index].value = value;
		} 
		else {
			col_count++;
			insert_helper(d, key, value, index + (col_count * col_count), col_count); 
		}
}

// Initializer
struct dict *new_dict(long total_slots, long used_slots) {
	struct dict *d = malloc(sizeof(struct dict));
	d->total_slots = total_slots;
	d->used_slots = used_slots;
	d->table = new_table(total_slots);
	return d;
}

struct entry *new_table(long total_slots) {
	struct entry *table = malloc(total_slots*sizeof(struct entry));
	long i;
	for (i = 0; i < total_slots; i++) {
		table[i].key = NULL;
		table[i].value = NULL;
	}
	return table;
}

// Utility
int len_of(char *string) {
	int len = 0;
	while (string[len] != NULL) {
		len++;
	}
	return len;
}

void print_string(char *string){
	int i = 0;
	while (string[i] != NULL) {
		printf("%c", string[i]);
		i++;
	}
}

void print_dict(struct dict *d) {
	long i;
	for (i = 0; i < d->total_slots; i++) {
		if (d->table[i].key != NULL) {
			print_string(d->table[i].key);
			printf(": ");
			print_string(d->table[i].value);
			printf("\n");
		}
		else {
			printf("empty slot at index %ld\n", i);
		}
	}
	printf("%ld total_slots\n", d->total_slots);
	printf("%ld used_slots\n\n", d->used_slots);
}

static long string_hash(char *string) {
	long x, len;
	x = *string << 7;
	len = len_of(string);
	while (--len >= 0) {
		x = (1000003*x) ^ *string++;
	}
	x ^= len;
	return x;
}

int string_eq(char *s1, char *s2) {
	int i = 0;
	if (len_of(s1) != len_of(s2)) {
		return 0;
	}
	while (s1[i] != NULL) {
		if (s1[i] != s2[i]) {
			return 0;
		}
		i++;
	}
	return 1;
}
