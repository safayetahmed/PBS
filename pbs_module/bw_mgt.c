/**********************************************************************

Global variables and functions associated with the sched_rt_bw_mgt file
and allocator task.

***********************************************************************/

#include "bw_mgt.h"
#include "jb_mgt.h"
#include "Allocator_cmd.h"
#include "pbs_mmap.h"
#include "LAMbS_models.h"
#include "LAMbS_mo.h"
#include "pbs_budget.h"

/**********************************************************************

Global variables and functions associated with the allocator task.

***********************************************************************/

unsigned char    allocator_state = MODULE_LOADED;

/*The number of SRT tasks active in the system*/
static atomic_t SRT_count;


/**********************************************************************

Function headers and structure deffinitions for the sched_bw_mgt file.

***********************************************************************/

ssize_t bw_mgt_write(   struct file *filep, 
                        const char __user *data, 
                        size_t count, 
                        loff_t *offset);
int bw_mgt_mmap(struct file *file, 
                struct vm_area_struct *vmas);
int bw_mgt_open(struct inode *inode, 
                struct file *file);
int bw_mgt_release( struct inode *inode, 
                    struct file *filp);

/*The procfs file that is accessed by the bandwidth allocation process*/
char*  bw_mgt_file_name = "sched_rt_bw_mgt";
struct proc_dir_entry* p_bw_mgt_file;
struct file_operations bw_mgt_fops = {
    .owner      =   THIS_MODULE,
    .write      =   bw_mgt_write,
    .mmap       =   bw_mgt_mmap,
    .open       =   bw_mgt_open,
    .release    =   bw_mgt_release
};
 
/**********************************************************************

File handling code associated with the sched_rt_bw_mgt file and the
helper function allocator_sleep

***********************************************************************/

void allocator_sleep(void)
{
    set_current_state(TASK_UNINTERRUPTIBLE);
    preempt_enable_no_resched();
    schedule();
    preempt_disable();
}

int bw_mgt_mmap(struct file *file, struct vm_area_struct *vmas)
{
    int ret;

    if((allocator_state == ALLOCATOR_OPEN) || (allocator_state == ALLOCATOR_START))
    {
        ret =  do_pbs_mmap(vmas);
    }
    else
    {
        printk(KERN_INFO "Process %d calling mmap while allocator state = %i!\n", current->pid, allocator_state);
        ret = -EINVAL;
    }

    return ret;
}

ssize_t bw_mgt_write(   struct file *filep, 
                        const char __user *data, 
                        size_t count, 
                        loff_t *offset)
{
    bw_mgt_cmd_t cmd;
    ssize_t ret;
    
    s64 allocator_runtime;
    s64 allocator_period;
    
    unsigned long irq_flags;
    
    if(sizeof(bw_mgt_cmd_t) != count)
    {
        printk(KERN_INFO    "The argument written through bw_mgt_write by process %d "
                            "is not of valid size.", current->pid);
        ret = -EINVAL;
        goto exit0;
    }

    if( copy_from_user( &cmd, data, sizeof(bw_mgt_cmd_t)))
    {
        ret = -EFAULT;
        goto exit0;
    }
    
    switch(cmd.cmd)
    {
        case ALLOCATOR_CMD_SETUP_START:
            /*The ALLOCATOR_CMD_SETUP command should only be 
            issued with the allocator in the ALLOCATOR_OPEN state*/
            if(ALLOCATOR_OPEN != allocator_state)
            {
                printk(KERN_INFO "Invalid attempt to issue ALLOCATOR_CMD_SETUP command "
                                 "while the allocator (%d) is not in the "
                                 "\"ALLOCATOR_OPEN\" state.\n", current->pid);
                ret = -EBUSY;
                goto exit0;
            }
        
            if(cmd.args[1] > 0)
            {
                if((cmd.args[2] > 0) && (cmd.args[2] <= cmd.args[1]))
                {
                    allocator_period = cmd.args[1];
                    allocator_runtime= cmd.args[2];
                    printk(KERN_INFO    "Received ALLOCATOR_CMD_SETUP command!"\
                                        "Setting allocator (%i) to a period of %li, "\
                                        "and a budget of %li.\n",
                                        current->pid,
                                        (long int)allocator_period,
                                        (long int)allocator_runtime);
                }
                else
                {
                    printk(KERN_INFO    "Received ALLOCATOR_CMD_SETUP command with "\
                                        "invalid value for the budget for the allocator "\
                                        "process (%i). The allocated budget must be "\
                                        "larger than the period. Received parameters:"
                                        "Period (%li), Budget (%li).\n", 
                                        current->pid,
                                        (long int)cmd.args[1],
                                        (long int)cmd.args[2]);
                    ret = -EINVAL;
                    goto exit0;
                }
            }
            else
            {
                printk(KERN_INFO    "Received ALLOCATOR_CMD_SETUP command with invalid "\
                                    "value for the period for the allocator process "\
                                    "(%i). The allocated period must be larger than 0."\
                                    "Received parameters: Period (%li), Budget (%li).\n",
                                    current->pid,
                                    (long int)cmd.args[1],
                                    (long int)cmd.args[2]);
                ret = -EINVAL;
                goto exit0;
            }
    
            ret = pbs_budget_settype((enum budget_type)cmd.args[0]);
            if( 0 != ret)
            {
                printk(KERN_INFO    "Received ALLOCATOR_CMD_SETUP command with invalid "\
                                    "value (%li) for argument 0 (budget_type). Argument "\
                                    "0 should have a value from the enumerator "\
                                    "budget_type. ", (long int)cmd.args[0]);
                ret = -EINVAL;
                goto exit0;
            }
    
            ret = setup_allocator(allocator_period, allocator_runtime);
            if(ret == 0)
            {
                allocator_state = ALLOCATOR_START;
            }
            else
            {
                printk(KERN_INFO "setup_allocator() failed! Returned (%li)", ret);
            }
            break;
            
        case ALLOCATOR_CMD_NEXTJOB:
            //Is this the first time the NEXTJOB command has been issued?
            if(allocator_state == ALLOCATOR_START)
            {
                printk(KERN_INFO    "ALLOCATOR_CMD_NEXTJOB command issued for the first "
                                    "time by allocator(%i)", current->pid);
            
                //from this point forward, donot allow other tasks to preempt the 
                //allocator task
                preempt_disable();
                ret = start_pbs_timing();
                if(ret)
                {
                    preempt_enable();
                    goto exit0;
                }
                
                //state is changed to ALLOCATOR LOOP after the first execution of the 
                //function "sp_timer_func" this is done to ensure that expires_next 
                //and expires_prev are initialized by it
                allocator_state = ALLOCATOR_LOOP;
                
                /*Execute the operation-mode schedule*/
        		/* LAMbS_cpufreq_schedule(LAMbS_mo_sched);*/
                allocator_sleep();
                
                //This is the start of the first scheduling period
                first_sched_period_tick();
            }
            else
            {
                //check before going to sleep
                if(allocator_state == ALLOCATOR_LOOP)
                {         
                    /*Enter critical section. The following function needs to be 
                    executed with interrupts disabled*/
                    local_irq_save(irq_flags);
                                        
                        /*Now that model coefficients have been update, update the 
                        current instruction retirement rate*/
                        LAMbS_update_current_instrate();
                    
                    /*Exit critical section*/
                    local_irq_restore(irq_flags);
                    
                    //assign new bandwidths
                    assign_budgets();

                    /*Execute the operation-mode schedule*/
            		/* LAMbS_cpufreq_schedule(LAMbS_mo_sched);*/
            		
                    //sleep until the next scheduling period
                    allocator_sleep();

                    //check after going to sleep
                    if(allocator_state != ALLOCATOR_LOOP)
                    {
                        ret = -ECANCELED;
                        goto exit0;
                    }

                    //This is the start of the next scheduling period
                    sched_period_tick();

                }
                else
                {
                    ret = -ECANCELED;
                    goto exit0;
                }
            }

            break;

        case ALLOCATOR_CMD_STOP:
            printk(KERN_INFO    "ALLOCATOR_CMD_STOP command issued by the allocator(%i)!", 
                                current->pid);
            if(ALLOCATOR_LOOP == allocator_state)
            {
                stop_pbs_timing(0);

                preempt_enable();
            }
            else 
            {
                if(ALLOCATOR_PRECLOSING == allocator_state)
                {
                    preempt_enable();
                }
            }
            
            allocator_state = ALLOCATOR_CLOSING;
            break;
            
        default:
            printk(KERN_INFO    "Invalid cmd code in bw_mgt_cmd_t structure passed to "
                                "bw_mgt_write by process %d.", current->pid);
            ret = -EINVAL;
            goto exit0;
    }

    ret = count;
exit0:
    return ret;
}


int bw_mgt_open(struct inode *inode, struct file *file)
{
    int ret = 0;

    if(allocator_state != MODULE_LOADED)
    {
        ret = -EBUSY;
        goto error0;
    }

    printk(KERN_INFO "*********************************\n");
    printk(KERN_INFO "bw_mgt_open called by process %d.\n", current->pid);

    ret = init_loaddataList();
    if(ret != 0)
    {
        printk(KERN_INFO "bw_mgt_open: init_loaddataList failed!");
        goto error0;
    }

    allocator_state = ALLOCATOR_OPEN;

    return 0;
error0:
    return ret;
}

int bw_mgt_release(struct inode *inode, struct file *filp)
{
    int local_count;
    printk(KERN_INFO "bw_mgt_release called by process %d.\n", current->pid);

    switch(allocator_state)
    {
        case ALLOCATOR_LOOP:

            stop_pbs_timing(0);

        case ALLOCATOR_PRECLOSING:
            preempt_enable();
            
        default:
            allocator_state = ALLOCATOR_CLOSING;
            local_count = atomic_read(&SRT_count);
            if(local_count == 0)
            {
                allocator_state = MODULE_LOADED;
            }
            else
            {
                printk(KERN_INFO "Exiting with %i tasks in system.\n", local_count);
            }
            
            break;
    }
    
    return 0;
}

/**********************************************************************

Functions for keeping track of the number of SRT tasks in the system 
and asynchronously disabling the allocator.

***********************************************************************/

void reset_SRT_count(void)
{
    atomic_set(&SRT_count, 0);
}

void increment_SRT_count(void)
{
    atomic_inc(&SRT_count);
}

void decrement_SRT_count(void)
{
    int local_count;

    preempt_disable();

    local_count = atomic_dec_return(&SRT_count);

    if(local_count == 0)
    {
        if(allocator_state == ALLOCATOR_CLOSING)
        {
            allocator_state = MODULE_LOADED;
        }
    }

    preempt_enable();
}

int disable_allocator(void)
{
    int ret = 0;

    preempt_disable();
    if(allocator_state == ALLOCATOR_LOOP)
    {
        ret = stop_pbs_timing(1);
        if(ret == 0)
        {
            allocator_state = ALLOCATOR_PRECLOSING;
        }
    }
    else
    {
        ret = -EBUSY;
    }
    preempt_enable();

    return ret;
}

/**********************************************************************

Initialization/Uninitialization code associated with the sched_rt_bw_mgt file

***********************************************************************/

int init_pbs_actv(void);
void uninit_pbs_actv(void);

int init_bw_mgt(void)
{
    int ret = 0;

    /*Allocate pages for mapping*/
    ret = allocate_mapping_pages();
    if(0 != ret)
    {
        printk(KERN_INFO "init_bw_mgt: allocate_mapping_pages failed");
        goto error0;
    }

    /*Initialize the sched_pbs_actv mechanism*/
    ret = init_pbs_actv();
    if(0 != ret)
    {
        printk(KERN_INFO "init_bw_mgt: init_pbs_actv failed");
        goto error1;
    }

    /*Create the file in the root of the profcs directory, initially with no permissions for others*/
    p_bw_mgt_file = create_proc_entry(bw_mgt_file_name, 0600, NULL);
    if(p_bw_mgt_file == NULL)
    {
        printk(KERN_INFO "init_bw_mgt: create_proc_entry failed");
        ret = -ENOMEM;
        goto error2;
    }
    p_bw_mgt_file->data = NULL;
    p_bw_mgt_file->proc_fops = &bw_mgt_fops;
    p_bw_mgt_file->mode = 0666;

    allocator_state = MODULE_LOADED;

    return 0;

error2:
    uninit_pbs_actv();
error1:
    free_mapping_pages();
error0:
    return ret;
}

void uninit_bw_mgt(void)
{
    remove_proc_entry(bw_mgt_file_name, NULL);

    uninit_pbs_actv();
    
    free_mapping_pages();
}

