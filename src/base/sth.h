#ifndef _STH_H_
#define _STH_H_

#define STH_VERSION_MAJOR 0
#define STH_VERSION_MINOR 1
#define STH_VERSION_PATCH 4
#define STH_VERSION_STRING "0.1.4"

#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS (1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#if defined(__linux__) /* linux */ \
    || (defined(__APPLE__) && defined(__MACH__)) /* apple */ \
    || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) /* bsd */

    // unix-like platforms
    #define STH_PLAT_UNIX
    #include <sys/mman.h>
    #include <unistd.h>

#elif defined(_WIN32) || defined(_WIN64)

    // windows platform
    #define STH_PLAT_WINDOWS
    #include <windows.h>
    #include <memoryapi.h>
    #include <sysinfoapi.h>

#else // not unix-like nor windows
    #error "unsupported platform"
#endif

/*
 ************************************************************
 Declarations
 ***********************************************************
 */

#define STH_MALLOC(s)       malloc((s))
#define STH_CALLOC(n, s)    calloc((n), (s))
#define STH_REALLOC(p, s)   realloc((p), (s))
#define STH_FREE(p)         free((p))
#define STH_ASSERT(c)       assert((c))

#define STH_UNUSED(v)   ((void)(v))
#define STH_ARRAY_LEN(a) (sizeof((a)) / sizeof((a)[0]))

#define STH_TIME_SPENT_MS(__elapsed, __block) \
    do { \
        clock_t __clock_start; \
        __clock_start = clock(); \
        do { __block; } while (0); \
        __elapsed = ((double)(clock() - __clock_start)) * 1000.0 / CLOCKS_PER_SEC; \
    } while (0)

#define sth_offsetof(_type, _field) (&((_type*)NULL)->_field)
#ifndef offsetof
    #define offsetof sth_offsetof
#endif

// Branch prediction hints
#if defined(__clang__) || defined(__GNUC__)
    #define STH_EXPECT(expr, val) __builtin_expect((expr), (val))
#else
    #define STH_EXPECT(expr, val) (expr)
#endif
#define STH_LIKELY(expr)    STH_EXPECT(expr, 1)
#define STH_UNLIKELY(expr)  STH_EXPECT(expr, 0)


// Compiler specific macros
#if defined(__GNUC__) || defined(__clang__)
    #define STH_TRAP() __builtin_trap()
    #define STH_PACKED(__decl__) __decl__ __attribute__((packed))
#elif defined (_MSC_VER)
    #define STH_TRAP() __debugbreak()
    #define STH_PACKED(__decl__) __pragma(pack(push, 1)) __decl__ __pragma(pack(pop))
#else
    #define STH_TRAP() (*(volatile char*)0)
#endif


#if defined(NDEBUG)
    #define sth_assert(cond)
    #define sth_trap_assert(cond)
#else
    #define sth_assert(cond) \
        do { \
            if (!(cond)) { \
                fprintf(stderr, "%s(%d): sth_assert(" #cond ") failed\n", __FILE__, __LINE__); \
                abort(); \
            } \
        } while (0)

    #define sth_trap_assert(cond) \
        do { \
            if (!(cond)) { \
                fprintf(stderr, "%s(%d): sth_trap_assert(" #cond ") failed\n", __FILE__, __LINE__); \
                STH_TRAP(); \
            } \
        } while (0)
#endif

#define __sth_concat_(A,B) A##B
#define __sth_concat(A,B) __sth_concat_(A,B)
#define sth_static_assert(condition, id) \
    char __sth_concat(id, __LINE__)[ ((condition)) ? 1 : -1 ]

// Change byte order of a numeric value
#define sth_bswap16(x) ((((x) << 8) & 0xff00)  | (((x) >> 8) & 0x00ff))
#define sth_bswap32(x) (sth_bswap16(x) << 16 | sth_bswap16((x) >> 16))
#define sth_bswap64(x) (sth_bswap32(x) << 32 | sth_bswap32((x) >> 32))

// align up a number to a power-of-2 alignment
#define sth_align_pow2(num, alignment) \
    ((((uintptr_t)num) + ((alignment) - 1)) & (~((alignment) - 1)))

#define STH_KB(value) ((value) * 1024)
#define STH_MB(value) (STH_KB(value) * 1024)
#define STH_GB(value) (STH_MB(value) * 1024)

#ifdef __cplusplus
    #define STH_DECLTYPE_CAST(T) (decltype(T))
#else
    #define STH_DECLTYPE_CAST(T)
#endif


/*
 * Every type with these fields can treated as a dynamic array:
 *  - T *items
 *  - size_t len
 *  - size_t cap
 */

#define STH_DA_INIT_CAP (256)

#define sth_da_init(da, _cap) \
    do { \
        (da)->len = 0; \
        (da)->cap = ((_cap) <= 0) ? STH_DA_INIT_CAP : (_cap); \
        (da)->items = STH_MALLOC(((da)->cap) * sizeof(*(da)->items)); \
        STH_ASSERT((da)->items); \
    } while (0)

#define sth_da_reserve(da, expected_cap) \
    do { \
        if ((expected_cap) > (da)->cap) { \
            if ((da)->cap == 0) { \
                (da)->cap = STH_DA_INIT_CAP; \
            } \
            while ((da)->cap < (expected_cap)) { \
                (da)->cap <<= 1; \
            } \
            (da)->items = STH_DECLTYPE_CAST((da)->items) STH_REALLOC((da)->items, (da)->cap * sizeof(*(da)->items)); \
            STH_ASSERT((da)->items); \
        } \
    } while (0)

#define sth_da_append(da, item) \
    do { \
        sth_da_reserve((da), (da)->len + 1); \
        memcpy(&(da)->items[(da)->len], item, sizeof(*(da)->items)); \
        (da)->len++; \
    } while (0)

#define sth_da_append_many(da, new_items, new_items_count) \
    do { \
        sth_da_reserve((da), (da)->len + (new_items_count)); \
        memcpy(&(da)->items[(da)->len], (new_items), (new_items_count) * sizeof(*(da)->items)); \
        (da)->len += (new_items_count); \
    } while (0)

#define sth_da_resize(da, new_size) \
    do { \
        sth_da_reserve((da), (new_size)); \
        (da)->len = (new_size); \
    } while (0)

#define sth_da_free(da) STH_FREE((da)->items)


#define STH_ARENA_DEFAULT_ALIGNMENT sizeof(uintptr_t)

enum {
    STH_ARENA_NONE = 0,
    // arena will be fixed in size and not grow in case of space limitation
    STH_ARENA_FIXED = (1 << 0),
    // use large pages
    STH_ARENA_LARGPAGES = (1 << 1),
};

typedef struct {
    // the "reserve" and "commit" fields will be aligned by operating system's
    // page size
    size_t reserve, commit, alignment, flags;
} sth_arena_config_t;

typedef struct sth_arena {
    // all the fields are read-only to the user
    sth_arena_config_t config;
    struct sth_arena *prev, *current;
    // the "pos_base" field helps to track all the memory allocated and helps
    // to view the arena as a single buffer even with multiple buffers
    size_t pos, pos_base, reserved, commited;
} sth_arena_t;

// by using scopes you can take a snapshot from an existing arena, use that and
// restore the old state.
typedef struct {
    sth_arena_t *arena;
    size_t __pos; // read-only
} sth_arena_scope_t;

#define STH_ARENA_INITIAL_POS (sizeof(sth_arena_t))
#define STH_ARENA_DEFAULT_RESERVE_SIZE STH_MB(16)
#define STH_ARENA_DEFAULT_COMMIT_SIZE  STH_KB(16)

#ifdef __cplusplus
    #define STH_ARENA_DEFAULT_CONFIG \
        { \
            STH_ARENA_DEFAULT_RESERVE_SIZE, \
            STH_ARENA_DEFAULT_COMMIT_SIZE, \
            STH_ARENA_DEFAULT_ALIGNMENT, \
            STH_ARENA_NONE \
        }
#else
    #define STH_ARENA_DEFAULT_CONFIG \
        ((sth_arena_config_t){ \
            .reserve = STH_ARENA_DEFAULT_RESERVE_SIZE, \
            .commit = STH_ARENA_DEFAULT_COMMIT_SIZE, \
            .alignment = STH_ARENA_DEFAULT_ALIGNMENT, \
            .flags = STH_ARENA_NONE \
        })
#endif

sth_arena_t *sth_arena_new(const sth_arena_config_t *config);

void sth_arena_destroy(sth_arena_t *arena);

// Allocate memory on arena with specified alignment. The alignment value  MUST
// be a power of 2
void *sth_arena_alloc_align(sth_arena_t *arena, size_t size, size_t alignment);

// Helper macro to use arena's alignment value for allocations
#define sth_arena_alloc(arena, size) \
    sth_arena_alloc_align((arena), (size), (arena)->config.alignment)

// Is arena empty?
int sth_arena_is_empty(const sth_arena_t *arena);

// Get total bytes allocated
size_t sth_arena_pos(const sth_arena_t *arena);

// Set arena's position to a position specified by pos
void sth_arena_pop_to(sth_arena_t *arena, size_t pos);

// Seek back arena's pointer by offset.
void sth_arena_pop(sth_arena_t *arena, size_t offset);

// Reset the arena. If the arena had more that one buffer, free all of them and
// just keep the first buffer and also reset that.
void sth_arena_reset(sth_arena_t *arena);

// Take a snapshot from an arena, use that and restore the old state with
// arena_scope_end function. This function writes data to scope_out.
void sth_arena_scope_begin(sth_arena_t *arena, sth_arena_scope_t *scope_out);

// Restore an arena's state from an snapshot.
void sth_arena_scope_end(sth_arena_scope_t *scope);

// Allocate n+1 bytes on the arena and copy string s to that buffer.
char *sth_arena_strndup(sth_arena_t *arena, const char *s, size_t n);


typedef union __sth_mempool_chunk __sth_mempool_chunk_t;
typedef struct {
    sth_arena_t *arena;
    __sth_mempool_chunk_t *freelist;
    size_t chunk_size;
} sth_mempool_t;

void sth_mempool_init(sth_mempool_t *pool, sth_arena_t *arena, size_t chunk_size);
void *sth_mempool_get(sth_mempool_t *pool);
void sth_mempool_put(sth_mempool_t *pool, void *v);


void *sth_memcpy_fast(void *dest, const void *src, size_t n);
void *sth_memmove_fast(void *dest, const void *src, size_t n);


#ifdef STH_IMPL
/*
 ************************************************************
 Implementations
 ***********************************************************
 */

/*
 * Begin operating system abstractions
 */
static void *sth_os_mem_reserve(size_t size, int with_large_pages) {
    void *p; int wlp;
#ifdef STH_PLAT_UNIX
    wlp = (with_large_pages) ? MAP_HUGETLB : 0;
    p = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | wlp, -1, 0);
    if (p == MAP_FAILED)
        p = NULL;
#else
    wlp = (with_large_pages) ? (MEM_COMMIT | MEM_LARGE_PAGES) : 0;
    p = VirtualAlloc(NULL, size, MEM_RESERVE | wlp, PAGE_READWRITE);
#endif
    return p;
}

static int sth_os_mem_commit(void *p, size_t size, int with_large_pages) {
#ifdef STH_PLAT_UNIX
    (void)with_large_pages;
    return (mprotect(p, size, PROT_READ | PROT_WRITE) == 0);
#else
    if (with_large_pages)
        return 1;
    return (VirtualAlloc(p, size, MEM_COMMIT, PAGE_READWRITE) != 0);
#endif
}

// release a reserved memory block
static void sth_os_mem_release(void *p, size_t size) {
#ifdef STH_PLAT_UNIX
    munmap(p, size);
#else
    (void)size;
    VirtualFree(p, 0, MEM_RELEASE);
#endif
}

static inline size_t sth_os_get_pagesize(void) {
#ifdef STH_PLAT_UNIX
    return sysconf(_SC_PAGESIZE);
#else
    SYSTEM_INFO sysinfo;
    memset(&sysinfo, 0, sizeof(sysinfo));
    GetSystemInfo(&sysinfo);
    return sysinfo.dwPageSize;
#endif
}

static inline size_t sth_os_get_largepagesize(void) {
#ifdef STH_PLAT_UNIX
    // 2 MB is a safe value for Linux and most BSD systems
    return STH_MB(2);
#else
    return GetLargePageMinimum();
#endif
}
/*
 * End operating system abstractions
 */


/*
 * Begin Arena memory allocator
 */
sth_static_assert((sizeof(uintptr_t) == sizeof(void*)), validate_uintptr_size);

sth_arena_t *sth_arena_new(const sth_arena_config_t *config) {
    sth_arena_t *a;
    size_t pagesize, reserve, commit;

    STH_ASSERT(config);
    int lp = config->flags & STH_ARENA_LARGPAGES;
    pagesize = (lp) ? sth_os_get_largepagesize() : sth_os_get_pagesize();

    // align reserve and commit fields by operating system's page size
    reserve = sth_align_pow2(config->reserve, pagesize);
    commit = sth_align_pow2(config->commit, pagesize);

    a = (sth_arena_t*) sth_os_mem_reserve(reserve, lp);
    if (!a)
        return NULL;
    if (!sth_os_mem_commit(a, commit, lp)) {
        sth_os_mem_release(a, reserve);
        return NULL;
    }

    memcpy(&a->config, config, sizeof(*config));

    // store aligned values
    a->config.reserve = reserve;
    a->config.commit  = commit;
    a->commited = commit;
    a->reserved = reserve;

    a->pos_base = 0;
    a->pos = STH_ARENA_INITIAL_POS;
    a->prev = NULL;
    a->current = a;

    return a;
}

void *sth_arena_alloc_align(sth_arena_t *arena, size_t size, size_t alignment) {
    unsigned char *raw, *aligned;
    sth_arena_t *current, *new_arena;
    size_t padding;

    STH_ASSERT(arena);
    // If size is zero or requested size is bigger than the arena return NULL
    if (size == 0 || size > (arena->reserved - sizeof(*arena)))
        return NULL;

    current = arena->current;
    raw = (unsigned char*)current + current->pos;
    aligned = (unsigned char*) sth_align_pow2(raw, alignment);
    padding = aligned - raw;

    if ((size + padding) > (current->reserved - current->pos)) {
        if (current->config.flags & STH_ARENA_FIXED)
            return NULL;

        if ( !(new_arena = sth_arena_new(&current->config)))
            return NULL;

        new_arena->pos_base = current->pos_base + current->reserved;
        new_arena->prev = current;
        arena->current = new_arena;

        // reinitialize allocation info
        current = new_arena;
        raw = (unsigned char*)current + current->pos;
        aligned = (unsigned char*) sth_align_pow2(raw, alignment);
        padding = aligned - raw;
    }
    current->pos += size + padding;

    // commit new pages if needed
    if (current->pos > current->commited) {
        // Since "reserve" and "commit" fields in arena config are already
        // aligned by operating system's page size, the "commit" field is
        // divisible by "reserve" field. So we can divied the arena's buffer to
        // blocks with "commit" size each.
        sth_os_mem_commit((unsigned char*)current + current->commited,
                          current->config.commit,
                          current->config.flags & STH_ARENA_LARGPAGES);
        current->commited += current->config.commit;
    }

    return aligned;
}

size_t sth_arena_pos(const sth_arena_t *arena) {
    STH_ASSERT(arena);
    return (arena->current->pos_base + arena->current->pos);
}

int sth_arena_is_empty(const sth_arena_t *arena) {
    STH_ASSERT(arena);
    return ((arena->current->pos_base == 0) && (arena->pos == STH_ARENA_INITIAL_POS));
}

void sth_arena_pop_to(sth_arena_t *arena, size_t pos) {
    STH_ASSERT(arena);
    sth_arena_t *current = arena->current, *prev = NULL;
    while (current->pos_base >= pos) {
        prev = current->prev;
        sth_os_mem_release(current, current->reserved);
        current = prev;
    }
    arena->current = current;
    current->pos = pos - current->pos_base;
}

void sth_arena_pop(sth_arena_t *arena, size_t offset) {
    STH_ASSERT(arena);
    size_t curr_pos = sth_arena_pos(arena);
    if (offset <= curr_pos)
        sth_arena_pop_to(arena, curr_pos - offset);
}

void sth_arena_reset(sth_arena_t *arena) {
    STH_ASSERT(arena);
    sth_arena_pop_to(arena, STH_ARENA_INITIAL_POS);
}

void sth_arena_destroy(sth_arena_t *arena) {
    sth_arena_t *current, *prev;
    STH_ASSERT(arena);
    current = arena->current;
    while (current) {
        prev = current->prev;
        sth_os_mem_release(current, current->reserved);
        current = prev;
    }
}

void sth_arena_scope_begin(sth_arena_t *arena, sth_arena_scope_t *scope_out) {
    STH_ASSERT(arena);
    STH_ASSERT(scope_out);
    scope_out->arena = arena;
    scope_out->__pos = sth_arena_pos(arena);
}

void sth_arena_scope_end(sth_arena_scope_t *scope) {
    STH_ASSERT(scope);
    sth_arena_pop_to(scope->arena, scope->__pos);
}

char *sth_arena_strndup(sth_arena_t *arena, const char *s, size_t n) {
    STH_ASSERT(arena);
    STH_ASSERT(s);
    if (n == 0)
        return NULL;
    char *buf = STH_DECLTYPE_CAST(buf) sth_arena_alloc(arena, n+1);
    memcpy(buf, s, n);
    buf[n] = 0;
    return buf;
}
/*
 * End Arena memory allocator
 */


/*
 * Begin Pool allocator
 */
union __sth_mempool_chunk {
    __sth_mempool_chunk_t *next;
};

void sth_mempool_init(sth_mempool_t *pool, sth_arena_t *arena, size_t chunk_size) {
    chunk_size = (chunk_size >= sizeof(__sth_mempool_chunk_t)) ? chunk_size : sizeof(__sth_mempool_chunk_t);
    *pool = (sth_mempool_t){
        .arena = arena,
        .freelist = NULL,
        .chunk_size = chunk_size,
    };
}

void *sth_mempool_get(sth_mempool_t *pool) {
    __sth_mempool_chunk_t *c;
    if (!pool->freelist)
        return sth_arena_alloc(pool->arena, pool->chunk_size);
    c = pool->freelist;
    pool->freelist = c->next;
    return c;
}

void sth_mempool_put(sth_mempool_t *pool, void *v) {
    __sth_mempool_chunk_t *c = (__sth_mempool_chunk_t*) v;
    c->next = pool->freelist;
    pool->freelist = c;
}
/*
 * End Pool allocator
 */


/*
 * Begin string and memory utilities
 */
typedef unsigned long long __sth_word_t;

#define __STH_WORD_BIT_SIZE         (sizeof(__sth_word_t) << 3U)
#define __STH_SHIFT_N_BY            ((__STH_WORD_BIT_SIZE >> 5U) + 1U)
#define __STH_LAST_BYTES_ADDR_MASK  ((__STH_WORD_BIT_SIZE >> 3U) - 1U)

#define __sth_aligned_mem_info(__buffer, __n) \
    __sth_word_t *__buffer##_word = (__sth_word_t*) __buffer; \
    __sth_word_t *__buffer##_word_end = __buffer##_word + ((__n) >> __STH_SHIFT_N_BY); \
    uint8_t *__buffer##_byte = (uint8_t*) __buffer##_word_end; \
    uint8_t *__buffer##_byte_end = __buffer##_byte + ((__n) & __STH_LAST_BYTES_ADDR_MASK);


void *sth_memcpy_fast(void *dest, const void *src, size_t n) {
    __sth_aligned_mem_info(src, n);
    __sth_aligned_mem_info(dest, n);
    STH_UNUSED(dest_byte_end);

    while (src_word != src_word_end) {
        *dest_word = *src_word;
        dest_word++;
        src_word++;
    }
    while (src_byte != src_byte_end) {
        *dest_byte = *src_byte;
        dest_byte++;
        src_byte++;
    }

    return dest;
}

void *sth_memmove_fast(void *dest, const void *src, size_t n) {
    if (dest == src)
        return dest;

    // took from musl libc
    if ((((uintptr_t)dest - (uintptr_t)src - n) <= (-2*n)) || (dest < src))
        return sth_memcpy_fast(dest, src, n);

    __sth_aligned_mem_info(src, n);
    __sth_aligned_mem_info(dest, n);

    dest_byte_end--;
    dest_word_end--;
    src_byte_end--;
    src_word_end--;

    while (src_byte <= src_byte_end) {
        *dest_byte_end = *src_byte_end;
        dest_byte_end--;
        src_byte_end--;
    }
    while (src_word <= src_word_end) {
        *dest_word_end = *src_word_end;
        dest_word_end--;
        src_word_end--;
    }

    return dest;
}
/*
 * End string utilities
 */
#endif // STH_IMPL

#ifdef __cplusplus
}
#endif

#endif // _STH_H_
