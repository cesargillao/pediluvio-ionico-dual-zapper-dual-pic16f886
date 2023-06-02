#include <16F886.h>
#device ADC=10
#use delay(internal=4MHz)
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOMCLR                   //Master Clear pin used for I/O
#FUSES PROTECT                  //Code protected from reads
#FUSES CPD                      //Data EEPROM Code Protected

#define LCD_DATA_PORT getenv("SFR:PORTB")
#define LCD_RS_PIN PIN_B1
#define LCD_RW_PIN PIN_B2
#define LCD_ENABLE_PIN  PIN_B3
#include <lcd.c>

#bit BArriba    = 0x005.2
#bit BAbajo     = 0x005.3
//#bit BMenu      = 0x005.4
#bit BIzquierda = 0x005.5
#bit BDerecha   = 0x005.7

#bit PediluvioA = 0x007.0
#bit PediluvioB = 0x007.3
#bit Buzzer     = 0x007.4

int16 L1,L2;
float AmperajeA=0,AmperajeB=0;
int C1=0,S1=0;
int C2=0,S2=0;
int1 OperandoA=0,OperandoB=0,ExcesoA=0,ExcesoB=0;
int1 Periodo=0,AscDesc=0,i;
int ReposicionA=5,ReposicionB=5;
int Terapia[2]={0x00,0x00};    // Ter{ TerapiaB , TerapiaA }
int Tiempo[2]={0x00,0x00};    // Min{ MinutosB , MinutosA }
int1 Zapper[2]={0x00,0x00}; // Zapper{ ZapperB , ZapperA }

void FreqOFF();
#int_timer0
void timer() 
{
   if(OperandoA)
   {
      set_timer0(58);
      if(C1==0)
      {
         if(ExcesoA)
         {
            ReposicionA--;
            if(!ReposicionA)
            {
               PediluvioA=1;
               ReposicionA=5;
               ExcesoA=0;
            }
         }
         C1=20;
         if(S1==0)
         {
            S1=60;
            if(Tiempo[1]==0)
            {
               Tiempo[1]=60;
            }
            Tiempo[1]--;
         }
         S1--;
         if(!S1 && !Tiempo[1])
         {
            PediluvioA=0;
            OperandoA=0;
            i=1;
            FreqOFF();
            // Titilar el tiempo + sonar buzzer 5 veces
         }
      }
      C1--;
   }
   if(OperandoB)
   {
      set_timer0(58);
      if(C2==0)
      {
         if(ExcesoB)
         {
            ReposicionB--;
            if(!ReposicionB)
            {
               PediluvioB=1;
               ReposicionB=5;
               ExcesoB=0;
            }
         }
         C2=20;
         if(S2==0)
         {
            S2=60;
            if(Tiempo[0]==0)
            {
               Tiempo[0]=60;
            }
            Tiempo[0]--;
         }
         S2--;
         if(!S2 && !Tiempo[0])
         {
            PediluvioB=0;
            OperandoB=0;
            i=1;
            FreqOFF();
            // Titilar el tiempo + sonar buzzer 5 veces
         }
      }
      C2--;
   }
}
void RetBoton()
{
   delay_ms(200);
}
void LeerADC()
{
   set_adc_channel(0);
   delay_us(20);
   L1=read_adc();
   AmperajeA=(L1*5.0)/1024.0;
   set_adc_channel(1);
   delay_us(20);
   L2=read_adc();
   AmperajeB=(L2*5.0)/1024.0;
}
void Estado()
{
   if(!ExcesoA)
   {
      lcd_gotoxy(1,1);
      printf(lcd_putc,"A %02u:%02u %1.2fA Z%u"Tiempo[1],S1,AmperajeA,Terapia[1]);
   }
   if(!ExcesoB)
   {
      lcd_gotoxy(1,2);
      printf(lcd_putc,"B %02u:%02u %1.2fA Z%u"Tiempo[0],S2,AmperajeB,Terapia[0]);
   }
}

void FreqOFF()
{
   if(i)
   {
      setup_ccp1(CCP_OFF);
      setup_timer_2(T2_DISABLED,1,1);
   }
   if(!i)
   {
      setup_ccp2(CCP_OFF);
      setup_timer_2(T2_DISABLED,1,1);
   }
}
void Freq30KHz()
{
   if(i)
   {
      setup_timer_2(T2_DIV_BY_1,32,1);   // PWM 30.3KHz
      set_pwm1_duty(128);
      setup_ccp1(CCP_PWM);
   }
   if(!i)
   {
      setup_timer_2(T2_DIV_BY_1,32,1);   // PWM 30.3KHz
      set_pwm2_duty(128);
      setup_ccp2(CCP_PWM);
   }
}
void Freq170KHz()
{
   if(i)
   {
      setup_timer_2(T2_DIV_BY_1,5,1);   // PMW 166.6KHz
      set_pwm1_duty(128);
      setup_ccp1(CCP_PWM);
   }
   if(!i)
   {
      setup_timer_2(T2_DIV_BY_1,5,1);   // PMW 166.6KHz
      set_pwm2_duty(128);
      setup_ccp2(CCP_PWM);
   }
}
void FreqAcordeon()
{
   if(i)
   {
      setup_timer_2(T2_DIV_BY_1,Periodo,2);   // PWM 1953Hz a 500KHz
      set_pwm1_duty(128);
      setup_ccp1(CCP_PWM);
   }
   if(!i)
   {
      setup_timer_2(T2_DIV_BY_1,Periodo,2);   // PWM 1953Hz a 500KHz
      set_pwm2_duty(128);
      setup_ccp2(CCP_PWM);
   }
}

void Operatividad()
{
   LeerADC();
   Estado();
   if(OperandoA)
   {
      if(AmperajeA>=1.5)
      {
         PediluvioA=0;
         ExcesoA=1;
         lcd_gotoxy(1,1);
         printf(lcd_putc," EXCESO DE SAL  ");
      }
   }
   if(OperandoB)
   {
      if(AmperajeB>=1.5)
      {
         PediluvioB=0;
         ExcesoB=1;
         lcd_gotoxy(1,2);
         printf(lcd_putc," EXCESO DE SAL  ");
      }
   }
   for(i=0;i<1;i++)
   {
      if(Terapia[i]==0)
      {
         FreqOFF();
      }
      if(Terapia[i]==1)
      {
         switch(Tiempo[i])
         {
            case 60:Freq30KHz();break;    // 7
            case 54:FreqOFF();break;      // 20
            case 34:Freq30KHz();break;    // 7
            case 27:FreqOFF();break;      // 20
            case 7:Freq30KHz();break;     // 7
         }
      }
      if(Terapia[i]==2)
      {
         switch(Tiempo[i])
         {
            case 60:Freq30KHz();break;
            case 30:Freq170KHz();break;
         }
      }
      if(Terapia[i]==3)
      {
         switch(Tiempo[i])
         {
            case 60:Freq30KHz();break;
         }
      }
      if(Terapia[i]==4)
      {
         switch(Tiempo[i])
         {
            case 60:Freq170KHz();break;
         }
      }
      if(Terapia[i]==5)
      {
         FreqAcordeon();
         switch(Periodo)
         {
            case 0:  AscDesc=0;break;       // Sube
            case 255:AscDesc=1;break;       // Baja
         }
         switch(AscDesc)
         {
            case 0:Periodo++;break;         // Incrementar Periodo
            case 1:Periodo--;break;         // Decrementar Periodo
         }
      }
   }
}
void main()
{
   set_tris_a(0b11111111);
   set_tris_b(0x00);
   set_tris_c(0x00);
   output_b(0x00);
   output_c(0x00);
   enable_interrupts(global|int_timer0);
   setup_timer_0(T0_INTERNAL|T0_DIV_256|T0_8_BIT);
   setup_adc_ports(sAN0|sAN1);
   setup_adc(ADC_CLOCK_DIV_2);
   set_timer0(58);
   lcd_init();
   PediluvioA=PediluvioB=1;
   while(TRUE)
   {
      Operatividad();
      if(!BArriba)     // Pediluvio A - Iniciar / Detener
      {
         while(!BArriba){RetBoton();}
         if(!OperandoA)
         {
            if(PediluvioA|Zapper[1])
            {
               OperandoA=1;
            }
         }
         else
         {
            OperandoA=0;
            Tiempo[1]=0;
            S1=0;
         }
      }
      if(!BAbajo)      // Pediluvio B - Iniciar / Detener
      {
         while(!BAbajo){RetBoton();}
         if(!OperandoB)
         {
            if(PediluvioB|Zapper[0])
            {
               OperandoB=1;
            }
         }
         else
         {
            OperandoB=0;
            Tiempo[0]=0;
            S2=0;
         }
      }
      if(!BIzquierda)  // Zapper A - Terapia
      {
         while(!BIzquierda){RetBoton();}
         if(!OperandoA)
         {
            Terapia[1]++;
            if(Terapia[1]==6)
            {
               Terapia[1]=0;
            }
         }
      }
      if(!BDerecha)    // Zapper B - Terapia
      {
         while(!BDerecha){RetBoton();}
         if(!OperandoB)
         {
            Terapia[0]++;
            if(Terapia[0]==6)
            {
               Terapia[0]=0;
            }
         }
      }
   }
}
