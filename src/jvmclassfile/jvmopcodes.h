#define bipush 0x10
#define sipush 0x11


#define putfield 	0xb5
#define getfield 	0xb4
#define getstatic	0xb2
#define putstatic	0xb3

#define iadd 		0x60
#define isub 		0x64
#define imul 		0x68
#define idiv 		0x6c
#define irem		0x70
#define ishl		120
#define ishr		122
#define ineg		116

#define fadd 		0x62
#define fsub 		0x66
#define fmul 		0x6a
#define fdiv 		0x6e

#define aload		0x19
#define aload_0		0x2a
#define aload_1		0x2b
#define aload_2		0x2c
#define aload_3		0x2d

#define astore_0		0x4b
#define astore_1		0x4c
#define astore_2		0x4d
#define astore_3		0x4e


#define op_return	0xb1
#define ireturn		0xac
#define	freturn		0xae
#define ldc			0x12
#define ldc_w		0x13
#define ifne		0x9a
#define ifeq		0x99
#define ifnull	0xc6
#define ifnonnull 0xc7
#define op_goto 	0xa7
#define iload		0x15
#define iload_0		0x1a
#define iload_1		0x1b
#define iload_2		0x1c
#define iload_3		0x1d
#define istore		0x36
#define istore_0		0x3b
#define istore_1		0x3c
#define istore_2		0x3d
#define istore_3		0x3e

#define iastore	0x4f
#define iaload	0x2e

#define fload		0x17
#define fstore		0x38
#define op_swap		0x5f

#define op_goto		0xa7

#define op_dup			0x59
#define op_dup2		92

#define i2f			0x86
#define f2i			0x8b

#define invokevirtual	0xb6
#define	invokespecial	0xb7
#define invokestatic	0xb8

#define astore		0x3a
#define aload		0x19
#define areturn		0xb0
#define op_new		0xbb
#define op_newarray	188
#define arraylength 190

#define op_pop		0x57
#define aconst_null	0x01
#define iconst_0	0x03
#define iconst_1	0x04
#define iconst_2	0x05
#define iconst_3	0x06
#define iconst_4	0x07
#define iconst_5	0x08


#define if_icmpeq	159
#define if_icmpne	160
#define if_icmplt	161
#define if_icmpge	162
#define if_icmpgt	163
#define if_icmple	164

#define iflt	155
#define ifge	156
#define ifle	158


#define castore 0x55
#define caload 0x34

#define iand		126
#define ior			128

#define ineg		116
#define fneg		0x76
#define iinc		0x84

#define	lconst_0	0x09
#define lconst_1	0x0a
