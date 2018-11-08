/* getmem.c - getmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>

/*------------------------------------------------------------------------
 *  getmem  --  allocate heap storage, returning lowest integer address
 *------------------------------------------------------------------------
 */
char *getmem(nbytes)
word nbytes;
{
	int	ps;
	struct	mblock	*p, *q, *leftover;

	disable(ps);
	if ( nbytes==0 ) {
		restore(ps);
		return( NULL );
	}
	nbytes = roundew(nbytes);
	for ( q=&memlist, p=q->mnext ;
		(char *)p != NULL ;
		q=p, p=p->mnext )
		if ( p->mlen == nbytes) {
			q->mnext = p->mnext;
			restore(ps);
			return( (char *) p );
		} else if ( p->mlen > nbytes ) {
			leftover = (struct mblock *)( (char *)p + nbytes );
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlen = p->mlen - nbytes;
			restore(ps);
			return( (char *) p );
		}
	restore(ps);
	return( NULL );
}
