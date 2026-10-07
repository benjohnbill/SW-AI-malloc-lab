#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "config.h"
#include "memlib.h"

team_t team = {
    /* Team name */
    "Krafton-Jungle",
    /* full name */
    "JinGeun, Cho",
    /* email address */
    "chobenjohn@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* Single word (4) & Double word (8) Alignment based on 32-bit Computer*/
#define WSIZE       4       /* Word size (bytes) */
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE (1<<12)   /* Extend heap by this amount (bytes) */

#define HSIZE WSIZE /* HDR size(bytes) */
#define FSIZE WSIZE /* FTR size(bytes) */

/* =========== Used in mm_init() =========== */
#define PROLOG 8
#define PADDING WSIZE
#define EPILOG WSIZE
/* =========== Used in mm_init() =========== */

#define ALLOCATED 1 /* Signal-Number when allocated */
#define FREE 0 /* Signal-Number when freed */

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

/* Read and write a word at address p */
#define GET(p)      (*(unsigned int *)(p))
#define SET(p, val) (*(unsigned int *)(p) = (val))

/* Insert the number depending on whether allocated or freed. */
#define ALLOC(p, alloced) (SET(p, (GET(p) & ~0x1) | (alloced)))
#define METADATA_SET(p, size, alloced) SET(p, (size) | (alloced))
#define PONLY(bsize) ((unsigned int)exact_psize(bsize)) - (HSIZE) - (FSIZE)


/* Get a HDR/FTR location of the target pointer bp */
#define HDR(bp) ((char *)(bp) - (HSIZE))
#define FTR(bp) ((char *)(bp) + GET_SIZE(bp) - (2 * (HSIZE)))

/* Read the size and allocated fields from address HDR */
#define GET_SIZE(bp)  ((unsigned int)(GET(HDR(bp)) & ~0x7))
#define GET_ALLOC(bp) ((unsigned int)(GET(HDR(bp)) & 0x1))

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_P(bp) ((char *)(bp) + GET_SIZE(bp))
#define PREV_P(bp) ((char *)(bp) - (GET((char *)(bp) - (2 * (HSIZE))) & ~0x7))
/*
 * mm_init - initialize the malloc package.
 */
static char *prolog_p;
int mm_init(void) {
    char *start = (char *)mem_sbrk(PADDING + PROLOG + EPILOG);
    if (start == (char *)-1){
        return -1;
    } else { // prolog HDR, prolog FTR, EPILOG
        METADATA_SET(start + PADDING, PROLOG, ALLOCATED);
        METADATA_SET(start + PADDING + HSIZE, PROLOG, ALLOCATED);
        METADATA_SET(start + PADDING + HSIZE + FSIZE, 0, ALLOCATED);
        prolog_p = start + PADDING + HSIZE;
    } return 0;
}

static unsigned int exact_psize(size_t psize){
    if (psize % 8 == 0){
        return (unsigned int)psize;
    } else{
        return (
            DSIZE * ((unsigned int)psize / DSIZE) +
            DSIZE // Padding.
        );
    }
}

static unsigned int bsize(size_t psize){
    return HSIZE + (unsigned int)exact_psize(psize) + FSIZE;
}

// static char *first_fit(char *bp, size_t size){
//     while (GET_SIZE(bp) != 0){
//         if (GET_ALLOC(bp) == FREE && GET_SIZE(bp) >= (size_t)bsize(size)){
//             return bp;
//         } bp = NEXT_P(bp);
//     } return NULL;
// }

static char *next_refit(char *bp, size_t size, char *fit_spot){
    if (fit_spot == NULL){
        return NULL;
    } while (bp != fit_spot){
        if (GET_ALLOC(bp) == FREE && GET_SIZE(bp) >= (size_t)bsize(size)){
            return bp;
        } bp = NEXT_P(bp);
    } return NULL;
}

static char *fit_spot = NULL;
static char *next_fit(char *bp, size_t size){
    if (fit_spot != NULL){
        bp = fit_spot;
    } while (GET_SIZE(bp) != 0){
        if (GET_ALLOC(bp) == FREE && GET_SIZE(bp) >= (size_t)bsize(size)){
            fit_spot = bp;
            return bp;
        } bp = NEXT_P(bp);
    } fit_spot = next_refit(prolog_p, size, fit_spot);
    return fit_spot;
}

static char *coalescence(char *bp){
    char *prev_bp = PREV_P(bp);
    char *next_bp = NEXT_P(bp);
    unsigned int prev_alloc = GET_ALLOC(prev_bp);
    unsigned int next_alloc = GET_ALLOC(next_bp);
    unsigned int size = GET_SIZE(bp);

    if (prev_alloc == ALLOCATED && next_alloc == ALLOCATED){
        return bp; //
    } else if (prev_alloc == ALLOCATED && next_alloc == FREE){
        size += GET_SIZE(next_bp);
    } else if (prev_alloc == FREE && next_alloc == ALLOCATED){
        size += GET_SIZE(prev_bp);
        bp = prev_bp;
    } else {
        size += GET_SIZE(prev_bp) + GET_SIZE(next_bp);
        bp = prev_bp;
    }

    METADATA_SET(HDR(bp), size, FREE);
    METADATA_SET(FTR(bp), size, FREE);
    fit_spot = bp;
    return bp;
}

static char *extend_heap(size_t size){
    size_t extendsize = MAX(bsize(size), CHUNKSIZE);
    char *old_end = mem_sbrk(extendsize);
    char *new_bp;

    if (old_end == (char *)-1){
        return NULL;
    } /* Set the Metadata in Free-Block & Epilog. */
    METADATA_SET(HDR(old_end), extendsize, FREE);
    METADATA_SET(FTR(old_end), extendsize, FREE);
    METADATA_SET(((char *)mem_sbrk(0) - EPILOG), 0, ALLOCATED);
    return new_bp = coalescence(old_end);
}

static void place(char *new_bp, size_t size){
    // Store the value, before changing HDR.
    size_t old_size = GET_SIZE(new_bp);
    // Entire free block size after coalescing.
    size_t ad_size = bsize(size);

    if ((old_size - ad_size) >= (2*DSIZE)){
        char *next_bp;
        size_t free_size;

        /* New allocation. */
        METADATA_SET(HDR(new_bp), ad_size, ALLOCATED);
        METADATA_SET(FTR(new_bp), ad_size, ALLOCATED);

        /* Free Block allocation. */
        free_size = old_size - ad_size;
        next_bp = NEXT_P(new_bp);
        METADATA_SET(HDR(next_bp), free_size, FREE);
        METADATA_SET(FTR(next_bp), free_size, FREE);
    } else {
        /* when the rest smaller than minimum block */
        METADATA_SET(HDR(new_bp), old_size, ALLOCATED);
        METADATA_SET(FTR(new_bp), old_size, ALLOCATED);
    } return;
}

void *mm_malloc(size_t size){
    if (size == 0 || size >= MAX_HEAP){
        return NULL;
    } // Edge Case.

    char *fptr = next_fit(prolog_p, size);
    if (fptr){
        place(fptr, size);
        return fptr;
    } // When first_fit exist.

    char *extended_ptr = extend_heap(size);
    if (extended_ptr == NULL){
        return NULL;
    } else{ // Placing the size/alloced in extended_ptr's HDR/FTR.
        place(extended_ptr, size);
    } return extended_ptr;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr){
    char *fptr = coalescence(ptr);
    METADATA_SET(HDR(fptr), GET_SIZE(fptr), FREE);
    METADATA_SET(FTR(fptr), GET_SIZE(fptr), FREE);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */

void *free_case(void *bp, void *next_bp, size_t total_size, size_t old_size, size_t size, unsigned int new_bsize){
    // 1. new_size > old_size
    if(new_bsize > old_size){
        // 1-1. total_size >= new_size
        if (total_size >= new_bsize){
            SET(HDR(bp), total_size);
            SET(FTR(bp), total_size);
            place(bp, PONLY(total_size));
            fit_spot = NEXT_P(bp);
            return bp;
        } // 1-2. total_size < new_size
        else{
            void *new_bp = mm_malloc(size);
            if (new_bp == NULL){
            return NULL;
            } memcpy(new_bp, bp, PONLY(old_size));
            mm_free(bp);
            return new_bp;
        }
    } else { // 2. new_size <= old_size
        place(bp, size);
        coalescence(next_bp);
        // Handling remainder free block
        return bp;
    }
}

void *mm_realloc(void *bp, size_t size){
    // Edge cases
    if(bp == NULL){
        return mm_malloc(size);
    } if(size == 0){
        mm_free(bp);
        return NULL;
    } if (size > MAX_HEAP - 16){
        return NULL;
    };
    // arr[4]는 되는데, 왜 끄트머리 패딩에 접근할 때는 UB가 뜰까?
    // memmove()(방향에 가까운 쪽부터 움직인다?) vs. memcpy().
    char *next_bp = NEXT_P(bp);
    unsigned int next_size = GET_SIZE(next_bp);
    unsigned int next_alloc = GET_ALLOC(next_bp);

    size_t old_size = GET_SIZE(bp);
    size_t total_size = old_size + (size_t)next_size;
    unsigned int new_bsize = bsize(size);

    if (next_alloc == FREE){
        bp = free_case(bp, next_bp, total_size, old_size, size,new_bsize);
        return bp;
    } else{ // next_alloc = ALLOCATED.
        if(new_bsize > old_size){
            void *new_bp = mm_malloc(size);
            if (new_bp == NULL){
                return NULL;
            } memcpy(new_bp, bp, PONLY(old_size));
            mm_free(bp);
            return new_bp;
        }else {
            place(bp, size);
            fit_spot = bp;
            return bp;
        }
    }
}

// void *mm_realloc(void *bp, size_t size){
//     // Edge cases
//     if(bp == NULL){
//         return mm_malloc(size);
//     } if(size == 0){
//         mm_free(bp);
//         return NULL;
//     } if (size > MAX_HEAP - 16){
//         return NULL;
//     };
//     // arr[4]는 되는데, 왜 끄트머리 패딩에 접근할 때는 UB가 뜰까?
//     // memmov()(방향에 가까운 쪽부터 움직인다?) vs. memcpy().
//     char *next_bp = NEXT_P(bp);
//     unsigned int next_size = GET_SIZE(next_bp);
//     unsigned int next_alloc = GET_ALLOC(next_bp);

//     size_t old_size = GET_SIZE(bp);
//     size_t total_size = old_size + (size_t)next_size;
//     unsigned int new_bsize = bsize(size);

//     if (next_alloc == FREE){
//         // 1. new_size > old_size
//         if(new_bsize > old_size){
//             // 1-1. total_size >= new_size
//             if (total_size >= new_bsize){
//                 SET(HDR(bp), total_size);
//                 SET(FTR(bp), total_size);
//                 place(bp, PONLY(total_size));
//                 return bp;
//             } else{ // 1-2. total_size < new_size
//                 goto new_malloc;
//             }
//         } else { // 2. new_size <= old_size
//             place(bp, size);
//             coalescence(next_bp);
//             // Handling remainder free block
//             return bp;
//         }
//     } else{ // next_alloc = ALLOCATED.
//         if(new_bsize > old_size){
//             goto new_malloc;
//         }else {
//             place(bp, size);
//             return bp;
//         }
//     } new_malloc:
//     void *new_bp = mm_malloc(size);
//     if (new_bp == NULL){
//     return NULL;
//     } memcpy(new_bp, bp, PONLY(old_size));
//     mm_free(bp);
//     return new_bp;
// }
