/* clkint.c - clkint */

#include <conf.h>
#include <kernel.h>
#include <sleep.h>
#include <io.h>
#include <proc.h>

extern int sched_arr_pid[];
extern int sched_arr_int[];

extern int gcycle_length;
extern int point_in_cycle;
extern int gno_of_pids;

SYSCALL noresched_send(pid, msg)
int	pid;
int	msg;
{
	struct	pentry	*pptr;		/* receiver's proc. table addr.	*/
	int	ps;

	disable(ps);
	if (isbadpid(pid) || ( (pptr = &proctab[pid])->pstate == PRFREE)
	   || pptr->phasmsg != 0) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pmsg = msg;		/* deposit message		*/
	pptr->phasmsg++;
	if (pptr->pstate == PRRECV) {	/* if receiver waits, start it	*/
		ready(pid);
	}
	restore(ps);
	return(OK);
} // noresched_send



/*------------------------------------------------------------------------
 *  clkint  --  clock service routine
 *  called at every clock tick and when starting the deferred clock
 *------------------------------------------------------------------------
 */
INTPROC clkint(mdevno)
int mdevno;				/* minor device number		*/
{
	int	i;
        int resched_flag;

        
	tod++;

        resched_flag = 0;
	if (slnempty)
		if ( (--*sltop) <= 0 )
                     {
                        resched_flag = 1;
			wakeup();
                     } /* if */

	if ( (--preempt) <= 0 )
             resched_flag = 1;

       point_in_cycle++;
       if (point_in_cycle == gcycle_length)
         point_in_cycle = 0;

       for(i=0; i < gno_of_pids; i++) 
       {
          if(point_in_cycle == sched_arr_int[i])
            {
             noresched_send(sched_arr_pid[i], 11);
             resched_flag = 1;
            } // if
       } // for

       if (resched_flag == 1)
 		resched();

} // clkint

