/* disk.h - dssync, dsdirec */

typedef	unsigned int	DBADDR;		/* disk data block addresses	*/
#define	DBNULL		(DBADDR)0177777	/* null disk block address	*/

#include <iblock.h>
#include <dir.h>

struct	dsblk	{			/* disk driver control block	*/
	struct	dreq	*dreqlst;	/* list of pending requests	*/
	int	dnum;			/* device number of this disk	*/
	int	dibsem;			/* i-block mutual exclusion sem.*/
	int	dflsem;			/* free list  "        "      "	*/
	int	ddirsem;		/* directory  "        "      "	*/
	int	dnfiles;		/* num. of currently open files	*/
	struct dir *ddir;		/* address of in-core directory	*/
	int	dsprocnum;		/* disk server process number	*/
};

extern	struct	dsblk	dstab[];

struct	dreq	{			/* node in list of requests	*/
	DBADDR	drdba;			/* disk block address to use	*/
	int	drpid;			/* process id making request	*/
	char	*drbuff;		/* buffer address for read/write*/
	int	drop;			/* operation: READ/WRITE/SEEK	*/
	int	drstat;			/* returned status OK/SYSERR	*/
	struct	dreq	*drnext;	/* ptr to next node on req. list*/
};

#define	DRNULL	(struct dreq *) 0	/* null pointer in request list	*/
#define	DIRBLK	0			/* block used to hold directory	*/
#define	DONQ	2			/* status if request enqueued	*/
#define	DBUFSIZ	512			/* size of disk data block	*/
#define	DREQSIZ	sizeof(struct dreq)	/* size of disk request node	*/
#define NDSECT	720			/* no. of physical disk sectors	*/

#ifndef	NDBUFF
#define	NDBUFF	10			/* number of disk data buffers	*/
#endif
#define	NDREQ	10			/* number of disk request buf.	*/
#define	DREAD	0			/* read command in dreq.drop	*/
#define	DWRITE	1			/* write   "			*/
#define	DSEEK	2			/* seek    "			*/
#define	DSYNC	3			/* sync    "  (test-disk-ready)	*/

#define	DSPRIO	200			/* disk server process priority	*/

extern	int	dskrbp;			/* disk request node buffer pool*/
extern	int	dskdbp;			/* disk data block buffer pool	*/
extern	char	*dskbcpy();		/* copy to new disk block	*/

/* disk control function codes */

#define	DSKSYNC	0			/* synchronize (flush all I/O)	*/
#define DSKNAME 1			/* rename a file		*/
#define DSKZAP	2			/* remove a file		*/
#define	dssync(ddev)	control((ddev),DSKSYNC);

#define	dsdirec(ddev)	(((struct dsblk *)(devtab[ddev].dvioblk))->ddir)
