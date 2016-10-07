#include <16F628A.h>

#fuses INTRC_IO, 
#fuses NOWDT, NOPROTECT, NOLVP, NOPUT
#fuses NOBROWNOUT, NOMCLR

#use delay(clock=4MHz)
#use rs232(baud=1200, xmit=PIN_A0, bits=8, parity=N)     

void main()
{
   char msg[]="FindMe";
   
   while(true)
   {
      putc('$');
      for(int i=0;i<6;i++)
      {
         putc(msg[i]);   
      }
      
      delay_ms(1000);
   }
}
