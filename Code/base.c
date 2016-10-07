#include <16F628A.h>
//#device *= 16

#fuses INTRC_IO, 
#fuses NOWDT, NOPROTECT, NOLVP, NOPUT
#fuses NOBROWNOUT, NOMCLR

#use delay(clock=4MHz)
#use rs232(baud=1200, rcv=PIN_A0, bits=8, parity=N) 
#use rs232(baud=9600, xmit=PIN_B2, bits=8, parity=N,stream=RaspberryPi) 
          
#include <string.h>

char msgToFind[]="FindMe";

void main()
{
   char msg[10]="\0";
   
   char c;

   while(true)
   {
      c=getc();
      if(c=='$')
      {
        for(int i=0;i<6;i++)
        {
            msg[i]=getc();
        }
        msg[i]="\0";
        
        if(stricmp(msg,msgToFind)==0)
        {
           fprintf(RaspberryPi,"myRFButton,ON");
            delay_ms(1000);
        }                
      }          
   }
}
