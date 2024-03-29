#ifndef MOCSAMPLE_INCLUDE
#define MOCSAMPLE_INCLUDE
    
    #include <linux/kernel.h>
    #include <linux/sched.h>
    #include <linux/hrtimer.h>
    #include <linux/interrupt.h>    
    #include "MOCsample_timer_command.h"
    
    typedef struct MOCsample_timer_s
    {
        ktime_t         sample_period;
        u64             sampled_count;
        u64             buffer_length;
        MOCsample_timed_sample_t    *samples_buffer;
        struct tasklet_hrtimer  timer;
    } MOCsample_timer_t;
    
    struct MOCsample_s;
    
    typedef int     (*MOCsample_init_t)(struct MOCsample_s*);
    typedef u64     (*MOCsample_read_t)(struct MOCsample_s*);
    typedef void    (*MOCsample_free_t)(struct MOCsample_s*);
    
    typedef struct MOCsample_s
    {
        struct preempt_notifier preempt_notifier;
        MOCsample_init_t    init;
        MOCsample_read_t    read;
        MOCsample_free_t    free;
        void*               state;
        u64                 last_sample;
        u64                 running_total;
        MOCsample_timer_t   timer;
    } MOCsample_t;
    
    /**/
    int MOCsample_alloc_init(void);
    void MOCsample_alloc_uninit(void);
    int MOCsample_alloc(const MOCsample_t *template, 
                        MOCsample_t **retval_p);
    void MOCsample_free(MOCsample_t *MOCsample_p);    

    extern const MOCsample_t MOCsample_inst_template;
    extern const MOCsample_t MOCsample_userinst_template;
    extern const MOCsample_t MOCsample_cycl_template;
    extern const MOCsample_t MOCsample_nsec_template;
    extern const MOCsample_t MOCsample_VIC_template;
    
    void    MOCsample_timer_threshold_init(void);
    void    MOCsample_timer_init(   MOCsample_t *MOCsample_p);
    ssize_t MOCsample_timer_write(  MOCsample_t *MOCsample_p, const char __user *src, size_t count);
    void    MOCsample_timer_free(   MOCsample_t *MOCsample_p);
    
#endif
