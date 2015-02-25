#ifndef CC1101_INTERFACE_CC430_H
#define CC1101_INTERFACE_CC430_H


#include <stdint.h>
#include <stdbool.h>

#include <msp430.h>

#include "cc1101_phy.h"

#define RADIO_INST_READY_WAIT()		while(!(RF1AIFCTL1 & RFINSTRIFG))
#define RADIO_DIN_READY_WAIT()		while(!(RF1AIFCTL1 & RFDINIFG))
#define RADIO_STAT_READY_WAIT()		while(!(RF1AIFCTL1 & RFSTATIFG))
#define RADIO_DOUT_READY_WAIT()		while(!(RF1AIFCTL1 & RFDOUTIFG))

#define ENTER_CRITICAL_SECTION(x)  	x = __get_interrupt_state(); __disable_interrupt()
#define EXIT_CRITICAL_SECTION(x)    __set_interrupt_state(x)

// Radio Core Interrupt Flags
#define RFIFG_FLAG_IOCFG0        	0x0001  // RFIFG0
#define RFIFG_FLANK_IOCFG0       	0x0000
#define RFIFG_FLAG_IOCFG1        	0x0002  // RFIFG1
#define RFIFG_FLANK_IOCFG1       	0x0000
#define RFIFG_FLAG_IOCFG2        	0x0004  // RFIFG2
#define RFIFG_FLANK_IOCFG2       	0x0000
#define RFIFG_FLAG_RXFilled      	0x0008  // RFIFG3 Positive Edge
#define RFIFG_FLANK_RXFilled      	0x0000
#define RFIFG_FLAG_RXDrained  		0x0008  // RFIFG3 Negative Edge
#define RFIFG_FLANK_RXDrained  		0x0008
#define RFIFG_FLAG_RXFilledOrEOP 	0x0010  // RFIFG4 Positive Edge
#define RFIFG_FLANK_RXFilledOrEOP   0x0000
#define RFIFG_FLAG_RXEmpty          0x0010  // RFIFG4 Negative Edge
#define RFIFG_FLANK_RXEmpty    		0x0010
#define RFIFG_FLAG_TXFilled      	0x0020  // RFIFG5 Positive Edge
#define RFIFG_FLANK_TXFilled      	0x0000
#define RFIFG_FLAG_TXBelowThresh    0x0020  // RFIFG5 Negative Edge
#define RFIFG_FLANK_TXBelowThresh   0x0020
#define RFIFG_FLAG_TXFull           0x0040  // RFIFG6 Positive Edge
#define RFIFG_FLANK_TXFull          0x0000
#define RFIFG_FLAG_TXDrained     	0x0040  // RFIFG6 Negative Edge
#define RFIFG_FLANK_TXDrained     	0x0040
#define RFIFG_FLAG_RXOverflow       0x0080  // RFIFG7 Positive Edge
#define RFIFG_FLANK_RXOverflow      0x0000
#define RFIFG_FLAG_RXFlushed        0x0080  // RFIFG7 Negative Edge
#define RFIFG_FLANK_RXFlushed       0x0080
#define RFIFG_FLAG_TXUnderflow    	0x0100  // RFIFG8 Positive Edge
#define RFIFG_FLANK_TXUnderflow   	0x0000
#define RFIFG_FLAG_TXFlushed        0x0100  // RFIFG8 Negative Edge
#define RFIFG_FLANK_TXFlushed       0x0100
#define RFIFG_FLAG_SyncWord         0x0200  // RFIFG9 Positive Edge
#define RFIFG_FLANK_SyncWord        0x0000
#define RFIFG_FLAG_EndOfPacket      0x0200  // RFIFG9 Negative Edge
#define RFIFG_FLANK_EndOfPacket     0x0200
#define RFIFG_FLAG_CRCOK         	0x0400  // RFIFG10 Positive Edge
#define RFIFG_FLANK_CRCOK         	0x0000
#define RFIFG_FLAG_RXFirstByte      0x0400  // RFIFG10 Negative Edge
#define RFIFG_FLANK_RXFirstByte     0x0400
#define RFIFG_FLAG_PQTReached       0x0800  // RFIFG11 Positive Edge
#define RFIFG_FLANK_PQTReached      0x0000
#define RFIFG_FLAG_LPW              0x0800  // RFIFG11 Negative Edge
#define RFIFG_FLANK_LPW             0x0800
#define RFIFG_FLAG_ClearChannel     0x1000  // RFIFG12 Positive Edge
#define RFIFG_FLANK_ClearChannel    0x0000
#define RFIFG_FLAG_RSSIAboveThr     0x1000  // RFIFG12 Negative Edge
#define RFIFG_FLANK_RSSIAboveThr    0x1000
#define RFIFG_FLAG_CarrierSense     0x2000  // RFIFG13 Positive Edge
#define RFIFG_FLANK_CarrierSense    0x0000
#define RFIFG_FLAG_CSRSSIAboveThr   0x2000  // RFIFG13 Negative Edge
#define RFIFG_FLANK_CSRSSIAboveThr  0x2000
#define RFIFG_FLAG_WOREvent0        0x4000  // RFIFG14 Positive Edge
#define RFIFG_FLANK_WOREvent0       0x0000
#define RFIFG_FLAG_WOREvent0ACLK    0x4000  // RFIFG14 Negative Edge
#define RFIFG_FLANK_WOREvent0ACLK   0x4000
#define RFIFG_FLAG_WORevent1        0x8000  // RFIFG15 Positive Edge
#define RFIFG_FLANK_WORevent1       0x0000
#define RFIFG_FLAG_OscStable        0x8000  // RFIFG15 Negative Edge
#define RFIFG_FLANK_OscStable       0x8000
#define RFIFG_FLAG_AllPositiveEdge  0xFFFF
#define RFIFG_FLANK_AllPositiveEdge 0x0000
#define RFIFG_FLAG_AllNegativeEdge  0xFFFF
#define RFIFG_FLANK_AllNegativeEdge 0xFFFF

void no_interrupt_isr()
{}

// TODO only end of packet ISR needed?
// Interrupt branch table
InterruptHandler interrupt_table[34] = {
	//Rising Edges
	no_interrupt_isr,        	// 00 No RF core interrupt pending
	rx_timeout_isr,			    // 01 RFIFG0 - Timeout
	no_interrupt_isr,           // 02 RFIFG1 - RSSI_VALID
	no_interrupt_isr,           // 03 RFIFG2
	no_interrupt_isr,//TODOrx_data_isr,				// 04 RFIFG3 - RX FIFO filled or above the RX FIFO threshold
	no_interrupt_isr,           // 05 RFIFG4 - RX FIFO filled or above the RX FIFO threshold or end of packet is reached (Do not use for eop! Will not reset until rxfifo emptied)
	no_interrupt_isr,		    // 06 RFIFG5 - TX FIFO filled or above the TX FIFO threshold
	no_interrupt_isr,			// 07 RFIFG6 - TX FIFO full
	rx_fifo_overflow_isr, 	    // 08 RFIFG7 - RX FIFO overflowed
	rxtx_finish_isr,			// 09 RFIFG8 - TX FIFO underflowed
	no_interrupt_isr,			// 10 RFIFG9 - Sync word sent or received
	no_interrupt_isr,           // 11 RFIFG10 - Packet received with CRC OK
	no_interrupt_isr,           // 12 RFIFG11 - Preamble quality reached (PQI) is above programmed PQT value
	no_interrupt_isr,           // 13 RFIFG12 - Clear channel assessment when RSSI level is below threshold
	no_interrupt_isr,           // 14 RFIFG13 - Carrier sense. RSSI level is above threshold
	no_interrupt_isr,           // 15 RFIFG14 - WOR event 0
	no_interrupt_isr,       	// 16 RFIFG15 - WOR event 1

	//Falling Edges
	no_interrupt_isr,        	// No RF core interrupt pending
	no_interrupt_isr,			// 17 RFIFG0 TODO: timeout not implemented yet
	no_interrupt_isr,           // 18 RFIFG1 - RSSI_VALID
	no_interrupt_isr,           // 19 RFIFG2 -
	no_interrupt_isr,			// 20 RFIFG3 - RX FIFO drained below RX FIFO threshold
	no_interrupt_isr,           // 21 RFIFG4 - RX FIFO empty
	no_interrupt_isr,//TODO tx_data_isr,				// 22 RFIFG5 - TX FIFO below TX FIFO threshold
	no_interrupt_isr,			// 23 RFIFG6 - TX FIFO below TX FIFO threshold
	no_interrupt_isr,           // 24 RFIFG7 - RX FIFO flushed
	no_interrupt_isr,			// 25 RFIFG8 - TX FIFO flushed
	end_of_packet_isr,			// 26 RFIFG9 - End of packet or in RX when optional address check fails or RX FIFO overflows or in TX when TX FIFO underflows
	no_interrupt_isr,           // 27 RFIFG10 - First byte read from RX FIFO
	no_interrupt_isr,           // 28 RFIFG11 - (LPW)
	no_interrupt_isr,           // 29 RFIFG12 - RSSI level is above threshold
	no_interrupt_isr,           // 30 RFIFG13 - RSSI level is below threshold
	no_interrupt_isr,           // 31 RFIFG14 - WOR event 0 + 1 ACLK
	no_interrupt_isr,       	// 32 RFIFG15 - RF oscillator stable or next WOR event0 triggered
};


// CC1101 ISR
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=CC1101_VECTOR
__interrupt void CC1101_VECTOR_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(CC1101_VECTOR))) CC1101_VECTOR_ISR (void)
#else
#error Compiler not supported!
#endif
{
	uint16_t isr_vector = RF1AIV >> 1;
	uint16_t edge = (1 << (isr_vector - 1)) & RF1AIES;
	if(edge) isr_vector += 0x11;

//	#ifdef LOG_PHY_ENABLED
//		uart_transmit_data(0xDD);
//		uart_transmit_data(LOG_TYPE_STRING);
//		uart_transmit_data(13);
//		uart_transmit_message("CC1101_ISR ", 11);
//		uart_transmit_data(48 + (isr_vector / 10));
//		uart_transmit_data(48 + (isr_vector - (10 * (isr_vector / 10))));
//	#endif

	interrupt_table[isr_vector]();
	LPM4_EXIT;  //Todo should system call be made here?
}


// functions used by cc1101_interface.c when accessing the CC1101 of a CC430 SoC.
void _cc1101_interface_init()
{

}

void _c1101_interface_set_interrupts_enabled(bool enable)
{
    if(enable)
    {
        // TODO only end of packet needed?
        RF1AIES = /*RFIFG_FLANK_TXBelowThresh | RFIFG_FLANK_TXUnderflow |*/ RFIFG_FLANK_EndOfPacket;
        RF1AIFG = 0;
        RF1AIE  = /*RFIFG_FLAG_TXBelowThresh | RFIFG_FLAG_TXUnderflow |*/ RFIFG_FLAG_EndOfPacket;
    }
    else
    {
        RF1AIE  = 0;
        RF1AIFG = 0;
    }
}

uint8_t _strobe(uint8_t strobe)
{
    uint16_t int_state;
    uint8_t strobe_tmp = strobe & 0x7F;
    ENTER_CRITICAL_SECTION(int_state);

    // Clear the Status read flag
    RF1AIFCTL1 &= ~(RFSTATIFG);

    // Wait for the Radio to be ready for the next instruction
    RADIO_INST_READY_WAIT();

    // Write the strobe instruction
    RF1AINSTRB = strobe;

    if (strobe_tmp != RF_SRES) {
        if(RF1AIN & 0x04) {
            if ((strobe_tmp != RF_SXOFF) && (strobe_tmp != RF_SWOR) && (strobe_tmp != RF_SPWD)  && (strobe_tmp != RF_SNOP)) {
                while (RF1AIN & 0x04);	// Is chip ready?
                //__delay_cycles(810);	// Delay for ~810usec at 1MHz CPU clock, see erratum RF1A7
                __delay_cycles(12960);	// Delay for ~810usec at 16MHz CPU clock, see erratum RF1A7
            }
        }

        RADIO_STAT_READY_WAIT();
    }

    uint8_t statusByte = RF1ASTATB;
    EXIT_CRITICAL_SECTION(int_state);

    return statusByte;
}

uint8_t _reset_radio_core()
{
    _strobe(RF_SRES);                          // Reset the Radio Core
    return _strobe(RF_SNOP);                          // Get Radio Status
}

uint8_t _read_single_reg(uint8_t addr)
{
    uint8_t value;
    uint16_t int_state;

    ENTER_CRITICAL_SECTION(int_state);

    // Check for valid configuration register address, 0x3E refers to PATABLE
    if ((addr <= 0x2E) || (addr == 0x3E))
        RF1AINSTR1B = RF_SNGLREGRD | addr;
    else
        RF1AINSTR1B = RF_STATREGRD | addr;

    RADIO_DOUT_READY_WAIT();
    value = RF1ADOUTB;

    EXIT_CRITICAL_SECTION(int_state);

    return value;
}

void _write_single_reg(uint8_t addr, uint8_t value)
{
    uint16_t int_state;

    ENTER_CRITICAL_SECTION(int_state);

    RADIO_INST_READY_WAIT();
    RF1AINSTRW = ((RF_REGWR | addr) << 8) + value;

    EXIT_CRITICAL_SECTION(int_state);
}

void _read_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
    uint8_t i;
    uint16_t int_state;

    ENTER_CRITICAL_SECTION(int_state);

    RADIO_INST_READY_WAIT();
    RF1AINSTR1B = RF_REGRD | addr;

    for (i = 0; i < (count-1); i++) {
        RADIO_DOUT_READY_WAIT();
        buffer[i] = RF1ADOUT1B;
    }
    buffer[count-1] = RF1ADOUT0B;

    EXIT_CRITICAL_SECTION(int_state);
}


void _write_burst_reg(uint8_t addr, uint8_t* buffer, uint8_t count)
{
    uint8_t i;
    uint16_t int_state;

    ENTER_CRITICAL_SECTION(int_state);

    RADIO_INST_READY_WAIT();
    RF1AINSTRW = ((RF_REGWR | addr) << 8) + buffer[0];

    for (i = 1; i < count; i++) {
        RADIO_DIN_READY_WAIT();
        RF1ADINB = buffer[i];
    }

    EXIT_CRITICAL_SECTION(int_state);
}

void _write_single_patable(uint8_t value)
{
    uint16_t int_state;

    ENTER_CRITICAL_SECTION(int_state);

    RADIO_INST_READY_WAIT();
    RF1AINSTRW = (RF_SNGLPATABWR << 8) + value;

    RADIO_INST_READY_WAIT();
    RF1AINSTRB = RF_SNOP; // Reset PA_Table pointer

    EXIT_CRITICAL_SECTION(int_state);
}

void _write_burst_patable(uint8_t* buffer, uint8_t count)
{
    uint8_t i;
    uint16_t int_state;

    ENTER_CRITICAL_SECTION(int_state);

    RADIO_INST_READY_WAIT();
    RF1AINSTRW = (RF_PATABWR << 8) + buffer[0];

    for (i = 1; i < count; i++) {
        RADIO_DIN_READY_WAIT();
        RF1ADINB = buffer[i];
    }

    RADIO_INST_READY_WAIT();
    RF1AINSTRB = RF_SNOP; // Reset PA Table pointer

    EXIT_CRITICAL_SECTION(int_state);
}

#endif // CC1101_INTERFACE_CC430_H