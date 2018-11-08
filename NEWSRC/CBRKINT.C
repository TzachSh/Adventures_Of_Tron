/* cbrkint.c - cbrkint */

#include <conf.h>
#include <kernel.h>
#include <butler.h>

/*------------------------------------------------------------------------
 *  cbrkint  --  send a MSGKILL message to the butler
 *------------------------------------------------------------------------
 */

INTPROC	cbrkint()
{
	sendf(butlerpid,MSGKILL);
}
