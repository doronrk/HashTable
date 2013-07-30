from cffi import FFI
import os, random, string, time
random.seed('hello')
# $ gcc -Wall hash_table.c -o hash_table.so && python test_hash_table.py

ffi = FFI()
ffi.cdef("""
  struct entry {
		char *key;
		char *value;
	};

	struct dict {
		long total_slots;
		long used_slots;
		struct entry *table;
	};
	int len_of(char *string);

	void print_string(char *string);
	void print_dict(struct dict *d);
	long string_hash(char *string);
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

""")

ht = ffi.dlopen('hash_table.so')

py_dict = {}

def rand_str():
	length = random.randint(1,10)
	s = ''.join(random.choice(string.ascii_uppercase + string.digits) for x in range(length))
	return s

def build_dict(size):
	n = 0
	d = ht.new_dict(8,0)
	for i in range(size):
		key = rand_str()
		value = rand_str()
		py_dict[key] = value
		ht.insert_entry(d, key, value)
	return d

def check_values(d):
	for key in py_dict:
		value = ht.get(d, key)
		if not value:
			print 'key not found', key
			print 'value should be', py_dict[key], '\n'
		else:
			if not ht.string_eq(value, py_dict[key]):
				return False
	return True

d = build_dict(9999)	
print '\n'

ht.print_dict(d)

if check_values(d):	
	print "Test passed"
else:
	print "Test failed"




