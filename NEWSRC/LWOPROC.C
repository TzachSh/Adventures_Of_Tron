/* lwoproc.c - lwoproc */

#include <conf.h>
#include <kernel.h>
#include <tty.h>
#include <io.h>
#include <bios.h>

/*------------------------------------------------------------------------
 *  lwoproc  --  lower-half tty device driver process for window output
 *------------------------------------------------------------------------
 */

PROCESS	lwoproc(i)
int i;					/* tty minor device number	*/
{
	register struct	tty   *iptr;
	int	ps;
	int	ct;
	char	ch;
	Bool	enl, onl;
	CURSOR	*c;
	int	wput();
	int	rcvchr();

	iptr = &tty[i];			/* pointer to tty structure	*/
	onl = enl = FALSE;
	c = &(iptr->curcur);
	disable(ps);
	for ( ; ; ) {			/* endless loop			*/
		if (enl) {
			enl = FALSE;
			wput(iptr,c,NEWLINE);
			continue;
		}
		/* look at the echo buffer */
		if ( iptr->ecnt ) {	/* any chars in echo buffer?	*/
			ch = iptr->ebuff[iptr->etail++];
			--iptr->ecnt;
			if (iptr->etail	>= EBUFLEN)
				iptr->etail = 0;
			if ( (ch==RETURN || ch==NEWLINE) && iptr->ecrlf ) {
				enl = TRUE;
				ch = RETURN;
			}
			wput(iptr,c,ch);
			continue;
		}
		if (iptr->oheld) {
			rcvchr();
			continue;
		}
		if (onl) {
			onl = FALSE;
			wput(iptr,c,NEWLINE);
			continue;
		}
		if ( (ct=iptr->ocnt) > 0 ) {
			ch = iptr->obuff[iptr->otail++];
			--iptr->ocnt;
			if (iptr->otail	>= OBUFLEN)
				iptr->otail = 0;
			if ( ct < (OBUFLEN-OBMINSP) && iptr->odsend == 0 )
				signal(iptr->osem);
			else if	( ++(iptr->odsend) == OBMINSP ) {
				iptr->odsend = 0;
				signaln(iptr->osem, OBMINSP);
			}
			if ( (ch==RETURN || ch==NEWLINE) && iptr->ocrlf ) {
				onl = TRUE;
				ch = RETURN;
			}
			wput(iptr,c,ch);
			continue;
		}
		rcvchr();
	}
}

/*------------------------------------------------------------------------
 *  wput  --  put a single character to the window; honor NEWLINE, etc.
 *------------------------------------------------------------------------
 */
LOCAL wput(iptr, c, ch)
register struct	tty   *iptr;
CURSOR	*c;
char	ch;
{
	int	wrap();
	int	pcx;

	if ( ch < BLANK ) {
		switch (ch) {
		case RETURN:
			c->col = 0;
			break;
		case NEWLINE:
			if ( c->row == iptr->rowsiz-1 )
				scrollup(iptr->topleft,iptr->botright,
					((iptr->rowsiz>1)?1:0),iptr->attr);
			else
				c->row++;
			break;
		case BELL:
			wtty(ch);
			return;
		case BACKSP:
			if ( c->col > 0 )
				c->col--;
			break;
		case TAB:
			c->col += TABSTOP;
			c->col -= (c->col % TABSTOP);
			wrap(iptr,c);
			break;
		default:
			return;		/* do nothing */
		}
		wputcsr(iptr,*c);
	} else {
		xdisable(pcx);
		wputcsr(iptr,*c);
		putchr(ch, 1, 0);
		xrestore(pcx);
		c->col++;
		wrap(iptr,c);
	}
}

/*------------------------------------------------------------------------
 *  wrap  --  wrap around to a new line in the window; scroll if necessary
 *------------------------------------------------------------------------
 */
LOCAL wrap(iptr,c)
register struct	tty   *iptr;
CURSOR	*c;
{
	if (c->col >= iptr->colsiz ) {
		if (c->row >= iptr->rowsiz-1)
		   scrollup(iptr->topleft,iptr->botright,
			((iptr->rowsiz>1)?1:0),iptr->attr);
		else
	   	   c->row++;
		c->col = 0;
	}
}

/*------------------------------------------------------------------------
 *  rcvchr  --  wait for another character to arrive
 *------------------------------------------------------------------------
 */
LOCAL rcvchr()
{
	struct	tty	*iiptr;

	if ( winofcur != 0 ) {
		iiptr = &tty[winofcur];
		wputcsr(iiptr,iiptr->curcur);
	}
	if ( receive() == TMSGEFUL ) {
		wtty(BELL);
	}
}
