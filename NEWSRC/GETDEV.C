/* getdev.c - getdev */

#include <conf.h>
#include <kernel.h>

/*------------------------------------------------------------------------
 *  getdev  --  get the device number from a character string name
 *------------------------------------------------------------------------
 */

int getdev(cp)
char *cp;
{
	int i;

	for ( i=0; i<NDEVS; i++ )
		if ( strcmp(cp,devtab[i].dvnam) == 0 )
			return(i);
	return(SYSERR);
}
