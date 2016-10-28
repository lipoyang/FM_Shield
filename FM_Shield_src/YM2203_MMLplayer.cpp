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

#include <string.h>

// just for algorithm debug on PC
#ifdef PC_DEBUG
#include <stdint.h>
#include <stdio.h>
#define delay(i)				;
#define DEBUG_PRINT(fmt, ...)	printf(fmt, __VA_ARGS__)

// for real machine
#else
#include <iodefine_gcc63n.h>
#include <intvect.h>
#define DEBUG_PRINT(fmt, ...)	;
#endif

#define _YM2203_MML_PLAYER_C_
#include "YM2203_MMLplayer.h"

/**
 * MML error trap (for Debug)
 *
 * @param error error type.
 */
static void onError(const char error)
{
	volatile int stop = 1;
	while(stop){
		char c = error;
	}
}

/**
 * constructor.
 */
YM2203_MMLplayer::YM2203_MMLplayer()
{
	int ch;
	
	for(ch=0; ch<ALL_CH_NUM; ch++){
		m_noteTop [ch] = NULL;
		m_note    [ch] = NULL;
		m_octave  [ch] = 4;
		m_length  [ch] = 24;    // 24 is for quarter note
		m_gateTime[ch] = 7;
	}
	m_isPlaying = false;
}

/**
 * destructor.
 */
YM2203_MMLplayer::~YM2203_MMLplayer()
{
	// nothing to do
}

/**
 * initialize this player.
 */
void YM2203_MMLplayer::begin()
{
	// initialize YM2203
	m_ym2203.begin();

	// initialize timers
	this->initTMR();
	
	// initialize timbre table
	this->setPresetTimbre();
}

/**
 * initialize TMR0,1 timers.
 */
void YM2203_MMLplayer::initTMR(void)
{
#ifndef PC_DEBUG
	// TMR0(8bit) + TMR1(8bit) cascaded 16bit timer mode
	// TMR0: upper 8 bits / TMR1: lower 8 bits
	// TMR1 is clocked at PCLKB with selected prescaler
	// TMR0 is clocked at overflow signal from TMR1
	// enables compare match A interrupt
	
	// GR-SAKURA has external 12MHz clock, 
	// PLLCR.STC=16   (x16 multiple),
	// PLLCR.PLIDIV=0 (1/1 prescale),
	// SCKCR.PCKB=2   (1/4 prescale)
	// -> PCLK = 12MHz*16/4 = 48MHz
	
	SYSTEM.PRCR.WORD = 0xA50B;		// enable writing to proteced registers
	MSTP(TMR01) = 0;				// turn on TMR0,1
//	SYSTEM.PRCR.WORD = 0xA500;		// disable writing to protected registers

	TMR0.TCCR.BIT.CSS = 0x03;		// TMR0 clocked by TMR1 overflow
	TMR1.TCCR.BIT.CSS = 0x01;		// TMR1 clocked by PCLKB / prescaler
	TMR1.TCCR.BIT.CKS = 0x04;		// 1/64  prescaler for TMR1

	// default BPM = 80 (80 quarter notes in 1 nimute)
	// interrupt interval = 1/8 * 96th note interval
	//                    = 60sec / (BPM*24*8)
	// compare match = 60 * 1,000,000us / (BPM*24*8) * (48MHz / 64)
	TMR01.TCORA = (uint16_t)((60000000UL * 48) / (80*24*8*64));

	TMR0.TCR.BIT.CCLR = 0x01;		// set counter clear by compare match A
	TMR0.TCR.BIT.CMIEA = 0x01;		// enable compare match A interrupt
	TMR01.TCNT = 0x0000;			// clear the counter

	IPR(TMR0, CMIA0) = 1;			// set interrupt priority level

	IEN(TMR0, CMIA0) = 1;			// enable compare match A interrupt
#endif
}

/**
 * set temo.
 *
 * @param bpm beat per minute (how many quarter notes in 1 nimute)
 */
void YM2203_MMLplayer::setTempo(int bpm)
{
#ifndef PC_DEBUG
	// GR-SAKURA has external 12MHz clock, 
	// PLLCR.STC=16   (x16 multiple),
	// PLLCR.PLIDIV=0 (1/1 prescale),
	// SCKCR.PCKB=2   (1/4 prescale)
	// -> PCLK = 12MHz*16/4 = 48MHz

	int slot_per_minute;
	
	IEN(TMR0, CMIA0) = 0;			// disable compare match A interrupt

	// default BPM = 80 (80 quarter notes in 1 nimute)
	// interrupt interval = 1/8 * 96th note interval
	//                    = 60sec / (BPM*24*8)
	// compare match = 60 * 1,000,000us / (BPM*24*8) * (48MHz / 64)
	TMR01.TCORA = (uint16_t)((60000000UL * 48) / ((uint32_t)bpm*24*8*64));

	TMR01.TCNT = 0x0000;			// clear the counter
	IR (TMR0, CMIA0) = 0;			// clear interrupt
	IEN(TMR0, CMIA0) = 1;			// enable compare match A interrupt
#endif
}

/**
 * set volume to a channel.
 *
 * @param ch channel. 0-2:FM, 3-5:SSG. or FM_CH1,FM_CH2,FM_CH3,SSG_CH_A,SSG_CH_B,SSG_CH_C
 * @param volume 0(min)-15(max).
 */
void YM2203_MMLplayer::setVolume(int ch, int volume)
{
	m_ym2203.setVolume(ch, volume);	
} 

/**
 * set envelope to a channel. (SSG)
 *
 * @param ch channel. 3-5 or SSG_CH_A,SSG_CH_B,SSG_CH_C (SSG channel only)
 * @param type envelope type (8-15)
 * @param interval envelope interval
 */
void YM2203_MMLplayer::setEnvelope(int ch, int type, int interval)
{
	m_ym2203.setEnvelope(ch, type, interval);
}

/**
 * set tone/noise mode to a chennel. (SSG)
 *
 * @param ch channel. 3-5 or SSG_CH_A,SSG_CH_B,SSG_CH_C (SSG channel only)
 * @param mode noise mode (TONE_MODE, NOISE_MODE, TONE_NOISE_MODE)
 */
void YM2203_MMLplayer::setToneNoise(int ch, int mode)
{
	m_ym2203.setToneNoise(ch, mode);	
}
	
/**
 * set timbre to a channel. (FM)
 *
 * @param ch channel. 0-2 or FM_CH1,FM_CH2,FM_CH3 (FM channel only)
 * @param timbre pointer to the timbre structure.
 */
void YM2203_MMLplayer::setTimbre(int ch, YM2203_Timbre *timbre)
{
	m_ym2203.setTimbre(ch, timbre);
}

/**
 * set gate time rate.
 *
 * @param ch channel. 0-2:FM, 3-5:SSG. or FM_CH1,FM_CH2,FM_CH3,SSG_CH_A,SSG_CH_B,SSG_CH_C
 * @param gateTime gate time rate (1-8 for 1/8-8/8)
 */
void YM2203_MMLplayer::setGateTime(int ch, int gateTime)
{
	// parameter check
	if(ch<0 || ch>=ALL_CH_NUM) return;
	if(gateTime<1 || gateTime>8) return;
	
	m_gateTime[ch] = gateTime;
}

/**
 * set note to a channel.
 *
 * @param ch channel. 0-2:FM, 3-5:SSG. or FM_CH1,FM_CH2,FM_CH3,SSG_CH_A,SSG_CH_B,SSG_CH_C
 * @param note pointer to a MML string
 */
void YM2203_MMLplayer::setNote(int ch, const char* note)
{
	int len;
	
//	this->stop();
	
	m_noteTop[ch] = note;
}

/**
 * start to play note.
 */
void YM2203_MMLplayer::play(void)
{
	int ch;
	
	for(ch=0; ch<ALL_CH_NUM; ch++)
	{
		m_note   [ch] = m_noteTop[ch];	// top of note.
		m_stepCnt[ch] = 1;	// ready to play the first note
		m_isEnd[ch] = false;
		m_isTied[ch] = false;
	}
	m_isPlaying = true;
}

/**
 * start to play note, and wait for end of note.
 */
void YM2203_MMLplayer::playAndWait(void)
{
	this->play();
	
	while(m_isPlaying)
	{
		delay(1);
	}
}

/**
 * stop playing note, and clear note.
 */
void YM2203_MMLplayer::stop(void)
{
	int ch;
	
	// note off all channels.
	for(ch=0; ch<ALL_CH_NUM; ch++)
	{
		m_ym2203.noteOff(ch);
	}
	
	m_isPlaying = false;
}

/**
 * interval procedure for playing music.
 */
void YM2203_MMLplayer::onTimer(void)
{
	int ch;
	
	if(m_isPlaying)
	{
		// for each channel
		for(ch=0; ch<ALL_CH_NUM; ch++)
		{
			if(!m_isEnd[ch])
			{
				// gate time elapsed => note off
				if(m_gateCnt[ch]>0){
					m_gateCnt[ch]--;
					if( m_gateCnt[ch] <= 0){
						// if tie or slur, don't note off
						if(!m_isTied[ch]){
							m_ym2203.noteOff(ch);
						}
					}
				}
			
				// step time elapsed => next note
				if(m_stepCnt[ch]>0){
					m_stepCnt[ch]--;
					if( m_stepCnt[ch] <= 0){
						this->MMLparser(ch);
					}
				}
			}
		}
		
		// when all channel notes are terminated, stop playing.
		if( m_isEnd[FM_CH1] && m_isEnd[FM_CH2] && m_isEnd[FM_CH3] && 
		    m_isEnd[SSG_CH_A] && m_isEnd[SSG_CH_B] && m_isEnd[SSG_CH_C] )
		{
			this->stop();
		}
	}
}

/**
 * MML parser. (execute one note.)
 *
 * @param ch channel
 */
void YM2203_MMLplayer::MMLparser(int ch)
{
	char mml;
	char nxt;
	int key;
	int volume;
	int gateTime;
	bool note_on = false;
	int timbre_num;
	
	// until one note(C,D,E,F,G,A,B or R) executed
	while( !note_on )
	{
		mml = *m_note[ch];
		m_note[ch]++;
		
		// a-z => A-Z
		if( mml >= 'a' && mml <= 'z' )
		{
			mml -= 0x20;
		}
		
		// MML command
		switch(mml){
			// O: set octave (1-8)
			case 'O':
				nxt = *m_note[ch];
				if( (nxt >= '1') && (nxt <= '8') ){
					m_note[ch]++;
					m_octave[ch] = (int)(nxt - '0');
					DEBUG_PRINT("Command O (%d, %d)\n",ch,(int)(nxt - '0'));
				}else{
					DEBUG_PRINT("ERROR!:Command O NG (%d, %c)\n",ch, nxt);
					onError('O');
				}
				break;
			// >: up octave
			case '>':
				DEBUG_PRINT("Command > (%d)\n",ch);
				if(m_octave[ch] < 8) m_octave[ch]++;
				break;
			// <: down octave
			case '<':
				DEBUG_PRINT("Command < (%d)\n",ch);
				if(m_octave[ch] > 1) m_octave[ch]--;
				break;
			// L: set default note length (1,2,4,8,16)
			case 'L':
				nxt = *m_note[ch];
				if( nxt == '1' ){
					m_note[ch]++;
					nxt = *m_note[ch];
					if( nxt == '6' ){
						m_note[ch]++;
						m_length[ch] = 6;	// 16th note
						DEBUG_PRINT("Command L16 (%d)\n",ch);
					}else if( nxt == '2' ){
						m_note[ch]++;
						m_length[ch] = 8;	// 12th note
						DEBUG_PRINT("Command L12 (%d)\n",ch);
					}else{
						m_length[ch] = 96;	// whole note
						DEBUG_PRINT("Command L1 (%d)\n",ch);
					}
				}else if( nxt == '2' ){
					m_note[ch]++;
					nxt = *m_note[ch];
					if( nxt == '4' ){
						m_note[ch]++;
						m_length[ch] = 4;	// 24th note
						DEBUG_PRINT("Command L24 (%d)\n",ch);
					}else{
						m_length[ch] = 48;	// half note
						DEBUG_PRINT("Command L2 (%d)\n",ch);
					}
				}else if( nxt == '4' ){
					m_note[ch]++;
					m_length[ch] = 24;		// quarter note
					DEBUG_PRINT("Command L4 (%d)\n",ch);
				}else if( nxt == '8' ){
					m_note[ch]++;
					m_length[ch] = 12;		// 8th note
					DEBUG_PRINT("Command L8 (%d)\n",ch);
				}else if( nxt == '3' ){
					m_note[ch]++;
					nxt = *m_note[ch];
					if( nxt == '2' ){
						m_note[ch]++;
						m_length[ch] = 3;	// 32th note
						DEBUG_PRINT("Command L32 (%d)\n",ch);
					}else{
						m_length[ch] = 32;	// 3rd note
						DEBUG_PRINT("Command L3 (%d)\n",ch);
					}
				}else if( nxt == '6' ){
					m_note[ch]++;
					m_length[ch] = 16;		// 6th note
					DEBUG_PRINT("Command L6 (%d)\n",ch);
				}else{
					DEBUG_PRINT("ERROR!:Command L (%d,%c)\n",ch,nxt);
					onError('L');
				}
				break;
			// @: set timbre
			case '@':
				if( ch >  FM_CH3){
					DEBUG_PRINT("ERROR!:Command @ is unavailable for SSG ch.\n");
					break;
				}
				nxt = *m_note[ch];
				if( (nxt >= '0') && (nxt <='9') ){
					m_note[ch]++;
					timbre_num = (int)(nxt - '0');
					nxt = *m_note[ch];
					if( (nxt >= '0') && (nxt <='9') ){
						m_note[ch]++;
						timbre_num = timbre_num * 10 + (int)(nxt - '0');
					}
					if( timbre_num < 0 || timbre_num >= TIMBRE_MAX ){
						DEBUG_PRINT("ERROR!:Command @ unavailable timbre (%d,%d)\n",ch,timbre_num);
						onError('@');
						break;
					}
					m_ym2203.setTimbre(ch, &m_timbre[timbre_num]);
					DEBUG_PRINT("Command @ (%d,%d)\n",ch,timbre_num);
				}else{
					DEBUG_PRINT("ERROR!:Command @ (%d,%c)\n",ch,nxt);
					onError('@');
				}
				break;
			// V: set volume (0-15)
			case 'V':
				nxt = *m_note[ch];
				if( (nxt >= '0') && (nxt <='9') ){
					m_note[ch]++;
					volume = (int)(nxt - '0');
					nxt = *m_note[ch];
					if( (volume == 1) && (nxt >= '0') && (nxt <='5') ){
						m_note[ch]++;
						volume = 10 + (int)(nxt - '0');
					}
					m_ym2203.setVolume(ch, volume);
					DEBUG_PRINT("Command V (%d,%d)\n",ch,volume);
				}else{
					DEBUG_PRINT("ERROR!:Command V (%d,%c)\n",ch,nxt);
					onError('V');
				}
				break;
			// Q: set gate time (1-8)
			case 'Q':
				nxt = *m_note[ch];
				if( (nxt >= '1') && (nxt <='8') ){
					m_note[ch]++;
					gateTime = (int)(nxt - '0');
					nxt = *m_note[ch];
					setGateTime(ch, gateTime);
					DEBUG_PRINT("Command Q (%d,%d)\n",ch,gateTime);
				}else{
					DEBUG_PRINT("ERROR!:Command Q (%d,%c)\n",ch,nxt);
					onError('Q');
				}
				break;
			// end of note string
			case '\0':
				DEBUG_PRINT("note %d end\n",ch);
				m_isEnd[ch] = true;
				note_on = true; // break this loop
				break;
				
			default:
				// C,D,E,F,G,A,B and R: play a note
				if( (mml >= 'A' && mml <= 'G') || (mml == 'R') ){
					commandCDEFGABR(ch, mml);
					note_on = true; // break this loop
				}else{
					DEBUG_PRINT("ERROR!:Command Unknown (%d,%c)\n",ch,mml);
					onError('U');
				}
				
				// ignore any undefined command.
				;
		}
	}
}

/**
 * MML parser sub routine. (Command C,D,E,F,G,A,B and R)
 *
 * @param ch channel
 * @param key C,D,E,F,G,A,B or R
 */
void YM2203_MMLplayer::commandCDEFGABR(int ch, char key)
{
	// {C,D,E,F,G,A,B} -> {0,2,4,5,7,9,11} (order in octave)
	const int TABLE_ABC_TO_12[7]=
	{
		9, 11, 0, 2, 4, 5, 7
	};
	const char REST = 99;
	
	char nxt;
	int len;
	int octave = m_octave[ch];
	
	// R is for rest
	if(key == 'R')
	{
		key = REST;
	}
	// C,D,E,F,G,A,B
	else
	{
		key = TABLE_ABC_TO_12[key - 'A'];
		
		// Command #,+,- : sharp and flat
		nxt = *m_note[ch];
		if(nxt == '#' || nxt == '+'){
			key++;
			m_note[ch]++;
			if(key==12){
				key = 0;
				octave++;
				if(octave > 8){
					DEBUG_PRINT("ERROR!:Too much high pitch (%d)\n",ch);
					onError('#');
					return;
				}
			}
		}else if(nxt == '-'){
			key--;
			m_note[ch]++;
			if(key < 0){
				key = 11;
				octave--;
				if(octave < 1){
					DEBUG_PRINT("ERROR!:Too much low pitch (%d)\n",ch);
					onError('-');
					return;
				}
			}
		}
	}
	
	// Command 1,2,4,8,16 : length of note
	len = m_length[ch]; // default length
	nxt = *m_note[ch];
	
	if( nxt == '1' ){
		m_note[ch]++;
		nxt = *m_note[ch];
		if( nxt == '6' ){
			m_note[ch]++;
			len = 6;	// 16th note
		}else if( nxt == '2' ){
			m_note[ch]++;
			len = 8;	// 12th note
		}else{
			len = 96;	// whole note
		}
	}else if( nxt == '2' ){
		m_note[ch]++;
		nxt = *m_note[ch];
		if( nxt == '4' ){
			m_note[ch]++;
			len = 4;	// 24th note
		}else{
			len = 48;	// half note
		}
	}else if( nxt == '4' ){
		m_note[ch]++;
		len = 24;		// quarter note
	}else if( nxt == '8' ){
		m_note[ch]++;
		len = 12;		// 8th note
	}else if( nxt == '3' ){
		m_note[ch]++;
		nxt = *m_note[ch];
		if( nxt == '2' ){
			m_note[ch]++;
			len = 3;	// 32th note
		}else{
			len = 32;	// 3rd note
		}
	}else if( nxt == '6' ){
		m_note[ch]++;
		len = 16;		// 6th note
	}
	
	// Command . : dotted note
	nxt = *m_note[ch];
	if( nxt == '.' ){
		m_note[ch]++;
		len = len + (len>>1);
	}
	
	// Command & : tie and slur
	bool tied = m_isTied[ch];
	int tiedKey = m_tiedKey[ch];
	nxt = *m_note[ch];
	if( nxt == '&' ){
		m_note[ch]++;
		m_isTied[ch] = true;
		m_tiedKey[ch] = key;
	}else{
		m_isTied[ch] = false;
	}
	
	// set step time, gate time and pitch. then key on.
	DEBUG_PRINT("Length (%d,%d)\n",ch,len);
	m_stepCnt[ch] = len * 8;
	m_gateCnt[ch] = len * m_gateTime[ch];
	if( key != REST){
		if( tied && (key == tiedKey)){
			// if tie, don't not on again.
		}else{
			m_ym2203.setPitch(ch, octave, key);
			m_ym2203.noteOn(ch);
		}
	}else{
		DEBUG_PRINT("Rest (%d)\n",ch);
	}
}

/**
 * whether playing now or not.
 *
 * @return whether playing now(true) or not(false)
 */
bool YM2203_MMLplayer::isPlaying(void)
{
	return m_isPlaying;
}


/**
 * set preset timbres to the table.
 */
void YM2203_MMLplayer::setPresetTimbre(void)
{
	// TODO timbre data set
	
	// ?
	m_timbre[0].algorithm = 2;
	m_timbre[0].feedback  = 12;
	m_timbre[0].opMask    = MASK_ALL;
	m_timbre[0].setAR(31, 31, 31, 31);
	m_timbre[0].setDR(24, 15, 24, 19);
	m_timbre[0].setSR( 0, 17,  0, 17);
	m_timbre[0].setRR( 8, 12,  8, 12);
	m_timbre[0].setSL(11,  2, 11,  2);
	m_timbre[0].setTL(12, 17, 19,  0);
	m_timbre[0].setKS( 0,  0,  0,  0);
	m_timbre[0].setML( 0,  0,  0,  0);
	m_timbre[0].setDT( 0,  0,  0,  0);
	
	// Piano
	m_timbre[13].algorithm = 4;
	m_timbre[13].feedback  = 5;
	m_timbre[13].opMask    = MASK_ALL;
	m_timbre[13].setAR(31, 20, 31, 31);
	m_timbre[13].setDR( 5, 10,  3, 12);
	m_timbre[13].setSR( 0,  3,  0,  3);
	m_timbre[13].setRR( 0,  7,  0,  7);
	m_timbre[13].setSL( 0,  8,  0, 10);
	m_timbre[13].setTL(23,  0, 25,  2);
	m_timbre[13].setKS( 1,  1,  1,  1);
	m_timbre[13].setML( 1,  1,  1,  1);
	m_timbre[13].setDT( 3,  3,  7,  7);
	
	// Trumpet
	m_timbre[23].algorithm = 2;
	m_timbre[23].feedback  = 7;
	m_timbre[23].opMask    = MASK_ALL;
	m_timbre[23].setAR(13, 15, 21, 18);
	m_timbre[23].setDR( 6,  8,  7,  4);
	m_timbre[23].setSR( 0,  0,  0,  0);
	m_timbre[23].setRR( 8,  8,  8,  8);
	m_timbre[23].setSL( 1,  1,  2,  2);
	m_timbre[23].setTL(25, 32, 42,  0);
	m_timbre[23].setKS( 2,  1,  0,  1);
	m_timbre[23].setML( 2,  6,  2,  2);
	m_timbre[23].setDT( 3,  7,  3,  0);
	
	// Strings1
	m_timbre[24].algorithm = 2;
	m_timbre[24].feedback  = 7;
	m_timbre[24].opMask    = MASK_ALL;
	m_timbre[24].setAR(25, 25, 28, 14);
	m_timbre[24].setDR(10, 11, 13,  4);
	m_timbre[24].setSR( 0,  0,  0,  0);
	m_timbre[24].setRR( 5,  8,  6,  6);
	m_timbre[24].setSL( 1,  5,  2,  0);
	m_timbre[24].setTL(29, 15, 45,  0);
	m_timbre[24].setKS( 1,  1,  1,  1);
	m_timbre[24].setML( 1,  5,  1,  1);
	m_timbre[24].setDT( 1,  1,  0,  0);
	
	// Strings2
	m_timbre[25].algorithm = 2;
	m_timbre[25].feedback  = 0;
	m_timbre[25].opMask    = MASK_ALL;
	m_timbre[25].setAR(21, 20, 16, 14);
	m_timbre[25].setDR( 7, 11,  8,  5);
	m_timbre[25].setSR( 0,  0,  0,  0);
	m_timbre[25].setRR( 7, 12, 12, 12);
	m_timbre[25].setSL( 3,  3,  3,  1);
	m_timbre[25].setTL(37, 15, 45,  0);
	m_timbre[25].setKS( 1,  1,  1,  1);
	m_timbre[25].setML( 1,  5,  1,  1);
	m_timbre[25].setDT( 3,  7,  0,  0);
	
	// E.Piano
	m_timbre[27].algorithm = 4;
	m_timbre[27].feedback  = 6;
	m_timbre[27].opMask    = MASK_ALL;
	m_timbre[27].setAR(22, 16, 20, 17);
	m_timbre[27].setDR( 5,  8,  5,  8);
	m_timbre[27].setSR( 0,  8,  0,  7);
	m_timbre[27].setRR( 3,  7,  3,  7);
	m_timbre[27].setSL( 5,  2,  5,  2);
	m_timbre[27].setTL(30,  0, 34,  0);
	m_timbre[27].setKS( 0,  1,  0,  1);
	m_timbre[27].setML( 2,  2,  4,  2);
	m_timbre[27].setDT( 3,  3,  7,  7);
	
	// E.BASS1
	m_timbre[30].algorithm = 2;
	m_timbre[30].feedback  = 5;
	m_timbre[30].opMask    = MASK_ALL;
	m_timbre[30].setAR(31, 31, 31, 31);
	m_timbre[30].setDR( 8, 14, 16, 12);
	m_timbre[30].setSR( 0,  6,  3,  5);
	m_timbre[30].setRR( 0,  9,  0,  8);
	m_timbre[30].setSL( 3,  2,  2,  2);
	m_timbre[30].setTL(34, 42, 20,  0);
	m_timbre[30].setKS( 0,  0,  0,  0);
	m_timbre[30].setML( 0,  8,  0,  1);
	m_timbre[30].setDT( 3,  0,  7,  0);
	
	// E.BASS2
	m_timbre[31].algorithm = 0;
	m_timbre[31].feedback  = 7;
	m_timbre[31].opMask    = MASK_ALL;
	m_timbre[31].setAR(31, 28, 31, 28);
	m_timbre[31].setDR( 8, 18,  7,  9);
	m_timbre[31].setSR( 0,  5,  7,  6);
	m_timbre[31].setRR( 6,  6,  6,  6);
	m_timbre[31].setSL(10, 13,  8,  1);
	m_timbre[31].setTL(38, 47, 23,  0);
	m_timbre[31].setKS( 1,  1,  2,  2);
	m_timbre[31].setML( 1, 10,  0,  0);
	m_timbre[31].setDT( 3,  7,  2,  0);
	
	// Clarinet
	m_timbre[39].algorithm = 3;
	m_timbre[39].feedback  = 7;
	m_timbre[39].opMask    = MASK_ALL;
	m_timbre[39].setAR(31, 20, 20, 20);
	m_timbre[39].setDR( 7, 10, 10, 15);
	m_timbre[39].setSR( 0,  0,  0,  0);
	m_timbre[39].setRR( 5, 11,  6,  7);
	m_timbre[39].setSL( 0,  5, 10,  0);
	m_timbre[39].setTL(40, 50, 40,  0);
	m_timbre[39].setKS( 1,  1,  1,  1);
	m_timbre[39].setML( 2,  3,  4,  1);
	m_timbre[39].setDT( 0,  0,  0,  0);
	
	// Zitar
	m_timbre[44].algorithm = 0;
	m_timbre[44].feedback  = 6;
	m_timbre[44].opMask    = MASK_ALL;
	m_timbre[44].setAR(18, 31, 31, 31);
	m_timbre[44].setDR( 5,  5,  5, 10);
	m_timbre[44].setSR( 3,  4,  3,  2);
	m_timbre[44].setRR( 1,  1,  3,  5);
	m_timbre[44].setSL( 2,  1,  2,  4);
	m_timbre[44].setTL(30, 28, 35,  0);
	m_timbre[44].setKS( 1,  1,  1,  0);
	m_timbre[44].setML( 3,  2,  1,  1);
	m_timbre[44].setDT( 7,  0,  0,  3);
	
	// Clav
	m_timbre[45].algorithm = 2;
	m_timbre[45].feedback  = 6;
	m_timbre[45].opMask    = MASK_ALL;
	m_timbre[45].setAR(31, 31, 31, 31);
	m_timbre[45].setDR(15,  6,  6,  6);
	m_timbre[45].setSR( 8,  2,  2,  6);
	m_timbre[45].setRR( 6,  6,  6,  7);
	m_timbre[45].setSL( 2,  2,  1,  4);
	m_timbre[45].setTL(35, 32, 32,  0);
	m_timbre[45].setKS( 0,  0,  0,  0);
	m_timbre[45].setML(12,  3,  1,  2);
	m_timbre[45].setDT( 3,  0,  7,  0);
	
	// Harpsic
	m_timbre[46].algorithm = 2;
	m_timbre[46].feedback  = 5;
	m_timbre[46].opMask    = MASK_ALL;
	m_timbre[46].setAR(31, 31, 31, 31);
	m_timbre[46].setDR(13, 11,  2,  6);
	m_timbre[46].setSR( 0,  2,  0,  6);
	m_timbre[46].setRR(15,  0,  0,  7);
	m_timbre[46].setSL(10,  3,  1,  1);
	m_timbre[46].setTL(30, 32, 30,  0);
	m_timbre[46].setKS( 1,  1,  0,  1);
	m_timbre[46].setML( 0,  7,  0,  4);
	m_timbre[46].setDT( 3,  3,  7,  7);
}

#ifndef PC_DEBUG
#include <rxduino.h>

/**
 * TMR0 compare match A interrupt service routine. 
 * This is a global function.
 */
//extern "C" void TMR0_CompareMatchA_ISR(void)

extern "C" void Excep_TMR0_CMIA0(void) __INTTERUPT_FUNC;
extern "C" void Excep_TMR0_CMIA0(void)
{
	int i;
	static int tgl=0;
	
	// enable multiple interrupt
	// setpsw_i();				// for Renesas CCRX
	__builtin_rx_setpsw('I');	// for GCC
	
	i = 100;
    // interval procedure for playing music
    MMLplayer.onTimer();
    
	tgl = 1- tgl;
	digitalWrite(0, tgl);
    // clear interrupt
    IR(TMR0, CMIA0) = 0;
}
#endif