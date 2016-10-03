
#define MAX_NUM_BITS 32
#define MAX_VALUE 0xFFFFFFFF
#define READ 1
#define WRITE 2
#define DEBUG_CACHE 0
typedef struct s_input_params
{
    int i_blk_size; 
    int i_l1_size;
    int i_l1_assoc;
    int i_vc_num_blks;
    int i_l2_size;
    int i_l2_assoc;
    int i_l1_sets;
    int i_l2_sets;
}s_input_params;

typedef struct s_block
{
    int i_block_address;
    int i_valid_bit;
    int i_dirty_bit;
    int i_priority_num;
}s_block;

typedef struct s_set
{
    int i_set_num;
    void* pv_s_block;	 
}s_set;

typedef struct s_cache_g
{
    int i_size;
    int i_assoc;
    int i_blk_size;
    int i_level_id;
    int i_num_sets;
    void* pv_s_set; 
    void* pv_s_cache_victim_cache;
    void* pv_next_level_cache;
    void* pv_out_stats;
}s_cache_g;


typedef struct s_out_stats
{
    int i_num_reads;
    int i_num_read_misses;
    int i_num_writes;
    int i_num_write_misses;
    int i_num_writebacks;
    int i_num_swap_req;
    int i_no_of_swaps;    
}s_out_stats;


void output_cache_contents(void * pv_s_cache);
void collate_print_output_stats(void *pv_s_L1_cache);

void cache_read_write(void *pv_s_cache,
    int i_address,
    int i_read_or_write);

void loop_cache_trace(void *pv_s_L1_cache,
    FILE *fp_trace);

void memory_delete(void* pv_s_L1_cache);

void set_cache_default(void *pv_s_cache);

void memory_init(void** pv_s_L1_cache,
    void*    pv_s_ip_prms_t);

void vc_cache_read_write(void *pv_s_vc_cache, void *pv_s_read_block, void *pv_s_write_block);
