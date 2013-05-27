#include <stdint.h>

/*This is the user level header file for the PBS module*/

#define PBS_IOCTL_BWMGT_PERIOD 		    0
#define PBS_IOCTL_BWMGT_ALLOC_RUNTIME	1
#define PBS_IOCTL_BWMGT_START			3
#define PBS_IOCTL_BWMGT_MAX			    4

#define HISTLIST_SIZE	(1<<20) //1MB
#define HISTLIST_ORDER	(20-PAGE_SHIFT)

#define ALLOC_SIZE	(1<<17)
#define ALLOC_ORDER	(17-PAGE_SHIFT)


//Each SRT_history structure is 64 bytes long (to fit in 1 cache line)
//ideally the structure should be stored in an aligned address
//the elements of the structure has been arranged for compactness
//the actual history list should be aligned such that SIMD instructions can be applied to it

typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int32_t     s32;
typedef int64_t     s64;

typedef struct _SRT_history
{
	u64	job_release_time;	// 8 bytes
	s64 u_c0;               // 8 bytes
	s64 std_c0;             // 8 bytes
	s64 u_cl;               // 8 bytes
	s64 std_cl;             // 8 bytes
	u32	pid;			    // 4 bytes
	u32	current_runtime;	// 4 bytes
	u32	sp_till_deadline;   // 4 bytes
	u32	sp_per_tp;		    // 4 bytes

	unsigned short	queue_length;	// 2 bytes

	//In a 2MB page, can have at most 2^15 elements of size 64B
	//The index of the next valid structure can be stored in a
	//short
	unsigned short	next; 	// 2 bytes
	unsigned short	prev;	// 2 bytes

	char	        history_length; // 1 bytes

	char	        buffer_index;   //1 byte

    // 8 + 4*8 + 4*4 + 3*2 + 2*1 = 64 bytes

	u32	history[112]; //112*4 bytes = 448 bytes

    //448 + 64 = 512 bytes
} SRT_history_t;

typedef struct _history_list_header
{
	u64			prev_sp_boundary;		//8
	u64			scheduling_period;	//8

	u64			max_allocator_runtime;	//8
	u64			last_allocator_runtime;	//8

	unsigned short	SRT_count;			//2
	unsigned short	first;			//2
	//sum so far = 20 bytes
	unsigned char	buffer[492];//492 bytes to make the structure 512 bytes
} history_list_header_t;


