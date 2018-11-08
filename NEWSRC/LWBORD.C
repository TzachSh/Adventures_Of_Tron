/* lwbord.c - lwbord */

	#include <ctype.h>
#include <conf.h>
#include <kernel.h>
#include <tty.h>

/*------------------------------------------------------------------------
 *  lwbord  --  parse window border attribute string
 *------------------------------------------------------------------------
 */
int lwbord(bp,w)
char *bp;
CURSOR w[2];				/* pointer to cursor array	*/
{
	int	i,j;
	int	coord;			/* row/col coordinate generated	*/
	int	brdr;			/* true if window has a border	*/

	if ( bp == NULL )
		return(SYSERR);
	if ( brdr = (*bp=='#') )
		bp++;
	for(i=0 ; i<2 ; i++) {		/* loop for two sets of ints	*/
		for(j=0 ; j<2 ; j++) {	/* loop for two ints per set	*/
			if ( isdigit(*bp) == 0 )
				return(SYSERR);
			coord = 0;
			while ( isdigit(*bp) ) {
				coord *= 10;
				coord += (*bp++) - '0';
			}
			if ( coord < 0 || coord >= 256 )
				return(SYSERR);
			switch(j) {
				case 0:
					w[i].col  = coord;
					break;
				case 1:
					w[i].row  = coord;
					break;
			}
			if ( *bp == ',' && j == 0 )
				bp++;
		}
		if ( *bp == ':' && i == 0 )
			bp++;
	}
	if ( *bp != 0 )
		return(SYSERR);
	return(brdr);
}
