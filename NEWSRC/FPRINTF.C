/* fprintf.c - fprintf */

#include <conf.h>
#include <kernel.h>

/*------------------------------------------------------------------------
 *  fprintf  --  write formatted output on a specified device
 *------------------------------------------------------------------------
 */
fprintf(dev,fmt,args)
int dev;
char *fmt;
int args;
{
	int	putc();
	
	_doprnt(fmt, &args, putc, dev);
	return(OK);
}
