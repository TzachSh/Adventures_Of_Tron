/* initialize.c - main, sysinit */

#include <dos.h>
#include <conf.h>
#include <kernel.h>
#include <io.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <q.h>
#include <mark.h>
#include <butler.h>
#include <bios.h>
#include <kbdio.h>

#ifdef	Ntty
#include <tty.h>
int	winofcur;		/* define the current input window	*/
struct tty	tty[Ntty];	/* window buffers and mode control	*/
#endif

#ifdef	Ndsk
#include <disk.h>
#endif

#ifdef 	Nmf
#include <mffile.h>
struct mfblk	mftab[Nmf];
#endif

/* Declarations of major kernel variables */

struct	pentry	proctab[NPROC]; /* process table			*/
int	nextproc;		/* next process slot to use in create	*/
struct	sentry	semaph[NSEM];	/* semaphore table			*/
int	nextsem;		/* next semaphore slot to use in screate*/
struct	qent	q[NQENT];	/* q table (see queue.c)		*/
int	nextqueue;		/* next slot in q structure to use	*/

struct	mblock	memlist;	/* list of free memory blocks		*/
char	*end;			/* beginning of free memory		*/
char	*maxaddr;		/* end of free memory			*/

/* PC-specific variables */

int	nmaps;			/* no. of active intmap entries		*/

/* active system status */

int	numproc;		/* number of live user processes	*/
int	currpid;		/* id of currently running process	*/

/* real-time clock variables and sleeping process queue pointers	*/

long	tod;			/* time-of-day (tics since startup)	*/
int     defclk;			/* non-zero, then deferring clock count */
int     clkdiff;		/* deferred clock ticks			*/
int     slnempty;		/* FALSE if the sleep queue is empty	*/
int     *sltop;			/* address of key part of top entry in	*/
				/* the sleep queue if slnonempty==TRUE	*/
int     clockq;			/* head of queue of sleeping processes  */
int	preempt;		/* preemption counter.	Current process */
				/* is preempted when it reaches zero;	*/
				/* set in resched; counts in ticks	*/

/* miscellaneous system variables */

int	butlerpid;		/* pid of butler process		*/
int	rdyhead,rdytail;	/* head/tail of ready list (q indexes)	*/

#define	NULLNM	"**NULL**"	/* null process name			*/

/************************************************************************/
/***				NOTE:				      ***/
/***								      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED, and      ***/
/***   must eventually be enabled explicitly.  This routine turns     ***/
/***   itself into the null process after initialization.  Because    ***/
/***   the null process must always remain ready to run, it cannot    ***/
/***   execute code that might cause it to be suspended, wait for a   ***/
/***   semaphore, or put to sleep, or exit.  In particular, it must   ***/
/***   not do I/O unless it uses kprintf for console output.          ***/
/***								      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  main  --  initialize system and become the null process (id==0)
 *------------------------------------------------------------------------
 */
main(argc,argv)				/* babysit CPU when no one home */
int argc;
char *argv[];
{
	int 	xmain();		/* user's main procedure	*/
	int	butler();		/* BUTLER process		*/
	int	ps;			/* save processor flags		*/
	int	pcx;			/* reschedule flag		*/

	while ( kbdgetc() != NOCH )	/* eat remaining kbd chars	*/
			;
	kprintf("Initializing . . .\n");
	xdisable(pcx);
	disable(ps);
	if ( sysinit() == SYSERR ) {	/* initialize all of PC-Xinu 	*/
		kprintf("PC-Xinu initialization error\n");
		maprestore();
		restore(ps);
		exit(1);
	}
	restore(ps);
	kprintf("\nPC-Xinu Version %s\n", VERSION);
	kprintf("%u real mem\n",(word)maxaddr);
	kprintf("%u base addr\n",(word)end);
	kprintf("%u avail mem\n", maxaddr-end);
        kprintf("XINU CAN BE TERMINATED BY CTRL-C\n");
        kprintf("CTRL-1 is equivalent to Ctrl-F1\n");
        kprintf("CTRL-1 can be used to invoke process snapshot\n");
	kprintf("\nHit any key to continue . . . ");
	kgetc(0);			/* wait for keyboard input	*/
	scrollup(0,0x184f,0,7);		/* clear the screen		*/
	putcsrpos(0,0);			/* home the cursor		*/
	xrestore(pcx);

	/* start the butler process				 	*/
	resume( butlerpid=create(butler,BTLRSTK,BTLRPRIO,BTLRNAME,0) );

	/* start up the user process					*/
	resume( create(xmain,INITSTK,INITPRIO,"xmain",2,argc,argv) );

	while (TRUE)		/* run forever without actually */
		pause();	/*  executing instructions	*/
}

/*------------------------------------------------------------------------
 *  sysinit  --  initialize all PC-Xinu data structures and devices
 *------------------------------------------------------------------------
 */

LOCAL sysinit()
{
	int	i,j;
	struct	pentry	*pptr;
	struct	sentry	*sptr;
	struct	mblock	*mptr;
	char	*malloc();		/* C memory allocator		*/
	char	*realloc();
        void set_new_int9_newisr();

#ifndef TURBOC
	char	*sys_stknpb();
#endif
	int	xdone();		/* terminate xinu		*/
	word	sizmem;			/* memory sizing		*/
	char	*namep;			/* null process name pointer	*/
	struct	devsw	*dvptr;		/* pointer to devtab entry	*/

	nmaps=0;			/* no. of active intmap entries	*/
	numproc = 0;			/* initialize system variables	*/
	nextproc = NPROC-1;
	nextsem = NSEM-1;
	nextqueue = NPROC;		/* q[0..NPROC-1] are processes	*/

#ifdef TURBOC
	sizmem = coreleft() - MDOS;
	if ( (end=malloc(sizmem)) == NULL )
		return(SYSERR);
#else
	for (	sizmem = MMAX;
		sizmem >= MMIN && (end=malloc(sizmem)) == NULL;
		sizmem -= MBLK )
			;
	if ( sizmem < MMIN )
		return(SYSERR);
	sizmem -= MDOS;			/* save some for MSDOS		*/
	if ( (end=realloc(end,sizmem)) == NULL )
		return(SYSERR);
#endif
	maxaddr = end+sizmem;		/* top of free memory		*/
	end = (char *) roundew( (word)end );
	maxaddr = (char *) truncew( (word)maxaddr );
	memlist.mnext = mptr =		/* initialize free memory list	*/
		(struct mblock *) end;
	mptr->mnext = (struct mblock *) NULL;
	mptr->mlen = maxaddr-end;

	for (i=0 ; i<NPROC ; i++)	/* initialize process table	*/
		proctab[i].pstate = PRFREE;

	pptr = &proctab[NULLPROC];	/* initialize null process entry */
	pptr->pstate = PRCURR;
	pptr->pprio = 0;
#ifdef TURBOC
	pptr->pbase = maxaddr;		/* null process pbase stack ptr	*/
#else
	pptr->pbase = sys_stknpb();	/* null process pbase stack ptr	*/
#endif
	namep=NULLNM;
	for (j=0; j<PNMLEN; j++)
		pptr->pname[j] = (*namep ? *namep++ : ' ');
	pptr->pname[PNMLEN] = '\0';
	pptr->paddr = main;
	pptr->pargs = 0;
	currpid = NULLPROC;
#ifndef TURBOC
	sys_stkinit();			/* initialize run-time stacks	*/
#endif
	for (i=0 ; i<NSEM ; i++) {	/* initialize semaphores	*/
		(sptr = &semaph[i])->sstate = SFREE;
		sptr->sqtail = 1 + (sptr->sqhead = newqueue());
	}

	rdytail = 1 + ( rdyhead=newqueue() );	/* initialize ready list */

#ifdef	MEMMARK
	_mkinit();			/* initialize memory marking	*/
#else
	pinit();			/* initialize ports		*/
	poolinit();			/* initialize the buffer pools	*/
#endif

	clkinit();			/* initialize real-time clock	*/

#ifdef	Ndsk
	dskdbp= mkpool(DBUFSIZ, NDBUFF); /* initialize disk buffers 	*/
	dskrbp= mkpool(DREQSIZ, NDREQ);
#endif

#ifdef	Ntty
	winofcur = 0;			/* initialize current window	*/
#endif

	mapinit(DB0VEC, _panic, DB0VEC);	/* divide by zero	*/
	mapinit(SSTEPVEC, _panic, SSTEPVEC);	/* single step		*/
	mapinit(BKPTVEC, _panic, BKPTVEC);	/* breakpoint		*/
	mapinit(OFLOWVEC, _panic, OFLOWVEC);	/* overflow		*/
	mapinit(CBRKVEC, cbrkint, CBRKVEC);	/* ctrl-break		*/

#ifdef	NDEVS
	for ( i=0 ; i<NDEVS ; i++ ) {	/* initialize devices		*/
		dvptr = &devtab[i];
		if ( dvptr->dvivec && mapinit(dvptr->dvivec,
				dvptr->dviint, dvptr->dvminor) == SYSERR )
			return(SYSERR);
		if (dvptr->dvovec && mapinit(dvptr->dvovec,
				dvptr->dvoint, dvptr->dvminor) == SYSERR )
			return(SYSERR);
		init(i);		/* initialize the device	*/
	}
#endif

        set_new_int9_newisr();
	if ( mapinit( CLKVEC | BIOSFLG, clkint, 0 ) == SYSERR )
		return(SYSERR);

	return(OK);
}

int (*old9newisr)(int);


INTPROC new_int9(int mdevno)
{
 char result;
 int scan = 0;
 static int ctrl_pressed  = 0;

asm {
   PUSH AX
   IN AL,60h
   MOV BYTE PTR scan,AL
   POP AX
 } //asm

 if (scan == 29)
    ctrl_pressed  = 1;
 else
   if (scan == 157)
     ctrl_pressed  = 0;
   else  
     if ((scan == 46) && (ctrl_pressed == 1)) // Control-C?
     {
       asm INT 27
     } // if
     else
     if ((scan == 2) && (ctrl_pressed == 1)) // Control-C?
        send(butlerpid, MSGPSNAP);
     else
     if ((scan == 3) && (ctrl_pressed == 1)) // Control-C?
        send(butlerpid, MSGTSNAP);
     else
     if ((scan == 4) && (ctrl_pressed == 1)) // Control-C?
        send(butlerpid, MSGDSNAP);

  old9newisr(mdevno);

return 0;
  
} // new_int9

void set_new_int9_newisr()
{
  int i;
  for(i=0; i < 32; i++)
    if (sys_imp[i].ivec == 9)
    {
     old9newisr = sys_imp[i].newisr;
     sys_imp[i].newisr = new_int9;
     return;
    } // if

} // set_new_int9_newisr


