/* dskio.h */

#define DSKREAD		2		/* DSK read subfunction code	*/
#define DSKWRITE	3		/* DSK write subfunction code	*/
#define RETRY		2		/* no. of disk retry operations	*/

#ifndef NDSECT
#define NDSECT		720		/* no. of disk sectors		*/
#endif

extern int dskio();			/* defined in dskio.asm		*/
extern int dskint();			/* defined in dskio.asm		*/
extern int dread();			/* defined in dio.c		*/
extern int dwrite();			/* defined in dio.c		*/
