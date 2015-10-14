#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

#ifndef	MAP_NORESERVE
#define	MAP_NORESERVE	0
#endif

volatile char ch;

main(){
    struct stat statbuf;
    uchar *buf;
    fstat(0, &statbuf);
    buf = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED|MAP_NORESERVE,
	0, 0);
    if(buf != (uchar*)(-1)){
	uchar *cur, *lim = &buf[statbuf.st_size];
	for(cur = buf; buf != lim; ++cur){
	    ch = *cur;
	}
	munmap(buf, statbuf.st_size);
    }
}
