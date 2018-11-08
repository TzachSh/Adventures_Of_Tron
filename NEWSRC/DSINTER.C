/* dsinter.c - dsinter */

#include <conf.h>
#include <kernel.h>
#include <disk.h>
#include <dskio.h>

/*------------------------------------------------------------------------
 *  dsinter  --  process to handle disk requests
 *------------------------------------------------------------------------
 */
PROCESS	dsinter(dsptr,dsknum)
struct	dsblk	*dsptr;
int	dsknum;
{
	int	ps;
	struct	dreq	*drptr;
	int	status;

	disable(ps);
	for(;;) {
		drptr = dsptr->dreqlst;
		if ( drptr == DRNULL ) {
			receive();
			continue;
		}
		dsptr->dreqlst = drptr->drnext;
		switch (drptr->drop) {
		case DREAD:
			status = dread(drptr->drbuff,dsknum,drptr->drdba);
			if ( status != 0 )
			kprintf("\nRead error: code=%02xH drive=%d block=%d\n",
					status,dsknum,drptr->drdba);
			drptr->drstat = ( status ? SYSERR : OK );
		case DSYNC:
			if ( resume(drptr->drpid) == SYSERR )
				panic("Disk request block pid error");
			break;
		case DWRITE:
			status = dwrite(drptr->drbuff,dsknum,drptr->drdba);
			if ( status != 0 )
			kprintf("\nWrite error: code=%02xH drive=%d block=%d\n",
					status,dsknum,drptr->drdba);
			freebuf(drptr->drbuff);
			/* fall through */
		case DSEEK:
			freebuf(drptr);
		}
	}
}
