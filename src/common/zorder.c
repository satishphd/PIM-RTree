#include <stdint.h>

#include "common/point.h"

// z-order bit interleaving
uint64_t z_xy2d(const ordpair_t xy)
{ 
            uint64_t x = xy.x;
			x = (x ^ (x <<  16)) & 0x0000ffff0000ffff; 
			x = (x ^ (x <<  8))  & 0x00ff00ff00ff00ff; 
			x = (x ^ (x <<  4))  & 0x0f0f0f0f0f0f0f0f; 
			x = (x ^ (x <<  2))  & 0x3333333333333333; 
			x = (x ^ (x <<  1))  & 0x5555555555555555; 
			
			uint64_t y = xy.y;
			y = (y ^ (y <<  16)) & 0x0000ffff0000ffff;
			y = (y ^ (y <<  8))  & 0x00ff00ff00ff00ff; 
			y = (y ^ (y <<  4))  & 0x0f0f0f0f0f0f0f0f; 
			y = (y ^ (y <<  2))  & 0x3333333333333333; 
			y = (y ^ (y <<  1))  & 0x5555555555555555; 
			
			return (y<<1) | x;
}