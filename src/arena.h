// It's not actually an arena but more of a big dynamic array
// that's used for a variety of language constructs (mostly
// "flattened" trees like the Types and AST)
//
// TODO(NeGate): Each of these arenas has a "null" element
// meaning that the first element is reserved and cleared to
// zero.
#pragma once
#include "common.h"

// Let's you forward decl the index
#define decl_arena_index(type, arena) \
typedef int type ## Index;

#define decl_arena(type, arena) \
typedef struct type ## Arena { \
size_t capacity, count; \
type* data; \
} type ## Arena; \
extern type ## Arena arena; \
void init_ ## arena (size_t c); \
void free_ ## arena (); \
type ## Index push_ ## arena (size_t c);

#define impl_arena(type, arena) \
type ## Arena arena; \
void init_ ## arena (size_t c) { \
if (arena.data) { \
arena.count = 1; \
memset(&arena.data[0], 0, sizeof(arena.data[0])); \
} else { \
arena.capacity = c; \
arena.count = 1; \
arena.data = malloc(arena.capacity * sizeof(arena.data[0])); \
memset(&arena.data[0], 0, sizeof(arena.data[0])); \
} \
} \
void free_ ## arena () { \
arena.capacity = arena.count = 0; \
free(arena.data); \
} \
type ## Index push_ ## arena (size_t c) { \
if (arena.count + c >= arena.capacity) { \
arena.capacity *= 2; \
arena.data = realloc(arena.data, arena.capacity * sizeof(arena.data[0])); \
if (arena.data == NULL) { printf("Out of memory: " #arena "!\n"); abort(); } \
} \
size_t i = arena.count; \
arena.count += c; \
return i; \
}

void* arena_alloc(size_t size, size_t align);
void arena_free();
