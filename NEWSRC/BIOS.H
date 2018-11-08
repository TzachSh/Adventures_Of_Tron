/* bios.h */

/*------------------------------------------------------------------------
 *  ROM BIOS interface information for PCs
 *------------------------------------------------------------------------
 */

#define DB0VEC		0x00		/* divide by zero exception vec	*/
#define SSTEPVEC	0x01		/* single step exception vector	*/
#define BKPTVEC		0x03		/* breakpoint exception vector	*/
#define OFLOWVEC	0x04		/* overflow exception vector	*/

#define	CLKVEC	0x08			/* Clock interrupt vector	*/
#define	KBDVEC	0x09			/* Keyboard interrupt vector	*/
#define	COM1VEC	0x0b			/* COM1 interrupt vector	*/
#define	COM2VEC	0x0c			/* COM2 interrupt vector	*/
#define	FLOPVEC	0x0e			/* Floppy interrupt vector	*/
#define	PRLLVEC	0x0f			/* Parallel port interrupt vec	*/
#define CBRKVEC	0x1b			/* Ctrl-Break interrupt vector	*/

#define	BIOSFLG	0x100			/* BIOS flag for intmap		*/

extern int _panic();			/* exception handler		*/
extern int cbrkint();			/* ctrl-break handler		*/
extern int clkint();			/* clock interrupt handler	*/
