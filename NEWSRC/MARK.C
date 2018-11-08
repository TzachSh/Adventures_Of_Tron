/* mark.c - _mkinit, mark */

#include <conf.h>
#include <kernel.h>
#include <mark.h>

#ifdef MEMMARK
int		*marks[MAXMARK];
int		nmarks;
int		mkmutex;

/*------------------------------------------------------------------------
 *  _mkinit  --  initialize memory marking; called once at system startup
 *------------------------------------------------------------------------
 */

_mkinit()
{
	mkmutex = screate(1);
	nmarks = 0;
}

/*------------------------------------------------------------------------
 *  mark  --  mark a location if it has not been marked
 *------------------------------------------------------------------------
 */

mark(loc)
int loc[];
{
	if ( unmarked(loc) == 0 )
		return(0);
	if (nmarks>=MAXMARK)
		return(SYSERR);
	wait(mkmutex);
	marks[ loc[0] = nmarks++ ] = loc;
	signal(mkmutex);
	return(OK);
}
#endif
