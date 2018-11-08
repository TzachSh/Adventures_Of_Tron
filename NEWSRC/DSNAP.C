/* dsnap.c - dsnap */

#include <conf.h>
#include <kernel.h>
#include <disk.h>

/*------------------------------------------------------------------------
 *  dsnap  --  print a snapshot of disk requests
 *------------------------------------------------------------------------
 */
dsnap()
{
	struct	dsblk	*dsptr;
	struct	dreq	*drptr;
	int	ps;
	int	i,j;

	disable(ps);
	for (i=0 ; i<Ndsk ; i++) {
		dsptr = &dstab[i];
		if ( dsptr->dsprocnum < 0 )
			continue;
		drptr = dsptr->dreqlst;
		kprintf("disk=%d, server pid=%d\n",i,dsptr->dsprocnum);
		if ( (drptr=dsptr->dreqlst) != DRNULL) {
			j = 0;
			kprintf("  no.  dba pid  buf op\n");
			while ( drptr != DRNULL ) {
				kprintf("  %2d. %4d %3d %04x %2d\n",
					 j++,drptr->drdba,drptr->drpid,
					 drptr->drbuff,drptr->drop);
				drptr = drptr->drnext;
			}
		} else
			kprintf("  <empty request queue>\n");
	}
	restore(ps);
}
