/* freemem.c - freemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>

/*--------------------------------------------------------------------
 *  freemem  --  free a memory block, returning it to memlist
 *--------------------------------------------------------------------
 */
SYSCALL	freemem(block, size)
char *block;
word size;
{
	int	ps;
	struct	mblock	*p, *q;
	char	*top;

	size = roundew(size);
	block = (char *) truncew( (word)block );
	if ( size==0 || block > maxaddr || (maxaddr-block) < size ||
		block < end )
		return(SYSERR);
	disable(ps);
	for( q = &memlist, p=memlist.mnext ;
		(char *)p != NULL && (char *)p < block ;
		q=p, p=p->mnext )
			;
	if ( q != &memlist && (top=(char *)q+q->mlen) > block
		 || (char *)p != NULL && (block+size) > (char *)p ) {
		restore(ps);
		return(SYSERR);
	}
	if ( q != &memlist && top == block )
		q->mlen += size;
	else {
		((struct mblock *)block)->mlen = size;
		((struct mblock *)block)->mnext = p;
		q->mnext = (struct mblock *)block;
		(char *)q = block;
	}
	if ( ((char *)q + q->mlen) == (char *)p ) {
		q->mlen += p->mlen;
		q->mnext = p->mnext;
	}
	restore(ps);
	return(OK);
}
