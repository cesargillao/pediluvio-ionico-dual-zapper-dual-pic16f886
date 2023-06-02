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
#bit BMenu      = 0x005.4
#bit BIzquierda = 0x005.5
#bit BDerecha   = 0x005.7

#bit PediluvioA = 0x007.0
#bit PediluvioB = 0x007.3
#bit Buzzer     = 0x007.4

int16 L1,L2;
float AmperajeA=0,AmperajeB=0;
int C1=0,S1=0;
int C2=0,S2=0;
int Tiempo=0,Frecuencia=0,Terapia=0;
int NivelMenu=0;
int1 OperandoA=0,OperandoB=0,ExcesoA=0,ExcesoB=0;
int1 Indicador=1,Pagina=0,Opcion,Periodo=0,AscDesc=0,i;
int Ind[2]={0x00,0x3E};
int Ter[2]={0x00,0x00};    // Ter{ TerapiaB , TerapiaA }
int Tie[2]={0x00,0x00};    // Min{ MinutosB , MinutosA }
int Zapper[2]={0x00,0x00}; // Zapper{ ZapperB , ZapperA }

void FreqOFF();
#int_timer0
void timer() 
{
   if(OperandoA)
   {
      set_timer0(58);
      if(C1==0)
      {
         C1=20;
         if(S1==0)
         {
            S1=60;
            if(Tie[1]==0)
            {
               Tie[1]=60;
            }
            Tie[1]--;
         }
         S1--;
         if(!S1 && !Tie[1])
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
         C2=20;
         if(S2==0)
         {
            S2=60;
            if(Tie[0]==0)
            {
               Tie[0]=60;
            }
            Tie[0]--;
         }
         S2--;
         if(!S2 && !Tie[0])
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
void LimpiarPant()
{
   printf(lcd_putc,"\f");
}
void LeerAmp()
{
   set_adc_channel(0);
   delay_us(20);
   L1=read_adc();
   AmperajeA=(L1*4.5454)/1024.0;
   set_adc_channel(1);
   delay_us(20);
   L2=read_adc();
   AmperajeB=(L2*4.5454)/1024.0;
}
void Estado()
{
   if(!ExcesoA)
   {
      lcd_gotoxy(1,1);
      printf(lcd_putc,"A %02u:%02u %1.2fA Z%u"Tie[1],S1,AmperajeA,Ter[1]);
   }
   if(!ExcesoB)
   {
      lcd_gotoxy(1,2);
      printf(lcd_putc,"B %02u:%02u %1.2fA Z%u"Tie[0],S2,AmperajeB,Ter[0]);
   }
}
void OnOff()
{
   printf(lcd_putc,"%c ENCENDIDO\n%c APAGAR",Ind[Indicador],Ind[!Indicador]);
}
void OffOn()
{
   printf(lcd_putc,"%c ENCENDER\n%c APAGADO",Ind[Indicador],Ind[!Indicador]);
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

void AsignarTiempo()
{
   LimpiarPant();
   Tie[Opcion]=Tiempo;
}
void AsignarTerapia()
{
   LimpiarPant();
   Ter[Opcion]=Terapia;
}
void MenuZapperTiempo()
{
   while(NivelMenu==4)
   {
      Tiempo=Tie[Opcion];
      lcd_gotoxy(1,1);
      printf(lcd_putc,"TIEMPO\n%02u MINUTOS",Tie[Opcion]);
      if(!BArriba)
      {
         RetBoton();
         Tiempo++;
         if(Tiempo==61)
         {
            Tiempo=1;
         }
         AsignarTiempo();
      }
      if(!BAbajo)
      {
         RetBoton();
         if(Tiempo==0)
         {
            Tiempo=61;
         }
         Tiempo--;
         AsignarTiempo();
      }
      if(!BIzquierda)
      {
         while(!BIzquierda){RetBoton();}
         NivelMenu--;
      }
      if(!BMenu)
      {
         while(!BMenu){RetBoton();}
         NivelMenu=0;
         Pagina=0;
      }
   }
   LimpiarPant();
}
void MenuZapperFrecuencia()
{
   while(NivelMenu==4)
   {
      
   }
}
void MenuZapperTerapia()
{
   while(NivelMenu==4)
   {
      Terapia=Ter[Opcion];
      switch(Ter[Opcion])
      {
         case 0:printf(lcd_putc,"\nNINGUNA");break;
         case 1:printf(lcd_putc,"\nZAPPER");break;
         case 2:printf(lcd_putc,"\nFRECUENCIA DUAL");break;
         case 3:printf(lcd_putc,"\nFRECUENCIA BAJA");break;
         case 4:printf(lcd_putc,"\nFRECUENCIA ALTA");break;
         case 5:printf(lcd_putc,"\nACORDEON");break;
      }
      lcd_gotoxy(1,1);
      printf(lcd_putc,"TERAPIA #%u",Ter[Opcion]);
      if(!BArriba)
      {
         while(!BArriba){RetBoton();}
         Terapia++;
         if(Terapia==6)
         {
            Terapia=1;
         }
         AsignarTerapia();
      }
      if(!BAbajo)
      {
         while(!BAbajo){RetBoton();}
         if(Terapia==0)
         {
            Terapia=6;
         }
         Terapia--;
         AsignarTerapia();
      }
      if(!BIzquierda)
      {
         while(!BIzquierda){RetBoton();}
         NivelMenu--;
      }
      if(!BMenu)
      {
         while(!BMenu){RetBoton();}
         NivelMenu=0;
      }
   }
   LimpiarPant();
}
void MenuZapperEstado()
{
   while(NivelMenu==4)
   {
      lcd_gotoxy(1,1);
      if((Zapper[Opcion] && Opcion)^(Zapper[!Opcion] && !Opcion))    // (ZapperA && Opcion)^(ZapperB && !Opcion)
      {
         OnOff();
      }
      if((!Zapper[Opcion] && Opcion)^(!Zapper[!Opcion] && !Opcion))  // (!ZapperA && Opcion)^(!ZapperB && !Opcion)
      {
         OffOn();
      }
      if(!BArriba)
      {
         while(!BArriba){RetBoton();}
         Indicador=1;
      }
      if(!BAbajo)
      {
         while(!BAbajo){RetBoton();}
         Indicador=0;
      }
      if(!BIzquierda)
      {
         while(!BIzquierda){RetBoton();}
         if(NivelMenu==3)
         {
            Pagina=0;
         }
         NivelMenu--;
      }
      if(!BDerecha)
      {
         while(!BDerecha){RetBoton();}
         LimpiarPant();
         Zapper[Opcion]=Ter[Opcion]=Indicador;
      }
      if(!BMenu)
      {
         while(!BMenu){RetBoton();}
         Pagina=0;
         NivelMenu=0;
      }
   }
   LimpiarPant();
}
void MenuZapper()
{
   while(NivelMenu==3)
   {
      if(!Pagina)
      {
         lcd_gotoxy(1,1);
         printf(lcd_putc,"%c ESTADO\n%c TERAPIA",Ind[Indicador],Ind[!Indicador]);
         if(!BArriba)
         {
            while(!BArriba){RetBoton();}
            Indicador=1;
         }
         if(!BAbajo)
         {
            while(!BAbajo){RetBoton();}
            if(!Indicador)
            {
               Pagina=1;
               Indicador=1;
               LimpiarPant();
            }else{
            Indicador=0;
            }
         }
         if(!BIzquierda)
         {
            while(!BIzquierda){RetBoton();}
            if(NivelMenu==3)
            {
               Pagina=0;
            }
            NivelMenu--;
         }
         if(!BDerecha)
         {
            while(!BDerecha){RetBoton();}
            NivelMenu++;
            LimpiarPant();
            if(Indicador)
            {
               MenuZapperEstado();
            }
            if(!Indicador)
            {
               Indicador=1;
               MenuZapperTerapia();
            }
         }
         if(!BMenu)
         {
            while(!BMenu){RetBoton();}
            Pagina=0;
            NivelMenu=0;
         }
      }
      if(Pagina)
      {
         lcd_gotoxy(1,1);
         printf(lcd_putc,"%c FRECUENCIA\n%c TIEMPO",Ind[Indicador],Ind[!Indicador]);
         if(!BArriba)
         {
            while(!BArriba){RetBoton();}
            if(Indicador)
            {
               Pagina=0;
               Indicador=0;
               LimpiarPant();
            }else{
            Indicador=1;
            }
         }
         if(!BAbajo)
         {
            while(!BAbajo){RetBoton();}
            if(!Indicador)
            {
               Pagina=1;
               Indicador=1;
               LimpiarPant();
            }else{
            Indicador=0;
            }
         }
         if(!BIzquierda)
         {
            while(!BIzquierda){RetBoton();}
            if(NivelMenu==3)
            {
               Pagina=0;
            }
            NivelMenu--;
         }
         if(!BDerecha)
         {
            while(!BDerecha){RetBoton();}
            NivelMenu++;
            LimpiarPant();
            if(Indicador)
            {
               MenuZapperFrecuencia();
            }
            if(!Indicador)
            {
               Indicador=1;
               MenuZapperTiempo();
            }
         }
         if(!BMenu)
         {
            while(!BMenu){RetBoton();}
            Pagina=0;
            NivelMenu=0;
         }
      }
   }
   LimpiarPant();
}
void MenuPediluvio()
{
   while(NivelMenu==3)
   {
      lcd_gotoxy(1,1);
      if((PediluvioA && Opcion)^(PediluvioB && !Opcion))
      {
         OnOff();
      }
      if((!PediluvioA && Opcion)^(!PediluvioB && !Opcion))
      {
         OffOn();
      }
      if(!BArriba)
      {
         while(!BArriba){RetBoton();}
         Indicador=1;
      }
      if(!BAbajo)
      {
         while(!BAbajo){RetBoton();}
         Indicador=0;
      }
      if(!BIzquierda)
      {
         while(!BIzquierda){RetBoton();}
         if(NivelMenu==3)
         {
            Pagina=0;
         }
         NivelMenu--;
      }
      if(!BDerecha)
      {
         while(!BDerecha){RetBoton();}
         if(Opcion)
         {
            PediluvioA=Indicador;
         }
         if(!Opcion)
         {
            PediluvioB=Indicador;
         }
      }
      if(!BMenu)
      {
         while(!BMenu){RetBoton();}
         Pagina=0;
         NivelMenu=0;
      }
   }
   LimpiarPant();
}
void Menu()
{
   while(NivelMenu==2)
   {
      lcd_gotoxy(1,1);
      printf(lcd_putc,"%c PEDILUVIO\n%c ZAPPER",Ind[Indicador],Ind[!Indicador]);
      if(!BArriba)
      {
         while(!BArriba){RetBoton();}
         Indicador=1;
      }
      if(!BAbajo)
      {
         while(!BAbajo){RetBoton();}
         Indicador=0;
      }
      if(!BIzquierda)
      {
         while(!BIzquierda){RetBoton();}
         if(NivelMenu==3)
         {
            Pagina=0;
         }
         NivelMenu--;
      }
      if(!BDerecha)
      {
         while(!BDerecha){RetBoton();}
         NivelMenu++;
         LimpiarPant();
         if(Indicador)
         {
            MenuPediluvio();
         }
         if(!Indicador)
         {
            Indicador=1;
            MenuZapper();
         }
      }
      if(!BMenu)
      {
         while(!BMenu){RetBoton();}
         Pagina=0;
         NivelMenu=0;
      }
   }
   LimpiarPant();
}
void Opciones()
{
   while(NivelMenu==1)
   {
      lcd_gotoxy(1,1);
      printf(lcd_putc,"%c OPCIONES A\n%c OPCIONES B",Ind[Indicador],Ind[!Indicador]);
      if(!BArriba)
      {
         while(!BArriba){RetBoton();}
         Indicador=1;
      }
      if(!BAbajo)
      {
         while(!BAbajo){RetBoton();}
         Indicador=0;
      }
      if(!BIzquierda)
      {
         while(!BIzquierda){RetBoton();}
         if(NivelMenu==3)
         {
            Pagina=0;
         }
         NivelMenu--;
      }
      if(!BDerecha)
      {
         while(!BDerecha){RetBoton();}
         Opcion=Indicador;
         NivelMenu++;
         Indicador=1;
         LimpiarPant();
         Menu();
      }
      if(!BMenu)
      {
         while(!BMenu){RetBoton();}
         Pagina=0;
         NivelMenu=0;
      }
   }
   LimpiarPant();
}
void Operatividad()
{
   LeerAmp();
   Estado();
   if(OperandoA)
   {
      if(AmperajeA>=1.5)
      {
         PediluvioA=0;
         ExcesoA=1;
         lcd_gotoxy(1,1);
         printf(lcd_putc," EXCESO DE SAL");
      }
   }
   if(OperandoB)
   {
      if(AmperajeB>=1.5)
      {
         PediluvioB=0;
         ExcesoB=1;
         lcd_gotoxy(1,2);
         printf(lcd_putc,"EXCESO DE SAL");
      }
   }
   for(i=0;i<1;i++)
   {
      if(Ter[i]==1)
      {
         switch(Tie[i])
         {
            case 61:Freq30KHz();break;    // 7
            case 54:FreqOFF();break;      // 20
            case 34:Freq30KHz();break;    // 7
            case 27:FreqOFF();break;      // 20
            case 7:Freq30KHz();break;     // 7
         }
      }
      if(Ter[i]==2)
      {
         switch(Tie[i])
         {
            case 60:Freq30KHz();break;
            case 30:Freq170KHz();break;
         }
      }
      if(Ter[i]==3)
      {
         switch(Tie[i])
         {
            case 60:Freq30KHz();break;
         }
      }
      if(Ter[i]==4)
      {
         switch(Tie[i])
         {
            case 60:Freq170KHz();break;
         }
      }
      if(Ter[i]==5)
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
         if(OperandoA==0)
         {
            if(PediluvioA|Zapper[Opcion])
            {
               OperandoA=1;
            }
         }else{
            OperandoA=0;
            Tie[1]=0;
            S1=0;
         }
      }
      if(!BAbajo)     // Pediluvio B - Iniciar / Detener
      {
         while(!BAbajo){RetBoton();}
         if(OperandoB==0)
         {
            if(PediluvioB|Zapper[!Opcion])
            {
               OperandoB=1;
            }
         }else{
            OperandoB=0;
            Tie[0]=0;
            S2=0;
         }
      }
      if(!BMenu)     // Menu
      {
         while(!BMenu){RetBoton();}
         NivelMenu=1;
         Indicador=1;
         LimpiarPant();
         Opciones();
      }
   }
}
