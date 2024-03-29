#include <linux/init.h>
#include <linux/module.h>
#include <asm/unistd.h>
#include <linux/hrtimer.h>

/*
 * struct hrtimer
 * hrtimer_init()
 * hrtimer_start()
 */

#include <linux/ktime.h>
/*
 * ktime_set
 */

#include "freq_change_test.h"
#include "LAMbS_molookup.h"

#define RP 20000000
#define RP_COUNT 2

struct hrtimer rp_timer;
u64 test_sched_0[32];
u64 test_sched_1[12] = {1000000, 0, 1000, 1000, 0, 1998000, 0, 15000000, 0, 1000000, 1000000, 0};
u64 test_sched_2[12] = {1000000, 0, 15000000, 1000000, 199800, 1000000, 1000, 1000, 0, 0, 0, 0};
u64* test_sched;
int rp_count;

static int __init test_setup(void) {
    u64 per_mo;
    int i;

    test_sched = test_sched_2;
    
    rp_count = 0;

    per_mo = (u64)RP/LAMbS_mo_struct.count;
    printk(KERN_ALERT "using per_mo of %llu ns\n", per_mo);
    for (i = 0; i < LAMbS_mo_struct.count; i++) {
	test_sched_0[i] = per_mo;
    }

    
    hrtimer_init(&rp_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    printk(KERN_ALERT "hrtimer_init: rp_timer initialized\n");
    rp_timer.function = &next_rp;
    hrtimer_start(&rp_timer, ktime_set(0, RP), HRTIMER_MODE_REL);

    return 0;
}

static void __exit end_count(void) {
    printk(KERN_ALERT "test module ended\n");
}
/*
static void change_freq(void) {


    next_rp(&rp_timer);

}
*/

enum hrtimer_restart next_rp(struct hrtimer* timer) {
    /*
    if (irqs_disabled()) 
	printk(KERN_ALERT "irqs_disabled in next_rp (callback function)\n");
    else
	printk(KERN_ALERT "irqs_enabled in next_rp (Callback function)\n");
    */
    if (rp_count < RP_COUNT) {
	rp_count++;
	hrtimer_forward_now(&rp_timer, ktime_set(0, RP));
	printk(KERN_ALERT "hrtimer_forward_now: rp_timer started\n");
	LAMbS_cpufreq_sched(test_sched);
	return HRTIMER_RESTART;
    } else {
	return HRTIMER_NORESTART;
    }
}

module_init(test_setup);
module_exit(end_count);

MODULE_LICENSE("GPL");
