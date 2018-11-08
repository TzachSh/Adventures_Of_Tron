/* kputc.c - kputc */

#include <conf.h>
#include <kernel.h>
#include <tty.h>
#include <vidio.h>

/*------------------------------------------------------------------------
 *  kputc  --  put a character to the video display
 *------------------------------------------------------------------------
 */
void kputc(d,ch)
int d;					/* dummy parameter		*/
char ch;				/* character to display		*/
{
	if ( ch==RETURN || ch==NEWLINE ) { /* expand newline		*/
		ch = NEWLINE;
		wtty(RETURN);
	}
	wtty(ch);
}
