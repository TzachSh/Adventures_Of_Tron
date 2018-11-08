/* create.c - create, newpid */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <mem.h>

#define	INITF	0x0200	/* initial flag register - set interrupt flag,	*/
			/* clear direction and trap flags		*/

extern	int	INITRET();	/* location to return upon termination	*/

/*------------------------------------------------------------------------
 *  create  --  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL create(procaddr,ssize,priority,namep,nargs,args)
int (*procaddr)();		/* procedure address			*/
word ssize;			/* stack size in words			*/
short priority;			/* process priority > 0			*/
char *namep;			/* name (for debugging)			*/
int nargs;			/* number of args that follow		*/
int args;			/* arguments (treated like an array)	*/
{
	int	pid;			/* stores new process id	*/
	struct	pentry	*pptr;		/* pointer to proc. table entry */
	int	i;			/* loop variable		*/
	int	*a;			/* points to list of args	*/
	char	*saddr;			/* start of stack address	*/
	int	*sp;			/* stack pointer		*/
	int	ps;			/* saved processor status	*/

	disable(ps);
	ssize = roundew(ssize);
	if ( ssize < MINSTK || priority < 1 ||
		(pid=newpid()) == SYSERR ||
		((saddr=getstk(ssize)) == NULL ) ) {
		restore(ps);
		return(SYSERR);
	}
	numproc++;
	pptr = &proctab[pid];
	pptr->pstate = PRSUSP;
	for (i=0 ; i<PNMLEN ; i++)
		pptr->pname[i] = (*namep ? *namep++ : ' ');
	pptr->pname[PNMLEN]='\0';
	pptr->pprio = priority;
	pptr->phasmsg = 0;		/* no message			*/
	pptr->pbase = saddr;
	pptr->plen = ssize;
	sp = (int *) (saddr+ssize);	/* simulate stack pointer	*/
	sp -= 4;			/* a little elbow room		*/
	pptr->pargs = nargs;
	a = (&args) + nargs;		/* point past last argument	*/
	for ( ; nargs > 0 ; nargs--)	/* machine dependent; copy args	*/
		*(--sp) = *(--a);	/*  onto created process' stack	*/
	*(--sp) = (int)INITRET;		/* push on return address	*/
	*(--sp) = (int)procaddr;	/* simulate a context switch	*/
	--sp ;				/* 1 word for bp		*/
	*(--sp) = INITF;		/* FLAGS value			*/
	sp -= 2;			/* 2 words for si and di	*/
	pptr->pregs = (char *)sp;	/* save for context switch	*/
	pptr->paddr = procaddr;
	restore(ps);
	return(pid);
}

/*------------------------------------------------------------------------
 *  newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=1 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if (proctab[i].pstate == PRFREE)
			return i;
	}
	return(SYSERR);
}
