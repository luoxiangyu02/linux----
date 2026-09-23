#define move_to_user_mode() \
__asm__ ("movl %%esp,%%eax\n\t" \
	"pushl $0x17\n\t" \
	"pushl %%eax\n\t" \
	"pushfl\n\t" \
	"pushl $0x0f\n\t" \
	"pushl $1f\n\t" \
	"iret\n" \
	"1:\tmovl $0x17,%%eax\n\t" \
	"movw %%ax,%%ds\n\t" \
	"movw %%ax,%%es\n\t" \
	"movw %%ax,%%fs\n\t" \
	"movw %%ax,%%gs" \
	:::"ax")

#define sti() __asm__ ("sti"::)
#define cli() __asm__ ("cli"::)
#define nop() __asm__ ("nop"::)

#define iret() __asm__ ("iret"::)

//“把一个中断处理函数地址和门属性合并成一条 IDT 描述符（一项8字节），并写到 gate_addr 指向的位置。”
//3 图 2-09
// dpl是特权级,type是门类型,addr是中断处理程序的地址
// 将idt项做出来,图2-07
// current pl,destionstion pl(目的特权级),reference pl  dpl 0 is kernel, dpl 3 is user
// type 15 is trap gate, 14 is interrupt gate
#define _set_gate(gate_addr,type,dpl,addr) \
// C 语言写汇编key word first need to be __asm__

__asm__ ("movw %%dx,%%ax\n\t" \
	"movw %0,%%dx\n\t" \
	"movl %%eax,%1\n\t" \
	"movl %%edx,%2" \
	: \
	// gnu use : to separate input and output operands
	// i is intermidate, o is output, a is eax, d is edx
	: "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	"o" (*((char *) (gate_addr))), \
	"o" (*(4+(char *) (gate_addr))), \
	"d" ((char *) (addr)),"a" (0x00080000))// 图2-10
	// d表示edx，a表示eax，o表示输出，i表示输入
// (char *) (addr)表示中断服务程序地址
// 00080000表示段选择符，0x8000表示存在位，dpl<<13表示特权级，type<<8表示门类型
// 同 jmp 08 1000 表示执行内核代码段 
//bit 15..3：段描述符索引 1 第二项GDT[1]
/* GDT[0] = null
GDT[1] = kernel code
GDT[2] = kernel data
GDT[3] = user code
GDT[4] = user data
*/
//bit 2：TI（Table Indicator） 0 GDT 1 LDT
//bit 1..0：RPL（请求特权级）00 kencel privilege level 0
	// 由原来的CS变为段选择器，选出指定的一项内容
#define set_intr_gate(n,addr) \
	_set_gate(&idt[n],14,0,addr)

	//2
#define set_trap_gate(n,addr) \
	_set_gate(&idt[n],15,0,addr)

#define set_system_gate(n,addr) \
	_set_gate(&idt[n],15,3,addr)

#define _set_seg_desc(gate_addr,type,dpl,base,limit) {\
	*(gate_addr) = ((base) & 0xff000000) | \
		(((base) & 0x00ff0000)>>16) | \
		((limit) & 0xf0000) | \
		((dpl)<<13) | \
		(0x00408000) | \
		((type)<<8); \
	*((gate_addr)+1) = (((base) & 0x0000ffff)<<16) | \
		((limit) & 0x0ffff); }

#define _set_tssldt_desc(n,addr,type) \
__asm__ ("movw $104,%1\n\t" \
	"movw %%ax,%2\n\t" \
	"rorl $16,%%eax\n\t" \
	"movb %%al,%3\n\t" \
	"movb $" type ",%4\n\t" \
	"movb $0x00,%5\n\t" \
	"movb %%ah,%6\n\t" \
	"rorl $16,%%eax" \
	::"a" (addr), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)), \
	 "m" (*(n+5)), "m" (*(n+6)), "m" (*(n+7)) \
	)

#define set_tss_desc(n,addr) _set_tssldt_desc(((char *) (n)),addr,"0x89")
#define set_ldt_desc(n,addr) _set_tssldt_desc(((char *) (n)),addr,"0x82")
