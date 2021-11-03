#include "aufgabe.h"




void aufgabe_A01_1_1(void)
{
	int i = 0;
	char out[20] = {0};

	while(1)
	{
		i++;
		sprintf(out,"i=%d\r\n",i);
		usart2_send(out);
		if ( i>9){ i=0;}
		wait_mSek(500);
	}
}
