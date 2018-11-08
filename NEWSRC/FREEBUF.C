/* freebuf.c - freebuf */

#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>

/*------------------------------------------------------------------------
 *  freebuf  --  free a buffer that was allocated from a pool by getbuf
 *------------------------------------------------------------------------
 */
int freebuf(buf)
char *buf;
{
	int	ps;
	int	poolid;
	int	*bf;

	if ( buf == NULL )
		return(SYSERR);
	disable(ps);
	bf = (int *) buf;
	poolid = *(--bf);
	if ( poolid<0 || poolid>=nbpools
#ifdef	MEMMARK
		|| unmarked(bpmark)
#endif
		) {
		restore(ps);
		return(SYSERR);
	}
	*bf = (int)bptab[poolid].bpnext;
	bptab[poolid].bpnext = bf;
	signal(bptab[poolid].bpsem);
	restore(ps);
	return(OK);
}
