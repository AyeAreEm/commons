# Commons
Commons is a header only library for common data structures, algorithms and some common modern niceties<br>
NOTE: none of these are guaranteed to be safe for production and is made for my use case

## Added "Keywords"
- let (type inference using __auto_type)
- defer (defer a function for cleaning up a variable)
- map_iter (for in loop for maps)

## Data Structures
- Dynamic Arrays (dyn)
- Allocated Strings (string)
- HashTables (map)
- Tuples (tuple)
- Options (option)
- Results (result)

## Algorithms
- Djb2 (hashing function)

## Utility Functions
- str_equal
- num_equal
NOTE: these utility functions might seem arbitary but they are for convenience with some of the data structs and algs, for example, map needs to compare generic types so in the `map_init` function, it takes a function that compares keys of type `T`

## Added Keywords Details
### Let
`let` is a macro to `__auto_type`. if you are using c23, auto should now be the type inference keyword

### Defer
This defer isn't to defer anything but specifically to defer free/deallocation<br>
To use:
```c
defer(string_deinit)
let word = string_from("hello world");
```
Once `word` goes out of scope, `string_deinit` is called to free the string. This uses `__attribute__((__cleanup__()))` which is provided by GCC and Clang. unsure if it's in MSVC or other C compilers

### Map Iter
`map_iter` is a for in loop over active entries in a map<br>
it is a macro that takes in a new variable name `entry`, new variable name `iter`, the map `map` and a codeblock.
To use:
```
defer(map_deinit_char*_int)
let table = map_init_char*_int(hash_djb2, num_equal_int);
map_insert_int_int(&table, "john", 18);

map_iter(entry, i, table, {
    printf("index: %zu, key: %d, value: %d\n", i, entry.key, entry.value);
})
```
This would print once with something akin to `index: 10, key: john, value: 18`<br>
NOTE: entry is not a reference to the data so mutating it here won't affect the data stored in the map

## Data Structure Details
### Dynamic Array
Generic dynamic array with a growth factor of 2<br>
Includes functions such as push, pop, clone, at, clear, remove (at index)<br>
To use:
```c
struct_option(char*, charptr);  // arg1: type, arg2: name of type
struct_result(char*, charptr);
// the above is needed because gen_dyn doesn't auto generate the needed option and result types in case you've already defined them previously
gen_dyn(char*, charptr);

// for convenience, there is a gen_dyn_with_deps that auto generates all dependencies
gen_dyn_with_deps(int, int);
```

### String
Heap allocated string that is just a dynamic array of `char`<br>
Includes functions such as contains, from, compare, lower, upper and functions from dynamic array

### Map
Generic hash table that uses open addressing.<br>
Allocates 97 elements to start with because according to this [website](https://planetmath.org/goodhashtableprimes) it works well, at least to my understanding<br>
When using `map_insert` and `map_get`, you must provide your own hashing function but there is an implementation for the djb2 hashing algorithmn in the library named `hash_djb2`
There's a macro `map_iter` to help iterating through the active elements.<br>
To use:
```c
gen_map(char*, int, charptr_int); // char *key, int value

// The types that are made from this example are
map_entry_charptr_int
option_map_entry_charptr_int
result_map_entry_charptr_int
dyn_map_entry_charptr_int
map_charptr_int
```
NOTE: this auto generates a dependency `map_entry_##typename` to store the key value pair. it should never conflict since map_entry is only used for maps

### Tuple
Generic tuple that only contains two items, .one and .two

### Option
Generic option that contains `bool` and `T`, .ok to see if the value is safe to use<br>
if .ok, can use .value

### Result
Generic result that contains `Err` and `T`, .err to see the error
if .err == ERR_NONE, .value is safe to use

## Algorithm Details
### Djb2
`hash_djb2` accepts a string and returns a hashed value. This works with maps but you are able to implement other hashing functions to provide to map related functions<br>
I don't know how this algorithm works nor how safe it is, use at your own peril
