#include <criterion/criterion.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "sfmm.h"

/**
 *  HERE ARE OUR TEST CASES NOT ALL SHOULD BE GIVEN STUDENTS
 *  REMINDER MAX ALLOCATIONS MAY NOT EXCEED 4 * 4096 or 16384 or 128KB
 */

Test(sf_memsuite, Malloc_an_Integer, .init = sf_mem_init, .fini = sf_mem_fini) {
    int *x = sf_malloc(sizeof(int));
    *x = 4;
    cr_assert(*x == 4, "Failed to properly sf_malloc space for an integer!");
}

Test(sf_memsuite, Free_block_check_header_footer_values, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *pointer = sf_malloc(sizeof(short));
    sf_free(pointer);
    pointer = pointer - 8;
    sf_header *sfHeader = (sf_header *) pointer;
    cr_assert(sfHeader->alloc == 0, "Alloc bit in header is not 0!\n");
    sf_footer *sfFooter = (sf_footer *) (pointer - 8 + (sfHeader->block_size << 4));
    cr_assert(sfFooter->alloc == 0, "Alloc bit in the footer is not 0!\n");
}

Test(sf_memsuite, PaddingSize_Check_char, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *pointer = sf_malloc(sizeof(char));
    pointer = pointer - 8;
    sf_header *sfHeader = (sf_header *) pointer;
    cr_assert(sfHeader->padding_size == 15, "Header padding size is incorrect for malloc of a single char!\n");
}

Test(sf_memsuite, Check_next_prev_pointers_of_free_block_at_head_of_list, .init = sf_mem_init, .fini = sf_mem_fini) {
    int *x = sf_malloc(4);
    memset(x, 0, 4);
    cr_assert(freelist_head->next == NULL);
    cr_assert(freelist_head->prev == NULL);
}

Test(sf_memsuite, Coalesce_no_coalescing, .init = sf_mem_init, .fini = sf_mem_fini) {
    int *x = sf_malloc(4);
    int *y = sf_malloc(4);
    memset(y, 0xFF, 4);
    sf_free(x);
    cr_assert(freelist_head == (void*)x-8);
    sf_free_header *headofx = (sf_free_header*) ((void*)x-8);
    sf_footer *footofx = (sf_footer*) ((void*)headofx + (headofx->header.block_size << 4)) - 8;

    //printf("Address of x: %p\n", x);
    //printf("Address of headofx: %p\n", headofx);
    //printf("headofx block_size%d\n", headofx->header.block_size);
    //printf("Address of footofx%p\n", footofx);
    //printf("footofx block_size%d\n", footofx->block_size);
    // All of the below should be true if there was no coalescing
    cr_assert(headofx->header.alloc == 0);
    cr_assert((headofx->header.block_size<<4) == 32);
    cr_assert(headofx->header.padding_size == 0);

    cr_assert(footofx->alloc == 0);
    cr_assert((footofx->block_size<<4) == 32);
}

/*
//############################################
// STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
// DO NOT DELETE THESE COMMENTS
//############################################
*/

Test(sf_memsuite, Malloc_4_Pages, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* address = sf_malloc(16368);
    cr_assert(address != NULL);

    void* address1 = sf_malloc(4);
    cr_assert(address1 == NULL);

    sf_free(address);
    sf_free_header* head = (sf_free_header*) ((void*)address - 8);

    cr_assert(head->header.alloc == 0);
}

Test(sf_memsuite, Malloc_Invalid_Input, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* address0 = sf_malloc(0);
    void* address1 = sf_malloc(-1);

    cr_assert(address0 == NULL);
    cr_assert(address1 == NULL);
}

Test(sf_memsuite, Free_Coalesce_Left, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* x = sf_malloc(32);
    void* y = sf_malloc(32);
    sf_malloc(64);

    sf_free(x);
    sf_free(y);

    sf_free_header* headofx = (sf_free_header*) (x - 8);
    sf_footer* footofx = (sf_footer*) (x - 8 + 96 - 8);
    
    cr_assert((headofx->header.block_size << 4) == 96);
    cr_assert(headofx->header.alloc == 0);

    cr_assert((footofx->block_size << 4) == 96);
    cr_assert(footofx->alloc == 0);

}
Test(sf_memsuite, Free_Coalesce_Right, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* x = sf_malloc(32);
    void* y = sf_malloc(32);
    sf_malloc(64);

    sf_free(y);
    sf_free(x);

    sf_free_header* headofx = (sf_free_header*) (x - 8);
    sf_footer* footofx = (sf_footer*) (x - 8 + 96 - 8);
    
    cr_assert((headofx->header.block_size << 4) == 96);
    cr_assert(headofx->header.alloc == 0);

    cr_assert((footofx->block_size << 4) == 96);
    cr_assert(footofx->alloc == 0);
}

Test(sf_memsuite, Free_No_Coalesce_Both, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_malloc(32);
    void* y = sf_malloc(32);
    sf_malloc(64);

    sf_free(y);
    sf_free_header* headofx = (sf_free_header*) (y - 8);
    sf_footer* footofx = (sf_footer*) (y - 8 + 48 - 8);

    cr_assert((headofx->header.block_size << 4) == 48);
    cr_assert(headofx->header.alloc == 0);

    cr_assert((footofx->block_size << 4) == 48);
    cr_assert(footofx->alloc == 0);
}

Test(sf_memsuite, Free_Coalesce_Both, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* x = sf_malloc(32);
    void* y = sf_malloc(32);
    void* z = sf_malloc(64);
    sf_malloc(34);

    sf_free(x);
    sf_free(z);
    sf_free(y);

    sf_free_header* headofx = (sf_free_header*) (x - 8);
    sf_footer* footofx = (sf_footer*) (x - 8 + 176 - 8);
    
    cr_assert((headofx->header.block_size << 4) == 176);
    cr_assert(headofx->header.alloc == 0);

    cr_assert((footofx->block_size << 4) == 176);
    cr_assert(footofx->alloc == 0);
}

Test(sf_memsuite, Realloc_Invalid_Address, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* z = sf_malloc(12);

    void* address = sf_realloc(z + 1, 20);
    cr_assert(address == NULL);
}

Test(sf_memsuite, Realloc_Invalid_Size, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* z = sf_malloc(12);

    void* address = sf_realloc(z, -1);
    cr_assert(address == NULL);
}

Test(sf_memsuite, Realloc_Equal_Size, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* d = sf_malloc(4);
    void* g = sf_realloc(d, 4);

    cr_assert(d == g);
}
Test(sf_memsuite, Realloc_Size_Less_Than_Old_Do_Split, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* d = sf_malloc(128);
    void* g = sf_realloc(d, 4);

    cr_assert(d == g);
    sf_free_header* po = (sf_free_header*) (d - 8);
    cr_assert((po->header.block_size << 4) == 32);
}
Test(sf_memsuite, Realloc_Size_Less_Than_Old_No_Split, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* d = sf_malloc(128);
    void* g = sf_realloc(d, 104);

    cr_assert(d == g);
    sf_free_header* po = (sf_free_header*) (d - 8);
    cr_assert((po->header.block_size << 4) == 144);
}
Test(sf_memsuite, Realloc_Size_Greater_Than_Old_BlockSize_Combine_Right, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* d = sf_malloc(4);
    void* g = sf_realloc(d, 64);

    cr_assert(d == g);
    sf_free_header* po = (sf_free_header*) (d - 8);
    cr_assert((po->header.block_size << 4) == 80);

    sf_free_header* next_block = (sf_free_header*) (g - 8 + 80);
    cr_assert(next_block->header.alloc == 0);
}

Test(sf_memsuite, Realloc_Size_Greater_Than_Old_BlockSize_Combine_Right_With_Malloc, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_malloc(4);
    void* d = sf_malloc(4);
    sf_malloc(4);
    void* g = sf_realloc(d, 64);

    cr_assert(d != g);
    sf_free_header* po = (sf_free_header*) (g - 8);
    cr_assert((po->header.block_size << 4) == 80);

    sf_free_header* d_block = (sf_free_header*) (d-8);
    sf_footer* d_footer = (sf_footer*) (d - 8 + 32 - 8);
    
    cr_assert(d_block->header.alloc == 0);
    cr_assert((d_block->header.block_size << 4) == 32);
    cr_assert(d_block->header.padding_size == 0);

    cr_assert(d_footer->alloc == 0);
    cr_assert((d_footer->block_size << 4) == 32);
}

Test(sf_memsuite, Realloc_Size_Greater_Than_Old_BlockSize_Malloc, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_malloc(4);
    void* d = sf_malloc(4);
    sf_malloc(4);
    void* g = sf_realloc(d, 64);

    cr_assert(d != g);
    sf_free_header* po = (sf_free_header*) (g - 8);
    sf_free_header* free_d = (sf_free_header*) (d- 8);

    cr_assert(free_d->header.alloc == 0);

    cr_assert((po->header.block_size << 4) == 80);
    cr_assert(po->header.alloc == 1);

}
Test(sf_memsuite, Free_Invalid_Inputs, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* g = sf_malloc(12);
    sf_free(g + 8);
    sf_free(NULL);
    cr_assert(errno == EFAULT);
}
Test(sf_memsuite, Realloc_Free_Block, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_malloc(4);
    sf_malloc(15);
    sf_malloc(40);
    void* d = sf_malloc(4);

    sf_free(d);
    sf_free(d);
    void* p = sf_realloc(d, 5000);
    cr_assert(p == NULL);
}
Test(sf_memsuite, Realloc_4_Pages, .init = sf_mem_init, .fini = sf_mem_fini) {
   void* d = sf_malloc(4);
   sf_realloc(d, 16368);
   cr_assert(d != NULL);
   sf_realloc(d, 45);
   cr_assert(d != NULL);
}
Test(sf_memsuite, Free_Free_Block, .init = sf_mem_init, .fini = sf_mem_fini) {
    void* d = sf_malloc(4);
    sf_free(d);
    sf_free(d);
    cr_assert(errno == EFAULT);
}
Test(sf_memsuite, MemInfo_Test, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_malloc(4);
    void* y = sf_malloc(4);

    info inf;
    inf.allocations = 0;
    inf.frees = 0;
    inf.coalesce = 0;
    sf_info(&inf);

    cr_assert(inf.allocations == 2);
    cr_assert(inf.frees == 0);

    cr_assert(inf.coalesce == 0);

    sf_free(y);

    sf_info(&inf);

    cr_assert(inf.allocations == 2);
    cr_assert(inf.frees == 1);
    cr_assert(inf.coalesce == 1);
}
Test(sf_memsuite, MemInfo_Test_Internal_External, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_malloc(4);
    void* y = sf_malloc(4);

    info inf;
    inf.internal = 0;
    inf.external = 0;
    sf_info(&inf);

    cr_assert(inf.internal == 56);
    cr_assert(inf.external == 4016);

    sf_free(y);

    sf_info(&inf);

    cr_assert(inf.internal == 28);
    cr_assert(inf.external == 4048);
}
Test(sf_memsuite, MemInfo_Test_Internal_External_2Lists, .init = sf_mem_init, .fini = sf_mem_fini) {
    sf_malloc(4);
    void* x = sf_malloc(4);
    sf_malloc(4);
    void* y = sf_malloc(4);

    info inf;
    inf.internal = 0;
    inf.external = 0;
    sf_info(&inf);

    cr_assert(inf.internal == 112);
    cr_assert(inf.external == 3952);

    sf_free(y);
    sf_free(x);

    sf_info(&inf);

    cr_assert(inf.internal == 56);

    cr_assert(inf.external == 4000);
}