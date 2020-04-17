#include <stdint.h>
#include <stdbool.h>
#include "Lab9_UART.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"				// This and below manually added
#include "inc/tm4c123gh6pm.h"
#include "driverlib/uart.h"

//*****************************************************************************
volatile uint32_t letter;

void
PortFunctionInit(void)
{
    //
    // Enable Peripheral Clocks 
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //
    // Enable pin PF3 for GPIOOutput (green)
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);

    //
    // Enable pin PF2 for GPIOOutput (blue)
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);

    //
    // Enable pin PF1 for GPIOOutput (red)
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1); 
}

void UART_Init(void)
{
	// Enable peripheral clocks for UART0 
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	
	//
  // Enable pin PA0 & PA1 for UART0 U0RX
  //
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	
	// Configure UART 115200 baud rate, clk freq, 8 bit word leng, 1 stop bit, & no parity.
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	
	// Disable FIFO (First In First Out) registers. 
	UARTFIFODisable(UART0_BASE);
	
	//enable the UART interrupt
	IntEnable(INT_UART0); 
	
	//only enable RX and TX interrupts
  UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_TX); 
}

// baud rate is bits/sec. Bit time is reciprocal of baud rate. bandwidth is amnt of useful info transmitted/sec
void UARTIntHandler(void)
{
	uint32_t status;
	
	// get interrupt status
	status = UARTIntStatus(UART0_BASE, true);
	
	// clear the asserted interrupts
	UARTIntClear(UART0_BASE, status);

	while(UARTCharsAvail(UART0_BASE)) //loop while there are chars
  {
		letter = UARTCharGet(UART0_BASE);
		
		//echo character (reads from receive register and sends to transmit registers)
    UARTCharPutNonBlocking(UART0_BASE, UARTCharGetNonBlocking(UART0_BASE)); 
		
		// go through letters RrBbGg, default is LED off and invalid option
		switch(letter)
		{
			// R case, Turn on red LED
			case 'R':	
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
				UARTCharPut(UART0_BASE, letter);
				UARTCharPut(UART0_BASE, '\n');
				UARTCharPut(UART0_BASE, '\r');
			break;
			
			// r case, Turn off red LED
			case 'r':
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
				UARTCharPut(UART0_BASE, letter);
				UARTCharPut(UART0_BASE, '\n');
				UARTCharPut(UART0_BASE, '\r');
			break;
			
			// B case, Turn on blue LED
			case 'B':
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);	
				UARTCharPut(UART0_BASE, letter);
				UARTCharPut(UART0_BASE, '\n');
				UARTCharPut(UART0_BASE, '\r');
			break;
			
			// b case, Turn off blue LED
			case 'b':
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
				UARTCharPut(UART0_BASE, letter);
				UARTCharPut(UART0_BASE, '\n');
				UARTCharPut(UART0_BASE, '\r');
			break;
			
			// G case, Turn on green LED
			case 'G':
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
				UARTCharPut(UART0_BASE, letter);
				UARTCharPut(UART0_BASE, '\n');
				UARTCharPut(UART0_BASE, '\r');
			break;
			
			// g case, Turn off green LED
			case 'g':
				GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);	
				UARTCharPut(UART0_BASE, letter);
				UARTCharPut(UART0_BASE, '\n');
				UARTCharPut(UART0_BASE, '\r');
			break;
			
			// Else, Off all LEDs, write Invalid input
			default:
			{
				// display Invalid Input error message
				char error[] = "Invalid Input\n\r";
				for (int k = 0; k<sizeof(error); k++)
				{
					UARTCharPut(UART0_BASE, error[k]);
				}
			}
			break;
		}
  }
}

int main(void)
{
	// Create 50MHz system clock frequency
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	
	// initialize GPIO F ports
	PortFunctionInit();
	
	// Call UART0A interrupt
	UART_Init();
	
	// globally enable interrupt, enable processor interrupt
	IntMasterEnable();
	
	// print Initial prompt message
	char str[] = "Turn ON/off LED by entering key R, r, B, b, G, or g: \n\r";
	for (int i = 0; i<sizeof(str); i++)
	{
		UARTCharPut(UART0_BASE, str[i]);
	}
	
	while (1) //let interrupt handler do the UART echo function
	{
		
	}
}
