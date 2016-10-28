#ifndef __YM2203_MML_PLAYER_H_
#define __YM2203_MML_PLAYER_H_

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

#include "YM2203.h"

#define TIMBRE_MAX	64		//!< tibmre table size

/**
 * YM2203 class
 */
class YM2203_MMLplayer
{
public:
	YM2203_MMLplayer();			//!< constructor.
	~YM2203_MMLplayer();		//!< destructor.
	
	void begin();			//!< initialize this player.
	void setTempo(int bpm);	//!< set temo.
	void setVolume(int ch, int volume);				//!< set volume to a channel.
	void setEnvelope(int ch, int type, int interval);//!< set envelope to a channel. (SSG)
	void setToneNoise(int ch, int mode);			//!< set tone/noise mode to a chennel. (SSG)
	void setTimbre(int ch, YM2203_Timbre *timbre);	//!< set timbre to a channel. (FM)
	void setGateTime(int ch, int gateTime);			//!< set gate time rate.
	void setNote(int ch, const char* note);				//!< set note to a channel.
	void play(void);		//!< start to play note.
	void playAndWait(void);	//!< start to play note, and wait for end of note.
	void stop(void);		//!< stop playing note, and clear note.
	bool isPlaying(void);	//!< whether playing now or not.
	void onTimer(void);		//!< interval procedure for playing music.
	
private:
	YM2203 m_ym2203;				//!< YM2203 device.
	const char* m_noteTop [ALL_CH_NUM];	//!< pointer to top of notes for each channel.
	const char* m_note    [ALL_CH_NUM];	//!< pointer to playing note for each channel.
	int   m_stepCnt [ALL_CH_NUM];	//!< step time counter for each channel.
	int   m_gateCnt [ALL_CH_NUM];	//!< gate time counter for each channel.
	int   m_octave  [ALL_CH_NUM];	//!< current octave of each channel.
	int   m_length  [ALL_CH_NUM];	//!< default note length of each channel.
	int   m_gateTime[ALL_CH_NUM];	//!< date time rate of each channel.
	bool  m_isEnd   [ALL_CH_NUM];	//!< whether each channel part is over or not.
	bool  m_isTied	[ALL_CH_NUM];	//!< tie or slur flag.
	int   m_tiedKey	[ALL_CH_NUM];	//!< tie or slur key.
	bool m_isPlaying;				//!< whether playing now or not.
	YM2203_Timbre m_timbre[TIMBRE_MAX];		//!< timbre table
	
	void initTMR(void);						//!< initialize TMR0,1 timers.
	void MMLparser(int ch);					//!< MML parser.
	void commandCDEFGABR(int ch, char key);	//!< MML parser sub routine.
	void setPresetTimbre(void);				//!< set preset timbres to the table.
};

#ifdef _YM2203_MML_PLAYER_C_

// for YM2203_MMLplayer.c
// define the global object of YM2203_MMLplayer
YM2203_MMLplayer MMLplayer;

#else

// for user application
// external declaration
extern YM2203_MMLplayer MMLplayer;

#endif

#endif
