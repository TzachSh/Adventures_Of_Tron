/* kbdio.h */

#define NOCH	-1
#define SPEC	0x100

extern int kbdgetc();			/* defined in kbdio.asm	*/
extern int kbdint();			/* defined in kbdio.asm		*/
extern int kgetc();			/* defined in kgetc.c		*/
