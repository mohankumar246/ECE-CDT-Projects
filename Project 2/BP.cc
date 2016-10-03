#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
using namespace std;

enum e_modes
{   BIMODAL=0,
    GSHARE,
    HYBRID
};

enum e_branch
{
    NOT_TAKEN=0,
    TAKEN
}; 

void call_to_print(int i_num, int* pa_counter)
{
    int k;
    for (k = 0; k<i_num; k++)
    {
        printf("%d %d\n", k, pa_counter[k]);
    }
}

void init_array(int i_num, int* pa_counter, int init_value)
{
    int k;
    for (k = 0; k<i_num; k++)
    {
        pa_counter[k] = init_value;
    }
}

e_branch query_model(int *counter)
{
    if (*counter > 1)
        return TAKEN;
    else
        return NOT_TAKEN;
}

int  update_model(e_branch e_branch_result, int *counter)
{
    int i_mispredicted = 0;

    if (*counter > 1)
    {
        if (e_branch_result == NOT_TAKEN)
            i_mispredicted = 1;
    }
    else
    {
        if (e_branch_result == TAKEN)
            i_mispredicted = 1;
    }

    if (e_branch_result == NOT_TAKEN)
    {
        *counter = *counter - 1;
        if (*counter <0)
            *counter = 0;
    }
    else
    {
        *counter = *counter + 1;
        if (*counter > 3)
            *counter = 3;
    }

    return i_mispredicted;
}


e_modes query_chooser(int *counter)
{
    if (*counter > 1)
        return GSHARE;
    else
        return BIMODAL;
}

void update_chooser(e_branch e_branch_result, e_branch e_gshare_prediction, e_branch e_bimodal_prediction, int *counter)
{

    if (e_gshare_prediction != e_bimodal_prediction)
    {
        if (e_branch_result == e_gshare_prediction)
        {
            *counter = *counter + 1;
            if (*counter > 3)
                *counter = 3;
        }
        if (e_branch_result == e_bimodal_prediction)
        {
            *counter = *counter - 1;
            if (*counter < 0)
                *counter = 0;
        }

    }
}

int main(int argc,
    char *argv[])
{
    int i_exit_flag = 0;
    char *filename=NULL;
    char *mode;
    int i_pc_bits_for_chooser_table,i_pc_bits_for_index_bimodal;
    int i_pc_bits_for_index_gshare,i_global_branch_history_bits; 
    e_modes e_current_mode;
    mode = argv[1];
    float f_mis;    
    if(strcmp(mode,"bimodal")==0)
    {
        e_current_mode = BIMODAL;
        if(argc == 4)
        {
            i_pc_bits_for_index_bimodal = strtol(argv[2],NULL,10);
            filename = argv[3];
        }
        else
            i_exit_flag =1;
    }
    else
    if(strcmp(mode,"gshare")==0)
    {
        e_current_mode = GSHARE;
        if(argc == 5)
        {
            i_pc_bits_for_index_gshare = strtol(argv[2],NULL,10);
            i_global_branch_history_bits = strtol(argv[3],NULL,10);
            filename = argv[4];
        }
        else
            i_exit_flag=1;
    }
    else
    if(strcmp(mode,"hybrid")==0)
    {
        e_current_mode = HYBRID;
        if(argc==7)
        {
            i_pc_bits_for_chooser_table   = strtol(argv[2],NULL,10);
            i_pc_bits_for_index_gshare    = strtol(argv[3],NULL,10);
            i_global_branch_history_bits  = strtol(argv[4],NULL,10);
            i_pc_bits_for_index_bimodal   = strtol(argv[5],NULL,10);
            filename = argv[6];
        }
        else
            i_exit_flag=1;
    }
    else
    {
        i_exit_flag=1;   
    }

    if(i_exit_flag==1)
    {
        printf("The command had less or no other arguments.\n");
        printf("For Bimodal:sim bimodal <M2> <tracefile>\n");
        printf("For Gshare :sim gshare <M1> <N> <tracefile>\n");    
        printf("For hybrid :sim hybrid <K> <M1> <N> <M2> <tracefile>\n"); 
        printf("Where:\n K is the number of PC bits used to index the chooser table\n");
        printf("M1 and N are the number of PC bits and global branch history register bits used to index the gshare table (respectively)\n");
        printf("M2 is the number of PC bits used to index the bimodal table \n");
    }

    if (i_exit_flag == 0)
    {
        unsigned int i_address=0,i_mispredicted=0;
        string line,s_address; 
        char c_branch_taken_or_not;
        ifstream pFile;
        int *ai_chooser = NULL, *ai_gshare = NULL, *ai_bimodal = NULL;
        int i_gbranch_history=0,i_num_predictions=0,i_num_mispredictions=0;
        unsigned int i_num_gshares, i_num_bimodals;
        unsigned int i_num_choosers;
        unsigned int i_index_bimodal=0; 
        int i_required_PC ,i_index_gshare,i_index_chooser;
        e_modes e_select = HYBRID;
        e_branch e_branch_result;
        
        if(filename != NULL)
            pFile.open(filename);

        if(pFile.is_open())
        {
            if(e_current_mode==HYBRID)
            {
                i_num_choosers = 1 << i_pc_bits_for_chooser_table;
                ai_chooser = new int[i_num_choosers];
                init_array(i_num_choosers,ai_chooser,1);
            }

            if((e_current_mode==GSHARE)||(e_current_mode==HYBRID))
            {
                i_num_gshares = 1 << i_pc_bits_for_index_gshare;
                ai_gshare = new int[i_num_gshares];
                init_array(i_num_gshares,ai_gshare,2);
            }    
            if ((e_current_mode == BIMODAL) || (e_current_mode == HYBRID))
            {
                i_num_bimodals = 1 << i_pc_bits_for_index_bimodal;
                ai_bimodal = new int[i_num_bimodals];
                init_array(i_num_bimodals, ai_bimodal, 2);
            }

            while (pFile.good())
            {
                getline(pFile,line);
                istringstream iss(line);
                iss >> s_address;
                iss >> c_branch_taken_or_not;
                if (line.size() > 2)
                {
                    i_address = strtoul(s_address.c_str(), NULL, 16);
                    i_address >>= 2;
                    if (c_branch_taken_or_not == 't')
                        e_branch_result = TAKEN;
                    else
                        e_branch_result = NOT_TAKEN;

                    i_mispredicted = 0;
                    if (e_current_mode == HYBRID)
                    {
                        e_branch e_bimodal_prediction, e_gshare_prediction;

                        i_index_bimodal = ((i_address)& (i_num_bimodals - 1));
                        e_bimodal_prediction = query_model(&ai_bimodal[i_index_bimodal]);

                        i_required_PC = i_address & (i_num_gshares - 1);
                        i_index_gshare = i_gbranch_history ^ (i_required_PC >> (i_pc_bits_for_index_gshare - i_global_branch_history_bits));
                        i_index_gshare <<= (i_pc_bits_for_index_gshare - i_global_branch_history_bits);
                        i_required_PC &= ((1 << (i_pc_bits_for_index_gshare - i_global_branch_history_bits)) - 1);
                        i_index_gshare |= i_required_PC;

                        assert(i_index_gshare >= 0);
                        assert(i_index_gshare < i_num_gshares);

                        e_gshare_prediction = query_model(&ai_gshare[i_index_gshare]);

                        i_index_chooser = i_address & (i_num_choosers - 1);

                        assert(i_index_chooser >= 0);
                        assert(i_index_chooser < i_num_choosers);

                        e_select = query_chooser(&ai_chooser[i_index_chooser]);
                        update_chooser(e_branch_result, e_gshare_prediction, e_bimodal_prediction, &ai_chooser[i_index_chooser]);
                    }

                    /*For the GSHARE model*/
                    if (e_current_mode == GSHARE)
                    {
                        i_required_PC = i_address & (i_num_gshares - 1);
                        i_index_gshare = i_gbranch_history ^ (i_required_PC >> (i_pc_bits_for_index_gshare - i_global_branch_history_bits));
                        i_index_gshare <<= (i_pc_bits_for_index_gshare - i_global_branch_history_bits);
                        i_required_PC &= ((1 << (i_pc_bits_for_index_gshare - i_global_branch_history_bits)) - 1);
                        i_index_gshare |= i_required_PC;
                        assert(i_index_gshare >= 0);
                        assert(i_index_gshare < i_num_gshares);
                        i_mispredicted = update_model(e_branch_result, &ai_gshare[i_index_gshare]);
                    }

                    /*For bimodal*/
                    if (e_current_mode == BIMODAL)
                    {
                        i_index_bimodal = ((i_address)& (i_num_bimodals - 1));
                        assert(i_index_bimodal >= 0);
                        assert(i_index_bimodal < i_num_bimodals);
                        i_mispredicted = update_model(e_branch_result, &ai_bimodal[i_index_bimodal]);
                    }

                    if (e_select == GSHARE)
                        i_mispredicted = update_model(e_branch_result, &ai_gshare[i_index_gshare]);
                    if (e_select == BIMODAL)
                        i_mispredicted = update_model(e_branch_result, &ai_bimodal[i_index_bimodal]);

                    i_num_predictions++;
                    if (i_mispredicted)
                        i_num_mispredictions++;

                    if ((e_current_mode == HYBRID) || (e_current_mode == GSHARE))
                    {
                        i_gbranch_history >>= 1;
                        if (e_branch_result == TAKEN)
                            i_gbranch_history |= (1 << (i_global_branch_history_bits - 1));
                    }
                }
            }
            /*Print the results*/
            f_mis = ((float)i_num_mispredictions * 100) / (float)i_num_predictions;
            printf("OUTPUT\n");
            printf("number of predictions:  %d\n", i_num_predictions);
            printf("number of mispredictions: %d\n", i_num_mispredictions);
            printf("misprediction rate: %.2f\n", f_mis);

            if (e_current_mode == HYBRID)
            {
                printf("FINAL CHOOSER CONTENTS\n");
                call_to_print(i_num_choosers, ai_chooser);
                delete ai_chooser;
            }

            if (e_current_mode == GSHARE || e_current_mode == HYBRID)
            {
                printf("FINAL GSHARE CONTENTS\n");
                call_to_print(i_num_gshares, ai_gshare);
                delete ai_gshare;
            }
            if (e_current_mode == BIMODAL || e_current_mode == HYBRID)
            {
                printf("FINAL BIMODAL CONTENTS\n");
                call_to_print(i_num_bimodals, ai_bimodal);
                delete ai_bimodal;
            }
            pFile.close();
        }
        else
            printf("Unable to open trace file\n");
    }
    else
    {
        printf("Enter a Key to exit:");
        getchar();
    }
    return 0;
}

