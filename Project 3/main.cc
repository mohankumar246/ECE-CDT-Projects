/*******************************************************
                          main.cc
********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <ios>
#include <sstream>
#include <istream>
#include <string>
#include <iostream>
#include <cstring>
#include <memory.h>
#include "dynamic_instruction_scheduling.h"

using namespace std;
int main(int argc, char *argv[])
{
	
    ifstream pFile;
	if(argv[1] == NULL){
		 printf("input format: ");
		 printf("./sim <ROB_SIZE> <IQ_SIZE> <WIDTH> <trace_file>\n");
		 exit(0);
        }

	/*****uncomment the next five lines*****/
	int  i_rob_size = atoi(argv[1]);
	int  i_ins_q_size = atoi(argv[2]);
	int  i_width = atoi(argv[3]), i_piority_num=0;
    string ac_line;
	char *fname=NULL;
 	fname = argv[4];
        
	

	//*********************************************//
        //*****create an array of caches here**********//
	//*********************************************//	
    if (fname != NULL)
    {
        pFile.open(fname);
    }

    if(pFile.is_open())
    {
        cpu_memories c_cpu,*p_cpu;
        p_cpu = &c_cpu;
        p_cpu->i_q_size = i_ins_q_size;
        p_cpu->i_rob_size = i_rob_size;
        p_cpu->i_width = i_width;
        memory_init(p_cpu);
        int i_cycles =0,i_dyn_ins_count=0;
        float f_ipc=0.0f;
        int i_rob_count, i_wb_count, i_ex_count, i_issu_count, i_disp_count, i_rr_count, i_renm_count, i_dec_count, i_fet_count;
        i_rob_count= i_wb_count= i_ex_count= i_issu_count= i_disp_count= i_rr_count= i_renm_count= i_dec_count= i_fet_count = 0;

        while (1)
        {
            Retire(p_cpu->p_rmt,
                p_cpu->p_rob_t, 
                p_cpu->i_rob_size,
                p_cpu->i_width,
                &i_rob_count,
                &i_dyn_ins_count);

            Writeback(p_cpu->ppa_pipe_reg_WB,
                p_cpu->p_rob_t,
                p_cpu->i_width,
                p_cpu->i_rob_size,
                &i_wb_count);

            Execute(p_cpu->ppa_pipe_reg_EX, 
                p_cpu->ppa_pipe_reg_WB, 
                p_cpu->i_width,
                &i_ex_count,
                &i_wb_count);

            Issue(p_cpu->p_issue_q_table, 
                p_cpu->ppa_pipe_reg_EX, 
                p_cpu->ppa_pipe_reg_WB, 
                p_cpu->p_rob_t,
                p_cpu->i_width,
                &i_ex_count);

            Dispatch(p_cpu->pa_pipe_reg_DI, 
                p_cpu->p_issue_q_table, 
                p_cpu->ppa_pipe_reg_WB, 
                p_cpu->p_rob_t,
                p_cpu->i_width, 
                p_cpu->i_q_size,
                &i_disp_count);

            Register_Read(p_cpu->pa_pipe_reg_RR, 
                p_cpu->pa_pipe_reg_DI, 
                p_cpu->ppa_pipe_reg_WB, 
                p_cpu->p_rob_t,
                p_cpu->i_width,
                &i_rr_count,
                &i_disp_count);

            Rename(p_cpu->pa_pipe_reg_RN, 
                p_cpu->pa_pipe_reg_RR, 
                p_cpu->p_rmt,
                p_cpu->p_rob_t,
                p_cpu->ppa_pipe_reg_WB,
                p_cpu->i_width,
                p_cpu->i_rob_size,
                &i_renm_count,
                &i_rr_count,
                &i_rob_count);

            Decode(p_cpu->pa_pipe_reg_DE,
                p_cpu->pa_pipe_reg_RN,
                p_cpu->i_width,
                &i_dec_count,
                &i_renm_count);

            Fetch(&pFile,
                p_cpu->pa_pipe_reg_DE,
                p_cpu->i_width,
                &i_fet_count,
                &i_dec_count,
                i_cycles,
                &i_piority_num);
            

#if 0
            print_contents(p_cpu);
#endif

            i_issu_count = p_cpu->p_issue_q_table->i_num_ins_in_queue;
            //printf("%d\n", i_cycles);
            i_cycles++;


           if (!(i_rob_count || i_wb_count || i_ex_count || i_issu_count || i_disp_count || i_rr_count || i_renm_count || i_dec_count || i_fet_count))
               break;
        }

        //********************************//
        //print out all caches' statistics //
        //********************************//

        printf("# === Simulator Command =========\n");
        printf("# ./sim %d %d %d %s\n", i_rob_size, i_ins_q_size, i_width, fname);
        printf("# === Processor Configuration ===\n");
        printf("# ROB_SIZE  = %d\n", i_rob_size);
        printf("# IQ_SIZE   = %d\n", i_ins_q_size);
        printf("# WIDTH     = %d\n", i_width);

        printf("# === Simulation Results ========\n");
        printf("# %-30s %c %10d\n","Dynamic Instruction Count", '=',i_dyn_ins_count); 
        printf("# %-30s %c %10d\n","Cycles",'=',i_cycles);
    f_ipc = (float)i_dyn_ins_count / i_cycles;
    printf(" #%-30s %c %10.2f\n","Instructions Per Cycle (IPC)",'=',f_ipc);
    memory_delete(p_cpu);
    
    }
    else
    {
        printf("Trace file problem\n");
        exit(0);
    }
    pFile.close();
    return(0);
}

