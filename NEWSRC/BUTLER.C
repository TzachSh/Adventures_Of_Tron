/* butler.c - butler */

#include <conf.h>
#include <kernel.h>
#include <butler.h>

/*------------------------------------------------------------------------
 *  butler  --  general housekeeping process.  Responds to messages and
 *	takes appropriate actions.  Messages recognized are:
 *		MSGKILL		- kill the system
 *		MSGPSNAP	- print a process table snapshot
 *		MSGTSNAP	- print tty structure snapshot
 *		MSGPSNAP	- print disk snapshot
 *------------------------------------------------------------------------
 */

PROCESS	butler()
{
	int	pcx;
	int	msg;

	for (;;) {
		msg = receive();
		xdisable(pcx);
		switch (msg) {		/* wait for & get the message	*/
		case MSGKILL:
			xrestore(pcx);
			xdone();
			break;		/* can never get here		*/
		case MSGPSNAP:
			psnap();
			break;
#ifdef Ntty
		case MSGTSNAP:
			tsnap();
			break;
#endif
#ifdef Ndsk
		case MSGDSNAP:
			dsnap();
			break;
#endif
		}
		kprintf("Press any key to continue . . .");
		kgetc(0);
		kprintf("\n");
		xrestore(pcx);
	}
}
