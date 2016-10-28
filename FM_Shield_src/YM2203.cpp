/*
 * FM-Shield for GR-SAKURA
 * Copyright (C) 2013 Bizan Nishimura (@lipoyang)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// just for algorithm debug on PC
#ifdef PC_DEBUG
#include <stdint.h>
#include <stdio.h>
#define DEBUG_PRINT(fmt, ...)	printf(fmt, __VA_ARGS__)
#define delayMicroseconds(i)	;
#define delay(i)				;
#define pinMode(i,j)			;
#define digitalWrite(i,j)		;
volatile unsigned char YM2203_STATUS;
volatile unsigned char YM2203_REG_ADDR;
volatile unsigned char YM2203_REG_DATA;

// for real machine
#else
#include <rxduino.h>
#include <iodefine_gcc63n.h>
#define DEBUG_PRINT(fmt, ...)	;

//! read YM2203 status
#define YM2203_STATUS   (*(volatile unsigned char*)0x05000000) // CS3,A16=0
//! read YM2203 register address
#define YM2203_REG_ADDR (*(volatile unsigned char*)0x05000000) // CS3,A16=0
//! read/write YM2203 register value
#define YM2203_REG_DATA (*(volatile unsigned char*)0x05010000) // CS3,A16=1

#endif

#include "YM2203.h"

//! #RESET pin number (GR-SAKURA IO pin number)
#define RESET_PIN	2	// 2 is for IO2(P22)

// register address (SSG)
#define ADDR_SSG_TONE_FREQ_L	0x00
#define ADDR_SSG_TONE_FREQ_H	0x01
#define ADDR_SSG_NOISE_FREQ		0x06
#define ADDR_SSG_MIXING			0x07
#define ADDR_SSG_LEVEL_ENV		0x08
#define ADDR_SSG_ENV_FREQ_L		0x0B
#define ADDR_SSG_ENV_FREQ_H		0x0C
#define ADDR_SSG_ENV_TYPE		0x0D

// register address (FM)
#define ADDR_FM_KEYON			0x28
#define ADDR_FM_PRESCALER_1		0x2D
#define ADDR_FM_PRESCALER_2		0x2E
#define ADDR_FM_PRESCALER_3		0x2F
#define ADDR_FM_DETUNE_MULTI	0x30
#define ADDR_FM_TL				0x40
#define ADDR_FM_AR_KEYSCALE		0x50
#define ADDR_FM_DR				0x60
#define ADDR_FM_SR				0x70
#define ADDR_FM_SL_RR			0x80
#define ADDR_FM_FREQ_L			0xA0
#define ADDR_FM_FREQ_H			0xA4
#define ADDR_FM_FB_ALGORITHM	0xB0

//! pitch parameter table for FM channel
const uint16_t YM2203::FM_PITCH_TABLE[KEY_NUM]={
	617, 654, 693, 734, 778, 824, 873, 925, 980, 1038, 1100, 1165
};

//! pitch parameter table for SSG channel
const uint16_t YM2203::SSG_PITCH_TABLE[KEY_NUM]={
//	1911, 1804, 1703, 1607, 1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012
	7645, 7215, 6810, 6428, 6067, 5727, 5405, 5102, 4816, 4545, 4290, 4050
};

/**
 * constructor
 */
YM2203::YM2203()
{
	// initial value
	m_timbre[FM_CH1] = NULL;
	m_timbre[FM_CH2] = NULL;
	m_timbre[FM_CH3] = NULL;
	m_volume[FM_CH1] = 0;
	m_volume[FM_CH2] = 0;
	m_volume[FM_CH3] = 0;
	m_enveloped[SSG_CH_A] = false;
	m_enveloped[SSG_CH_B] = false;
	m_enveloped[SSG_CH_C] = false;
	m_toneNoise[SSG_CH_A] = 0x01;
	m_toneNoise[SSG_CH_B] = 0x02;
	m_toneNoise[SSG_CH_C] = 0x04;
}

/**
 * destructor.
 */
YM2203::~YM2203()
{
	// nothing to do
}

/**
 * initialize the YM2203 device.
 */
void YM2203::begin(void)
{
	uint8_t data;
	uint8_t addr;
	
	// initialize the external memory bus of RX63N
	this->initExternalBus();
	
	// start to supply mastar clock to the YM2203 device
	this->startMasterClock();
	
	// reset the YM2203 device
	pinMode(RESET_PIN, OUTPUT);
	digitalWrite(RESET_PIN, LOW);
	delay(1);
	digitalWrite(RESET_PIN, HIGH);

	// key-off all SSG channel tone and noise
	m_ssgKeyOn = 0x3F;
	data = m_ssgKeyOn;
	addr = ADDR_SSG_MIXING;
	write(addr,data);
}

/**
 * initialize the external memory bus of RX63N.
 */
void YM2203::initExternalBus(void)
{
#ifndef PC_DEBUG
	SYSTEM.SYSCR0.WORD = 0x5a03;	// enable external bus
	
	SYSTEM.SCKCR.BIT.BCK = 4;		// BCLK 1/4 prescale
	// set counter clock and counter clear
	// GR-SAKURA has external 12MHz clock, 
	// PLLCR.STC=16   (x16 multiple),
	// PLLCR.PLIDIV=0 (1/1 prescale),
	// SCKCR.BCK=4    (1/16 prescale)
	// -> PCLK = 12MHz*16/16 = 12MHz

	MPC.PWPR.BIT.B0WI = 0;			// disable access protection
	MPC.PWPR.BIT.PFSWE = 1;			// 

	MPC.PFCSE.BIT.CS3E = 1; 		// enable CS3
	MPC.PFCSS0.BIT.CS3S = 2;		// set PC4 function as CS3
	MPC.PFAOE1.BIT.A16E = 1;		// enable A16
	MPC.PFBCR0.BYTE = 0;			// 8bit data bus, PCX as address bus
	
	BSC.CS3CR.WORD = 0x0001 | (2 << 4);  // 8bit data bus.
#endif
}

/**
 * start to supply mastar clock to the YM2203 device.
 */
void YM2203::startMasterClock(void)
{
#ifndef PC_DEBUG
	// set PC7 as MTIOC3A
	MPC.PWPR.BIT.B0WI = 0;		// disable access protection
	MPC.PWPR.BIT.PFSWE = 1; 	//
	MPC.PC7PFS.BIT.PSEL = 0x01; // set PC7 as MTIOC3A
	MPC.PWPR.BIT.PFSWE = 0; 	// enable access protection again
	MPC.PWPR.BIT.B0WI = 1;		// 
	PORTC.PMR.BIT.B7 = 1;		// use PC7 as peripheral

	SYSTEM.PRCR.WORD = 0xA502;	// disable access protection
	MSTP(MTU) = 0;				// eneble MTU (MTU0-MTU5)
	SYSTEM.PRCR.WORD = 0xA500;	// enable access protection
	
	// stop and clear MTU3's TCNT
	MTU.TSTR.BIT.CST3 = 0;
	MTU3.TCNT = 0x0000;

	// set counter clock and counter clear
	// GR-SAKURA has external 12MHz clock, 
	// PLLCR.STC=16   (x16 multiple),
	// PLLCR.PLIDIV=0 (1/1 prescale),
	// SCKCR.PCKB=2   (1/4 prescale)
	// -> PCLK = 12MHz*16/4 = 48MHz
	MTU3.TCR.BIT.TPSC = 0;	// counter clock: PCLK/1 = 48MHz
	MTU3.TCR.BIT.CKEG = 0;	// count on rising edge
	MTU3.TCR.BIT.CCLR = 1;	// clear TCNT on TGRA compare match

	// PWM mode 1
	MTU3.TMDR.BYTE = 0x02;
	
	// set output level
	MTU3.TIORH.BIT.IOA = 1; // initially output 0, output 0 on cycle compare match
	MTU3.TIORH.BIT.IOB = 2; // output 1 on duty compare match

	// set cycle and duty
	MTU3.TGRA = 11; 		// cycle = 1/4MHz = 250ns	(48MHz/12 =4MHz)
	MTU3.TGRB = 5;			// duty  = cycle/2 = 125ns
	
	// start MTU3's TCNT
	MTU.TSTR.BIT.CST3 = 1;
#endif
}

/**
 * note-on a channel.
 *
 * @param ch channel. 0-2:FM, 3-5:SSG. or FM_CH1,FM_CH2,FM_CH3,SSG_CH_A,SSG_CH_B,SSG_CH_C
 */
void YM2203::noteOn(int ch)
{
	uint8_t data;
	uint8_t addr;

	DEBUG_PRINT("noteOn(%d)\n",ch);
	
	// FM channel
	if( (FM_CH1<=ch) && (ch<=FM_CH3) )
	{
		if(m_timbre[ch] == NULL)return;
		data = (m_timbre[ch]->opMask << 4) | ch;
		addr = ADDR_FM_KEYON;
		write(addr,data);
	}
	
	// SSG channel
	else if( (SSG_CH_A<=ch) && (ch<=SSG_CH_C) )
	{
		m_ssgKeyOn &= ~m_toneNoise[ch-SSG_CH_A];
		data = m_ssgKeyOn;
		addr = ADDR_SSG_MIXING;
		write(addr,data);
		
		// if on-shot type envelope, set it again.
		if(m_enveloped[ch-SSG_CH_A]){
			if( (m_ssgEnvelopeType == 9) || (m_ssgEnvelopeType == 15)){
				addr = ADDR_SSG_ENV_TYPE;
				data = m_ssgEnvelopeType;
				write(addr,data);
			}
		}
	}
}

/**
 * note-off a channel.
 *
 * @param ch channel. 0-2:FM, 3-5:SSG. or FM_CH1,FM_CH2,FM_CH3,SSG_CH_A,SSG_CH_B,SSG_CH_C
 */
void YM2203::noteOff(int ch)
{
	uint8_t data;
	uint8_t addr;
	
	DEBUG_PRINT("noteOff(%d)\n",ch);

	// FM channel
	if( (FM_CH1<=ch) && (ch<=FM_CH3) )
	{
		data = 0 | ch;
		addr = ADDR_FM_KEYON;
		write(addr,data);
	}
	
	// SSG channel
	else if( (SSG_CH_A<=ch) && (ch<=SSG_CH_C) )
	{
		m_ssgKeyOn |= m_toneNoise[ch-SSG_CH_A];
		data = m_ssgKeyOn;
		addr = ADDR_SSG_MIXING;
		write(addr,data);
	}
}

/**
 * set pitch to a channel
 *
 * @param ch channel. 0-2:FM, 3-5:SSG. or FM_CH1,FM_CH2,FM_CH3,SSG_CH_A,SSG_CH_B,SSG_CH_C
 * @param octave octave number (0-7). 0 is the lowest, and 7 is the highest.
 * @param key pitch in the octave. 0-11 is for C,C#,D,D#,E,F,F#,G,G#,A,A#,B.
 */
void YM2203::setPitch (int ch, int octave, int key)
{
	uint8_t data;
	uint8_t addr;
	uint16_t ssg_f;

	DEBUG_PRINT("setPitch(%d, %d, %d)\n",ch,octave,key);

	// FM channel
	if( (FM_CH1<=ch) && (ch<=FM_CH3) )
	{
		addr = ADDR_FM_FREQ_H + ch;
		data = (((uint8_t)octave & 0x07) << 3) |
		       ((uint8_t)(FM_PITCH_TABLE[key] >> 8) & 0x07);
		write(addr,data);
		
		addr = ADDR_FM_FREQ_L + ch;
		data = (uint8_t)(FM_PITCH_TABLE[key] & 0x00FF);
		write(addr,data);
	}
	
	// SSG channel
	else if( (SSG_CH_A<=ch) && (ch<=SSG_CH_C) )
	{
		ssg_f = SSG_PITCH_TABLE[key];
		if( octave > 0 ){
			ssg_f >>= (octave-1);
			ssg_f = (ssg_f >> 1) + (ssg_f & 0x0001);
		}
		addr = ADDR_SSG_TONE_FREQ_L + (ch - SSG_CH_A) * 2;
		data = (uint8_t)(ssg_f & 0x00FF);
		write(addr,data);
		
		addr = ADDR_SSG_TONE_FREQ_H + (ch - SSG_CH_A) * 2;
		data = (uint8_t)(ssg_f >> 8) & 0x0F;
		write(addr,data);
	}
}

/**
 * set volume to a channel.
 * @param ch channel. 0-2:FM, 3-5:SSG. or FM_CH1,FM_CH2,FM_CH3,SSG_CH_A,SSG_CH_B,SSG_CH_C
 * @param volume 0(min)-15(max).
 */
void YM2203::setVolume(int ch, int volume)
{
	const uint8_t OP_OFFSET[]={0x00, 0x08, 0x04, 0x0c};
	uint8_t data;
	uint8_t addr;
	uint8_t algorithm;
	uint8_t attenate;

	DEBUG_PRINT("setVolume(%d,%d)\n",ch,volume);
	
	// parameter check
	if(volume<0 || volume>15) return;
	
	// FM channel
	if( (FM_CH1<=ch) && (ch<=FM_CH3) )
	{
		if(m_timbre[ch] == NULL) return;
		
		algorithm = m_timbre[ch]->algorithm;
		attenate = (uint8_t)(15 - volume) * 3;
		
		// Operator4 is carrier @ any altorithm
		data = (m_timbre[ch]->tl[OPERATOR_4] + attenate) & 0x7F;
		addr = ADDR_FM_TL + (uint8_t)ch + OP_OFFSET[OPERATOR_4];
		this->write(addr,data);
		
		// Operator2 is carrier @ algorithm 4,5,6,7
		if( algorithm >= ALGORITHM_4){
			data = (m_timbre[ch]->tl[OPERATOR_2] + attenate) & 0x7F;
			addr = ADDR_FM_TL + (uint8_t)ch + OP_OFFSET[OPERATOR_2];
			this->write(addr,data);
		}
		
		// Operator3 is carrier @ algorithm 5,6,7
		if( algorithm >= ALGORITHM_5){
			data = (m_timbre[ch]->tl[OPERATOR_3] + attenate) & 0x7F;
			addr = ADDR_FM_TL + (uint8_t)ch + OP_OFFSET[OPERATOR_3];
			this->write(addr,data);
		}

		// Operator1 is carrier @ algorithm 7
		if( algorithm == ALGORITHM_7){
			data = (m_timbre[ch]->tl[OPERATOR_1] + attenate) & 0x7F;
			addr = ADDR_FM_TL + (uint8_t)ch + OP_OFFSET[OPERATOR_1];
			this->write(addr,data);
		}
	}
	
	// SSG channel
	else if( (SSG_CH_A<=ch) && (ch<=SSG_CH_C) )
	{
		addr = ADDR_SSG_LEVEL_ENV + (ch - SSG_CH_A);
		data = (uint8_t)volume & 0x0F;
		this->write(addr,data);
		
		// volume setting and envelope setting are exclusive.
		this->m_enveloped[ch - SSG_CH_A] = false;
	}
}

/**
 * set envelope to a channel.
 *
 * @param ch channel. 3-5 or SSG_CH_A,SSG_CH_B,SSG_CH_C (SSG channel only)
 * @param type envelope type (8-15)
 * @param interval envelope interval (0-65535) * 1024/1000 [ms]
 */
void YM2203::setEnvelope(int ch, int type, uint16_t interval)
{
	uint8_t data;
	uint8_t addr;

	DEBUG_PRINT("setEnvelope(%d, %d, %d)\n",ch,type,interval);
	
	// parameter check;
	if( (ch<SSG_CH_A) || (ch>SSG_CH_C) ) return;
	//if( (type<8) || (type>15) ) return;
	
	// enable envelope
	// volume setting and envelope setting are exclusive.
	addr = ADDR_SSG_LEVEL_ENV + (ch - SSG_CH_A);
	data = 0x10;
	this->write(addr,data);
	this->m_enveloped[ch - SSG_CH_A] = true;
	
	// envelope type
	addr = ADDR_SSG_ENV_TYPE;
	data = (uint8_t)type & 0x0F;
	write(addr,data);
	m_ssgEnvelopeType = data;
	
	// envelope frequency
	addr = ADDR_SSG_ENV_FREQ_L;
	data = (uint8_t)(interval & 0xFF);
	write(addr,data);
	
	addr = ADDR_SSG_ENV_FREQ_H;
	data = (uint8_t)((interval >> 8) & 0xFF);
	write(addr,data);
}

/**
 * set tone/noise mode to a chennel.
 *
 * @param ch channel. 3-5 or SSG_CH_A,SSG_CH_B,SSG_CH_C (SSG channel only)
 * @param mode noise mode (TONE_MODE, NOISE_MODE, TONE_NOISE_MODE)
 */
void YM2203::setToneNoise(int ch, int mode)
{
	const uint8_t TONE_MASK [3]={0x01, 0x02, 0x04};
	const uint8_t NOISE_MASK[3]={0x08, 0x10, 0x20};
	
	// parameter check;
	if( (ch<SSG_CH_A) || (ch>SSG_CH_C) ) return;
	ch -= SSG_CH_A;
	
	switch(mode){
	case TONE_MODE:
		m_toneNoise[ch] = TONE_MASK[ch];
		break;
	case NOISE_MODE:
		m_toneNoise[ch] = NOISE_MASK[ch];
		break;
	case TONE_NOISE_MODE:
		m_toneNoise[ch] = TONE_MASK[ch] + NOISE_MASK[ch];
		break;
	}
}

/**
 * set timbre to a channel.
 *
 * @param ch channel. 0-2 or FM_CH1,FM_CH2,FM_CH3 (FM channel only)
 * @param timbre pointer to the timbre structure.
 */
void YM2203::setTimbre(int ch, YM2203_Timbre *timbre)
{
	const uint8_t OP_OFFSET[]={0x00, 0x08, 0x04, 0x0c};
	uint8_t offset;
	uint8_t addr;
	uint8_t data;
	int op;
	uint8_t tl,ar,dr,sr,sl,rr,detune,multiple,keyScale;
	int8_t sDetune;

	DEBUG_PRINT("setTimbre(%d, ****)\n",ch);
	
	// parameter check
	if( ch < 0 || ch >= FM_CH_NUM) return;
	
	// envelop parameters for each operator
	for(op=OPERATOR_1; op<=OPERATOR_4; op++)
	{
		tl = timbre->tl[op];
		ar = timbre->ar[op];
		dr = timbre->dr[op];
		sr = timbre->sr[op];
		sl = timbre->sl[op];
		rr = timbre->rr[op];
		sDetune  = timbre->detune[op];
		detune   = (sDetune >= 0) ? (uint8_t)sDetune : (uint8_t)(4-sDetune);
		multiple = timbre->multiple[op];
		keyScale = timbre->keyScale[op];
		
		offset = (uint8_t)ch + OP_OFFSET[op];
		
		// Multiple and Detune
		addr = ADDR_FM_DETUNE_MULTI + offset;
		data = ((detune & 0x07) << 4) | (multiple & 0x0F);
		this->write(addr,data);
		
		// Total Level
		addr = ADDR_FM_TL           + offset;
		data = tl & 0x7F;
		this->write(addr,data);
		
		// Key Scale and Attack Rate
		addr = ADDR_FM_AR_KEYSCALE  + offset;
		data = ((keyScale & 0x03) << 6) | (ar & 0x1F);
		this->write(addr,data);
		
		// Decay Rate
		addr = ADDR_FM_DR           + offset;
		data = dr & 0x1F;
		this->write(addr,data);
		
		// Sustain Rate
		addr = ADDR_FM_SR           + offset;
		data = sr & 0x1F;
		this->write(addr,data);
		
		// Sustain Level and Release Rate
		addr = ADDR_FM_SL_RR        + offset;
		data = ((sl & 0x0F) << 4) | (rr & 0x0F);
		this->write(addr,data);
	}
	
	// algorithm and feedback
	addr = ADDR_FM_FB_ALGORITHM + ch;
	data = ((timbre->feedback & 0x07) << 3) | (timbre->algorithm & 0x07);
	this->write(addr,data);

	m_timbre[ch] = timbre;
}

/**
 * read a register value.
 *
 * @param addr YM2203 register address
 * @return value read from the register
 */
uint8_t	YM2203::read(uint8_t addr)
{
	uint8_t data;
	
	YM2203_REG_ADDR = addr;
	
	delayMicroseconds(5);	// wait more than 17 clock
	
	data = YM2203_REG_DATA;
	
	return data;
}

/**
 * write a register value.
 *
 * @param addr YM2203 register address
 * @param data value to write to the register
 */
void YM2203::write(uint8_t addr,uint8_t data)
{
	YM2203_REG_ADDR = addr;
	
	delayMicroseconds(5);		// wait more than 17 clock
	
	YM2203_REG_DATA = data;
	
	if( addr >= ADDR_FM_FREQ_L){
		delayMicroseconds(12);	// wait more than 47 clock
	}else if( addr >= ADDR_FM_KEYON ){
		delayMicroseconds(21);	// wait more than 83 clock
	}else{
		delayMicroseconds(5);	// wait more than 17 clock
	}
}

/**
 * only write a register address. (for some special registers)
 *
 * @param addr YM2203 register address
 */
void YM2203::writeAddress(uint8_t addr)
{
	YM2203_REG_ADDR = addr;
	
	delayMicroseconds(5);	// wait more than 17 clock
}

/**
 * read status of YM2203.
 */
uint8_t YM2203::readStatus(void)
{
	uint8_t data;
	
	data = YM2203_STATUS;
	
	return data;
}
