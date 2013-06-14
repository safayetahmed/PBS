
#include <linux/cpufreq.h>
/*
    cpufreq_register_notifier
    cpufreq_unregister_notifier    
*/

#include <linux/notifier.h>
/*
    struct notifier_block
*/

#include "LAMbS_molookup.h"
#include "LAMbS_mostat.h"

static int LAMbS_motrans_notifier(  struct notifier_block *nb,
                                    unsigned long val, void *data)
{
    int ret = 0;
    
    /*Data contains a pointer to a cpufreq_freqs structure.
    The cpufreqs structure describes the previous frequency,
    next frequency, the cpu where the transition takes place,
    and flags related to the driver*/
    struct cpufreq_freqs *freq = data;
    int old_mo, new_mo;
    int old_moi, new_moi;    
    
    /*Check what type of event this notification is for*/
    switch(val)
    {
        case CPUFREQ_PRECHANGE:
            /*Ignored*/
            break;
            
        case CPUFREQ_POSTCHANGE:
            /*Pick the POSTCHANGE event since it is closer to the actual change in MO*/
            
            /*Determine the index of the previous and current modes of operation*/
            old_mo  = freq->old;
            new_mo  = freq->new;
            old_moi = LAMbS_molookup(old_mo);
            new_moi = LAMbS_molookup(new_mo);
            
            /*Perform the update operation for the mostat mechanism*/
            LAMbS_mostat_transition_p(old_moi, new_moi);
            break;
        
        case CPUFREQ_RESUMECHANGE:
            /*Ignored*/
            break;
            
        case CPUFREQ_SUSPENDCHANGE:
            /*Ignored*/
            break;
            
        default:
            printk(KERN_INFO "LAMbS_motrans_notifier: unrecognized CPUFREQ_CHANGE event");
            ret = -1;
            break;
    }
    
    return ret;
}

static struct notifier_block LAMbS_motrans_notifier_block = 
{
    .notifier_call = LAMbS_motrans_notifier
};

static int LAMbS_mo_setup = 0;

int LAMbS_mo_init(int verbose)
{
    int ret;
    
    /*Check that LAMbS_mo has not already been setup*/
    if(0 != LAMbS_mo_setup)
    {
        printk(KERN_INFO "LAMbS_mo_init: has been called previously");
        goto error0;
    }
    
    /*Setup the mo lookup table*/
    ret = LAMbS_molookup_init();
    if(0 != ret)
    {
        printk(KERN_INFO "LAMbS_mo_init: LAMbS_molookup_init failed!");
        goto error0;
    }

    /*Test the mo lookup table*/
    ret = LAMbS_molookup_test(verbose);
    if(ret == -1)
    {
        printk(KERN_INFO "LAMbS_mo_init: LAMbS_molookup_test failed!");
        goto error1;
    }

    /*Setup the MO (frequency) change notifier*/
    ret = cpufreq_register_notifier(    &LAMbS_motrans_notifier_block,
                                        CPUFREQ_TRANSITION_NOTIFIER);
    if(0 != ret)
    {
        printk(KERN_INFO "LAMbS_mo_init: cpufreq_register_notifier failed!\n");
        goto error1;
    }
    
    /*Setup the mostat mechanism*/
    ret = LAMbS_mostat_init();
    if(ret == -1)
    {
        printk(KERN_INFO "LAMbS_mo_init: LAMbS_mostat_init failed!");
        goto error2;
    }
    
    LAMbS_mo_setup = 1;
    
    return 0;

error2:
    /*Remove the MO(frequency) change notifier*/
    cpufreq_unregister_notifier(    &LAMbS_motrans_notifier_block,
                                    CPUFREQ_TRANSITION_NOTIFIER);
error1:
    /*cleanup the MO lookup table*/
    LAMbS_molookup_free();
error0:
    return -1;
}

void LAMbS_mo_free(void)
{
    if(0 != LAMbS_mo_setup)
    {
        LAMbS_mo_setup = 0;
        
        /*Cleanup the mostat mechanism*/
        LAMbS_mostat_free();

        /*FIXME*/
        {
            unsigned long irq_flags;
            int moi;
            
            local_irq_save(irq_flags);
                 _LAMbS_mostat_update();
            local_irq_restore(irq_flags);
        
            printk(KERN_INFO "LAMbS_mo_free: printing mostat.stat");
            for(moi = 0; moi < LAMbS_mo_count; moi++)
            {
                printk(KERN_INFO "\t%i) %llu", moi, mostat.stat[moi]);
            }
        }
        /*FIXME*/

        /*Remove the MO(frequency) change notifier*/
        cpufreq_unregister_notifier(    &LAMbS_motrans_notifier_block,
                                        CPUFREQ_TRANSITION_NOTIFIER);
        
        /*Clean up the MO lookup table*/
        LAMbS_molookup_free();
    }
}

