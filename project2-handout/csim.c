//523031910469 马筱宇

/*Your job for Part A is to fill in the csim.c file so that it takes the same command line arguments and 
produces the identical output as the reference simulator.*/
/*For this this lab, you should assume that memory accesses are aligned properly, such that a single 
memory access never crosses block boundaries. By making this assumption, you can ignore the 
request sizes in the valgrind traces. */
#include "cachelab.h"
#include <asm-generic/errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "stdbool.h"
#include "assert.h"
#define MAXN 1000000
#define uint unsigned int
bool verbose = false, helper = false;
int s = 0, E = 0, b = 0;
//s : Number of set index bits (S = 2^s is the number of sets)
//E : Associativity (number of lines per set)
//b : Number of block bits (B = 2^b is the block size in bytes)
char tracefile[100] = {0};

unsigned long long HIT, MISS, EVICTION; 

struct CacheOperation{
    char operation;
    unsigned long long address;
    unsigned int size;

}cacheOp[MAXN];

struct CacheLine{
    unsigned long long tag;
    bool valid, dirty;
    int lru; // Least Recently Used counter
}cacheline[MAXN];

unsigned int timer = 0; // Timer for LRU replacement policy
int n_sets, n_ways;
int tot = 0; // Total number of cache operations
int parseInput(){
    char filepath[sizeof(tracefile) + 9]; // room for "./traces/" prefix
    // snprintf(filepath, sizeof(filepath), "./traces/%s", tracefile);
    strcpy(filepath, tracefile);
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening trace file '%s'\n", filepath);
        return 1;
    }

    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        if(line[0] == 'I') continue; // Ignore instruction loads
        char operation;
        unsigned long long addr;
        unsigned int size;
        if(sscanf(line, " %c %llx,%u", &operation, &addr, &size) == 3){
            cacheOp[tot].operation = operation;
            cacheOp[tot].address = addr;
            cacheOp[tot].size = size;
            tot++;
        }else{
            break; // Stop reading if the line is not in the expected format
        }
    }
    return 0;
}

void handle_L(unsigned long long addr, unsigned int set_index, unsigned long long tag, int size){
    //load operation
    uint startWay = set_index * n_ways, endWay = startWay + n_ways;
    uint j;
    for(j= startWay ;j < endWay; j++){
        if(cacheline[j].valid && cacheline[j].tag == tag){
            // Hit
            HIT++;
            cacheline[j].lru = timer++; // Update LRU counter
            if(verbose) printf("L %llu %d HIT", addr, size);
            break;
        }else if(!cacheline[j].valid){
            // Miss and empty line found
            MISS++;
            cacheline[j].valid = true; //load from memory
            cacheline[j].tag = tag;
            cacheline[j].lru = timer++; // Update LRU counter
            cacheline[j].dirty = false; // Not dirty since it's a load operation
            if(verbose) printf("L %llu %d MISS", addr, size);
            break;
        }
    }

    if(j == endWay){
        //Miss and no empty line found
        MISS++;
        EVICTION++;
        int minLRU=INT_MAX, pos;
        for(int k  = startWay ; k <endWay;k++){
            if(cacheline[k].lru <minLRU){
                minLRU = cacheline[k].lru;
                pos = k; // Find the line with the minimum LRU value
            }
        }
        //load from memory
        cacheline[pos].tag = tag; // Replace the line with the new tag
        cacheline[pos].lru = timer++; // Update LRU counter
        cacheline[pos].valid = true; // Mark it as valid
        cacheline[j].dirty = false; // Not dirty since it's a load operation
        if(verbose) printf("L %llu %d MISS EVICTION", addr, size);
    }

    return ;
}

void handle_S(unsigned long long addr, unsigned int set_index, unsigned long long tag, int size){
    //store operation
    uint startWay = set_index * n_ways, endWay = startWay + n_ways;
    uint j;
    for(j= startWay ;j < endWay; j++){
        if(cacheline[j].valid && cacheline[j].tag == tag){
            // Hit
            HIT++;
            cacheline[j].lru = timer++; // Update LRU counter
            cacheline[j].dirty = true; // Mark as dirty since it's a store operation
            if(verbose) printf("S %llu %d HIT", addr, size);
            break;
        }else if(!cacheline[j].valid){
            // Miss and empty line found
            MISS++;
            cacheline[j].valid = true; //load from memory
            cacheline[j].tag = tag;
            cacheline[j].lru = timer++; // Update LRU counter
            cacheline[j].dirty = true; // Mark as dirty since it's a store operation
            if(verbose) printf("S %llu %d MISS", addr, size);
            break;
        }
    }

    if(j == endWay){
        //Miss and no empty line found, evict a line
        MISS++;
        EVICTION++;
        int minLRU=INT_MAX, pos;
        for(int k  = startWay ; k <endWay;k++){
            if(cacheline[k].lru <minLRU){
                minLRU = cacheline[k].lru;
                pos = k; // Find the line with the minimum LRU value
            }
        }
        //load from memory
        if(cacheline[pos].dirty){
            if(verbose) printf("evict dirty %llx block\n", cacheline[pos].tag);
        }
        cacheline[pos].tag = tag; // Replace the line with the new tag
        cacheline[pos].lru = timer++; // Update LRU counter
        cacheline[pos].valid = true; // Mark it as valid
        cacheline[pos].dirty = true; // Mark as dirty since it's a store operation
        if(verbose) printf("S %llu %d MISS EVICTION", addr, size);
    }

    return ;
}


int main(int argc, char *argv[]){
    memset(cacheline, 0 , sizeof(cacheline));
    /*Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile> */
    if(argc < 5){
        fprintf(stderr, "Usage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n", argv[0]);
        return 1;
    }else{
        for(int i=1 ;i<argc ;i++){
            if(strcmp(argv[i], "-h") == 0){
                helper = true;
                continue;
            }else if(strcmp(argv[i], "-v") == 0){
                verbose = true;
                continue;
            }else if(strcmp(argv[i], "-s") == 0){ 
                assert(i+1<argc);
                assert(argv[i+1][0] >= '0' && argv[i+1][0] <= '9');
                s = atoi(argv[i+1]);
                i++; //skip the next number
            }else if(strcmp(argv[i], "-E") == 0){
                assert(i+1<argc);
                assert(argv[i+1][0] >= '0' && argv[i+1][0] <= '9');
                E = atoi(argv[i+1]);
                i++; //skip the next number
            }else if(strcmp(argv[i], "-b") == 0){
                assert(i+1<argc);
                assert(argv[i+1][0] >= '0' && argv[i+1][0] <= '9');
                b = atoi(argv[i+1]);
                i++; //skip the next number
            }else if(strcmp(argv[i], "-t") == 0){
                assert(i+1<argc);
                assert(strlen(argv[i+1]) < 100);
                strcpy(tracefile, argv[i+1]);
                i++; //skip the next number
            }else{
                fprintf(stderr, "Unknown option: %s\n", argv[i]);
                return 1;
            }
        }
    }

    n_sets = 1 << s; // Number of sets is 2^s
    n_ways = E; // Number of ways is E
    parseInput();

    for(int i=0;i<tot;i++){
        unsigned long long off_mask = (1ULL << b) - 1;    // 低 b 位全 1
        unsigned long long set_mask = (1ULL << s) - 1;    // 接下来的 s 位全 1
        unsigned long long addr = cacheOp[i].address;
        unsigned long long tag = addr >> (s + b);
        unsigned int set_index = (unsigned int)((addr >> b) & set_mask);
        unsigned int offset = (unsigned int)(addr & off_mask);
        assert(offset < 1 << b);
        if(cacheOp[i].operation == 'L'){
            handle_L(addr,set_index, tag, cacheOp[i].size);
        }else if(cacheOp[i].operation == 'S'){
            handle_S(addr, set_index, tag,cacheOp[i].size);
        }else {
            assert(cacheOp[i].operation == 'M');
            handle_L(addr, set_index, tag,cacheOp[i].size); // First load
            handle_S(addr, set_index, tag,cacheOp[i].size); // Then store
            // For 'M', we handle it as a load followed by a store
        }
    } 

    printSummary(HIT, MISS, EVICTION);
    return 0;
}
