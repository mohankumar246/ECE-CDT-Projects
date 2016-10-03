#define MAX_REG 67
#define VALID 1
#define INVALID 0
class num_cycles {
public:
    int i_priority;
    int i_fetch_cycles;
    int i_fetch_cycle_start;
    int i_dec_cycles;
    int i_renam_cycles;
    int i_reg_r_cycles;
    int i_disp_cycles;
    int i_iss_cycles;
    int i_exec_cycles;
    int i_wb_cycles;
    int i_rt_cycles;
};

class Pipeline_reg {
    int i_src_reg_1;
    int i_src_1_rmt_flag;
    int i_src_reg_2;
    int i_src_2_rmt_flag;
    int i_dest_reg;
    int i_dest_rob_flag;
    int i_op;
    int i_pc;
public:
    Pipeline_reg();
    void write_inst(int pc, int op, int dest, int src1, int src2, int i_fe_cy,int i_fe_cycle_start, int i_dec_cycles, int i_renam_cycles, int i_reg_r_cycles
        , int i_disp_cycles, int i_iss_cycles, int i_exec_cycles, int i_wb_cycles, int i_rt_cycles,int i_priority_num);
    int read_src_1();
    int read_src_2();
    int read_dest();
    int read_op();
    int read_pc(); 
    void write_src_1(int);
    void write_src_2(int);
    void write_dest(int);
    void write_op(int);
    void write_pc(int);
    void set_src_1_rob_flag(int);
    void set_src_2_rob_flag(int);
    void set_dst_rob_flag();

    int read_src_1_rob_flag();
    int read_src_2_rob_flag();
    int read_dest_rob_flag();

    num_cycles ai_cycles;
    int i_empty_flag;
};

class reorder_buffer_elt{
    int value;
    int dst;
    int rdy;
    int exec;
    int mis;
    int pc;
public:
    reorder_buffer_elt();
    void write_value(int);
    void write_dst(int);
    void write_ready(int);
    void write_exec(int);
    void write_mis(int);
    void write_pc(int);
    int read_value();
    int read_dst();
    int read_ready();
    int read_exec();
    int read_mis();
    int read_pc();
    Pipeline_reg *p_pipe_line_reg;
};

class reorder_buffer_table{
public:
    reorder_buffer_elt* pa_reorder_elts;
    int i_head;
    int i_tail;
};

class rename_map_table{
    int ai_table[MAX_REG];
    int ai_valid[MAX_REG];
    public:
    rename_map_table();
    int read_table_value(int);
    void write_table(int index, int value);
    int read_valid(int index);
    void write_valid(int index, int value);
};


class issue_queue_table {
public:
    Pipeline_reg *pa_issue_queue_elt;
    int i_num_ins_in_queue;
    int i_q_size;
};

class cpu_memories{
public:
    Pipeline_reg  *pa_pipe_reg_DE,*pa_pipe_reg_RN,*pa_pipe_reg_RR,*pa_pipe_reg_DI;
    issue_queue_table *p_issue_q_table;
    Pipeline_reg **ppa_pipe_reg_WB, **ppa_pipe_reg_EX;
    reorder_buffer_table *p_rob_t;
    rename_map_table *p_rmt;
    int i_q_size,i_rob_size,i_width;
};

void memory_init(cpu_memories *p_cpu_mem);
void memory_delete(cpu_memories *p_cpu_mem);

void Retire(rename_map_table *p_rmt,
    reorder_buffer_table *p_rob_table,
    int i_rob_width,
    int i_width,
    int *pi_rob_count,
    int *i_dyn_ins_count);

void Writeback(Pipeline_reg **ppa_pipe_reg_WB,
    reorder_buffer_table *p_rob_table,
    int i_width,
    int i_rob_width,
    int *pi_wb_count);

void Execute(Pipeline_reg **ppa_pipe_reg_EX,
    Pipeline_reg **ppa_writeback,
    int i_width,
    int *pi_ex_count,
    int *pi_wb_count);

void Dispatch(Pipeline_reg *pa_dispatch,
    issue_queue_table *p_issue_q_table,
    Pipeline_reg **ppa_pipe_reg_WB,
    reorder_buffer_table *p_rob_table,
    int i_width,
    int i_q_size,
    int *pi_di_count);

void Issue(issue_queue_table *p_issue_q_table,
    Pipeline_reg **ppa_pipe_reg_EX,
    Pipeline_reg **ppa_pipe_reg_WB,
    reorder_buffer_table *p_rob_table,
    int i_width,
    int *pi_ex_count);

void Register_Read(Pipeline_reg *pa_reg_read,
    Pipeline_reg *pa_dispatch,
    Pipeline_reg **ppa_pipe_reg_WB,
    reorder_buffer_table *p_rob_table,
    int i_width,
    int *pi_reg_count,
    int *pi_di_count);

void Rename(Pipeline_reg *pa_rename,
    Pipeline_reg *pa_reg_read,
    rename_map_table *p_rmt,
    reorder_buffer_table *p_rob_table,
    Pipeline_reg **ppa_pipe_reg_WB,
    int i_width,
    int i_rob_width,
    int *pi_renm_count,
    int *pi_rr_read,
    int *pi_rob_count);

void Decode(Pipeline_reg *pa_decode,
    Pipeline_reg *pa_rename,
    int i_width,
    int *pi_dec_count,
    int *pi_renm_count);

void Fetch(std::ifstream *pFile,
    Pipeline_reg *pa_pipe_reg_DE,
    int i_width,
    int *pi_fet_count,
    int *pi_dec_count,
    int i_st_cycle,
    int *i_piority_num);

void print_contents(cpu_memories *p_cpu);