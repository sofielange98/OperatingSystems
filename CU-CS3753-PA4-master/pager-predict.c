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
 *  This file contains a predictive paging strategy
 * resources:
 * https://github.com/beala/CU-CS3753-2012-PA4/blob/master/pager-predict.c
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
/*
predictions based on behavior of processes
matrix tracks movement from one page to another by certain processes
so each process has an array of size MAXPROCPAGES
and for each of those pages we store any page that the process moved to from the first page
*/
void predict(int pc, int proc, int pc_prev, int pred_matrix[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES]){
    int i;
    int *moves;
    moves = pred_matrix[proc][pc_prev];

    for(i=0; i<MAXPROCPAGES; i++){
        if((moves[i] == -1)|(moves[i]==pc)){
            moves[i] = pc;
            break;
        }
    }
}

void pageit(Pentry q[MAXPROCESSES]) { 

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    static int pgs_prev[MAXPROCESSES]; //store the previous page counter so we can track movements
    //static int oops = 0;

    //matrix to store the movement of processes from one page to another
    static int pred_matrix[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];

    /* Local vars */
    int proc;
    int page;
    int byeee;
    int pg_prev;

    /* initialize static vars on first run */
    if(!initialized){
        int k;
        for(proc=0; proc < MAXPROCESSES; proc++){
            for(page=0; page < MAXPROCPAGES; page++){
                timestamps[proc][page] = 0;
                for(k=0;k<MAXPROCPAGES;k++){
                    pred_matrix[page][proc][k]=-1;
                }
            }
        }
        initialized = 1;
    }

    //add predictions
    for(proc=0; proc<MAXPROCESSES; proc++){
        if(q[proc].active)
        {
            if(pg_prev != -1)
            {
                pg_prev = pgs_prev[proc];
                pgs_prev[proc] = q[proc].pc/PAGESIZE;
                page = q[proc].pc/PAGESIZE;
                if(pg_prev != page)
                {
                    pageout(proc, pg_prev);
                    predict(page, proc, pg_prev, pred_matrix);
                }

            }
        }        
    }
    //use lru replacement if we didn't predict correctly
    for(proc=0; proc<MAXPROCESSES; proc++){
        if(q[proc].active){
            //which page do we need?               
            page = q[proc].pc/PAGESIZE; 
            //if we are accessing this page currently, we should update timestamp
            timestamps[proc][page] = tick;
            //do we need to find the page a home? or is it home already?
            //if we predicted correctly, we will skip
            if(!q[proc].pages[page]){
                //ok bring it in
                //oops ++;
                if(!pagein(proc,page)){
                    //couldn't find it a place, gotta kick someone out
                    //but who?
                    byeee = calc_lru_page(q,timestamps,proc);
                    pageout(proc,byeee);
                }
            }
        }
    }
//predictively page in pages based on the matrix which we have built, if it has values we can use to make predicitons
    for(proc=0; proc<MAXPROCESSES; proc++){
        int *pages;
        int future_page;
        int i;
        if(q[proc].active)
        {

            future_page = (q[proc].pc + 101)/PAGESIZE;
            pages = pred_matrix[proc][future_page];
            for(i=0;i<MAXPROCPAGES;i++)
            {
                if(pages[i]==-1)
                    break;
                pagein(proc, pages[i]);
            }
        }        
    }

    /* advance time for next pageit iteration */
    tick++;
}