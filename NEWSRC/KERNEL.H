/* kernel.h - isodd, disable, restore, enable, pause, halt, xdisable, xrestore */
/* 8086/8 PC-Xinu version - for IBM PC/XT/AT and Clones */

/* Symbolic constants used throughout Xinu */

typedef	char		Bool;		/* Boolean type			*/
typedef unsigned int	word;		/* word type			*/

#define	FALSE		0		/* Boolean constants		*/
#define	TRUE		1
#define	NULL		(char *)0	/* Null pointer for linked lists*/
#define	SYSCALL		int		/* System call declaration	*/
#define	LOCAL		static		/* Local procedure declaration	*/
#define	INTPROC		int		/* Interrupt procedure  	*/
#define	PROCESS		int		/* Process declaration		*/
#define WORD		word		/* 16-bit word			*/ 
#define	MININT		0100000		/* minimum integer (-32768)	*/
#define	MAXINT		0077777		/* maximum integer (+32767)	*/
#define	MINSTK		256		/* minimum process stack size	*/
#define	NULLSTK		256		/* process 0 stack size		*/
#define	OK		 1 		/* returned when system	call ok	*/
#define	SYSERR		-1		/* returned when sys. call fails*/

/* initialization constants */

#define	INITARGC	2		/* initial process argc		*/
#define	INITSTK		512		/* initial process stack	*/
#define	INITPRIO	20		/* initial process priority	*/
#define	INITNAME	"xmain"		/* initial process name		*/
#define	INITRET		userret		/* processes return address	*/
#define	INITREG		0		/* initial register contents	*/
#define	QUANTUM		1		/* clock ticks until preemption	*/

/* misc. utility functions */

#define	isodd(x)	(01&(int)(x))
#define	disable(x)	(x)=sys_disabl() /* save interrupt status	*/
#define	restore(x)	sys_restor(x)	/* restore interrupt status	*/
#define	enable()	sys_enabl()	/* enable interrupts		*/
#define	pause()		sys_wait()	/* machine "wait for interr."	*/
#define	halt()		sys_hlt()	/* halt PC-Xinu			*/
#define xdisable(x)	(x)=sys_xdisabl() /* save int & dosflag status	*/
#define xrestore(x)	sys_xrestor(x)	/* restore int & dosflag status	*/

/* system-specific functions and variables */

extern	int	sys_disabl();		/* return flags & disable ints	*/
extern	void	sys_restor();		/* restore the flag register	*/
extern	void	sys_enabl();		/* enable interrupts		*/
extern	void	sys_wait();		/* wait for an interrupt	*/
extern	void	sys_hlt();		/* halt the processor		*/
extern	int	sys_xdisabl();		/* Return interrupts to MS-DOS	*/
extern	void	sys_restor();		/* Interrupts back to Xinu	*/

/* process management variables */

extern	int	rdyhead, rdytail;
extern	int	preempt;
