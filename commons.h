#pragma once
#ifndef COMMONS_H
#define COMMONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define let __auto_type // type inference

// defer cleanup for allocated type
// example:
// defer(dyn_deinit_int) let nums = dyn_init_int();
// or
/*
    defer(dyn_deinit_int)
    let nums = dyn_init_int();
*/
#define defer(func_name) __attribute__((__cleanup__(func_name)))

#define struct_tuple(T, K, typename)\
typedef struct {\
    T one;\
    K two;\
} tuple_##typename;

typedef enum {
    ERR_NONE = 0,
    ERR_INDEX_OUT_OF_BOUNDS = 1,
} Err;

// generic result type
// if .err == ERR_NONE, can use .value
// otherwise don't use .value
#define struct_result(T, typename)\
typedef struct {\
    Err err;\
    T value;\
} result_##typename;

// generic option type.
// if .ok, can use .value
// otherwise don't use .value
#define struct_option(T, typename)\
typedef struct {\
    bool ok;\
    T value;\
} option_##typename;



/* ################# DYNAMIC ARRAY ################# */



// generic dynamic array type
// .buf is a pointer to the first T elem
// .len is the number of active elems where .len - 1 is the last elem
// .cap is the number of elems allocated
#define gen_dyn(T, typename)\
typedef struct {\
    T* buf;\
    size_t len;\
    size_t cap;\
} dyn_##typename;\
/*
    initalise dynamic array, allocates .buf
    cap is set to 32 by default

    NOTE: call dyn_deinit to free and set .len = 0, .cap = 0

    returns the dynamic array
*/\
dyn_##typename dyn_init_##typename() {\
    size_t cap = 32;\
    T* buf = (T*)calloc(sizeof(T), cap);\
    return (dyn_##typename){.buf = buf, .len = 0, .cap = cap};\
}\
/*
    same as dyn_init but you provide a capacity to allocate to start
*/\
dyn_##typename dyn_init_with_cap_##typename(size_t cap) {\
    T* buf = (T*)calloc(sizeof(T), cap);\
    return (dyn_##typename){.buf = buf, .len = 0, .cap = cap};\
}\
/*
    allocates a new .buf

    NOTE: call dyn_deinit to free and sets .len = 0, .cap = 0

    return cloned dynamic array;
*/\
dyn_##typename dyn_clone_##typename(dyn_##typename *self) {\
    T* buf = (T*)malloc(sizeof(T) * self->cap);\
    memcpy(buf, self->buf, self->len);\
    return (dyn_##typename){\
        .buf = buf,\
        .len = self->len,\
        .cap = self->cap,\
    };\
}\
/*
    resizes dynamic array, growth factor of 2
    NOTE: you usually won't have to use this function yourself
*/\
void dyn_resize_##typename(dyn_##typename *self) {\
    self->cap *= 2;\
    self->buf = (T*)realloc(self->buf, self->cap);\
}\
/*
    resizes dynamic array but with a specified addition
    i.e. cap * 2 + addition
*/\
void dyn_resize_with_add_##typename(dyn_##typename *self, size_t addition) {\
    self->cap = self->cap * 2 + addition;\
    self->buf = (T*)realloc(self->buf, self->cap);\
}\
/* 
    returns the elem at index as an option
    if index is out of bounds, returns .ok = false
*/\
option_##typename dyn_at_##typename(dyn_##typename *self, size_t index) {\
    if (index >= self->len) {\
        return (option_##typename){.ok = false, .value = 0};\
    }\
    return (option_##typename){.ok = true, .value = self->buf[index]};\
}\
void dyn_push_##typename(dyn_##typename *self, T elem) {\
    if (self->len + 1 >= self->cap) {\
        dyn_resize_##typename(self);\
    }\
    self->buf[self->len] = elem;\
    self->len += 1;\
}\
/* return pops out top element as an option */\
option_##typename dyn_pop_##typename(dyn_##typename *self) {\
    option_##typename elem = dyn_at_##typename(self, self->len - 1);\
    if (elem.ok) {\
        self->len -= 1;\
    }\
    return elem;\
}\
/* 
    removes element at index 
    returns said element as an option
*/\
option_##typename dyn_remove_##typename(dyn_##typename *self, size_t index) {\
    option_##typename elem = dyn_at_##typename(self, index);\
    if (!elem.ok) {\
        return elem;\
    }\
    for (; index + 1 < self->len; index++) {\
        self->buf[index - 1] = self->buf[index];\
    }\
    self->len -= 1;\
    return elem;\
}\
/*
    replace element at index with new element
    returns result with either ERR_INDEX_OUT_OF_BOUNDS or ERR_NONE, nothing of use will be used in the .value
*/\
result_##typename dyn_replace_##typename(dyn_##typename *self, size_t index, T elem) {\
    if (index >= self->len) {\
        return (result_##typename){.err = ERR_INDEX_OUT_OF_BOUNDS};\
    }\
    self->buf[index] = elem;\
    return (result_##typename){.err = ERR_NONE};\
}\
void dyn_clear_##typename(dyn_##typename *self) {\
    self->len = 0;\
}\
void dyn_deinit_##typename(dyn_##typename *self) {\
    free(self->buf);\
    self->len = 0;\
    self->cap = 0;\
}\

// generates dynamic array with it's dependencies such as option and result type
#define gen_dyn_with_deps(T, typename)\
struct_option(T, typename)\
struct_result(T, typename)\
gen_dyn(T, typename)



/* ################# STRING ################# */



gen_dyn_with_deps(char, char);
struct_tuple(bool, size_t, bool_size_t);

// heap allocated string with null terminator
typedef struct {
    dyn_char buf;
} string;
/*
    initalise a string that is just a dynamic array of chars

    NOTE: call string_deinit to free and sets .buf.len = 0, .buf.cap = 0

    returns a string
*/
string string_init() {
    return (string){ .buf = dyn_init_char() };
}
/*
    resize the string with a growth factor of 2
    NOTE: you usually won't have to call this yourself
*/
void string_resize(string *self) {
    dyn_resize_char(&self->buf);
}
/*
    allocates a new string .buf

    NOTE: call string_deinit to free and sets .len = 0, .cap = 0

    returns newly allocated string
*/
string string_clone(string *self) {
    string new_str = (string){ .buf = dyn_clone_char(&self->buf) };
    if (new_str.buf.len + 1 >= new_str.buf.cap) {
        string_resize(self);
    }
    new_str.buf.buf[new_str.buf.len] = 0;
    return new_str;
}
/* returns the string buf and can be used like a cstring since it also contains a null terminator */
const char* string_get(string self) {
    return self.buf.buf;
}
/* 
    returns the elem at index as an option
    if index is out of bounds, returns .ok = false
*/
option_char string_at(string* self, size_t index) {
    return dyn_at_char(&self->buf, index);
}
/* 
    string_push_char pushes the element but also pushes 0 to ensure it is null terminated
    self->buf.len -= 1; because dyn_push increases the len but we don't count 0 as part of the len
*/
void string_push_char(string* self, char elem) {
    dyn_push_char(&self->buf, elem);
    dyn_push_char(&self->buf, 0);
    self->buf.len -= 1;
}
/* 
    string_push_cstr pushes a c string that needs to be null terminated
    also ensures that the ending string is null terminated
*/
void string_push_cstr(string* self, const char* content) {
    for (size_t i = 0; i < strlen(content); i++) {
        if (self->buf.len + 1 >= self->buf.cap) {
            string_resize(self);
        }
        string_push_char(self, content[i]);
    }
    if (self->buf.len + 1 >= self->buf.cap) {
        string_resize(self);
    }
    string_push_char(self, 0);
    self->buf.len -= 1;
}
/* 
    this just calls string_push_cstr because it is guaranteed by the string functions to be null terminated
*/
void string_push_string(string *self, string content) {
    string_push_cstr(self, content.buf.buf);
}
option_char string_pop(string *self) {
    option_char elem = dyn_pop_char(&self->buf);
    self->buf.buf[self->buf.len] = 0;
    return elem;
}
/*
    removes a character at index from the string
*/
option_char string_remove(string *self, size_t index) {
    option_char elem = dyn_remove_char(&self->buf, index);
    self->buf.buf[self->buf.len] = 0;
    return elem;
}
/*
    same as doing self->buf.buf[index] = elem but with bounds checking
    returns ERR_INDEX_OUT_OF_BOUNDS if index is well... out of bounds
*/
result_char string_replace(string *self, size_t index, char elem) {
    return dyn_replace_char(&self->buf, index, elem);
}
/*
    if string does contain char, returns {true, index}
    else returns {false, 0}
*/
tuple_bool_size_t string_contains_char(string self, char pattern) {
    for (size_t i = 0; i < self.buf.len; i++) {
        if (self.buf.buf[i] == pattern) {
            return (tuple_bool_size_t){true, i};
        }
    }

    return (tuple_bool_size_t){false, 0};
}
/*
    if string does contain cstr, returns {true, index} where index is the start of the pattern
    else returns {false, 0}
*/
tuple_bool_size_t string_contains_cstr(string self, const char *pattern) {
    size_t head = 0;
    size_t index = 0;
    size_t pattern_len = strlen(pattern);

    if (self.buf.len < pattern_len) {
        return (tuple_bool_size_t){false, 0};
    }

    for (size_t i = 0; i < self.buf.len; i++) {
        if (head == pattern_len) {
            return (tuple_bool_size_t){true, index};
        }

        if (self.buf.buf[i] == pattern[head]) {
            head += 1;
        } else {
            head = 0;
            if (i == 0) {
                index = 1;
            } else {
                index = i;
            }
        }
    }

    if (head == pattern_len) {
        return (tuple_bool_size_t){true, index};
    }

    return (tuple_bool_size_t){false, 0};
}
/*
    if string does contain substring, returns {true, index} where index is the start of the pattern
    else returns {false, 0}
*/
tuple_bool_size_t string_contains_string(string self, string pattern) {
    return string_contains_cstr(self, pattern.buf.buf);
}
/*
    create string type from cstr (must be null terminated)
    i.e. string word = string_from("hello world");

    NOTE: call string_deinit to free
*/
string string_from(const char* content) {
    string str = string_init();
    string_push_cstr(&str, content);
    return str;
}
/*
    compares the whole string to the comparate (must be null terminated)
    returns true if they're the same, false if not
*/
bool string_compare_cstr(string self, const char* comparate) {
    size_t comparate_len = strlen(comparate);
    if (self.buf.len != comparate_len) {
        return false;
    }

    for (size_t i = 0; i < comparate_len; i++) {
        if (self.buf.buf[i] != comparate[i]) {
            return false;
        }
    }

    return true;
}
/*
    compares the whole string to the comparate (must be null terminated)
    returns true if they're the same, false if not
*/
bool string_compare_string(string self, string comparate) {
    return string_compare_cstr(self, comparate.buf.buf);
}
/*
    makes the whole string lowercase
*/
void string_lower(string *self) {
    for (size_t i = 0; i < self->buf.len; i++) {
        self->buf.buf[i] = tolower(self->buf.buf[i]);
    }
}
/*
    makes the whole string uppercase
*/
void string_upper(string *self) {
    for (size_t i = 0; i < self->buf.len; i++) {
        self->buf.buf[i] = toupper(self->buf.buf[i]);
    }
}
void string_clear(string *self) {
    dyn_clear_char(&self->buf);
}
void string_deinit(string *self) {
    dyn_deinit_char(&self->buf);
}



/* ################# MAP ################# */



#define gen_map(K, V, typenames)\
typedef struct {\
    K key;\
    V value;\
    bool active;\
} map_entry_##typenames;\
gen_dyn_with_deps(map_entry_##typenames, map_entry_##typenames);\
typedef struct {\
    dyn_map_entry_##typenames entries;\
    size_t active_count;\
    size_t (*hash)(K);\
    bool (*key_compare)(K, K);\
} map_##typenames;\
/*
    allocates a dynamic array with a starting capacity of 97. check readme for why specifically 97
    NOTE: call map_deinit to free after use
*/\
map_##typenames map_init_##typenames(size_t hash(K), bool key_compare(K, K)) {\
    return (map_##typenames){\
        .entries = dyn_init_with_cap_map_entry_##typenames(97),\
        .active_count = 0,\
        .hash = hash,\
        .key_compare = key_compare,\
    };\
}\
void map_deinit_##typenames(map_##typenames *self) {\
    dyn_deinit_map_entry_##typenames(&self->entries);\
}\
/*
    map_resize creates a new map and inserts the previous entries to the new one

    NOTE: old map is freed during this function, careful of dangling pointers
*/\
bool map_insert_##typenames(map_##typenames *self, K key, V value);\
void map_resize_##typenames(map_##typenames *self) {\
    map_##typenames new_map = {\
        .entries = dyn_init_with_cap_map_entry_##typenames(self->entries.cap * 2 + 1),\
        .active_count = 0,\
    };\
    for (size_t i = 0; i < self->entries.cap; i++) {\
        if (self->entries.buf[i].active) {\
            map_insert_##typenames(&new_map, self->entries.buf[i].key, self->entries.buf[i].value);\
        }\
    }\
\
    map_deinit_##typenames(self);\
    *self = new_map;\
}\
/*
    returns false if key isn't unqiue
*/\
bool map_insert_##typenames(map_##typenames *self, K key, V value) {\
    if (self->active_count >= self->entries.cap) {\
        map_resize_##typenames(self);\
    }\
    size_t index = self->hash(key) % self->entries.cap;\
    while (self->entries.buf[index].active) {\
        if (self->key_compare(self->entries.buf[index].key, key)) {\
            return false;\
        }\
        index = (index + 1) % self->entries.cap;\
    }\
    self->entries.buf[index].active = true;\
    self->entries.buf[index].key = key;\
    self->entries.buf[index].value = value;\
    self->active_count += 1;\
    return true;\
}\
/*
    get entry by key
    returns an option to the entry
 */\
option_map_entry_##typenames map_get_##typenames(map_##typenames *self, K key) {\
    size_t index = self->hash(key) % self->entries.cap;\
    for (size_t i = 0; i < self->entries.cap; i++) {\
        if (!self->entries.buf[index].active) {\
            break;\
        }\
        if (self->key_compare(self->entries.buf[index].key, key)) {\
            break;\
        } \
        index = (index + 1) % self->entries.cap;\
    }\
    if (self->entries.buf[index].active && self->key_compare(self->entries.buf[index].key, key)) {\
        return (option_map_entry_##typenames){\
            .ok = true,\
            .value = self->entries.buf[index],\
        };\
    }\
    return (option_map_entry_##typenames){\
        .ok = false,\
    };\
}\
bool map_update_##typenames(map_##typenames *self, K key, V value) {\
    size_t index = self->hash(key) % self->entries.cap;\
    bool found = false;\
    while (self->entries.buf[index].active) {\
        if (self->key_compare(self->entries.buf[index].key, key)) {\
            found = true;\
            break;\
        }\
        index = (index + 1) % self->entries.cap;\
    }\
    if (!found) {\
        return false;\
    }\
\
    self->entries.buf[index].value = value;\
    return true;\
}\
bool map_remove_##typenames(map_##typenames *self, K key) {\
    size_t index = self->hash(key) % self->entries.cap;\
    bool found = false;\
    while (self->entries.buf[index].active) {\
        if (self->key_compare(self->entries.buf[index].key, key)) {\
            found = true;\
            break;\
        }\
        index = (index + 1) % self->entries.cap;\
    }\
    if (!found) {\
        return false;\
    }\
    self->entries.buf[index].active = false;\
    self->active_count -= 1;\
    return true;\
}\

// little macro to iterate over the active entries in a map
#define map_iter(entry, iter, map, codeblock)\
for (size_t iter = 0; iter < map.entries.cap; iter++) {\
    typeof(map.entries.buf[0]) entry = map.entries.buf[iter];\
    if (entry.active) {\
        codeblock\
    }\
}

// a hashing function for string keys
size_t hash_djb2(char* str) {
    size_t hash = 5381;
    for (size_t i = 0; i < strlen(str); i++) {
        if (str[i] == 0) {
            break;
        }
        hash = ((hash << 5) + hash) + str[i];
    }
    return hash;
}

// string equal function that just returns if two strings are the same
// this is in the header for convenience when using a map with char* keys
bool str_equal(char* one, char* two) {
    return strcmp(one, two) == 0;
}

// see if two numbers of the same type are equal
// this is in the header for convenience when using a map with number keys
// might seem dumb but since map needs a function to compare generic types, this is needed
#define gen_num_equal(T, typename)\
bool num_equal_##typename(T one, T two) {\
    return one == two;\
}\

#endif // COMMONS_H
