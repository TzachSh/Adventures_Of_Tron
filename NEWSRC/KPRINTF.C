/* kprintf.c - kprintf */

#include <conf.h>
#include <kernel.h>
#include <io.h>
#include <tty.h>
#include <vidio.h>

/*------------------------------------------------------------------------
 *  kprintf  --  kernel printf: formatted, unbuffered output to the screen
 *------------------------------------------------------------------------
 */
int kprintf(fmt, args)		/* Derived by Bob Brown, Purdue U.	*/
char *fmt;
{
	void	kputc();
	int	pcx;

	xdisable(pcx);
        _doprnt(fmt, &args, kputc, CONSOLE);
        xrestore(pcx);
        return(OK);
}
