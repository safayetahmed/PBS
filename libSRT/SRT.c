#include <stdio.h>
/*
fprintf
stderr
*/
#include <fcntl.h>
/*
open
*/
#include <sys/ioctl.h>
/*
ioctl
*/

#include <stdlib.h>

#include <sys/types.h>

#include <sys/mman.h>
/*
mlock
*/

#include <unistd.h>
/*
read
fcntl
getpid
*/

#include <sched.h>

#include <stdint.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef int64_t  s64;
typedef int32_t  s32;

#include "SRT_cmd.h"

#include "SRT.h"

int SRT_setup(  uint64_t period, uint64_t estimated_mean_exectime, 
                double alpha,
                SRT_Predictor_t *predictor,
                SRT_handle *handle, 
                int loglevel, int logCount, char *logFileName)
{
    pid_t mypid;
    int min_priority;

    uint32_t alpha_fp;

    struct sched_param my_sched_params;

    int procfile;

    enum budget_type local_budget_type;
    job_mgt_cmd_t cmd;

    int ret_val;

    mypid =  getpid();


/***************************************************************************/
//check the alpha value
    alpha = alpha * (double)(((uint32_t)1) << 16);
    if(alpha >= (double)(((uint64_t)1) << 32))
    {
        fprintf(stderr, "pbs_SRT_setup: alpha value is too large. Must be less"
                        " than %f!\n", (double)((uint32_t)1 << 16));
        goto straight_exit;
    }
    alpha_fp = (uint32_t)alpha;

/***************************************************************************/
//setup Linux scheduling parameters

    min_priority = sched_get_priority_min(SCHED_FIFO);
    /*Set the priority*/
    my_sched_params.sched_priority = min_priority;

    /*Try to change the scheduling policy and priority*/
    ret_val = sched_setscheduler(mypid, SCHED_FIFO, &my_sched_params);
    if(0 != ret_val)
    {
        perror("ERROR: pbs_SRT_setup");
        fprintf(stderr, "pbs_SRT_setup: Failed to set scheduling policy!\n");
        goto straight_exit;
    }

/***************************************************************************/
//check if logging is enabled and if so allocate memory for it

    handle->loglevel = loglevel;
    handle->job_count = 0;
    
    if(loglevel >= SRT_LOGLEVEL_SUMMARY)
    {
        handle->log_file = fopen(logFileName, "w");
        if(handle->log_file == NULL)
        {
            perror("pbs_SRT_setup: Failed to open log file: ");
            ret_val = -1;
            goto straight_exit;
        }

        if(loglevel >= SRT_LOGLEVEL_FULL)
        {
            handle->log = (struct SRT_job_log*)malloc(logCount*sizeof(struct SRT_job_log));
            if(handle->log == NULL)
            {
                perror("ERROR: ");
                fprintf(stderr, "pbs_SRT_setup: Failed to allocate memory for log!\n");
                ret_val = -1;
                goto lclose_exit;
            }

            ret_val = mlock(handle->log, (logCount*sizeof(struct SRT_job_log)));
            if(ret_val)
            {
                perror("pbs_SRT_setup: mlock failed for the log array");
                ret_val = -1;
                goto free_exit;
            }

            handle->pu_c0   = (int64_t*)calloc(logCount*4, sizeof(int64_t));
            if(NULL == handle->pu_c0)
            {
                perror("pbs_SRT_setup: calloc failed for the predictor output arrays");
                ret_val = -1;
                goto free_exit;
            }
            else
            {
                handle->pvar_c0 = &(handle->pu_c0[logCount]);
                handle->pu_cl   = &(handle->pvar_c0[logCount]);
                handle->pvar_cl =  &(handle->pu_cl[logCount]);
            }
            
            ret_val = mlock(handle->pu_c0, (logCount*4 * sizeof(int64_t)));
            if(ret_val)
            {
                perror("pbs_SRT_setup: mlock failed for the predictor output arrays");
                ret_val = -1;
                goto free2_exit;
            }
            handle->log_size = logCount;
        }/* if(loglevel > SRT_LOGLEVEL_SUMMARY)*/
    }/* if(loglevel > SRT_LOGLEVEL_NONE)*/

/***************************************************************************/
//regster with the pbs module and setup the pbs scheduling parameters

    //setup PBS scheduling parameters
    ret_val = open("/proc/sched_rt_jb_mgt",  O_RDWR);
    if(ret_val == -1)
    {
        perror("pbs_SRT_setup: Failed to open \"/proc/sched_rt_jb_mgt\":\n");
        goto free2_exit;
    }
    procfile = ret_val;

    cmd.cmd = SRT_CMD_SETUP;
    cmd.args[0] = period;
    cmd.args[1] = (u64)&local_budget_type;
    cmd.args[2] = (u64)&(handle->reservation_period);
    ret_val = write(procfile, &cmd, sizeof(cmd));
    if(ret_val != sizeof(cmd))
    {
        perror("pbs_SRT_setup: write of a SRT_CMD_SETUP cmd failed!\n");
        goto close_exit;
    }
    
    handle->budget_type = local_budget_type;
    handle->period      = period;
    handle->estimated_mean_exectime 
                        = estimated_mean_exectime;
    handle->alpha_fp    = alpha_fp;

    cmd.cmd = SRT_CMD_START;
    ret_val = write(procfile, &cmd, sizeof(cmd));
    if(ret_val != sizeof(cmd))
    {
        perror("pbs_SRT_setup: write of a SRT_CMD_START cmd failed!\n");
        goto close_exit;
    }

    ret_val = 0;

/***************************************************************************/
//The execution-time predictor should have been already setup when passed
    handle->predictor = predictor;
    
    handle->procfile = procfile;
    return ret_val;

close_exit:
    close(procfile);
    handle->procfile = 0;
free2_exit:
    if(loglevel >= SRT_LOGLEVEL_FULL)
    {
        free(handle->pu_c0);
        handle->pu_c0   = NULL;
        handle->pvar_c0 = NULL;
        handle->pu_cl   = NULL;
        handle->pvar_cl = NULL;
    }
free_exit:
    if(loglevel >= SRT_LOGLEVEL_FULL)
    {
        free(handle->log);
        handle->log = NULL;
    }
lclose_exit:
    if(loglevel >= SRT_LOGLEVEL_SUMMARY)
    {
        fclose(handle->log_file);
        handle->log_file = NULL;
    }
straight_exit:
    return ret_val;
}

int SRT_sleepTillFirstJob(SRT_handle *handle)
{
    int ret = 0;
    job_mgt_cmd_t cmd;

    /*The execution time of the first job is not yet known.
    There is no valid data to read.
    The predictor should not be invoked*/
    
    /*Setup the NEXTJOB command with the predicted execution time
    specified in command-line arguments and 0 variance*/
    cmd.cmd = SRT_CMD_NEXTJOB;    
    cmd.args[0] = handle->estimated_mean_exectime;
    cmd.args[1] = 0;
    cmd.args[2] = handle->estimated_mean_exectime;
    cmd.args[3] = 0;
    cmd.args[4] = handle->alpha_fp;
    
    /*If "FULL" logging is enabled, log the predicted execution time and the estimated
    variance in the prediction error*/
    if( (handle->loglevel >= SRT_LOGLEVEL_FULL) && 
        (handle->job_count < handle->log_size))
    {
        handle->pu_c0[0]    = cmd.args[0];
        handle->pvar_c0[0]  = cmd.args[1];
        handle->pu_cl[0]    = cmd.args[2];
        handle->pvar_cl[0]  = cmd.args[3];
    }

    /*Issue the NEXTJOB command*/
    ret = write(handle->procfile, &cmd, sizeof(cmd));
    if(ret != sizeof(cmd))
    {
        perror("SRT_waitTillFirstJob: write of a SRT_CMD_NEXTJOB cmd failed!\n");
        ret = -1;
    }
    else
    {
        ret = 0;
    }

    return ret;
}

//FIXME: Need to handle closes forced by the system,
//like when the allocator task closes and the SRT task is forced to close
int SRT_sleepTillNextJob(SRT_handle *handle)
{
    int ret = 0;

    struct SRT_job_log local_job_log;
    struct SRT_job_log *job_log_p;
    u64 CPU_usage;

    job_mgt_cmd_t cmd;

    int64_t u_c0, var_c0;
    int64_t u_cl, var_cl;

    /*FIXME: Use VIC rather than runtime*/
    /*Get various data such as execution time for the job that just completed*/
    job_log_p = (   (handle->loglevel >= SRT_LOGLEVEL_FULL) && 
                    (handle->job_count < handle->log_size)  )?
                    &(handle->log[handle->job_count]) : &(local_job_log);

    ret = read( handle->procfile, 
                job_log_p, 
                sizeof(struct SRT_job_log));
    if(ret != sizeof(struct SRT_job_log))
    {
        ret = -1; 
        goto exit0;
    }
    
    CPU_usage = (BUDGET_VIC == handle->budget_type)? 
                job_log_p->runVIC2 :
                job_log_p->runtime2;

    /*Increment the job count*/
    (handle->job_count)++;

    /*Update the predictor and predict the execution time of the next job*/
    ret = handle->predictor->update(handle->predictor->state,
                                    CPU_usage,
                                    &u_c0, &var_c0,
                                    &u_cl, &var_cl);
    if(ret == -1)
    {
        /*If the predictor is not ready to produce valid output (still warming up)
        use the budget values specified in the command-line arguments*/
        u_c0    = handle->estimated_mean_exectime;
        var_c0  = 0;
        u_cl    = handle->estimated_mean_exectime;
        var_cl  = 0;
    }
    
    /*Issue the SRT_CMD_NEXTJOB command with the updated prediction*/
    cmd.cmd = SRT_CMD_NEXTJOB;
    cmd.args[0] = u_c0;
    cmd.args[1] = var_c0;
    cmd.args[2] = u_cl;
    cmd.args[3] = var_cl;
    cmd.args[4] = handle->alpha_fp;
    ret = write(handle->procfile, &cmd, sizeof(cmd));
    if(ret != sizeof(cmd))
    {
        perror("pbs_begin_SRT_job: write of a SRT_CMD_NEXTJOB cmd failed!\n");
        return -1;
    }

    /*If "FULL" logging is enabled, log the predicted execution time and the estimated
    variance in the prediction error*/
    if( (handle->loglevel >= SRT_LOGLEVEL_FULL) && 
        (handle->job_count < handle->log_size))
    {
        handle->pu_c0[handle->job_count] = cmd.args[0];
        handle->pvar_c0[handle->job_count] = cmd.args[1];
        handle->pu_cl[handle->job_count] = cmd.args[2];
        handle->pvar_cl[handle->job_count] = cmd.args[3];
    }
    
    ret = 0;

exit0:
    return ret;
}

void SRT_close(SRT_handle *handle)
{
    int i;
    struct SRT_job_log *log_entry;

    job_mgt_cmd_t cmd;
    SRT_summary_t summary = {0, 0, 0, 0};
    int ret_val;
    
    int64_t relative_LFT;
    unsigned long miss;

    /*Stop adaptive budget allocation and enforcement for the task*/
    cmd.cmd = SRT_CMD_STOP;
    ret_val = write(handle->procfile, &cmd, sizeof(cmd));
    if(ret_val != sizeof(cmd))
    {
        perror("pbs_SRT_close: write of a SRT_CMD_STOP cmd failed!\n");
    }
    
    if(handle->loglevel >= SRT_LOGLEVEL_SUMMARY)
    {
        cmd.cmd = SRT_CMD_GETSUMMARY;
        cmd.args[0] = (s64)&summary;
        ret_val = write(handle->procfile, &cmd, sizeof(cmd));
        if(ret_val != sizeof(cmd))
        {
            perror("pbs_SRT_close: write of a SRT_CMD_GETSUMMARY cmd failed!\n");
        }
        
        fprintf(handle->log_file,   "%i, %llu, %llu, %i, %llu, %llu, %llu, %llu, %llu, "
                                    "%llu, %llu\n\n", 
                                    (int)               getpid(),
                                    (unsigned long long)handle->period,
                                    (unsigned long long)handle->reservation_period,
                                    (int)               handle->budget_type,
                                    (unsigned long long)handle->estimated_mean_exectime,
                                    (unsigned long long)handle->job_count,
                                    (unsigned long long)summary.allocated_budget,
                                    (unsigned long long)summary.consumed_budget_ns,
                                    (unsigned long long)summary.consumed_budget_VIC,
                                    (unsigned long long)summary.total_misses,
                                    (unsigned long long)summary.total_CPU_budget_capacity);

        if(handle->loglevel >= SRT_LOGLEVEL_FULL)
        {
            /*Note: when the log is read, the runtime and run VIC correspond to the 
                    current job, whereas all other statistics correspond to the previous 
                    job. The first job log is for job -1 and the corrsponding is actually 
                    data is garbage. So account for the "phase difference" between 
                    runtime and runVIC, and the rest of the data*/
            for(i = 1; i < handle->job_count; i++)
            {
                log_entry = &(handle->log[i]);
                relative_LFT = log_entry->abs_LFT - log_entry->abs_releaseTime;
                miss = (relative_LFT > handle->period);
                fprintf(handle->log_file,   "%lu, %lu, %lu, "
                                            "%lu, %lu, %u, %u, %lu, %lu, %lu, %lu\n",
                                            (unsigned long)handle->log[(i-1)].runtime2,
                                            (unsigned long)handle->log[(i-1)].runVIC2,
                                            miss,
                                            (unsigned long)log_entry->abs_releaseTime,
                                            (unsigned long)log_entry->abs_LFT,
                                            log_entry->last_sp_budget_allocated,
                                            log_entry->last_sp_budget_used_sofar,
                                            (unsigned long)handle->pu_c0[i],
                                            (unsigned long)handle->pvar_c0[i],
                                            (unsigned long)handle->pu_cl[i],
                                            (unsigned long)handle->pvar_cl[i]);
            }

            free(handle->pu_c0);
            free(handle->log);
        }
        
        fclose(handle->log_file);
    }
    
    close(handle->procfile);
}

