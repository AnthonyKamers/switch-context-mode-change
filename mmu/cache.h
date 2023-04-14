#ifndef CACHE
#define CACHE
#include <stdint.h>
/* Register offsets */
#define L2_CACHE_CONFIG	0x000
#define L2_CACHE_ENABLE	0x008
#define L2_LIM_BASE 0x0a000000

#define MASK_NUM_WAYS	0xff00 
#define NUM_WAYS_SHIFT	8


int cache_enable_ways(void)
{
	uint64_t base;
	uint64_t config;
	uint64_t ways;

	volatile uint64_t *enable;

	base = L2_LIM_BASE;

	//config = *(( char *)base + L2_CACHE_CONFIG);
	ways = (MASK_NUM_WAYS) >> NUM_WAYS_SHIFT;

	enable = (volatile uint64_t *)(base + L2_CACHE_ENABLE);

	(*enable) = ways - 1;
	return 0;
}

#endif