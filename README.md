# SimpleMap
A simple 32-bit hash map implementation

Just include HashMap.h and HashMap.c in your project and use the API as described below.
At the moment, the one and only bucketsize is 13 (no. collisions before doubling the storage capability). 
The library is not thread safe.

# Windows
To run the test on Windows, simply clone and open the project with Visual Studio 2017.

# Linux
Quickstart: `git clone https://github.com/Simmesson/SimpleMap && cd SimpleMap/SimpleMap && make && ./SimpleMap-Test-Binary`


# The API

`void			    hashmap_benchmark();` - Performs a test in which it is storing, retrieving and then deleting 1 000 000 keys. At the time of writing, this process takes around one second with an i7 2600k @ 4.4 GHz and with ram clocked at 1600 MHz.

`hash_map_t		*hashmap_create(uint32_t size, int bucketSize);` - Creates a hash_map_t object and returns the pointer. Needs to be freed with hashmap_free when discarded.

`int				  hashmap_insert(hash_map_t *hm, const char *key, void *data);` - Inserts data at index derived from key. Doubles the storage surface if there are more than 13 collisions in chosen bucket.

`hash_pair_t	*hashmap_find(hash_map_t *hm, const char *key);` - Finds and returns a pointer to the data pair at the index derived from the given key, or NULL.

`int				  hashmap_delete(hash_map_t *hm, const char *key, int flags);` - Deletes the given key, returns non-zero if success.

`void			    hashmap_free(hash_map_t *hm);` - To be called upon discarding the hashmap. Takes a pointer to your hashmap.


# To-do
* Variable prime bucketsize
* Optimize algorithms
