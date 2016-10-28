#ifndef __YM2203_TIMBRE_H_
#define __YM2203_TIMBRE_H_

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

// for real machine
#else
#include <rxduino.h>
#endif

// Number of FM operators
#define OPERATOR_1		0
#define OPERATOR_2		1
#define OPERATOR_3		2
#define OPERATOR_4		3
#define OPERATOR_NUM	4

// Bit mask for YM2203_Timbre#opMask
#define MASK_OP1		0x01
#define MASK_OP2		0x02
#define MASK_OP3		0x04
#define MASK_OP4		0x08
#define MASK_ALL		0x0F

// Algorism number
#define ALGORITHM_0		0
#define ALGORITHM_1		1
#define ALGORITHM_2		2
#define ALGORITHM_3		3
#define ALGORITHM_4		4
#define ALGORITHM_5		5
#define ALGORITHM_6		6
#define ALGORITHM_7		7

/**
 * YM2203 FM synthesizer timble structure.
 */
struct YM2203_Timbre
{
	uint8_t algorithm;				//!< Algorithm
	uint8_t feedback;				//!< Feedback
	uint8_t opMask;					//!< Operator Mask
	uint8_t ar[OPERATOR_NUM];		//!< Attack Rate
	uint8_t dr[OPERATOR_NUM];		//!< Decay Rate
	uint8_t sr[OPERATOR_NUM];		//!< Sustain Rate
	uint8_t rr[OPERATOR_NUM];		//!< Release Rate
	uint8_t sl[OPERATOR_NUM];		//!< Systain Level
	uint8_t tl[OPERATOR_NUM];		//!< Total Level
	uint8_t keyScale[OPERATOR_NUM];	//!< Key Scale
	uint8_t multiple[OPERATOR_NUM];	//!< Multiple
	int8_t detune[OPERATOR_NUM];	//!< Detune
	
	// Utility functions to set 4 operators' parameters.
	YM2203_Timbre(const int16_t array[5][10]);
	YM2203_Timbre();
	void setAR(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4);
	void setDR(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4);
	void setSR(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4);
	void setRR(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4);
	void setSL(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4);
	void setTL(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4);
	void setKS(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4);
	void setML(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4);
	void setDT( int8_t op1,  int8_t op2,  int8_t op3,  int8_t op4);
};

#endif
