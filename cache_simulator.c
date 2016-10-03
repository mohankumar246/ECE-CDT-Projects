#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "cache.h"

#if DEBUG_CACHE
int ig_count_opns;
#endif
int main(int argc,
    char *argv[])
{
    int i_exit_flag = 0;
    s_input_params s_ip_prms_t;
    FILE *fp_trace;
    char *filename;

    if (argc == 8)
    {
        printf("===== Simulator configuration =====\n");
        printf("BLOCKSIZE:                   %s\n", argv[1]);
        printf("L1_SIZE:                     %s\n", argv[2]);
        printf("L1_ASSOC:                    %s\n", argv[3]);
        printf("VC_NUM_BLOCKS:               %s\n", argv[4]);
        printf("L2_SIZE:                     %s\n", argv[5]);
        printf("L2_ASSOC:                    %s\n", argv[6]);
        printf("trace_file:                  %s\n", argv[7]);
        s_ip_prms_t.i_blk_size = strtol(argv[1], NULL, 10);
        s_ip_prms_t.i_l1_size = strtol(argv[2], NULL, 10);
        s_ip_prms_t.i_l1_assoc = strtol(argv[3], NULL, 10);
        s_ip_prms_t.i_vc_num_blks = strtol(argv[4], NULL, 10);
        s_ip_prms_t.i_l2_size = strtol(argv[5], NULL, 10);
        s_ip_prms_t.i_l2_assoc = strtol(argv[6], NULL, 10);

        filename = argv[7];
        fp_trace = fopen(filename, "rb");
        if (fp_trace == NULL)
        {
            printf("Failed to open the trace file!!\n");
            i_exit_flag = 1;
        }

        /*Check the parameters*/
        if ((s_ip_prms_t.i_blk_size != 0) && (s_ip_prms_t.i_blk_size  & (s_ip_prms_t.i_blk_size  - 1)))
        {
            printf("Block size not a power of 2");
            i_exit_flag = 1;
        }

        if (s_ip_prms_t.i_l1_size <= 0)
        {
            printf("L1 size cannot be less than or equal to 0\n");
            i_exit_flag = 1;
        }

        if (s_ip_prms_t.i_l2_size < 0)
        {
            printf("L2 size cannot be less than 0\n");
            i_exit_flag = 1;
        }

        if (s_ip_prms_t.i_l1_assoc <= 0)
        {
            printf("L1 assoc cannot be less than or equal 0\n");
            i_exit_flag = 1;
        }

        if (s_ip_prms_t.i_l2_assoc < 0)
        {
            printf("L2 assoc cannot be less than 0\n");
            i_exit_flag = 1;
        }

        if (s_ip_prms_t.i_vc_num_blks < 0)
        {
            printf("VC num blks cannot be less than 0\n");
            i_exit_flag = 1;
        }

        s_ip_prms_t.i_l1_sets = s_ip_prms_t.i_l1_size / (s_ip_prms_t.i_l1_assoc * s_ip_prms_t.i_blk_size);

        if ((s_ip_prms_t.i_l1_sets != 0) && (s_ip_prms_t.i_l1_sets  & (s_ip_prms_t.i_l1_sets  - 1)))
        {
            printf("#Sets in L1 is not a power of  2\n");
            i_exit_flag = 1;
        }

        if (s_ip_prms_t.i_l2_size > 0)
        {
            s_ip_prms_t.i_l2_sets = s_ip_prms_t.i_l2_size / (s_ip_prms_t.i_l2_assoc * s_ip_prms_t.i_blk_size);
            
       	    if ((s_ip_prms_t.i_l2_sets != 0) && (s_ip_prms_t.i_l2_sets  & (s_ip_prms_t.i_l2_sets  - 1)))
	    {
                printf("#Sets in L2 is a non multiple of 2\n");
                i_exit_flag = 1;
            }
        }
        else
        {
            s_ip_prms_t.i_l2_sets = 0;
        }

    }
    else
    {
        printf("The command had no other arguments.\n");
        printf("<exe> <Blocksize> <L1_size> <L1_assoc> <VC_num_blocks> <L2_size> <L2_assoc> <trace_file>\n");
        i_exit_flag = 1;
    }

    if (i_exit_flag == 0)
    {
        s_cache_g *ps_s_L1_cache = NULL;

        /*Memory Init*/
        memory_init((void **)&ps_s_L1_cache,
            (void *)&s_ip_prms_t);


        /*Call the looping*/
        loop_cache_trace((void *)ps_s_L1_cache,
            fp_trace);

        /*Print the results*/
        collate_print_output_stats((void *)ps_s_L1_cache);

        /*Memory Delete*/
        memory_delete((void *)ps_s_L1_cache);

        fclose(fp_trace);
    }
    else
    {
        printf("Enter a Key to exit:");
        getchar();
    }
    return 0;
}

void memory_init(void**		ppv_s_L1_cache,
    void*    pv_s_ip_prms_t)
{
    s_cache_g			**pps_L1_cache = (s_cache_g**)(ppv_s_L1_cache), *ps_L1_cache=NULL;
    s_cache_g*			ps_L2_cache;
    s_input_params*    ps_ip_prms_t = (s_input_params*)(pv_s_ip_prms_t);
    int j_count,k_count, i_mem_fr_blocks;
    s_cache_g* ps_cache_temp;
    s_set* ps_set_temp;
    s_block *ps_block_temp;
    /*Fill the cache structure values*/
    if (ps_ip_prms_t->i_l1_size > 0)
    {
        *pps_L1_cache = malloc(sizeof(s_cache_g));
        ps_L1_cache = *pps_L1_cache;
        assert(pps_L1_cache != NULL);
        set_cache_default((void *)ps_L1_cache);

        ps_L1_cache->i_size = ps_ip_prms_t->i_l1_size;
        ps_L1_cache->i_assoc = ps_ip_prms_t->i_l1_assoc;
        ps_L1_cache->i_level_id = 1;
        ps_L1_cache->i_num_sets = ps_ip_prms_t->i_l1_sets;
        ps_L1_cache->i_blk_size = ps_ip_prms_t->i_blk_size;

        if (ps_ip_prms_t->i_vc_num_blks > 0)
        {
            s_cache_g *ps_victim_cache;
            ps_L1_cache->pv_s_cache_victim_cache = malloc(sizeof(s_cache_g));
            set_cache_default(ps_L1_cache->pv_s_cache_victim_cache);
            ps_victim_cache = (s_cache_g*)ps_L1_cache->pv_s_cache_victim_cache;

            ps_victim_cache->i_assoc = ps_ip_prms_t->i_vc_num_blks;
            ps_victim_cache->i_blk_size = ps_ip_prms_t->i_blk_size;
            ps_victim_cache->i_level_id = 11;
            ps_victim_cache->i_num_sets = 1;
            ps_victim_cache->i_size = ps_ip_prms_t->i_vc_num_blks * ps_victim_cache->i_blk_size;
        }
    }

    if (ps_ip_prms_t->i_l2_size > 0)
    {
        ps_L2_cache = malloc(sizeof(s_cache_g));
        ps_L1_cache->pv_next_level_cache = (void *)ps_L2_cache;
        assert(ps_L2_cache != NULL);

        set_cache_default((void *)ps_L2_cache);

        ps_L2_cache->i_size = ps_ip_prms_t->i_l2_size;
        ps_L2_cache->i_assoc = ps_ip_prms_t->i_l2_assoc;
        ps_L2_cache->i_level_id = 2;
        ps_L2_cache->i_num_sets = ps_ip_prms_t->i_l2_sets;
        ps_L2_cache->i_blk_size = ps_ip_prms_t->i_blk_size;

        if (ps_L1_cache->pv_s_cache_victim_cache != NULL)
        {
            s_cache_g *ps_L1_victim_cache = (s_cache_g *)ps_L1_cache->pv_s_cache_victim_cache;
            ps_L1_victim_cache->pv_next_level_cache = (void*)ps_L2_cache;
        }
    }

    /*Allocate memory for cache L1/ L2 */
    ps_cache_temp = ps_L1_cache;
    while (1)
    {
        /*Allocate for sets*/
        ps_cache_temp->pv_s_set = malloc(ps_cache_temp->i_num_sets*sizeof(s_set));
        assert(ps_cache_temp->pv_s_set != NULL);

        /*Allocate for output statistics*/
        ps_cache_temp->pv_out_stats = malloc(sizeof(s_out_stats));
        assert(ps_cache_temp->pv_out_stats != NULL);
        memset(ps_cache_temp->pv_out_stats, 0, sizeof(s_out_stats));

        /*Allocate for blocks*/
        assert(ps_cache_temp->i_num_sets >= 1);
        ps_set_temp = (s_set *)ps_cache_temp->pv_s_set;
        i_mem_fr_blocks = sizeof(s_block) * ps_cache_temp->i_assoc;

        for (j_count = 0; j_count < ps_cache_temp->i_num_sets; j_count++)
        {
            ps_set_temp->i_set_num = j_count;
            /*For Blocks*/
            ps_set_temp->pv_s_block = malloc(i_mem_fr_blocks);
            memset(ps_set_temp->pv_s_block, 0, i_mem_fr_blocks);
            ps_block_temp = (s_block *)ps_set_temp->pv_s_block;
            for (k_count = 0; k_count < ps_cache_temp->i_assoc; k_count++)
            {

                ps_block_temp->i_priority_num = ps_cache_temp->i_assoc;
                ps_block_temp++;
            }
            assert(ps_set_temp->pv_s_block != NULL);
            ps_set_temp++;
        }

        if (ps_cache_temp->pv_s_cache_victim_cache != NULL)
            ps_cache_temp = ps_cache_temp->pv_s_cache_victim_cache;
        else if (ps_cache_temp->pv_next_level_cache != NULL)
            ps_cache_temp = ps_cache_temp->pv_next_level_cache;
        else
            break;

    }
}

void set_cache_default(void *pv_s_cache)
{
    s_cache_g* ps_s_cache = (s_cache_g *)pv_s_cache;
    ps_s_cache->i_assoc = 0;
    ps_s_cache->i_blk_size = 0;
    ps_s_cache->i_level_id = 0;
    ps_s_cache->i_num_sets = 0;
    ps_s_cache->i_size = 0;
    ps_s_cache->pv_next_level_cache = NULL;
    ps_s_cache->pv_out_stats = NULL;
    ps_s_cache->pv_s_cache_victim_cache = NULL;
    ps_s_cache->pv_s_set = NULL;
}


/*Memory freeing of the data structures*/
void memory_delete(void*			pv_s_L1_cache)
{
    s_cache_g *ps_L1_cache = (s_cache_g *)pv_s_L1_cache;

    int j_count;
    s_cache_g *ps_cache_temp, *ps_cache_temp2;
    s_set* ps_set_temp;

    /*Free memory from cache L1/ L2 */
    ps_cache_temp = ps_L1_cache;
    while (1)
    {
        ps_set_temp = (s_set*)ps_cache_temp->pv_s_set;
        for (j_count = 0; j_count < ps_cache_temp->i_num_sets; j_count++)
        {

            assert(ps_set_temp->pv_s_block != NULL);
            free(ps_set_temp->pv_s_block);
            ps_set_temp++;
        }

        assert(ps_cache_temp->pv_s_set != NULL);
        free(ps_cache_temp->pv_s_set);

        assert(ps_cache_temp->pv_out_stats != NULL);
        free(ps_cache_temp->pv_out_stats);

        if (ps_cache_temp->pv_s_cache_victim_cache != NULL)
        {
            ps_cache_temp2 = ps_cache_temp;
            ps_cache_temp = ps_cache_temp->pv_s_cache_victim_cache;
            free(ps_cache_temp2);
        }
        else if (ps_cache_temp->pv_next_level_cache != NULL)
        {
            ps_cache_temp2 = ps_cache_temp;
            ps_cache_temp = ps_cache_temp->pv_next_level_cache;
            free(ps_cache_temp2);
        }
        else
        {
            free(ps_cache_temp);
            break;
        }
    }
}


void loop_cache_trace(void *pv_s_L1_cache,
    FILE *fp_trace)
{
    char s_line[128], s_address[15], c_r_or_w;
    int i_count, i_address, i_read_or_write = 0;

#if DEBUG_CACHE
    printf("#count cache_l R-1/W-2 \t address \tsetnum \tblocktag \tdirty_bit  evicted block evict_dirty_b\n");
#endif

    while (fgets(s_line, sizeof s_line, fp_trace) != NULL)
    {
        i_count = 0;
        i_address = -1;

        while (s_line[i_count] == 'w' || s_line[i_count] == 'r' || s_line[i_count] == ' ')
            i_count++;

        //strcpy(s_address, s_line[i_count]);
        strncpy(s_address, s_line + i_count, strlen(s_line) - i_count);
        c_r_or_w = s_line[0];
        i_address = strtol(s_address, NULL, 16);

        if (c_r_or_w == 'w' || c_r_or_w == 'W')
            i_read_or_write = WRITE;
        else
            i_read_or_write = READ;

        /*Call the generic cache core to perform the opns recursively*/
        cache_read_write(pv_s_L1_cache, i_address, i_read_or_write);

        memset(s_line, 0, sizeof(s_line));
    }
}

void cache_read_write(void *pv_s_cache,
    int i_address,
    int i_read_or_write)
{

    s_cache_g *ps_cache_g = (s_cache_g *)pv_s_cache;
    s_out_stats *ps_out_stats_t = (s_out_stats *)ps_cache_g->pv_out_stats;
    s_set* ps_set_temp = (s_set*)ps_cache_g->pv_s_set;

    s_block* ps_block_temp = (s_block*)ps_set_temp->pv_s_block; 
#if DEBUG_CACHE
    s_block *ps_block_temp2_for_debug;
    int i_evicted_block_address = 0, i_evicted_block_dirty_bit = 0;
#endif
    int i_set_num, i_count, i_cache_set_block_tag;
    int i_read = 0, i_write = 0, i_read_miss = 0, i_write_miss = 0;
    int i_block_tag = 0, i_bits_for_sets_block_off = 0;
    int i_write_back = 0, i_swap_req = 0, i_swap = 0;
    int i_num_bits_for_set = (int)log2((double)ps_cache_g->i_num_sets);
    int i_num_bits_for_blk = (int)log2((double)ps_cache_g->i_blk_size);
    int i_VC_read_hit = 0;

#if DEBUG_CACHE    
    if (ps_cache_g->i_level_id == 1)
        printf("----------------------------------------------------------------------------------------------\n");
    if (ps_cache_g->i_level_id == 1)
        ig_count_opns++;
#endif

    /*Separate the set num and tag*/
    //if (i_address > -1)
    {
        i_set_num = (i_address >> i_num_bits_for_blk) & (ps_cache_g->i_num_sets - 1);
        i_bits_for_sets_block_off = (i_num_bits_for_set + i_num_bits_for_blk);
        i_block_tag = i_address >> i_bits_for_sets_block_off;

        /*Go to the set num*/
        assert(i_set_num < ps_cache_g->i_num_sets);
        ps_set_temp = (s_set*)ps_cache_g->pv_s_set;
        ps_set_temp = ps_set_temp + i_set_num;

        /*Search for the tag and valid bit*/
        for (i_count = 0; i_count < ps_cache_g->i_assoc; i_count++)
        {
            ps_block_temp = (s_block*)ps_set_temp->pv_s_block + i_count;
            /*No need to search the blocks once we find a block with invalid status*/
            if (ps_block_temp->i_valid_bit == 0)
                break;

            if (ps_block_temp->i_valid_bit == 1)
            {
                i_cache_set_block_tag = ps_block_temp->i_block_address >> i_bits_for_sets_block_off;
                if (i_block_tag == i_cache_set_block_tag)
                {
                    if (i_read_or_write == READ)
                    {
                        i_read = 1;
                        break;
                    }
                    else
                    {
                        i_write = 1;
                        ps_block_temp->i_dirty_bit = 1;
                        break;
                    }
                }
            }
        }


        if ((i_read != 1) && (i_write != 1))
        {
            (i_read_or_write == READ) ? (i_read_miss = 1) : (i_write_miss = 1);
            /*Space is there or not in the set*/
            /*If not we have to evict some one to make space*/
            /*That can have write requests to VC or lower mem/cache*/

            if (ps_block_temp->i_valid_bit != 0)
            {
                /*Search for the LRU Block*/
                ps_block_temp = (s_block*)ps_set_temp->pv_s_block;
                for (i_count = 0; i_count < ps_cache_g->i_assoc; i_count++)
                {
                    if (ps_block_temp->i_priority_num == ps_cache_g->i_assoc)
                    {
                        break;
                    }
                    ps_block_temp++;
                }

                /*If the selected block is dirty it has to be written back to lower mem*/
                if (ps_block_temp->i_dirty_bit == 1)
                    i_write_back = 1;
            }

            /*Search for VC if enabled*/
            if (ps_cache_g->pv_s_cache_victim_cache != NULL)
            {
                /*Searching evicting etc*/
                if (ps_block_temp->i_valid_bit == 1)
                {
                    s_block s_read_block, s_write_block;
                    memmove(&s_write_block, ps_block_temp, sizeof(s_block));
                    s_read_block.i_block_address = i_address;
                    s_read_block.i_valid_bit = 1;

                    vc_cache_read_write(ps_cache_g->pv_s_cache_victim_cache, &s_read_block, &s_write_block);

                    if (s_read_block.i_valid_bit == 1)
                    {
                        i_VC_read_hit = 1;
                        ps_block_temp->i_dirty_bit = s_read_block.i_dirty_bit;
                    }
                    else
                    {
                        if (ps_cache_g->pv_next_level_cache != NULL)
                            cache_read_write(ps_cache_g->pv_next_level_cache, i_address, READ);
                    }
                }
                else
                {
                    if (ps_cache_g->pv_next_level_cache != NULL)
                        cache_read_write(ps_cache_g->pv_next_level_cache, i_address, READ);
                }
            }
            else if (ps_cache_g->pv_next_level_cache != NULL)
            {
                /*Incase of a write back we should write request lower memory*/

                if (i_write_back == 1)
                    cache_read_write(ps_cache_g->pv_next_level_cache, ps_block_temp->i_block_address, WRITE);

                cache_read_write(ps_cache_g->pv_next_level_cache, i_address, READ);
            }

            /*Once the eviction has happened replace the block address*/
            {

#if DEBUG_CACHE                
                i_evicted_block_address = ps_block_temp->i_block_address;
                i_evicted_block_dirty_bit = ps_block_temp->i_dirty_bit;
#endif
                if ((i_VC_read_hit == 0) && (i_read_or_write == READ))
                    ps_block_temp->i_dirty_bit = 0;

                if (i_read_or_write == WRITE)
                    ps_block_temp->i_dirty_bit = 1;

                (i_read_or_write == READ) ? (i_read = 1) : (i_write = 1);
                ps_block_temp->i_block_address = i_address;

            }
        }

        {
            /*Update the LRU list*/
            int i_cur_blks_prev_priority_num = ps_block_temp->i_priority_num;
            ps_block_temp->i_priority_num = 1;
            ps_block_temp->i_valid_bit = 1;
#if DEBUG_CACHE
            ps_block_temp2_for_debug = ps_block_temp;
#endif
            ps_block_temp = (s_block*)ps_set_temp->pv_s_block;
            for (i_count = 0; i_count < ps_cache_g->i_assoc; i_count++)
            {
                /*Increase the priority num for the blks whose num is lesser than that of the current */
                if ((ps_block_temp->i_priority_num < i_cur_blks_prev_priority_num) &&
                    ((ps_block_temp->i_block_address >> i_bits_for_sets_block_off) != i_block_tag))
                {
                    ps_block_temp->i_priority_num++;
                    assert((ps_block_temp->i_priority_num <= ps_cache_g->i_assoc) ||
                        (ps_block_temp->i_priority_num > 0));
                }
                ps_block_temp++;
            }
        }
    }
    /*Update the output statistics*/

    if (i_read)
        ps_out_stats_t->i_num_reads++;
    if (i_read_miss)
        ps_out_stats_t->i_num_read_misses++;
    if (i_write)
        ps_out_stats_t->i_num_writes++;
    if (i_write_miss)
        ps_out_stats_t->i_num_write_misses++;
    if (i_write_back)
        ps_out_stats_t->i_num_writebacks++;
    if (i_swap_req)
        ps_out_stats_t->i_num_swap_req++;
    if (i_swap)
        ps_out_stats_t->i_no_of_swaps++;

#if DEBUG_CACHE


    printf("#%d\t%d\t%d\t%9x\t%d\t%9x\t%d\t%9x\t%d\n", ig_count_opns, ps_cache_g->i_level_id,
        i_read_or_write, i_address, ps_set_temp->i_set_num,
        (ps_block_temp2_for_debug->i_block_address) >> i_bits_for_sets_block_off,
        ps_block_temp2_for_debug->i_dirty_bit, i_evicted_block_address, i_evicted_block_dirty_bit);


#endif


}

void vc_cache_read_write(void *pv_s_vc_cache, void *pv_s_read_block, void *pv_s_write_block)
{
    s_cache_g *ps_vc_cache = (s_cache_g *)pv_s_vc_cache;
    s_block *ps_read_block = (s_block *)pv_s_read_block;
    s_block *ps_write_block = (s_block *)pv_s_write_block;
    s_block *ps_block_temp;
    s_set *ps_set_temp;
    s_out_stats *ps_out_stats_t = (s_out_stats *)ps_vc_cache->pv_out_stats;
    int i_count, i_cache_set_block_tag;
    int i_read = 0, i_write = 0, i_read_miss = 0;
    int i_write_back = 0, i_swap_req = 0, i_swap = 0, i_write_miss=0;

    int i_num_bits_for_blk = (int)log2((double)ps_vc_cache->i_blk_size);
    int i_read_block_tag,i_write_block_tag = ps_write_block->i_block_address >> i_num_bits_for_blk;
    ps_set_temp = (s_set*)ps_vc_cache->pv_s_set;
    ps_block_temp = (s_block*)ps_set_temp->pv_s_block; 

    assert(ps_write_block->i_valid_bit == 1);

    if (ps_read_block->i_valid_bit == 1)
    {
        i_swap_req = 1;
        i_read_block_tag = ps_read_block->i_block_address >> i_num_bits_for_blk;
        /*Search for the tag and valid bit*/
        
        for (i_count = 0; i_count < ps_vc_cache->i_assoc; i_count++)
        {
            ps_block_temp = (s_block*)ps_set_temp->pv_s_block + i_count;
            /*No need to search the blocks once we find a block with invalid status*/
            if (ps_block_temp->i_valid_bit == 0)
                break;

            if (ps_block_temp->i_valid_bit == 1)
            {
                i_cache_set_block_tag = ps_block_temp->i_block_address >> i_num_bits_for_blk;
                if (i_read_block_tag == i_cache_set_block_tag)
                {
                    i_read = 1;
                    break;
                }
            }
        }
    }

    if (i_read == 1)
    {
        s_block s_block_temp_swap;

        i_swap = 1;

        memmove(&s_block_temp_swap, ps_block_temp, sizeof(s_block));

        memmove(ps_block_temp, ps_write_block, sizeof(s_block));
        ps_block_temp->i_priority_num = s_block_temp_swap.i_priority_num;

        ps_read_block->i_dirty_bit = s_block_temp_swap.i_dirty_bit;
        ps_read_block->i_priority_num = ps_write_block->i_priority_num;
        ps_read_block->i_valid_bit = 1;

    }
    else
    {
        i_read_miss = 1;
        ps_read_block->i_valid_bit = 0;
        for (i_count = 0; i_count < ps_vc_cache->i_assoc; i_count++)
        {
            ps_block_temp = (s_block*)ps_set_temp->pv_s_block + i_count;
            /*No need to search the blocks once we find a block with invalid status*/
            if (ps_block_temp->i_valid_bit == 0)
                break;

            if (ps_block_temp->i_priority_num == ps_vc_cache->i_assoc)
                break;

        }

        if (ps_block_temp->i_dirty_bit == 1)
        {
            /*Issue a WB*/
            i_write_back = 1;
            if (ps_vc_cache->pv_next_level_cache != NULL)
                cache_read_write(ps_vc_cache->pv_next_level_cache, ps_block_temp->i_block_address, WRITE);
        }

        /*Write*/
        ps_block_temp->i_block_address = ps_write_block->i_block_address;
        ps_block_temp->i_dirty_bit = ps_write_block->i_dirty_bit;

    }

    {
        /*Update the LRU list*/
        int i_cur_blks_prev_priority_num = ps_block_temp->i_priority_num;
        ps_block_temp->i_priority_num = 1;
        ps_block_temp->i_valid_bit = 1;
        ps_block_temp = (s_block*)ps_set_temp->pv_s_block;
        for (i_count = 0; i_count < ps_vc_cache->i_assoc; i_count++)
        {
            /*Increase the priority num for the blks whose num is lesser than that of the current */
            if ((ps_block_temp->i_priority_num < i_cur_blks_prev_priority_num) &&
                ((ps_block_temp->i_block_address >> i_num_bits_for_blk) != i_write_block_tag))
            {
                ps_block_temp->i_priority_num++;
                assert((ps_block_temp->i_priority_num <= ps_vc_cache->i_assoc) ||
                    (ps_block_temp->i_priority_num > 0));
            }
            ps_block_temp++;
        }
    }


    if (i_read)
        ps_out_stats_t->i_num_reads++;
    if (i_read_miss)
        ps_out_stats_t->i_num_read_misses++;
    if (i_write)
        ps_out_stats_t->i_num_writes++;
    if (i_write_miss)
        ps_out_stats_t->i_num_write_misses++;
    if (i_write_back)
        ps_out_stats_t->i_num_writebacks++;
    if (i_swap_req)
        ps_out_stats_t->i_num_swap_req++;
    if (i_swap)
        ps_out_stats_t->i_no_of_swaps++;

}

void collate_print_output_stats(void *pv_s_L1_cache)
{
    s_cache_g *ps_L1_cache = (s_cache_g *)pv_s_L1_cache;
    s_cache_g *ps_cache_temp = NULL;
    s_out_stats *ps_out_stats;
    int i_max_width = 15;
    int i_L1_reads = 0, i_L1_read_miss = 0, i_L1_writes = 0, i_L1_write_misses = 0, i_swap_requests = 0, i_num_swaps = 0;
    int i_L1_write_backs = 0, i_L2_reads = 0, i_L2_read_misses = 0, i_L2_writes = 0, i_L2_write_misses = 0, i_L2_writebacks = 0;

    int i_total_memory_traffic = 0;
    float f_swap_req_rate = 0.0f;
    float f_L2_miss_rate = 0.0f;
    float f_L1_VC_miss_rate = 0.0f;

    printf("\n===== L1 contents =====\n");

    output_cache_contents(pv_s_L1_cache);

    ps_out_stats = (s_out_stats*)ps_L1_cache->pv_out_stats;
    i_L1_reads = ps_out_stats->i_num_reads;
    i_L1_read_miss = ps_out_stats->i_num_read_misses;
    i_L1_writes = ps_out_stats->i_num_writes;
    i_L1_write_misses = ps_out_stats->i_num_write_misses;
    i_L1_write_backs = ps_out_stats->i_num_writebacks;
    i_total_memory_traffic = i_L1_write_misses + i_L1_read_miss + i_L1_write_backs;

    if (ps_L1_cache->pv_s_cache_victim_cache != NULL)
    {
        printf("\n===== VC contents =====\n");

        output_cache_contents(ps_L1_cache->pv_s_cache_victim_cache);

        ps_cache_temp = (s_cache_g*)ps_L1_cache->pv_s_cache_victim_cache;
        ps_out_stats = (s_out_stats*)ps_cache_temp->pv_out_stats;
        i_swap_requests = ps_out_stats->i_num_swap_req;
        i_num_swaps = ps_out_stats->i_no_of_swaps;
        f_swap_req_rate = (float)i_swap_requests / (i_L1_reads + i_L1_writes);
        
        i_L1_write_backs = ps_out_stats->i_num_writebacks;
        i_total_memory_traffic = i_L1_write_misses + i_L1_read_miss + i_L1_write_backs- i_num_swaps;
    }
        
    f_L1_VC_miss_rate = (float)(i_L1_read_miss + i_L1_write_misses - i_num_swaps) / (i_L1_reads + i_L1_writes);

    if (ps_L1_cache->pv_next_level_cache != NULL)
    {
        printf("\n===== L2 contents =====\n");

        output_cache_contents(ps_L1_cache->pv_next_level_cache);

        ps_cache_temp = (s_cache_g*)ps_L1_cache->pv_next_level_cache;
        ps_out_stats = (s_out_stats*)ps_cache_temp->pv_out_stats;
        i_L2_reads = ps_out_stats->i_num_reads;
        i_L2_read_misses = ps_out_stats->i_num_read_misses;
        i_L2_writes = ps_out_stats->i_num_writes;
        i_L2_write_misses = ps_out_stats->i_num_write_misses;
        i_L2_writebacks = ps_out_stats->i_num_writebacks;

        f_L2_miss_rate = (float)i_L2_read_misses / i_L2_reads;

        i_total_memory_traffic = i_L2_write_misses + i_L2_read_misses + i_L2_writebacks;

    }
    /*Calculate the rates*/


    printf("\n===== Simulation results =====\n");
    printf("  a. number of L1 reads:           %*d\n", i_max_width, i_L1_reads);
    printf("  b. number of L1 read misses:     %*d\n", i_max_width, i_L1_read_miss);
    printf("  c. number of L1 writes:          %*d\n", i_max_width, i_L1_writes);
    printf("  d. number of L1 write misses:    %*d\n", i_max_width, i_L1_write_misses);
    printf("  e. number of swap requests:      %*d\n", i_max_width, i_swap_requests);
    printf("  f. swap request rate:            %*.4f\n", i_max_width, f_swap_req_rate);
    printf("  g. number of swaps:              %*d\n", i_max_width, i_num_swaps);
    printf("  h. combined L1+VC miss rate:     %*.4f\n", i_max_width, f_L1_VC_miss_rate);
    printf("  i. number writebacks from L1/VC: %*d\n", i_max_width, i_L1_write_backs);
    printf("  j. number of L2 reads:           %*d\n", i_max_width, i_L2_reads);
    printf("  k. number of L2 read misses:     %*d\n", i_max_width, i_L2_read_misses);
    printf("  l. number of L2 writes:          %*d\n", i_max_width, i_L2_writes);
    printf("  m. number of L2 write misses:    %*d\n", i_max_width, i_L2_write_misses);
    printf("  n. L2 miss rate:                 %*.4f\n", i_max_width, f_L2_miss_rate);
    printf("  o. number of writebacks from L2: %*d\n", i_max_width, i_L2_writebacks);
    printf("  p. total memory traffic:         %*d\n", i_max_width, i_total_memory_traffic);

}
void output_cache_contents(void * pv_s_cache)
{
    s_cache_g *ps_cache_g = (s_cache_g *)pv_s_cache;
    s_block *ps_block_temp;
    s_set *ps_set_temp;
    int i_count, j_count, k_count, i_requ_priority = 1;
    int i_shift_bits = 0;
    char c_dirty;
    i_shift_bits = (int)(log2(ps_cache_g->i_num_sets) + log2(ps_cache_g->i_blk_size));

    ps_set_temp = (s_set*)ps_cache_g->pv_s_set;
    ps_block_temp = (s_block*)ps_set_temp->pv_s_block;

    for (i_count = 0; i_count < ps_cache_g->i_num_sets; i_count++)
    {
        i_requ_priority = 1;
        printf("set%4d:", i_count);
        /*Search for the LRU Block*/
        for (j_count = 0; j_count < ps_cache_g->i_assoc; j_count++)
        {
            ps_block_temp = (s_block*)ps_set_temp->pv_s_block;

            for (k_count = 0; k_count < ps_cache_g->i_assoc; k_count++)
            {
                if ((ps_block_temp->i_valid_bit == 0) || (ps_block_temp->i_priority_num != i_requ_priority))
                {
                    ps_block_temp++;
                    continue;
                }

                if (ps_block_temp->i_dirty_bit == 1)
                    c_dirty = 'D';
                else
                    c_dirty = ' ';

                printf("%9x %c", (ps_block_temp->i_block_address) >> i_shift_bits, c_dirty);
                ps_block_temp->i_valid_bit = 0;

                ps_block_temp++;
            }
            i_requ_priority++;
        }
        printf("\n");
        ps_set_temp++;
    }
}
