/* dio.c - dread, dwrite */

#include <kernel.h>
#include <dskio.h>

LOCAL int dio();

/*------------------------------------------------------------------------
 *  dread  --  read a sector from disk
 *------------------------------------------------------------------------
 */
int dread(buf,dskno,lsn)
char *buf;				/* transfer address		*/
int dskno;				/* disk number			*/
int lsn;				/* logical sector no.		*/
{
	return(dio(DSKREAD,buf,dskno,lsn));
}

/*------------------------------------------------------------------------
 *  dwrite  --  write a sector to disk
 *------------------------------------------------------------------------
 */
int dwrite(buf,dskno,lsn)
char *buf;				/* transfer address		*/
int dskno;				/* disk number			*/
int lsn;				/* logical sector no.		*/
{
	return(dio(DSKWRITE,buf,dskno,lsn));
}

/*------------------------------------------------------------------------
 *  dio  --  translate logical sector numbers to physical disk addresses
 *------------------------------------------------------------------------
 */
LOCAL int dio(op,buf,dskno,lsn)
int	op;				/* operation code		*/
char	*buf;				/* transfer address		*/
int	dskno;				/* disk number			*/
int	lsn;				/* logical sector no.		*/
{
	int	cyl,surf,sect;
	int	i;
	int	status;

	if ( lsn<0 || lsn>=NDSECT )
		return(SYSERR);
	sect = lsn % 9;			/* sector number		*/
	sect = ( 5*sect ) % 9;		/* sector interleaving		*/
	lsn /= 9;
	surf = lsn % 2;			/* surface			*/
	lsn /= 2;
	cyl = lsn;			/* cylinder			*/
	for (i=0; i<RETRY; i++)
		if ( (status=dskio(op,buf,dskno,cyl,surf,sect+1)) == 0 )
			break;
	return (status);
}
