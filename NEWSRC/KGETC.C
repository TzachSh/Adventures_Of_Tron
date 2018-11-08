/* kgetc.c - kgetc */

#include <conf.h>
#include <kernel.h>
#include <tty.h>
#include <kbdio.h>

/*------------------------------------------------------------------------
 *  kgetc  --  get the next character from the keyboard
 *------------------------------------------------------------------------
 */
int kgetc(d)
int d;					/* dummy parameter		*/
{
	int	ch;

	while ( (ch=kbdgetc()) == NOCH )
		;
	return ( (ch==RETURN) ? NEWLINE : ch );
}
