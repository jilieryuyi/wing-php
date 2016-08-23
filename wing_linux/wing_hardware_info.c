#include "wing_hardware_info.h"

unsigned int veax;
unsigned int vebx;
unsigned int vedx;
unsigned int vecx;

//执行CPUID指令
void cpuid(unsigned int veax1)
{
	asm("cpuid"
		:"=a"(veax),
		"=b"(vebx),
		"=c"(vecx),
		"=d"(vedx)
		:"a"(veax));
}
//做移位操作，把寄存器中的值以“%d”形式输出
void LM(unsigned int var,uint32_t *vx)
{
	int i;
	for(i=0;i<3;i++)
	{
		var=(var>>i);
		vx[i]=var;
	}
}

 void wing_get_cpu_id (char *id)
{
	uint32_t ax[3],cx[3],dx[3];
	cpuid(1);
	LM(veax,ax);
	cpuid(3);
	LM(vecx,cx);
	LM(vedx,dx);
	sprintf(id,"%u%u%u%u%u%u%u%u%u",ax[0],ax[1],ax[2],cx[0],cx[1],cx[2],dx[0],dx[1],dx[2]);
}