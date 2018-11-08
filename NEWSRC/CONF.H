/* conf.h */
/* (GENERATED FILE; DO NOT EDIT) */

#define	NULLPTR	(char *)0

/* Device table declarations */
struct	devsw	{			/* device table entry		*/
	int	dvnum;
	char	dvnam[8];
	int	(*dvinit)();
	int	(*dvopen)();
	int	(*dvclose)();
	int	(*dvread)();
	int	(*dvwrite)();
	int	(*dvseek)();
	int	(*dvgetc)();
	int	(*dvputc)();
	int	(*dvcntl)();
	int	dvport;
	int	dvivec;
	int	dvovec;
	int	(*dviint)();
	int	(*dvoint)();
	char	*dvioblk;
	int	dvminor;
	};

extern	struct	devsw devtab[];		/* one entry per device		*/

/* Device name definitions */

#define	CONSOLE	0		/* type tty	*/
#define	DS0	5		/* type dsk	*/
#define	DOS	11		/* type dos	*/

/* Control block sizes */

#define	Ntty	5
#define	Ndsk	1
#define	Ndf	5
#define	Ndos	1
#define	Nmf	4

#define	NDEVS	16

/* Declarations of I/O routines referenced */

extern	int	ioerr();
extern	int	ttyinit();
extern	int	ttyopen();
extern	int	ttyread();
extern	int	ttywrite();
extern	int	ttygetc();
extern	int	ttyputc();
extern	int	ttycntl();
extern	int	ttyiin();
extern	int	lwinit();
extern	int	ionull();
extern	int	lwclose();
extern	int	lwread();
extern	int	lwwrite();
extern	int	lwgetc();
extern	int	lwputc();
extern	int	lwcntl();
extern	int	dsinit();
extern	int	dsopen();
extern	int	dsread();
extern	int	dswrite();
extern	int	dsseek();
extern	int	dscntl();
extern	int	lfinit();
extern	int	lfclose();
extern	int	lfread();
extern	int	lfwrite();
extern	int	lfseek();
extern	int	lfgetc();
extern	int	lfputc();
extern	int	msopen();
extern	int	mscntl();
extern	int	mfinit();
extern	int	mfclose();
extern	int	mfread();
extern	int	mfwrite();
extern	int	mfseek();
extern	int	mfgetc();
extern	int	mfputc();


/* Configuration and size constants */

#define MEMMARK				/* enable memory marking	*/
#define	NPROC	30			/* number of user processes	*/
#define	NSEM    100			/* total number of semaphores	*/

#define	VERSION	"6pc (1-Dec-87)"	/* label printed at startup	*/
