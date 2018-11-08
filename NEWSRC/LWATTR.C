/* lwattr.c - lwattr */

	#include <ctype.h>
#include <conf.h>
#include <kernel.h>
#include <tty.h>

#define	ATTR_NOBLINK		0x3f
#define	ATTR_BLINK		0x80
#define	ATTR_NOINTENSE		0x87
#define	ATTR_INTENSE		0x08
#define	ATTR_DEFAULT		0x07

#define	newcol(c,n,j)	( ((c) & ~( 7<<(j) )) | ((n) << (j)) )

static char *clrs[] = { "blk","blu","grn","cyn","red","mag","yel","wht" };

/*------------------------------------------------------------------------
 *  lwattr  --  parse window color attribute string
 *------------------------------------------------------------------------
 */
int  lwattr(ap,attr)
char	*ap;
int	attr;				/* default value		*/
{
	char	tmp[4];			/* used to compare with clrs	*/
	int	i,j;
	int	cnum;			/* value of selected attr	*/
	int	shift = 0;		/* shift count			*/

	if ( ap == NULL || *ap == 0 )
		return(attr);
	while (  isalnum(*ap) == 0 && *ap != '/' ) {
		if ( *ap == 0 )
			return(attr);
		switch ( *ap++ ) {
			case '*':
				attr &= ATTR_NOBLINK;
				break;
			case '?':
				attr |= ATTR_BLINK;
				break;
			case '-':
				attr &= ATTR_NOINTENSE;
				break;
			case '+':
			 	attr |= ATTR_INTENSE;
				break;
			default:
				return(SYSERR);
		}
	}
	for( i=0 ; i<3 ; i++ ) {
		if ( *ap == 0 )
			return(attr);
		if ( i == 1 ) {
			if ( *ap != '/' ) 
				return(SYSERR);
			ap++;
			shift = 4;
			continue;
		}
		if ( isdigit(*ap) )
			cnum = (*ap++) - '0';
		else if ( isalpha(*ap) ) {
			for ( j=0 ; j<3 ; j++ ) {
				if ( isalpha(*ap) == 0 )
					return(SYSERR);
				tmp[j] = tolower(*ap);
				ap++;
			}
			tmp[j] = 0;
			for ( cnum=0 ; cnum<8 ; cnum++ )
				if ( strcmp(clrs[cnum],tmp) == 0 )
					break;
		} else 
			continue;
		if ( cnum >= 8 )
			return(SYSERR);
		attr = newcol(attr,cnum,shift);
	}
	if ( *ap == 0 )
		return(attr);
	return(SYSERR);
}
