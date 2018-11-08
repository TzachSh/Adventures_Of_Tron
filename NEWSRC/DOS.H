/* dos.h */

struct WORDREGS	{
	unsigned int ax, bx, cx, dx, si, di, cflag, flags;
};

struct BYTEREGS	{
	unsigned char al, ah, bl, bh, cl, ch, dl, dh;
};

union	REGS {
	struct	WORDREGS x;
	struct	BYTEREGS h;
};

struct	SREGS	{
	unsigned int es, cs, ss, ds;
};

#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))
