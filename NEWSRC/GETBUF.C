/* getbuf.c - getbuf */

#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>

/*------------------------------------------------------------------------
 *  getbuf  --  get a buffer from a preestablished buffer pool 
 *------------------------------------------------------------------------
 */
char *getbuf(poolid)
int poolid;
{
	int	ps;
	int	*buf;

	disable(ps);
	if ( poolid<0 || poolid >= nbpools
#ifdef	MEMMARK
		|| unmarked(bpmark)
#endif
		) {
		restore(ps);
		return(NULL);
	}
	wait( bptab[poolid].bpsem );
	buf = bptab[poolid].bpnext;
	bptab[poolid].bpnext = (int *) *buf;
	*buf++ = poolid;
	restore(ps);
	return((char *)buf);
}
