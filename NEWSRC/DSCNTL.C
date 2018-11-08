/* dscntl.c - dscntl */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <disk.h>

/*------------------------------------------------------------------------
 *  dscntl  --  control disk driver/device
 *------------------------------------------------------------------------
 */
dscntl(devptr, func, arg1, arg2)
struct	devsw	*devptr;
int	func;
char	*arg1, *arg2;
{
	int	stat;
	int	ps;

	disable(ps);
	switch (func) {

		case DSKSYNC:
			stat = dsksync(devptr);
			break;
		default:
			stat = SYSERR;
			break;
	}
	restore(ps);
	return(stat);
}
