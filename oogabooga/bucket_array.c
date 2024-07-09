


typedef struct Bucket_Array {

	

	u64 _block_size;
	u64 _bucket_count;
} Bucket_Array;

typedef struct Bucket_Array_Free_Node {
	struct Bucket_Array_Free_Node *next;
} Bucket_Array_Free_Node;
typedef struct Bucket_Array_Bucket {
	Bucket_Array_Free_Node *first_free;
	void *data;
	Bucket *next;
} Bucket_Array_Bucket;


Bucket_Array make_bucket_array(u64 block_size, u64 bucket_count) {
	Bucket_Array ba;
	
	ba._block_size = block_size;
	ba._bucket_count = bucket_count;
}