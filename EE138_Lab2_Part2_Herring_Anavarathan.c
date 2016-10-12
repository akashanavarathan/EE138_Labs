//////////////////////////////////////////////////////////////////////////////////////
// Lab 2: Part 2: DAC
// Scott Herring and Akash Anavarathan
//////////////////////////////////////////////////////////////////////////////////////

#include <asf.h>
#include <math.h>
#define PI 3.14159

void Simple_Clk_Init(void);
void configure_dac(void);
void configure_dac_clock(void);
Dac *portdac = DAC; 

int sinArr[256];
int cycles = 256;
int index =0;


int main(void)
{
	Simple_Clk_Init();
	configure_dac_clock();
	configure_dac();
	
	for (int store = 0; store < cycles; store++)
		{
			sinArr[store] = (((sin(2* PI* store/cycles) + 1.1) * 1023) / 3.3) / 2;
		}
	
	while (1) {

				/* Wait until the synchronization is complete */
		while (portdac->STATUS.reg & DAC_STATUS_SYNCBUSY) {
		};
		
		
			portdac->DATA.reg = sinArr[index++];
			if (index >= cycles)
			{
				index=0;
			}

	}

}

void configure_dac_clock(void)
{
	PM->APBCMASK.reg |= (1u << 18);		// PM_APBCMASK DAC is in the 18 position
	
	uint32_t temp = 0x1A; 			// ID for GCLK_DAC is 0x1A
	temp |= 0<<8; 					// Selection Generic clock generator 0
	GCLK->CLKCTRL.reg = temp; 			// Setup in the CLKCTRL register
	GCLK->CLKCTRL.reg |= 0x1u << 14; 		// enable it.
}

void configure_dac(void)
{
	//set pin as output for the dac
	Port *ports = PORT_INSTS;
	PortGroup *por = &(ports->Group[0]);
	
	por->PINCFG[2].bit.PMUXEN = 0x1;		// set to correct pin configuration
	por->PMUX[1].bit.PMUXE = 0x1;			// set to correct peripheral


	while (portdac->STATUS.reg & DAC_STATUS_SYNCBUSY) {
		/* Wait until the synchronization is complete */
	}

	/* Set reference voltage with CTRLB */
	portdac->CTRLB.reg = (1u << 6);


	while (portdac->STATUS.reg & DAC_STATUS_SYNCBUSY) {
		/* Wait until the synchronization is complete */
	}

	/* Enable the module with CTRLA */
	portdac->CTRLA.reg = (1u << 1);

	/* Enable selected output with CTRLB*/
	portdac->CTRLB.reg = (1u << 0);

}

//Simple Clock Initialization
void Simple_Clk_Init(void)
{
	/* Various bits in the INTFLAG register can be set to one at startup.
	   This will ensure that these bits are cleared */
	
	SYSCTRL->INTFLAG.reg = SYSCTRL_INTFLAG_BOD33RDY | SYSCTRL_INTFLAG_BOD33DET |
			SYSCTRL_INTFLAG_DFLLRDY;
			
	system_flash_set_waitstates(0);  		//Clock_flash wait state = 0

	SYSCTRL_OSC8M_Type temp = SYSCTRL->OSC8M;      	/* for OSC8M initialization  */

	temp.bit.PRESC    = 0;    			// no divide, i.e., set clock=8Mhz  (see page 170)
	temp.bit.ONDEMAND = 1;    			//  On-demand is true
	temp.bit.RUNSTDBY = 0;    			//  Standby is false
	
	SYSCTRL->OSC8M = temp;

	SYSCTRL->OSC8M.reg |= 0x1u << 1;  		// SYSCTRL_OSC8M_ENABLE bit = bit-1 (page 170)
	
	PM->CPUSEL.reg = (uint32_t)0;    		// CPU and BUS clocks Divide by 1  (see page 110)
	PM->APBASEL.reg = (uint32_t)0;     		// APBA clock 0= Divide by 1  (see page 110)
	PM->APBBSEL.reg = (uint32_t)0;     		// APBB clock 0= Divide by 1  (see page 110)
	PM->APBCSEL.reg = (uint32_t)0;     		// APBB clock 0= Divide by 1  (see page 110)

	PM->APBAMASK.reg |= 01u<<3;   			// Enable Generic clock controller clock (page 127)

	/* Software reset Generic clock to ensure it is re-initialized correctly */

	GCLK->CTRL.reg = 0x1u << 0;   			// Reset gen. clock (see page 94)
	while (GCLK->CTRL.reg & 0x1u ) {  /* Wait for reset to complete */ }
	
	// Initialization and enable generic clock #0

	*((uint8_t*)&GCLK->GENDIV.reg) = 0;  		// Select GCLK0 (page 104, Table 14-10)

	GCLK->GENDIV.reg  = 0x0100;   		 	// Divide by 1 for GCLK #0 (page 104)

	GCLK->GENCTRL.reg = 0x030600;  		 	// GCLK#0 enable, Source=6(OSC8M), IDC=1 (page 101)
}
