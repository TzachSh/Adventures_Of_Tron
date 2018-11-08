/* clkinit.c - clkinit */

#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <sleep.h>

/*------------------------------------------------------------------------
 *  clkinit  --  initialize the clock and sleep queue (called at startup)
 *------------------------------------------------------------------------
 */
clkinit()
{
	tod = 0L;			/* initial time of day		*/
	preempt = QUANTUM;		/* initial time quantum		*/
	slnempty = FALSE;		/* initially, no process asleep	*/
	clkdiff = 0;			/* zero deferred ticks		*/
	defclk = 0;			/* clock is not deferred	*/
	clockq = newqueue();		/* allocate clock queue in q	*/
}
