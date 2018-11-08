/* io.h -  fgetc, fputc, getchar, isbaddev, putchar */


#define	INTVECI	inint		/* input interrupt dispatch routine	*/
#define	INTVECO	outint		/* output interrupt dispatch routine	*/
extern	int	INTVECI();
extern	int	INTVECO();

#define NMAPS	0x20		/* number of intmap entries		*/
struct	intmap	{		/* device-to-interrupt routine mapping	*/
	char	ivec;		/* interrupt number			*/
	char	callinst;	/* the call instruction			*/
	word	intcom;		/* common interrupt code		*/
	word	oldisr_off;	/* old int. service routine offset	*/
	word	oldisr_seg;	/* old int. service routine segment	*/
	int	(*newisr)();	/* pointer to the new int. ser. routine */
	word	mdevno;		/* minor device number			*/
	word	iflag;		/* if nonzero, call the old isr		*/
	};

/*
 * NOTE: The intmap structure takes a total of 7 words or 14 bytes
 * per record.
 */

extern	struct intmap far *sys_imp;	/* pointer to intmap table	*/
extern	int	nmaps;		/* number of active intmap entries	*/

#define	isbaddev(f)	( (f)<0 || (f)>=NDEVS )

/* In-line I/O procedures */

#define	getchar()	getc(CONSOLE)
#define	putchar(ch)	putc(CONSOLE,(ch))
#define	fgetc(unit)	getc((unit))
#define	fputc(unit,ch)	putc((unit),(ch))

extern	int	_doprnt();	/* output formatter			*/
