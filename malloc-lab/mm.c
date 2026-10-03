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

#define HSIZE WSIZE /* Header size(bytes) */
#define FSIZE WSIZE /* Footer size(bytes) */

/* =========== Used in mm_init() =========== */
#define PROLOG 8
#define PADDING WSIZE
#define EPLILOG WSIZE
/* =========== Used in mm_init() =========== */

#define ALLOCATED 1 /* Signal-Number when allocated */
#define FREE 0 /* Signal-Number when freed */

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

/* Read and write a word at address p */
#define GET(p)      (*(unsigned int *)(p))
#define SET(p, val) (*(unsigned int *)(p) = (val))

/* Insert the number depending on whether allocated or freed. */
#define ALLOC(p, alloced) SET(p, (GET(p) & ~0x1) | (alloced))

/* Get a header/footer location of the target pointer bp */
#define HEADER(bp) ((char *)(bp) - (HSIZE))
#define FOOTER(bp) ((char *)(bp) + GET_SIZE(bp) - (2 * (HSIZE)))

/* Read the size and allocated fields from address header */
#define GET_SIZE(bp)  ((unsigned int)(GET(HEADER(bp)) & ~0x7))
#define GET_ALLOC(bp) ((unsigned int)(GET(HEADER(bp)) & 0x1))

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_P(bp) ((char *)(bp) + GET_SIZE(bp))
#define PREV_P(bp) ((char *)(bp) - (GET((char *)(bp) - (2 * (HSIZE))) & ~0x7))
/*
 * mm_init - initialize the malloc package.
 */
static char *prolog_p;
int mm_init(void) {

    char *start = (char *)mem_sbrk(PADDING + PROLOG + EPLILOG);
    if (start == (char *)-1){
        return -1;
    } else {
        // prolog header
        SET((start + PADDING), PROLOG);
        ALLOC((start + PADDING), ALLOCATED);

        // prolog footer
        SET((start + PADDING + HSIZE), PROLOG);
        ALLOC((start + PADDING + HSIZE), ALLOCATED);

        // eplilog
        SET((start + PADDING + HSIZE + FSIZE), 0);
        ALLOC((start + PADDING + HSIZE + FSIZE), ALLOCATED);

        prolog_p = start + PADDING + HSIZE;
    } return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

char *first_fit(char *p, size_t size){
    if (!p){ /* first_fit 으로 못찾으면 error */
        return NULL;
        // ============= 진짜 구상 =============
        /* 1. 뒤쪽이 뻥 뚫려있는 경우
         *      1-1. if(GET_SIZE(NEXT_p) <= eplilog 주소 - 현재 p의 주소)(진짜 뻥 뚫린 경우){
         *          return NEXT_p;
         *      }
         *      1-2. else(뚫리긴 했는데 size가 남은 공간보다 큼){
         *          return extend_heap(p)
         *      }
           2. 뒤쪽이 막혀있는 경우(일단 칸을 더 넘어가야 하긴 함. 찾을 때까지)
                한칸씩 넘어가는 방법(p 최신화 필요) :  p += GET_SIZE(p) - HSIZE
                무엇을 찾아야 하느냐
                    - Freed 상태일 것
                    - GET_SIZE(p) >= size일 것
        */

        // ============= 진짜 구상 =============
    } return p;
}

char *extend_heap(char *p, size_t size){
    if (!p){ /* extend가 불가능해지면 error */
        return NULL;
        // ============= 진짜 구상 =============
        /* 1. block_size(p, psize)
         *
         */

        // ============= 진짜 구상 =============

    } return p; // 새롭게 할당된 포인터인데, 그건 마지막 끄트머리(free)도 포함되어야 함.
}

unsigned int block_size(size_t psize){
    if (psize % 8 == 0){
        return HSIZE + (unsigned int)psize + FSIZE;
    } else{
        return (
            HSIZE + // Payload size without remainder of psize and DSIZE(8)
            DSIZE * ((unsigned int)psize / DSIZE) +
            DSIZE + // Padding
            FSIZE
        );
    }
}

void *mm_malloc(size_t size){
    if (size == 0 || size == MAX_HEAP){
        return NULL;
    } char *p = prolog_p;

    /* ========== extended가 필요없는 버전 ========== */


    /* ========== extended가 필요한 버전 ========== */

    char *extended = extend_heap(p, size);

    if (extended == NULL){
        return NULL;
    } // p += GET_SIZE(p) - HSIZE; - 이거는 한 칸씩 전진하는 로직임.

    char *fptr = first_fit(p, size);

    // Fit pointer's header setting
    SET((HEADER(fptr)), size);
    ALLOC(HEADER(fptr), ALLOCATED);

    // Fit pointer's footer setting
    SET((FOOTER(fptr)), size);
    ALLOC(FOOTER(fptr), ALLOCATED);
    /* ========== extended가 필요한 버전 ========== */

    GET_SIZE(p);
    return p;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    // void *oldptr = ptr;
    // void *newptr;
    // size_t copySize;

    // newptr = mm_malloc(size);
    // if (newptr == NULL)
    //     return NULL;
    // copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    // if (size < copySize)
    //     copySize = size;
    // memcpy(newptr, oldptr, copySize);
    // mm_free(oldptr);
    // return newptr;
    return NULL;
}
