/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>
#include <limits.h>

#include "simulator.h"
int calc_lru_page(Pentry q[MAXPROCESSES],int timestamps[MAXPROCESSES][MAXPROCESSES],int proc){
    int pg;
    int min_time;
    int victim = -1;
    min_time=INT_MAX;  
//find the page with smallest time stamp, meaning it was used the least recently
    for(pg=0;pg<MAXPROCPAGES;pg++)
    {
        if(q[proc].pages[pg])
        {

            if(timestamps[proc][pg]<min_time)
            {
                min_time=timestamps[proc][pg];
                victim=pg;

                if(min_time<=1)
                    break;
            }
        }
    }
    return victim;
}
void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    int proc;
    int pc;
    int page;
    int byeee; 
    /* initialize static vars on first run */
    if(!initialized){
	for(proc=0; proc < MAXPROCESSES; proc++){
	    for(page=0; page < MAXPROCPAGES; page++){
		timestamps[proc][page] = 0; 
	    }
	}
	initialized = 1;
    }
    
    /* TODO: Implement LRU Paging */
// Select a Process
    for(proc = 0;proc<MAXPROCESSES;proc++){ //for every process
    	
    	if(q[proc].active){
    		//which page do we need?
    		pc = q[proc].pc; 		        
	    	page = pc/PAGESIZE; 
	    	//if we are accessing this page currently, we should update timestamp
	    	timestamps[proc][page] = tick;
	    	//do we need to find the page a home? or is it home already?
    		if(!q[proc].pages[page]){
    			//ok bring it home
    			if(!pagein(proc,page)){
    				//couldn't find it a place, gotta kick someone out
    				//but who?
    				byeee = calc_lru_page(q,timestamps,proc);
    				if(pageout(proc,byeee)) {
				/* Break loop once swap-out starts*/
						break;
			    } 
    			}
    		}
    	}

    }
    /* advance time for next pageit iteration */
    tick++;
} 
