// Shell functions


//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include "gpio.h"
#include "shell.h"
#include "kernel.h"

// REQUIRED: Add header files here for your strings functions, ...

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------


void getsUart0(USER_DATA *data) // passed by reference
{
    char ch;
    int chcount=0;//compare with the max size of the buffer

    while(chcount<=MAX_CHARS )
    {
        ch =getcUart0();
        if (ch ==8 ||ch ==127)
        {
            if (chcount >0)//
                chcount--;
        }

        else if (ch==10||ch==13||(chcount==MAX_CHARS))
        {
            data->buffer[chcount]='\0';
            return;

        }
        else if (ch>=32)
//Add the remaining characters to the buffer itself
        {
            data->buffer[chcount]=ch;       // Update buffer array
            chcount++;
        }



    }
}


bool is_alpha(char ch) {
    return ((ch >= 'A' && ch <= 'Z')||(ch >= 'a' && ch <= 'z'));
}

bool is_numeric(char ch) {
    return ((ch >= '0')&&( ch <= '9'));
}

bool is_delimiter(char ch) {
    return((!is_alpha(ch)) && (!is_numeric(ch)));
}
void parseFields(USER_DATA *data)
{
    char prev, ch;
    uint8_t i=0;

    char * ptr=&data->buffer[0];
    prev ='D';

    data->fieldCount =0;



    while (*ptr!= '\0')
    {
        ch=*ptr;


        if(is_delimiter(ch))
        {

            *ptr ='\0';//thats what we do in strtok function
        }
        // updating the type stored in the fieldType and focus on the first itself that is right after the space /delimiter
        // (a transition from a delimiter to a alpha or numeric character).
        if (prev=='D' && (is_alpha(ch) || is_numeric(ch) ))
        {
            if(data->fieldCount<=MAX_FIELDS)
            {
                data->fieldType[data->fieldCount]= is_alpha(ch) ? 'A' : 'N';
                data->fieldPosition[data->fieldCount]= i;//as explained by TA example of cos
            }
            data->fieldCount++;
        }
        if(is_delimiter(ch))
            prev='D';
        else
            prev= ch;//Make the previous character stored equal to the new character(given in pdf)
        ptr++;
        i++;
//keep moving through the buffer string until the end is found
//if field count equals MAX_FIELDS, return from the function
        if (data->fieldCount== MAX_FIELDS)
        {
            return;
        }
    }
}
char* getFieldString(USER_DATA *data, uint8_t fieldNumber)
{

    if (fieldNumber < MAX_FIELDS)
    {
        if(data->fieldType[fieldNumber]=='A')
            return &data->buffer[data->fieldPosition[fieldNumber]];
    }

    return 0;

}

uint32_t getFieldInteger(USER_DATA *data, uint8_t fieldNumber)
{
    char c;
    int i=0;
    uint32_t num=0;
    uint32_t max_uint32 = 0xFFFFFFFF;
    // you will need to convert this to a number when returning and not a pointer
    if(fieldNumber<MAX_FIELDS) {
        i=data->fieldPosition[fieldNumber];
        if (data->fieldType[fieldNumber] =='N') {
            for (; data->buffer[i] !='\0';i++) {
                c = data->buffer[i] - 48;
                num =(num * 10 + c)% (max_uint32 + 1); // Take modulo to handle rollover
            }
            return num;
        }
    }
    return 0;
}

bool strcompare(char *str1, const char *str2)
{
    while ((*str1 != '\0') && (*str2 != '\0'))
    {
        if (*str1 != *str2)
        {
            return 0;
        }
        str1++;
        str2++;
    }

    if (*str1 == *str2)
    {
        return 1;
    }

    return 0;
}

bool isCommand(USER_DATA *data, const char strCommand[], uint8_t minArguments)
{
    if((strcompare(&data->buffer[data->fieldPosition[0]],strCommand))&&(minArguments < data->fieldCount))
        return 1;
    return 0;

}









// REQUIRED: add processing for the shell commands through the UART here
//void shell(void)
//{
//}




void shell(void)
{

    while(1)
    {
        if(kbhitUart0())
        {
        putsUart0("\n enter ch \n\r");
        USER_DATA data;
        //int i;
        getsUart0(&data);
        parseFields(&data);

        int preemption;

      /*  if(isCommand(&data,"preempt",1))//can on or off
        {
            char *p;

            p=getFieldString(&data,1);
            if(strcompare(p,"on"))
            {
                preemption = 1;
                preempt(preemption);
            }
            else if(strcompare(p,"off"))
            {
                preemption = 0;
                preempt(preemption);
            }
            else
            {
                putsUart0("\n Wrong Argument\n");
                putcUart0('\r');
                putcUart0('\n');
            }

        }
        int scheduler=0;
        if(isCommand(&data,"sched",1))//can on or off
        {
            char *p;

            p=getFieldString(&data,1);
            if (strcompare(p, "prio"))
            {
                scheduler = 1;
                type=1;
                sched(scheduler);
            }
            else if (strcompare(p, "rr"))
            {
                scheduler=0;
                type=0;
                sched(scheduler);
            }
            else
            {
                putsUart0("\nWrong Argument\n");
                putcUart0('\r');
                putcUart0('\n');
            }

        }

        */
        if(isCommand(&data,"pidof",1))
        {
            char str[40],*temp;
            uint8_t i=0;
            temp= getFieldString(&data, 1);
            while(*temp!='\0')
            {
                str[i++]=*temp;
                temp++;
            }
            str[i]='\0';
            pidofsv(str);
           // pidof(str);
        }


        if(isCommand(&data,"kill",1))
        {

            uint32_t num=0;
            //giving the pid value to it
            num=(uint32_t)getFieldInteger(&data, 1);
            killsv(num);

        }

        if(isCommand(&data,"run",1))
        {


            char *temp;
            temp= getFieldString(&data,1);
            runsv(temp);
       /*     putsUart0("\n");
            putsUart0("\r");
            putsUart0("RUN");
            putsUart0(temp);
            putsUart0("\n");
            putsUart0("\r");
            GPIO_PORTF_DATA_R|= 2;
*/

        }

        if(isCommand(&data,"pkill",1))
        {
            char *p;
            p=getFieldString(&data,1);
            pkillsv(p);
            /*
            putsUart0("\n");
            putsUart0("\r");
            putsUart0("killed");
            putsUart0(p);
            putsUart0("\n");
            putsUart0("\r");
               */

        }

       if(isCommand(&data,"preempt",1))//can on or off
                {
                    char *p;

                    p=getFieldString(&data,1);
                    preemptsv(p);
                 /*   if(strcompare(p,"on"))
                    {
                        preemption = 1;
                        preempt(preemption);
                    }
                    else if(strcompare(p,"off"))
                    {
                        preemption = 0;
                        preempt(preemption);
                    }
                    else
                    {
                        putsUart0("\n Wrong Argument\n");
                        putcUart0('\r');
                        putcUart0('\n');
                    }
                    */

                }




        int scheduler=0;
               if(isCommand(&data,"sched",1))//can on or off
               {
                   char *p;

                   p=getFieldString(&data,1);
                   schedsv(p);
               }




        if(isCommand(&data,"ipcs",0))
        {
            ipcsv();

        }
       if(isCommand(&data,"ps",0))
       {
            pssv();

       }


        if(isCommand(&data,"reboot",0))
        {
            rebootsv();

           // NVIC_APINT_R =NVIC_APINT_VECTKEY|NVIC_APINT_SYSRESETREQ;

        }



    }
        else
                yield();
}

}
