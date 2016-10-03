
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cstring>
#include <memory.h>
#include "dynamic_instruction_scheduling.h"

using namespace std;
void Pipeline_reg::write_inst(int pc,int op,int dest,int src1,int src2,int i_fe_cy,int i_fe_st_cycle,int i_dec_cycles,int i_renam_cycles, int i_reg_r_cycles
    , int i_disp_cycles, int i_iss_cycles, int i_exec_cycles, int i_wb_cycles, int i_rt_cycles,int i_priority_num)
{
    i_src_reg_1=src1;
    i_src_reg_2=src2;
    i_dest_reg=dest;
    i_op=op;
    i_pc=pc;
    i_empty_flag = 0;

    ai_cycles.i_fetch_cycles= i_fe_cy;
    ai_cycles.i_fetch_cycle_start = i_fe_st_cycle;
    ai_cycles.i_dec_cycles  =i_dec_cycles;
    ai_cycles.i_renam_cycles=i_renam_cycles;
    ai_cycles.i_reg_r_cycles=i_reg_r_cycles;
    ai_cycles.i_disp_cycles =i_disp_cycles;
    ai_cycles.i_iss_cycles  =i_iss_cycles;
    ai_cycles.i_exec_cycles =i_exec_cycles;
    ai_cycles.i_wb_cycles   =i_wb_cycles;
    ai_cycles.i_rt_cycles   =i_rt_cycles;
    ai_cycles.i_priority = i_priority_num;

    i_src_1_rmt_flag = 0;
    i_src_2_rmt_flag = 0;
}

Pipeline_reg::Pipeline_reg()
{
    i_src_reg_1=-1;
    i_src_reg_2=-1;
    i_dest_reg=-1;
    i_op=-1;
    i_pc=-1;
    i_src_1_rmt_flag=0;
    i_src_2_rmt_flag=0;
    i_dest_rob_flag=0;
    i_empty_flag = 1;
    memset(&ai_cycles, 0, sizeof(num_cycles));
}

int Pipeline_reg::read_src_1(){return i_src_reg_1;}
int Pipeline_reg::read_src_2(){return i_src_reg_2;}
int Pipeline_reg::read_dest(){return i_dest_reg;}
int Pipeline_reg::read_op(){return i_op;}
int Pipeline_reg::read_pc(){return i_pc;} 
void Pipeline_reg::set_src_1_rob_flag(int val) {i_src_1_rmt_flag=val ; }
void Pipeline_reg::set_src_2_rob_flag(int val) { i_src_2_rmt_flag = val; }

void Pipeline_reg::write_src_1(int val) { i_src_reg_1 = val; }
void Pipeline_reg::write_src_2(int val) { i_src_reg_2 = val; }
void Pipeline_reg::write_dest(int val) { i_dest_reg = val; }
void Pipeline_reg::write_op(int val) { i_op = val; }
void Pipeline_reg::write_pc(int val) { i_pc = val; }
void Pipeline_reg::set_dst_rob_flag() { i_dest_rob_flag = 1; }


int Pipeline_reg::read_src_1_rob_flag() { return i_src_1_rmt_flag; }
int Pipeline_reg::read_src_2_rob_flag() { return i_src_2_rmt_flag; }
int Pipeline_reg::read_dest_rob_flag() { return i_dest_rob_flag; }

reorder_buffer_elt::reorder_buffer_elt()
{
    value = 0;
    dst = -1;
    rdy = -1;
    exec = -1;
    mis = -1;
    pc = -1;
}

void reorder_buffer_elt::write_value(int v){value = v;}
void reorder_buffer_elt::write_dst(int dest){dst=dest;}
void reorder_buffer_elt::write_ready(int r){rdy=r;}
void reorder_buffer_elt::write_exec(int e){exec=e;}
void reorder_buffer_elt::write_mis(int m){mis=m;}
void reorder_buffer_elt::write_pc(int p){pc =p;}

int reorder_buffer_elt::read_value(){return value;}
int reorder_buffer_elt::read_dst(){return dst;}
int reorder_buffer_elt::read_ready(){return rdy;}
int reorder_buffer_elt::read_exec(){return exec;}
int reorder_buffer_elt::read_mis(){return mis;}
int reorder_buffer_elt::read_pc(){return pc;}



int rename_map_table::read_table_value(int index){
    assert(index >=0 && index <= MAX_REG);
    return ai_table[index];
}

rename_map_table::rename_map_table()
{
memset(ai_table,0,sizeof(int)*MAX_REG);
memset(ai_valid, 0, sizeof(int)*MAX_REG);
}

void rename_map_table::write_table(int index, int value) 
{ 
    assert(index >= 0 && index <= MAX_REG);
    ai_table[index] = value; 
}

int rename_map_table::read_valid(int index) 
{ 
    assert(index >= 0 && index <= MAX_REG); 
    return ai_valid[index]; 
}

void rename_map_table::write_valid(int index, int value) 
{ 
    assert(index >= 0 && index <= MAX_REG);
    ai_valid[index] = value; 
}


void Fetch(ifstream *pFile, 
    Pipeline_reg *pa_pipe_reg_DE,
    int i_width,
    int *pi_fet_count,
    int *pi_dec_count,
    int i_current_cycle,
    int *i_piority_num)
{
   string ac_line,s_pc;
   int i_pc,i_op_type,i_dest_reg,i_source_reg1,i_source_reg2,i_count=0;
   i_pc=i_op_type=i_dest_reg=i_source_reg1=i_source_reg2=-1;
   if(pFile->good())
   {
       *pi_fet_count = i_width;
       while (i_count < i_width)
       {
           if (pa_pipe_reg_DE[i_count].i_empty_flag != 1)
               break;
           i_count++;
       }
       
       if (i_count != i_width)
           return;

       i_count = 0;
       
       while ((i_count < i_width) && (pFile->good()))
       {
           getline(*pFile, ac_line);
           istringstream iss(ac_line);
           if (ac_line.size() > 2)
           {
               iss >> s_pc;
               iss >> i_op_type;
               iss >> i_dest_reg;
               iss >> i_source_reg1;
               iss >> i_source_reg2;
               
               i_pc = strtoul(s_pc.c_str(), NULL, 16);
               pa_pipe_reg_DE[i_count].write_inst(i_pc, i_op_type, 
                   i_dest_reg,i_source_reg1,i_source_reg2,1, 
                   i_current_cycle,0,0,0,0, 0, 0, 0, 0,*i_piority_num);
               pa_pipe_reg_DE[i_count].i_empty_flag = 0;
               i_count++;
               *i_piority_num = *i_piority_num + 1;
           }           
       }
       *pi_dec_count = 1;
       *pi_fet_count = 0;
   }
return;
}

void Decode(Pipeline_reg *pa_decode, 
    Pipeline_reg *pa_rename, 
    int i_width,
    int *pi_dec_count,
    int *pi_rename_count)
{
    int i_count = 0,i_num_dec=0;
    
    if(*pi_dec_count == 0)
        return;

    while (i_count < i_width)
    {
        if (pa_decode[i_count].i_empty_flag == 0)
        {
            pa_decode[i_count].ai_cycles.i_dec_cycles++;
            i_num_dec++;
        }
        i_count++;
        
    }

    i_count = 0;
    while (i_count < i_width)
    {
        if (pa_rename[i_count].i_empty_flag != 1)
            break;
        i_count++;
    }

    if (i_count != i_width)
        return;

    i_count = 0;
    
    while (i_count < i_num_dec)
    {
        pa_rename[i_count] = pa_decode[i_count];
        pa_decode[i_count].i_empty_flag = 1;
        i_count++;
    }
    *pi_dec_count = 0;
    *pi_rename_count = 1;
    return;
}

void Rename(Pipeline_reg *pa_rename,
    Pipeline_reg *pa_reg_read,
    rename_map_table *p_rmt,
    reorder_buffer_table *p_rob_table,
    Pipeline_reg **ppa_pipe_reg_WB,
    int i_width,
    int i_rob_width,
    int *pi_rename_count,
    int *pi_reg_read,
    int *pi_rob_count)
{
    int i_count = 0,i_rename_reg=0;
    int i_dest, i_src_1, i_src_2;
    
    if (*pi_rename_count == 0)
        return;

    while (i_count < i_width)
    {
        if (pa_rename[i_count].i_empty_flag != 1)
            pa_rename[i_count].ai_cycles.i_renam_cycles++;
        else
            break;

        i_count++;
        i_rename_reg++;
    }

    i_count = 0;

    while (i_count < i_width)
    {
        if (pa_reg_read[i_count].i_empty_flag != 1)
            break;
        i_count++;
    }
    
    int i_rob_full;

    if (p_rob_table->i_head > p_rob_table->i_tail)
        i_rob_full = ((p_rob_table->i_head - p_rob_table->i_tail) < (i_width));
    else if(p_rob_table->i_head < p_rob_table->i_tail)
        i_rob_full = ((p_rob_table->i_tail - p_rob_table->i_head) > (i_rob_width - i_width));
    else
        i_rob_full = p_rob_table->pa_reorder_elts[p_rob_table->i_head].read_value();

    if ((i_count != i_width) || 
        i_rob_full)
        return;

    i_count = 0;

    while (i_count < i_rename_reg)
    {
        i_dest = pa_rename[i_count].read_dest();
        i_src_1 = pa_rename[i_count].read_src_1();
        i_src_2 = pa_rename[i_count].read_src_2();
        //int pc = pa_rename[i_count].read_pc();
        if (i_src_1 != -1)
        {
            if (p_rmt->read_valid(i_src_1) == VALID)
            {
                int i_src_1_reg = p_rob_table->pa_reorder_elts[p_rmt->read_table_value(i_src_1)].read_dst();
                assert(p_rob_table->pa_reorder_elts[p_rmt->read_table_value(i_src_1)].read_value() == 1);
                if (p_rob_table->pa_reorder_elts[p_rmt->read_table_value(i_src_1)].read_ready() == 1)
                {
                    pa_rename[i_count].write_src_1(i_src_1_reg);
                    pa_rename[i_count].set_src_1_rob_flag(0);
                }
                else
                {
                    pa_rename[i_count].set_src_1_rob_flag(1);
                    pa_rename[i_count].write_src_1(p_rmt->read_table_value(i_src_1));
                }
            }
        }

        if (i_src_2 != -1)
        {
            
            if (p_rmt->read_valid(i_src_2) == VALID)
            {
                int i_src_2_reg = p_rob_table->pa_reorder_elts[p_rmt->read_table_value(i_src_2)].read_dst();
                assert(p_rob_table->pa_reorder_elts[p_rmt->read_table_value(i_src_2)].read_value() == 1);
                if (p_rob_table->pa_reorder_elts[p_rmt->read_table_value(i_src_2)].read_ready() == 1)
                {
                    pa_rename[i_count].write_src_2(i_src_2_reg);
                    pa_rename[i_count].set_src_2_rob_flag(0);
                }
                else
                {
                    pa_rename[i_count].set_src_2_rob_flag(1);
                    pa_rename[i_count].write_src_2(p_rmt->read_table_value(i_src_2));
                }
            }
        }

        if (i_dest != -1)
        {
            p_rmt->write_valid(i_dest, 1);
            p_rmt->write_table(i_dest, p_rob_table->i_tail);
            pa_rename[i_count].set_dst_rob_flag();
        }

        p_rob_table->pa_reorder_elts[p_rob_table->i_tail].write_value(1);
        p_rob_table->pa_reorder_elts[p_rob_table->i_tail].write_dst(i_dest);
        p_rob_table->pa_reorder_elts[p_rob_table->i_tail].write_ready(0);
        p_rob_table->pa_reorder_elts[p_rob_table->i_tail].write_exec(0);
        p_rob_table->pa_reorder_elts[p_rob_table->i_tail].write_mis(0);
        p_rob_table->pa_reorder_elts[p_rob_table->i_tail].write_pc(pa_rename[i_count].read_pc());
        pa_rename[i_count].write_dest(p_rob_table->i_tail);
        p_rob_table->i_tail = (p_rob_table->i_tail + 1) % i_rob_width;
        *pi_rob_count = *pi_rob_count + 1;
        pa_reg_read[i_count] = pa_rename[i_count];
        *pi_reg_read = *pi_reg_read + 1;
        pa_rename[i_count].i_empty_flag = 1;
        i_count++;
    }
    i_count = 0;
    int i_rob_flag1, i_src1, i_rob_flag2, i_src2,flag1,flag2;
    while (i_count < i_rename_reg)
    {
        if (pa_reg_read[i_count].i_empty_flag == 0)
        {
            i_rob_flag1 = pa_reg_read[i_count].read_src_1_rob_flag();
            i_src1 = pa_reg_read[i_count].read_src_1();
            i_rob_flag2 = pa_reg_read[i_count].read_src_2_rob_flag();
            i_src2 = pa_reg_read[i_count].read_src_2();

            if (i_rob_flag1 || i_rob_flag2)
            {
                flag1 = !i_rob_flag1;
                flag2 = !i_rob_flag2;

                for (int i = 0; i < 5; i++)
                {
                    for (int j = 0; j < i_width; j++)
                    {
                        if ((ppa_pipe_reg_WB[i][j].read_dest_rob_flag()) && (ppa_pipe_reg_WB[i][j].i_empty_flag == 0))
                        {
                            if ((i_src1 == ppa_pipe_reg_WB[i][j].read_dest()) && (i_rob_flag1 == 1))
                            {
                                //pa_reg_read[i_count].write_src_1(ppa_pipe_reg_WB[i][j].read_dest());
                                int temp = p_rob_table->pa_reorder_elts[i_src1].read_dst();
                                assert(p_rob_table->pa_reorder_elts[i_src1].read_value() == 1);
                                pa_reg_read[i_count].write_src_1(temp);
                                pa_reg_read[i_count].set_src_1_rob_flag(0);
                                flag1 = 1;
                            }

                            if ((i_src2 == ppa_pipe_reg_WB[i][j].read_dest()) && (i_rob_flag2 == 1))
                            {
                                int temp = p_rob_table->pa_reorder_elts[i_src2].read_dst();
                                assert(p_rob_table->pa_reorder_elts[i_src2].read_value() == 1);
                                pa_reg_read[i_count].write_src_2(temp);
                                pa_reg_read[i_count].set_src_2_rob_flag(0);
                                flag2 = 1;
                            }
                        }
                        if (flag1 && flag2)
                            break;
                    }
                    if (flag1 && flag2)
                        break;
                }
            }
        }
        i_count++;
    }
    *pi_rename_count = 0;
    return;
}

void Register_Read(Pipeline_reg *pa_reg_read, 
    Pipeline_reg *pa_dispatch,
    Pipeline_reg **ppa_pipe_reg_WB, 
    reorder_buffer_table *p_rob_table,
    int i_width,
    int *pi_reg_count,
    int *pi_di_count)
{
    int i_count = 0,flag1,flag2, i_reg_read=0;
    int i_src1, i_src2, i_rob_flag1, i_rob_flag2; 
    
    if(*pi_reg_count == 0)
        return;

    while (i_count < i_width)
    {
        if (pa_reg_read[i_count].i_empty_flag == 0)
        {
            pa_reg_read[i_count].ai_cycles.i_reg_r_cycles++;
            i_rob_flag1 = pa_reg_read[i_count].read_src_1_rob_flag();
            i_src1 = pa_reg_read[i_count].read_src_1();
            i_rob_flag2 = pa_reg_read[i_count].read_src_2_rob_flag();
            i_src2 = pa_reg_read[i_count].read_src_2();

            if (i_rob_flag1 || i_rob_flag2)
            {
                flag1 = !i_rob_flag1;
                flag2 = !i_rob_flag2;

                for (int i = 0; i < 5; i++)
                {
                    for (int j = 0; j < i_width; j++)
                    {
                        if ((ppa_pipe_reg_WB[i][j].read_dest_rob_flag()) && (ppa_pipe_reg_WB[i][j].i_empty_flag == 0))
                        {
                            if ((i_src1 == ppa_pipe_reg_WB[i][j].read_dest()) && (i_rob_flag1 == 1))
                            {
                                int temp = p_rob_table->pa_reorder_elts[i_src1].read_dst();
                                assert(p_rob_table->pa_reorder_elts[i_src1].read_value() == 1);
                                pa_reg_read[i_count].write_src_1(temp);
                                pa_reg_read[i_count].set_src_1_rob_flag(0);
                                flag1 = 1;
                            }

                            if ((i_src2 == ppa_pipe_reg_WB[i][j].read_dest()) && (i_rob_flag2 == 1))
                            {

                                int temp = p_rob_table->pa_reorder_elts[i_src2].read_dst();
                                assert(p_rob_table->pa_reorder_elts[i_src2].read_value() == 1);
                                pa_reg_read[i_count].write_src_2(temp);
                                pa_reg_read[i_count].set_src_2_rob_flag(0);
                                flag2 = 1;
                            }
                        }
                        if (flag1 && flag2)
                            break;
                    }
                    if (flag1 && flag2)
                        break;
                }
            }
            i_reg_read++;
        }
        i_count++;

    }

    i_count = 0;

    while (i_count < i_width)
    {
        if (pa_dispatch[i_count].i_empty_flag != 1)
            break;
        i_count++;
    }

    if (i_count != i_width)
        return;

    i_count = 0;

    while (i_count < i_reg_read)
    {
        pa_dispatch[i_count] = pa_reg_read[i_count];
        pa_reg_read[i_count].i_empty_flag = 1;
        *pi_di_count = *pi_di_count + 1;
        *pi_reg_count = *pi_reg_count - 1;
        i_count++;
    }

    return;
}

void Dispatch(Pipeline_reg *pa_dispatch,
    issue_queue_table *p_issue_q_table,
    Pipeline_reg **ppa_pipe_reg_WB,
    reorder_buffer_table *p_rob_table,
    int i_width,
    int i_q_size,
    int *pi_di_count)
{
    int i_count = 0,i_rob_flag1, i_src1, i_rob_flag2, i_src2;
    int flag1, flag2;
    if (*pi_di_count == 0)
        return;
    while (i_count < i_width)
    {
        pa_dispatch[i_count].ai_cycles.i_disp_cycles++;
        i_rob_flag1 = pa_dispatch[i_count].read_src_1_rob_flag();
        i_src1 = pa_dispatch[i_count].read_src_1();
        i_rob_flag2 = pa_dispatch[i_count].read_src_2_rob_flag();
        i_src2 = pa_dispatch[i_count].read_src_2();
        
        if (i_rob_flag1 || i_rob_flag2)
        {
            flag1 = !i_rob_flag1;
            flag2 = !i_rob_flag2;
            for (int i = 0; i < 5; i++)
            {
                for (int j = 0; j < i_width; j++)
                {
                    if ((ppa_pipe_reg_WB[i][j].read_dest_rob_flag()) && (ppa_pipe_reg_WB[i][j].i_empty_flag == 0))
                    {
                        if ((i_src1 == ppa_pipe_reg_WB[i][j].read_dest()) && (i_rob_flag1 == 1))
                        {
                            int temp = p_rob_table->pa_reorder_elts[i_src1].read_dst();
                            assert(p_rob_table->pa_reorder_elts[i_src1].read_value() == 1);
                            pa_dispatch[i_count].write_src_1(temp);
                            pa_dispatch[i_count].set_src_1_rob_flag(0);
                            flag1 = 1;
                        }

                        if ((i_src2 == ppa_pipe_reg_WB[i][j].read_dest()) && (i_rob_flag2 == 1))
                        {
                            int temp = p_rob_table->pa_reorder_elts[i_src2].read_dst();
                            assert(p_rob_table->pa_reorder_elts[i_src2].read_value() == 1);
                            pa_dispatch[i_count].write_src_2(temp);

                            pa_dispatch[i_count].set_src_2_rob_flag(0);
                            flag2 = 1;
                        }

                        if (flag1 && flag2)
                            break;
                    }
                }
                if (flag1 && flag2)
                    break;
            }
        }
        i_count++;
    }

    i_count = 0;

    if ((i_q_size - p_issue_q_table->i_num_ins_in_queue) < i_width)
        return;

    while (i_count < i_width)
    {
        if (pa_dispatch[i_count].i_empty_flag == 0)
        {
            for (int i = 0; i < i_q_size; i++)
            {
                if (p_issue_q_table->pa_issue_queue_elt[i].i_empty_flag == 1)
                {
                    p_issue_q_table->pa_issue_queue_elt[i] = pa_dispatch[i_count];
                    break;
                }
            }
            p_issue_q_table->i_num_ins_in_queue++;
            pa_dispatch[i_count].i_empty_flag = 1;
            *pi_di_count = *pi_di_count - 1;
        }
        i_count++;
    }
    return;
}

void Issue(issue_queue_table *p_issue_q_table, 
    Pipeline_reg **ppa_pipe_reg_EX, 
    Pipeline_reg **ppa_pipe_reg_WB, 
    reorder_buffer_table *p_rob_table,
    int i_width,
    int *pi_ex_count)
{
    int i_q_size = p_issue_q_table->i_q_size, flag1, flag2;
    int i_count=0,i,j, i_sent_flag,i_rob_flag1,i_rob_flag2,i_src1,i_src2;
    Pipeline_reg *pa_issue_queue_elt = p_issue_q_table->pa_issue_queue_elt;

    if (p_issue_q_table->i_num_ins_in_queue == 0)
        return;

    while (i_count < i_q_size)
    {
        pa_issue_queue_elt[i_count].ai_cycles.i_iss_cycles++;
        i_rob_flag1 = pa_issue_queue_elt[i_count].read_src_1_rob_flag();
        i_src1 = pa_issue_queue_elt[i_count].read_src_1();
        i_rob_flag2 = pa_issue_queue_elt[i_count].read_src_2_rob_flag();
        i_src2 = pa_issue_queue_elt[i_count].read_src_2();

        flag1 = !i_rob_flag1;
        flag2 = !i_rob_flag2;

        if (i_rob_flag1 || i_rob_flag2)
        {
            for (int i = 0; i < 5; i++)
            {
                for (int j = 0; j < i_width; j++)
                {
                    if ((ppa_pipe_reg_WB[i][j].read_dest_rob_flag()) && (ppa_pipe_reg_WB[i][j].i_empty_flag == 0))
                    {
                        if ((i_src1 == ppa_pipe_reg_WB[i][j].read_dest()) && (i_rob_flag1 == 1))
                        {
                            int temp = p_rob_table->pa_reorder_elts[i_src1].read_dst();
                            assert(p_rob_table->pa_reorder_elts[i_src1].read_value() == 1);
                            pa_issue_queue_elt[i_count].write_src_1(temp);
                            pa_issue_queue_elt[i_count].set_src_1_rob_flag(0);
                            flag1 = 1;
                        }

                        if ((i_src2 == ppa_pipe_reg_WB[i][j].read_dest()) && (i_rob_flag2 == 1))
                        {
                            int temp = p_rob_table->pa_reorder_elts[i_src2].read_dst();
                            assert(p_rob_table->pa_reorder_elts[i_src2].read_value() == 1);
                            pa_issue_queue_elt[i_count].write_src_2(temp);
                            pa_issue_queue_elt[i_count].set_src_2_rob_flag(0);
                            flag2 = 1;
                        }
                    }

                    if (flag1 && flag2)
                        break;
                }
                if (flag1 && flag2)
                    break;
            }
        }
        i_count++;
    }

    for (i_count = 0; i_count < i_width; i_count++)
    {
        int i_min_pc = 0x3FFFFFFF,i_min_index=-1;

        for ( j = 0; j < i_q_size; j++)
        {
            if ((pa_issue_queue_elt[j].i_empty_flag == 0) && 
                (pa_issue_queue_elt[j].read_src_1_rob_flag() == 0) &&
                (pa_issue_queue_elt[j].read_src_2_rob_flag() == 0))
            {
                if (i_min_pc > pa_issue_queue_elt[j].ai_cycles.i_priority)
                {
                    i_min_index = j;
                    i_min_pc = pa_issue_queue_elt[j].ai_cycles.i_priority;
                }
            }
        }

        if (i_min_index > -1)
        {
            i_sent_flag = 0;
            for ( i = 0; i < 5; i++)
            {
                for (j = 0; j < i_width; j++)
                {
                    if (ppa_pipe_reg_EX[i][j].i_empty_flag == 1)
                    {
                        ppa_pipe_reg_EX[i][j] = pa_issue_queue_elt[i_min_index];
                        pa_issue_queue_elt[i_min_index].i_empty_flag = 1;
                        p_issue_q_table->i_num_ins_in_queue--;
                        *pi_ex_count = *pi_ex_count + 1;
                        i_sent_flag = 1;
                        break;
                    }
                }
                if (i_sent_flag == 1)
                    break;
            }
        }
    }

    return;
}

void Execute(Pipeline_reg **ppa_pipe_reg_EX, 
    Pipeline_reg **ppa_writeback, 
    int i_width, 
    int *pi_ex_count,
    int *pi_wb_count)
{
    int i, j;
    if (*pi_ex_count > 0)
    {
        for (i = 0; i < 5; i++)
        {
            for (j = 0; j < i_width; j++)
            {
                if (ppa_pipe_reg_EX[i][j].i_empty_flag == 0)
                {
                    ppa_pipe_reg_EX[i][j].ai_cycles.i_exec_cycles++;

                    if (ppa_pipe_reg_EX[i][j].read_op() == 0)
                    {
                        //int pc = ppa_pipe_reg_EX[i][j].read_pc();
                        ppa_writeback[i][j] = ppa_pipe_reg_EX[i][j];
                        ppa_pipe_reg_EX[i][j].i_empty_flag = 1;
                        *pi_ex_count = *pi_ex_count - 1;
                        *pi_wb_count = *pi_wb_count + 1;
                    }

                    if (ppa_pipe_reg_EX[i][j].read_op() == 1 && ppa_pipe_reg_EX[i][j].ai_cycles.i_exec_cycles == 2)
                    {
                        //int pc = ppa_pipe_reg_EX[i][j].read_pc();
                        ppa_writeback[i][j] = ppa_pipe_reg_EX[i][j];
                        ppa_pipe_reg_EX[i][j].i_empty_flag = 1;
                        *pi_ex_count = *pi_ex_count - 1;
                        *pi_wb_count = *pi_wb_count + 1;
                    }

                    if (ppa_pipe_reg_EX[i][j].read_op() == 2 && ppa_pipe_reg_EX[i][j].ai_cycles.i_exec_cycles == 5)
                    {
                        //int pc = ppa_pipe_reg_EX[i][j].read_pc();
                        ppa_writeback[i][j] = ppa_pipe_reg_EX[i][j];
                        ppa_pipe_reg_EX[i][j].i_empty_flag = 1;
                        *pi_ex_count = *pi_ex_count - 1;
                        *pi_wb_count = *pi_wb_count + 1;
                    }
                }
            }
        }
    }
    return;
}

void Writeback(Pipeline_reg **ppa_pipe_reg_WB,
    reorder_buffer_table *p_rob_table,
    int i_width,
    int i_rob_width,
    int *pi_wb_count)
{
    Pipeline_reg *temp;
    int i, j, i_dest;
   
    if (*pi_wb_count > 0)
    {
        for (i = 0; i < 5; i++)
        {
            for (j = 0; j < i_width; j++)
            {
                temp = &(ppa_pipe_reg_WB[i][j]);
                if (temp->i_empty_flag == 0)
                {
                    int i_dest_reg;
                    temp->ai_cycles.i_wb_cycles++;
                    i_dest = temp->read_dest();
                    i_dest_reg = p_rob_table->pa_reorder_elts[i_dest].read_dst();
                    assert(i_dest >= 0 && i_dest < i_rob_width);
                    p_rob_table->pa_reorder_elts[i_dest].write_ready(1);
                    *(p_rob_table->pa_reorder_elts[i_dest].p_pipe_line_reg) = *temp;
                    p_rob_table->pa_reorder_elts[i_dest].p_pipe_line_reg->write_dest(i_dest_reg);
                    //printf("%x\t", temp->read_pc());
                    temp->i_empty_flag = 1;
                    
                    *pi_wb_count = *pi_wb_count - 1;
                }
                if (*pi_wb_count == 0)
                    break;
            }
            if (*pi_wb_count == 0)
                break;
        }
    }
    return;

}


void Retire(rename_map_table *p_rmt,
    reorder_buffer_table *p_rob_table,
    int i_rob_width,
    int i_width, 
    int *pi_rob_count,
    int *pi_dyn_ins_count)
{
    int i_count=0,i_head=p_rob_table->i_head,i;
    int i_dest;
    Pipeline_reg *temp;
    num_cycles* temp_cycle;
    int i_dec_st_cycle, i_ren_st_cycle, i_di_st_cycle, i_rr_st_cycle, i_iss_st_cycle, i_ex_st_cycle, i_wb_st_cycle, i_ret_st_cycle;
    if (*pi_rob_count == 0)
        return;

    for (i = 0; i < i_rob_width; i++)
    {
        if (p_rob_table->pa_reorder_elts[i].read_value() == 1)
        {
            temp = p_rob_table->pa_reorder_elts[i].p_pipe_line_reg;
            temp->ai_cycles.i_rt_cycles++;
        }
    }

    for (i_count = 0; i_count < i_width; i_count++)
    {
        if (p_rob_table->pa_reorder_elts[i_head].read_ready()== 1)
        {
            i_dest = p_rob_table->pa_reorder_elts[i_head].read_dst();
            if (i_dest > -1)
            {
                int i_rmt_val = p_rmt->read_table_value(i_dest);
                if(i_rmt_val == i_head)
                    p_rmt->write_valid(i_dest, 0);
            }
            p_rob_table->pa_reorder_elts[i_head].write_value(0);           
            {
                temp = p_rob_table->pa_reorder_elts[i_head].p_pipe_line_reg;
                temp_cycle = &temp->ai_cycles;
                        
                i_dec_st_cycle = temp_cycle->i_fetch_cycle_start + temp_cycle->i_fetch_cycles;
                i_ren_st_cycle = i_dec_st_cycle + temp_cycle->i_dec_cycles;
                i_rr_st_cycle = i_ren_st_cycle + temp_cycle->i_renam_cycles;
                i_di_st_cycle = i_rr_st_cycle + temp_cycle->i_reg_r_cycles;
                i_iss_st_cycle = i_di_st_cycle + temp_cycle->i_disp_cycles;
                i_ex_st_cycle = i_iss_st_cycle + temp_cycle->i_iss_cycles;
                i_wb_st_cycle = i_ex_st_cycle + temp_cycle->i_exec_cycles;
                i_ret_st_cycle = i_wb_st_cycle + temp_cycle->i_wb_cycles;
                printf("%d fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} RR{%d,%d} DI{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d}\n",
                    *pi_dyn_ins_count, temp->read_op(), temp->read_src_1(), temp->read_src_2(), temp->read_dest(), temp_cycle->i_fetch_cycle_start, 
                    temp_cycle->i_fetch_cycles, i_dec_st_cycle, temp_cycle->i_dec_cycles,i_ren_st_cycle, temp_cycle->i_renam_cycles, i_rr_st_cycle, 
                    temp_cycle->i_reg_r_cycles,i_di_st_cycle,temp_cycle->i_disp_cycles, i_iss_st_cycle, temp_cycle->i_iss_cycles, i_ex_st_cycle, 
                    temp_cycle->i_exec_cycles, i_wb_st_cycle, temp_cycle->i_wb_cycles, i_ret_st_cycle, (temp_cycle->i_rt_cycles));
                       
            }
            p_rob_table->pa_reorder_elts[i_head].write_ready(0);
            *pi_dyn_ins_count = *pi_dyn_ins_count + 1;
            i_head = (i_head + 1) % i_rob_width;
            *pi_rob_count = *pi_rob_count - 1;     
            assert(*pi_rob_count >= 0);
        }
        else
            break;
    }
   
    p_rob_table->i_head = i_head;

    return;

}




void memory_init(cpu_memories *p_cpu_mem)
{
    int i_q_size = p_cpu_mem->i_q_size;
    int i_rob_size = p_cpu_mem->i_rob_size;
    int i_width = p_cpu_mem->i_width;

    p_cpu_mem->pa_pipe_reg_DE = new Pipeline_reg[i_width];
    p_cpu_mem->pa_pipe_reg_RN = new Pipeline_reg[i_width];
    p_cpu_mem->pa_pipe_reg_RR = new Pipeline_reg[i_width];
    p_cpu_mem->pa_pipe_reg_DI= new Pipeline_reg[i_width];

    p_cpu_mem->p_issue_q_table= new issue_queue_table;
    p_cpu_mem->p_issue_q_table->i_num_ins_in_queue = 0;
    p_cpu_mem->p_issue_q_table->i_q_size = i_q_size;
    p_cpu_mem->p_issue_q_table->pa_issue_queue_elt = new Pipeline_reg[i_q_size];

    p_cpu_mem->ppa_pipe_reg_WB = new Pipeline_reg*[5];
    p_cpu_mem->ppa_pipe_reg_EX = new Pipeline_reg*[5];

    for (int i = 0; i < 5; i++)
    {
        p_cpu_mem->ppa_pipe_reg_WB[i] = new Pipeline_reg[i_width];
        p_cpu_mem->ppa_pipe_reg_EX[i] = new Pipeline_reg[i_width];
    }
    p_cpu_mem->p_rob_t= new reorder_buffer_table;
    p_cpu_mem->p_rob_t->i_head = 0;
    p_cpu_mem->p_rob_t->i_tail = 0;
    p_cpu_mem->p_rob_t->pa_reorder_elts = new reorder_buffer_elt[i_rob_size];

    for (int i = 0; i < i_rob_size; i++)
    {
        p_cpu_mem->p_rob_t->pa_reorder_elts[i].p_pipe_line_reg = new Pipeline_reg;
    }

    p_cpu_mem->p_rmt= new rename_map_table;

}


void memory_delete(cpu_memories *p_cpu_mem)
{
    int i_rob_size = p_cpu_mem->i_rob_size;

    delete[] p_cpu_mem->pa_pipe_reg_DE;
    delete[] p_cpu_mem->pa_pipe_reg_RN;
    delete[] p_cpu_mem->pa_pipe_reg_RR;
    delete[] p_cpu_mem->pa_pipe_reg_DI;

    
    delete[] p_cpu_mem->p_issue_q_table->pa_issue_queue_elt;
    delete[] p_cpu_mem->p_issue_q_table;

    for (int i = 0; i < 5; i++)
    {
        delete[] p_cpu_mem->ppa_pipe_reg_WB[i];
        delete[] p_cpu_mem->ppa_pipe_reg_EX[i];
    }

    delete[] p_cpu_mem->ppa_pipe_reg_WB;
    delete[] p_cpu_mem->ppa_pipe_reg_EX;

    for (int i = 0; i < i_rob_size; i++)
    {
        delete  p_cpu_mem->p_rob_t->pa_reorder_elts[i].p_pipe_line_reg;
    }

    delete[] p_cpu_mem->p_rob_t->pa_reorder_elts;
delete[] p_cpu_mem->p_rob_t;


delete[] p_cpu_mem->p_rmt;
}

void print_contents(cpu_memories *p_cpu)
{
    printf("Rename\n");
    printf("PC OP SRC_1 SRC_1_ROB SRC_2 SRC_2_ROB DST DST_ROB\n");
    for (int i = 0; i < p_cpu->i_width; i++)
    {
        if (p_cpu->pa_pipe_reg_RN[i].i_empty_flag == 0)
            printf("%10x %10d %10d %10d %10d %10d %10d %10d\n", p_cpu->pa_pipe_reg_RN[i].read_pc(),
                p_cpu->pa_pipe_reg_RN[i].read_op(), p_cpu->pa_pipe_reg_RN[i].read_src_1(),
                p_cpu->pa_pipe_reg_RN[i].read_src_1_rob_flag(), p_cpu->pa_pipe_reg_RN[i].read_src_2(),
                p_cpu->pa_pipe_reg_RN[i].read_src_2_rob_flag(), p_cpu->pa_pipe_reg_RN[i].read_dest(),
                p_cpu->pa_pipe_reg_RN[i].read_dest_rob_flag());
    }
    printf("Reg_read\n");
    printf("PC OP SRC_1 SRC_1_ROB SRC_2 SRC_2_ROB DST DST_ROB\n");
    for (int i = 0; i < p_cpu->i_width; i++)
    {
        if (p_cpu->pa_pipe_reg_RR[i].i_empty_flag == 0)
            printf("%10x %10d %10d %10d %10d %10d %10d %10d\n", p_cpu->pa_pipe_reg_RR[i].read_pc(),
                p_cpu->pa_pipe_reg_RR[i].read_op(), p_cpu->pa_pipe_reg_RR[i].read_src_1(),
                p_cpu->pa_pipe_reg_RR[i].read_src_1_rob_flag(), p_cpu->pa_pipe_reg_RR[i].read_src_2(),
                p_cpu->pa_pipe_reg_RR[i].read_src_2_rob_flag(), p_cpu->pa_pipe_reg_RR[i].read_dest(),
                p_cpu->pa_pipe_reg_RR[i].read_dest_rob_flag());
    }
    printf("Dispatch\n");
    printf("PC OP SRC_1 SRC_1_ROB SRC_2 SRC_2_ROB DST DST_ROB\n");
    for (int i = 0; i < p_cpu->i_width; i++)
    {
        if (p_cpu->pa_pipe_reg_DI[i].i_empty_flag == 0)
            printf("%10x %10d %10d %10d %10d %10d %10d %10d\n", p_cpu->pa_pipe_reg_DI[i].read_pc(),
                p_cpu->pa_pipe_reg_DI[i].read_op(), p_cpu->pa_pipe_reg_DI[i].read_src_1(),
                p_cpu->pa_pipe_reg_DI[i].read_src_1_rob_flag(), p_cpu->pa_pipe_reg_DI[i].read_src_2(),
                p_cpu->pa_pipe_reg_DI[i].read_src_2_rob_flag(), p_cpu->pa_pipe_reg_DI[i].read_dest(),
                p_cpu->pa_pipe_reg_DI[i].read_dest_rob_flag());
    }
    printf("Issue\n");
    printf("PC OP SRC_1 SRC_1_ROB SRC_2 SRC_2_ROB DST DST_ROB\n");
    for (int i = 0; i < p_cpu->i_q_size; i++)
    {
        if (p_cpu->p_issue_q_table->pa_issue_queue_elt[i].i_empty_flag == 0)
            printf("%10x %10d %10d %10d %10d %10d %10d %10d\n", p_cpu->p_issue_q_table->pa_issue_queue_elt[i].read_pc(),
                p_cpu->p_issue_q_table->pa_issue_queue_elt[i].read_op(), p_cpu->p_issue_q_table->pa_issue_queue_elt[i].read_src_1(),
                p_cpu->p_issue_q_table->pa_issue_queue_elt[i].read_src_1_rob_flag(), p_cpu->p_issue_q_table->pa_issue_queue_elt[i].read_src_2(),
                p_cpu->p_issue_q_table->pa_issue_queue_elt[i].read_src_2_rob_flag(), p_cpu->p_issue_q_table->pa_issue_queue_elt[i].read_dest(),
                p_cpu->p_issue_q_table->pa_issue_queue_elt[i].read_dest_rob_flag());
    }

    printf("Execute\n");
    printf("PC OP SRC_1 SRC_1_ROB SRC_2 SRC_2_ROB DST DST_ROB\n");
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < p_cpu->i_width; j++)
        {
            if (p_cpu->ppa_pipe_reg_EX[i][j].i_empty_flag == 0)
                printf("%10x %10d %10d %10d %10d %10d %10d %10d\n", p_cpu->ppa_pipe_reg_EX[i][j].read_pc(),
                    p_cpu->ppa_pipe_reg_EX[i][j].read_op(), p_cpu->ppa_pipe_reg_EX[i][j].read_src_1(),
                    p_cpu->ppa_pipe_reg_EX[i][j].read_src_1_rob_flag(), p_cpu->ppa_pipe_reg_EX[i][j].read_src_2(),
                    p_cpu->ppa_pipe_reg_EX[i][j].read_src_2_rob_flag(), p_cpu->ppa_pipe_reg_EX[i][j].read_dest(),
                    p_cpu->ppa_pipe_reg_EX[i][j].read_dest_rob_flag());
        }
    }
    printf("Writeback\n");
    printf("PC OP SRC_1 SRC_1_ROB SRC_2 SRC_2_ROB DST DST_ROB\n");
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < p_cpu->i_width; j++)
        {
            if (p_cpu->ppa_pipe_reg_WB[i][j].i_empty_flag == 0)
                printf("%10x %10d %10d %10d %10d %10d %10d %10d\n", p_cpu->ppa_pipe_reg_WB[i][j].read_pc(),
                    p_cpu->ppa_pipe_reg_WB[i][j].read_op(), p_cpu->ppa_pipe_reg_WB[i][j].read_src_1(),
                    p_cpu->ppa_pipe_reg_WB[i][j].read_src_1_rob_flag(), p_cpu->ppa_pipe_reg_WB[i][j].read_src_2(),
                    p_cpu->ppa_pipe_reg_WB[i][j].read_src_2_rob_flag(), p_cpu->ppa_pipe_reg_WB[i][j].read_dest(),
                    p_cpu->ppa_pipe_reg_WB[i][j].read_dest_rob_flag());
        }
    }
    printf("ROB\n");
    printf("index READY DST PC head %d tail %d\n", p_cpu->p_rob_t->i_head, p_cpu->p_rob_t->i_tail);
    for (int i = 0; i < p_cpu->i_rob_size; i++)
    {
        if (p_cpu->p_rob_t->pa_reorder_elts[i].read_value() == 1)
        {

            printf("%10d %10d %10d %10x\n",i, p_cpu->p_rob_t->pa_reorder_elts[i].read_ready(), p_cpu->p_rob_t->pa_reorder_elts[i].read_dst(),
                p_cpu->p_rob_t->pa_reorder_elts[i].read_pc());
           
        }
    }
    printf("RMT\n");
    printf("INDX ROB_TAG\n");
    for (int i = 0; i < MAX_REG; i++)
    {
        if (p_cpu->p_rmt->read_valid(i) == 1)
        {
            printf("%10d %10d\n",i, p_cpu->p_rmt->read_table_value(i));
        }
    }

}
