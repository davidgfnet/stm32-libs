
#include <stdlib.h>
#include <stdint.h>

// This implementation works with overlapping buffers
void *memcpy(void * dst, const void * src, size_t count) {
        uint8_t * dstb = (uint8_t*)dst;
        uint8_t * srcb = (uint8_t*)src;
        while (count--)
                *dstb++ = *srcb++;
        return dst;
}

void *memmove(void *dest, const void *src, size_t n) {
	return memcpy(dest, src, n);
}

size_t strlen(const char *s) {
        size_t ret = 0;
        while (*s++)
                ret++;
        return ret;
}

void *memset(void *s, int c, size_t n) {
        uint8_t * dstb = (uint8_t*)s;
        while (n--)
                *dstb++ = c;
        return s;
}

char *strcat(char *dest, const char *src) {
	char *c = dest;
	while (*dest != 0)
		dest++;
	while (*src != 0)
		*dest++ = *src++;
	*dest = 0; // Null terminator
	return c;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	uint8_t *u1 = (uint8_t*)s1;
	uint8_t *u2 = (uint8_t*)s2;
	while (n--) {
		int r = (int)(*u1++) - (int)(*u2++);
		if (r)
			return r;
	}
	return 0;
}

