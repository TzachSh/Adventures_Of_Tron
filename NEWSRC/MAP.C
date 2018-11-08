/* map.c - mapinit, maprestore */

#include <dos.h>
#include <conf.h>
#include <kernel.h>
#include <io.h>

/*------------------------------------------------------------------------
 *  mapinit  --  fill in an intmap table entry
 *------------------------------------------------------------------------
 */
int mapinit(vec,newisr,mdevno)
int vec;				/* interrupt vector no.		*/
int (*newisr)();			/* addr. of new service routine	*/
int mdevno;				/* minor device number		*/
{
	int i;				/* intmap entry			*/
	word 	far *addr;		/* far address pointer		*/
	struct 	intmap far *imp;	/* pointers to intmap		*/
	int	flag;			/* upper byte of vector		*/

	i = nmaps;
	if ( i >= NMAPS )
		return(SYSERR);
	nmaps++;
	imp = &sys_imp[i];		/* point to our intmap entry	*/
	flag = (vec>>8) & 0xff;		/* pick up flag byte		*/
	vec = vec & 0xff;		/* only low-order byte counts	*/
	FP_SEG(addr) = 0;		/* interrupts are on page 0	*/
	FP_OFF(addr) = vec * 4;		/* offset of this interrupt no.	*/

/* set up the input intmap entry */
	imp->iflag = flag;		/* deposit flag byte in iflag	*/
	imp->oldisr_off = *addr;	/* offset			*/
	imp->oldisr_seg = *(addr + 1);	/* segment			*/

/* the following is highly machine dependent */
	*addr = FP_OFF(imp)+1;		/* point to call instruction	*/
	*(addr+1) = FP_SEG(imp);	/* this code segment		*/
	imp->newisr = newisr;		/* our input handler		*/
	imp->mdevno = mdevno;		/* minor device no.		*/
	imp->ivec = (char) vec;		/* interrupt vector		*/
	return(OK);
}

/*------------------------------------------------------------------------
 *  maprestore  --  restore all old interrupt vectors from the intmap
 *------------------------------------------------------------------------
 */
int maprestore()
{
	int	i;			/* intmap entry number		*/
	word 	far *addr;		/* far address pointer		*/
	struct 	intmap far *imp;	/* pointers to intmap		*/

	if ( nmaps > NMAPS )
		nmaps = NMAPS;		/* just to be sure		*/
	for ( i=0; i<nmaps; i++) {
		imp = &sys_imp[i];	/* point to this intmap entry	*/
		if ( (int)(imp->newisr) == -1 )
			continue;	/* if unused entry		*/
		FP_SEG(addr) = 0;	/* interrupts are on page 0	*/
		FP_OFF(addr) = imp->ivec * 4;	/* offset to the vector	*/
		*addr = imp->oldisr_off;	/* offset		*/
		*(addr+1) = imp->oldisr_seg;	/* segment		*/
	}
}
