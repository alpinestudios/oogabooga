
// Very naive implementation but it should be very cache efficient so it's alright
// for non-excessive use for now.

/*

	Example Usage:
	
	
	// Make a table with key type 'string' and value type 'int', allocated on the heap
	Hash_Table table = make_hash_table(string, int, get_heap_allocator());
	
	// Set key "Key string" to integer value 69. This returns whether or not key was newly added.
	string key = STR("Key string");
	bool newly_added = hash_table_set(&table, key, 69);
	
	// Find value associated with given key. Returns pointer to that value.
	string other_key = STR("Some other key");
	int* value = hash_table_find(&table, other_key);
	
	if (value) {
		// Pointer is OK, item with key exists
	} else {
		// Pointer is null, item with key does NOT exist
	}
	
	// Same as hash_table_find() != NULL
	string another_key = STR("Another key");
	if (hash_table_contains(&table, another_key)) {
		
	}
	
	// Reset all entries (but keep allocated memory)
	hash_table_reset(&table);
	
	// Free allocated entries in hash table
	hash_table_destroy(&table);
	
	
	Limitations:
		- Key can only be a base type or pointer
		- Key and value passed to the following function needs to be lvalues (we need to be able to take their addresses with '&'):
			- hash_table_add
			- hash_table_find
			- hash_table_contains
			- hash_table_set
			
			Example:
			
			hash_table_set(&table, my_key+5, my_value+3); // ERROR
			
			int key = my_key+5;
			int value = my_value+3;
			hash_table_set(&table, key, value); // OK
			

*/

typedef struct Hash_Table Hash_Table;

// API:
#define make_hash_table_reserve(Key_Type, Value_Type, capacity_count, allocator) \
	make_hash_table_reserve(sizeof(Key_Type), sizeof(Value_Type), capacity_count, allocator)
	
#define make_hash_table(Key_Type, Value_Type, allocator) \
	make_hash_table_raw(sizeof(Key_Type), sizeof(Value_Type), allocator)

#define hash_table_add(table_ptr, key, value) \
	hash_table_add_raw((table_ptr), get_hash(key), &(key), &(value), sizeof(key), sizeof(value))

#define hash_table_find(table_ptr, key) \
	hash_table_find_raw((table_ptr), get_hash(key))
	
#define hash_table_contains(table_ptr, key) \
	hash_table_contains_raw((table_ptr), get_hash(key))
	
#define hash_table_set(table_ptr, key, value) \
	hash_table_set_raw((table_ptr), get_hash(key), &key, &value, sizeof(key), sizeof(value))

void hash_table_reserve(Hash_Table *t, u64 required_count);


typedef struct Hash_Table {
	
	// Each entry is hash-key-value
	// Hash is sizeof(u64) bytes, key is _key_size bytes and value is _value_size bytes
	void *entries; 
	
	u64 count; // Number of valid entries
	u64 capacity_count; // Number of allocated entries
	
	u64 _key_size;
	u64 _value_size;
	
	Allocator allocator;
} Hash_Table;

Hash_Table make_hash_table_reserve_raw(u64 key_size, u64 value_size, u64 capacity_count, Allocator allocator) {

	capacity_count = min(capacity_count, 8);

	Hash_Table t = ZERO(Hash_Table);
	
	t._key_size = key_size;
	t._value_size = value_size;
	t.allocator = allocator;
	
	u64 entry_size = value_size+sizeof(u64);
	t.entries = alloc(t.allocator, entry_size*capacity_count);
	memset(t.entries, 0, entry_size*capacity_count);
	t.capacity_count = capacity_count;
	
	return t;
}
inline Hash_Table make_hash_table_raw(u64 key_size, u64 value_size, Allocator allocator) {
	return make_hash_table_reserve_raw(key_size, value_size, 128, allocator);
}

void hash_table_reset(Hash_Table *t) {
	t->count = 0;
}
void hash_table_destroy(Hash_Table *t) {
	dealloc(t->allocator, t->entries);
	
	t->entries = 0;
	t->count = 0;
	t->capacity_count = 0;
}

void hash_table_reserve(Hash_Table *t, u64 required_count) {
	u64 entry_size = t->_value_size+sizeof(u64);
	
	u64 required_size = required_count*entry_size;
	
	u64 current_size = t->capacity_count*entry_size;
	
	if (current_size >= required_size) return;
	
	u64 new_count = get_next_power_of_two(required_count);
	u64 new_size = new_count*entry_size;
	
	void *new_entries = alloc(t->allocator, new_size);
	memcpy(new_entries, t->entries, current_size);
	
	dealloc(t->allocator, t->entries);
	
	t->entries = new_entries;
	t->capacity_count = new_count;
}

// This can add multiple entries of same hash, beware!
void hash_table_add_raw(Hash_Table *t, u64 hash, void *k, void *v, u64 key_size, u64 value_size) {

	assert(t->_key_size == key_size, "Key type size does not match hash table initted key type size");
	assert(t->_value_size == value_size, "Value type size does not match hash table initted value type size");

	hash_table_reserve(t, t->count+1);
	
	u64 entry_size = t->_value_size+sizeof(u64);
	
	u64 index = entry_size*t->count;
	t->count += 1;
	
	u64 hash_offset = 0;
	u64 value_offset = hash_offset + sizeof(u64);
	
	memcpy((u8*)t->entries+index+hash_offset,  &hash, sizeof(u64));
	memcpy((u8*)t->entries+index+value_offset, v,     value_size);
}

void *hash_table_find_raw(Hash_Table *t, u64 hash) {

	// #Speed #Incomplete
	// Do quadratic probe 'triangular numbers'

	u64 entry_size = t->_value_size+sizeof(u64);
	u64 hash_offset = 0;
	u64 value_offset = hash_offset + sizeof(u64);
	
	for (u64 i = 0; i < t->count; i += 1) {
		u64 existing_hash = *(u64*)((u8*)t->entries+i*entry_size+hash_offset);
		if (existing_hash == hash) {
			void *value = ((u8*)t->entries+i*entry_size+value_offset);
			return value;
		}
	}
	return 0;
}

bool hash_table_contains_raw(Hash_Table *t, u64 hash) {
	return hash_table_find_raw(t, hash) != 0;
}

// Returns true if key was newly added or false if it already existed
bool hash_table_set_raw(Hash_Table *t, u64 hash, void *k, void *v, u64 key_size, u64 value_size) {
	bool newly_added = true;
	
	if (hash_table_contains_raw(t, hash)) newly_added = false;
	
	if (newly_added) {
		hash_table_add_raw(t, hash, k, v, key_size, value_size);
	} else {
		memcpy(hash_table_find_raw(t, hash), v, value_size);
	}
	
	return newly_added;
}