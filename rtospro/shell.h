// Shell functions


//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef SHELL_H_
#define SHELL_H_

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------
#define RED_LED_MASK 2
#define MAX_CHARS 80
#define MAX_FIELDS 5
typedef struct _USER_DATA
{
    char buffer[MAX_CHARS + 1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;

void shell(void);
void getsUart0(USER_DATA *data);
bool is_alpha(char ch);
bool is_numeric(char ch);
bool is_delimiter(char ch);
void parseFields(USER_DATA *data);
char* getFieldString(USER_DATA *data, uint8_t fieldNumber);
uint32_t getFieldInteger(USER_DATA *data, uint8_t fieldNumber);
bool strcompare( char *str1, const char *str2);
bool isCommand(USER_DATA *data, const char strCommand[], uint8_t minArguments);




#endif
