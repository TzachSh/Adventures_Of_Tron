/* butler.h */

/* butler demon process definitions for housekeeping and other chores	*/

#define	BTLRNAME	"*BUTLER*"	/* process name			*/
#define	BTLRSTK		512		/* butler process stack size	*/
#define BTLRPRIO	1000		/* butler priority		*/

extern	int	butler();		/* butler process code		*/
extern	int	butlerpid;		/* butler process pid for msgs	*/

#define	MSGKILL		-1		/* kill xinu			*/
#define	MSGPSNAP    	1		/* print a process snapshot	*/
#define	MSGTSNAP	2		/* print a tty snapshot		*/
#define	MSGDSNAP	3		/* print a disk snapshot	*/

extern	int	psnap();
extern	int	tsnap();
extern	int	dsnap();
