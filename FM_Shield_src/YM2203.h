#ifndef __YM2203_H_
#define __YM2203_H_

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

#include "YM2203_Timbre.h"

// Number of channels
#define FM_CH1			0	//!< FM channel 1
#define FM_CH2			1	//!< FM channel 2
#define FM_CH3			2	//!< FM channel 3
#define SSG_CH_A		3	//!< SSG channel A
#define SSG_CH_B		4	//!< SSG channel B
#define SSG_CH_C		5	//!< SSG channel C

// Total number of channels
#define FM_CH_NUM		3	//!< a YM2203 has 3 FM channels
#define SSG_CH_NUM		3	//!< a YM2203 has 3 SSG channels
#define ALL_CH_NUM		6	//!< 3 FM channels + 3 SSG channels

// Number of Key 
#define KEY_C			0	//!< C
#define KEY_CS			1	//!< C#
#define KEY_D			2	//!< D
#define KEY_DS			3	//!< D#
#define KEY_E			4	//!< E
#define KEY_F			5	//!< F
#define KEY_FS			6	//!< F#
#define KEY_G			7	//!< G
#define KEY_GS			8	//!< G#
#define KEY_A			9	//!< A
#define KEY_AS			10	//!< A#
#define KEY_B			11	//!< B
#define KEY_NUM			12	//!< 12 keys in a octave

// Tone/Noise mode of SSG channels
#define TONE_MODE		0	//!< tone output mode (default)
#define NOISE_MODE		1	//!< noise output mode
#define TONE_NOISE_MODE	2	//!< tone & noise output mode

/**
 * YM2203 class
 */
class YM2203
{
public:
	
	YM2203();			//!< constructor.
	~YM2203();			//!< destructor.
	
	// Common APIs
	void begin(void);								//!< initialize the YM2203 device.
	void noteOn   (int ch);							//!< note-on a channel.
	void noteOff  (int ch);							//!< note-off a channel.
	void setPitch (int ch, int octave, int key);	//!< set pitch to a channel
	void setVolume(int ch, int volume);				//!< set volume to a channel.
	
	// SSG APIs
	void setEnvelope(int ch, int type, uint16_t interval);//!< set envelope to a channel.
	void setToneNoise(int ch, int mode);			//!< set tone/noise mode to a chennel.
	
	// FM APIs
	void setTimbre(int ch, YM2203_Timbre *timbre);	//!< set timbre to a channel.
	
	// Low Level APIs
	uint8_t	read(uint8_t addr);						//!< read a register value.
	void write(uint8_t addr,uint8_t data);			//!< write a register value.
	void writeAddress(uint8_t addr);				//!< only write a register address.
	uint8_t readStatus(void);						//!< read status of YM2203.

private:
	YM2203_Timbre *m_timbre[FM_CH_NUM];				//!< pointer to timble data of each FM channel
	uint8_t m_volume[FM_CH_NUM];					//!< volume of each FM channel
	bool m_enveloped[SSG_CH_NUM];					//!< is each SSG channel enveloped?
	uint8_t m_toneNoise[SSG_CH_NUM];				//!< mask of SSG channel mode (tone/noise)
	uint8_t m_ssgKeyOn;								//!< status of SSG channels key-on/off
	uint8_t m_ssgEnvelopeType;						//!< SSG envelope type
	
	static const uint16_t FM_PITCH_TABLE[KEY_NUM];	//!< pitch parameter table for FM channel
	static const uint16_t SSG_PITCH_TABLE[KEY_NUM];	//!< pitch parameter table for SSG channel
	
	void initExternalBus(void);		//!< initialize the external memory bus of RX63N.
	void startMasterClock(void);	//!< start to supply mastar clock to the YM2203 device.
};

#endif
