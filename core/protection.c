#include <inttypes.h>


void __loadcheck(void* pointer, __int64_t access_size) {
	asm("nop");
}

void __storecheck(void* pointer, __int64_t access_size) {
	asm("nop");
}
