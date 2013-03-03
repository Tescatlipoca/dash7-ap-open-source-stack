/*
 * The PHY layer API
 *  Created on: Nov 22, 2012
 *  Authors:
 * 		maarten.weyn@artesis.be
 *  	glenn.ergeerts@artesis.be
 *  	alexanderhoet@gmail.com
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "phy.h"
#include "fec.h"

#ifdef D7_PHY_USE_FEC

const uint8_t fec_lut[16] = {0, 3, 1, 2, 3, 0, 2, 1 , 3 , 0, 2, 1, 0, 3, 1, 2};

uint8_t* iobuffer;

uint8_t packetlength;
uint16_t fecpacketlength;

uint8_t processedbytes;
uint16_t fecprocessedbytes;

uint16_t pn9;
uint16_t fecstate;
VITERBISTATE vstate;

void fec_init_encode(uint8_t* input)
{
	iobuffer = input;

	packetlength = 255;
	fecpacketlength = 512;

	processedbytes = 0;
	fecprocessedbytes = 0;

	pn9 = INITIAL_PN9;
	fecstate = INITIAL_FECSTATE;
}

void fec_init_decode(uint8_t* output)
{
	iobuffer = output;

	packetlength = 255;
	fecpacketlength = 512;

	processedbytes = 0;
	fecprocessedbytes = 0;

	pn9 = INITIAL_PN9;

	vstate.path_size = 0;

	vstate.states1[0].cost = 0;
	vstate.states1[1].cost = 100;
	vstate.states1[2].cost = 100;
	vstate.states1[3].cost = 100;
	vstate.states1[4].cost = 100;
	vstate.states1[5].cost = 100;
	vstate.states1[6].cost = 100;
	vstate.states1[7].cost = 100;

	vstate.old = vstate.states1;
	vstate.new = vstate.states2;
}

void fec_set_length(uint8_t length)
{
	packetlength = length;
	fecpacketlength = ((length & 0xFE) + 2) << 1;
}

bool fec_encode(uint8_t* output)
{
	uint8_t i;
	uint16_t tmppn9;
	uint8_t pn9buffer;
	uint16_t fecbuffer[2];

	if(fecprocessedbytes >= fecpacketlength)
		return false;

	for(i = 0; i < 2; i++)
	{
		//Get byte from the input buffer if available and apply data whitening, otherwise append trellis terminator
		if(processedbytes < packetlength) {
			//Pn9 data whitening
			pn9buffer = *iobuffer++ ^ (uint8_t)pn9;

			//Rotate pn9 code
			tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
			pn9 = tmppn9 | (pn9 >> 4);
			tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
			pn9 = tmppn9 | (pn9 >> 4);

			processedbytes++;
		} else {
			pn9buffer = TRELLIS_TERMINATOR;
		}

		//Convolutional encoding
		fecstate |= pn9buffer;

		fecbuffer[i] = fec_lut[fecstate >> 7] << 14;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7] << 12;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7] << 10;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7] << 8;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7] << 6;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7] << 4;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7] << 2;
		fecstate = (fecstate << 1) & 0x07FF;
		fecbuffer[i] |= fec_lut[fecstate >> 7];
		fecstate = (fecstate << 1) & 0x07FF;
	}

	//Interleaving and write to output buffer
	output[0] = ((fecbuffer[0] >> 8) & 0x03);
	output[0] |= (fecbuffer[0] & 0x03) << 2;
	output[0] |= ((fecbuffer[1] >> 8) & 0x03) << 4;
	output[0] |= (fecbuffer[1] & 0x03) << 6;
	output[1] = (fecbuffer[0] >> 10) & 0x03;
	output[1] |= ((fecbuffer[0] >> 2) & 0x03) << 2;
	output[1] |= ((fecbuffer[1] >> 10) & 0x03) << 4;
	output[1] |= ((fecbuffer[1] >> 2) & 0x03) << 6;
	output[2] = ((fecbuffer[0] >> 12) & 0x03);
	output[2] |= ((fecbuffer[0] >> 4) & 0x03) << 2;
	output[2] |= ((fecbuffer[1] >> 12) & 0x03) << 4;
	output[2] |= ((fecbuffer[1] >> 4) & 0x03) << 6;
	output[3] = (fecbuffer[0] >> 14) & 0x03;
	output[3] |= ((fecbuffer[0] >> 6) & 0x03) << 2;
	output[3] |= ((fecbuffer[1] >> 14) & 0x03) << 4;
	output[3] |= ((fecbuffer[1] >> 6) & 0x03) << 6;

	fecprocessedbytes += 4;

	return true;
}

bool fec_decode(uint8_t* input)
{
	int8_t i, j, k;
	uint8_t min_state;
	uint8_t symbol, inputbit;
	uint8_t cost0, cost1;
	uint8_t state0, state1;
	uint16_t tmppn9;
	uint16_t fecbuffer[2];

	if(fecprocessedbytes >= fecpacketlength)
		return false;

	//Deinterleaving
	fecbuffer[0] = (input[0] >> 2) & 0x03;
	fecbuffer[0] |= ((input[1] >> 2) & 0x03) << 2;
	fecbuffer[0] |= ((input[2] >> 2) & 0x03) << 4;
	fecbuffer[0] |= ((input[3] >> 2) & 0x03) << 6;
	fecbuffer[0] |= (input[0] & 0x03) << 8;
	fecbuffer[0] |= (input[1] & 0x03) << 10;
	fecbuffer[0] |= (input[2] & 0x03) << 12;
	fecbuffer[0] |= (input[3] & 0x03) << 14;
	fecbuffer[1] = (input[0] >> 6) & 0x03;
	fecbuffer[1] |= ((input[1] >> 6) & 0x03) << 2;
	fecbuffer[1] |= ((input[2] >> 6) & 0x03) << 4;
	fecbuffer[1] |= ((input[3] >> 6) & 0x03) << 6;
	fecbuffer[1] |= ((input[0] >> 4) & 0x03) << 8;
	fecbuffer[1] |= ((input[1] >> 4) & 0x03) << 10;
	fecbuffer[1] |= ((input[2] >> 4) & 0x03) << 12;
	fecbuffer[1] |= ((input[3] >> 4) & 0x03) << 14;

	for (i = 0; i < 2; i++) {

		//Viterbi decoding
		for (j = 14; j >= 0; j-=2) {
			symbol = (fecbuffer[i] >> j) & 0x03;

			for(k = 0; k < 8; k++) {
				inputbit = k & 0x01;

				state0 = (k & 0x06) >> 1;
				cost0 = vstate.old[state0].cost;
				cost0 += ((fec_lut[(state0 << 1) | inputbit] ^ symbol) + 1) >> 1;

				state1 = state0 + 4;
				cost1 = vstate.old[state1].cost;
				cost1 += ((fec_lut[(state1 << 1) | inputbit] ^ symbol) + 1) >> 1;

				if(cost0 <= cost1) {
					vstate.new[k].cost = cost0;
					vstate.new[k].path = vstate.old[state0].path << 1;
					vstate.new[k].path |= inputbit;
				} else {
					vstate.new[k].cost = cost1;
					vstate.new[k].path = vstate.old[state1].path << 1;
					vstate.new[k].path |= inputbit;
				}
			}
		}

		vstate.path_size++;
		fecprocessedbytes +=2;

		//Flush out byte if path is full
		if (vstate.path_size == 4) {
			//Calculate path with lowest cost
			min_state = 0;
			for (j = 1; j < 8; j++) {
				if(vstate.new[j].cost < vstate.new[min_state].cost)
					min_state = j;
			}

	        //Normalize costs
	        for (j = 0; j < 8; j++)
	        	vstate.new[j].cost -= vstate.new[min_state].cost;

			//Pn9 data dewhitening
			*iobuffer++ = (vstate.new[min_state].path >> 24) ^ pn9;

			//Rotate pn9 code
			tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
			pn9 = tmppn9 | (pn9 >> 4);
			tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
			pn9 = tmppn9 | (pn9 >> 4);

			vstate.path_size--;
			processedbytes++;
		}

		//Flush out remaining bytes if all input bytes have been processed
		if (fecprocessedbytes >= fecpacketlength) {
			//Calculate path with lowest cost
			min_state = 0;
			for (j = 1; j < 8; j++) {
				if(vstate.new[j].cost < vstate.new[min_state].cost)
					min_state = j;
			}

			while(processedbytes < packetlength) {
				//Pn9 data dewhitening
				*iobuffer++ = (vstate.new[min_state].path >> ((vstate.path_size - 1) << 3)) ^ pn9;

				//Rotate pn9 code
				tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
				pn9 = tmppn9 | (pn9 >> 4);
				tmppn9 = ((pn9 << 5) ^ pn9) & 0x01E0;
				pn9 = tmppn9 | (pn9 >> 4);

				vstate.path_size--;
				processedbytes++;
			}
		}
	}

	return true;
}

#endif /* D7_PHY_USE_FEC */
