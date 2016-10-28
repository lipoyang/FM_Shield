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

/**
 * constructor.
 *
 */
YM2203_Timbre::YM2203_Timbre()
{
}

/**
 * constructor. initialize with integer array
 *
 * @param array integer array (N88-BASIC format)
 */
YM2203_Timbre::YM2203_Timbre(const int16_t array[5][10])
{
	algorithm =  array[0][0]       & 0x07;
	feedback  = (array[0][0] >> 3) & 0x07;
	opMask    =  array[0][1] & 0x0F;
	// [0][2-7] : ビブラート、トレモロ関係の設定は未対応
	// [0][8,9] : 未使用
	// [1-4][9] : ビブラート関係の設定は未対応
	setAR((uint8_t)array[1][0], (uint8_t)array[2][0], (uint8_t)array[3][0], (uint8_t)array[4][0]);
	setDR((uint8_t)array[1][1], (uint8_t)array[2][1], (uint8_t)array[3][1], (uint8_t)array[4][1]);
	setSR((uint8_t)array[1][2], (uint8_t)array[2][2], (uint8_t)array[3][2], (uint8_t)array[4][2]);
	setRR((uint8_t)array[1][3], (uint8_t)array[2][3], (uint8_t)array[3][3], (uint8_t)array[4][3]);
	setSL((uint8_t)array[1][4], (uint8_t)array[2][4], (uint8_t)array[3][4], (uint8_t)array[4][4]);
	setTL((uint8_t)array[1][5], (uint8_t)array[2][5], (uint8_t)array[3][5], (uint8_t)array[4][5]);
	setKS((uint8_t)array[1][6], (uint8_t)array[2][6], (uint8_t)array[3][6], (uint8_t)array[4][6]);
	setML((uint8_t)array[1][7], (uint8_t)array[2][7], (uint8_t)array[3][7], (uint8_t)array[4][7]);
	setDT( (int8_t)array[1][8],  (int8_t)array[2][8],  (int8_t)array[3][8],  (int8_t)array[4][8]);
}

/**
 * set Total Level to 4 operators.
 *
 * @param op1 parameter for operator1
 * @param op2 parameter for operator2
 * @param op3 parameter for operator3
 * @param op4 parameter for operator4
 */
void YM2203_Timbre::setTL(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4)
{
	tl[OPERATOR_1] = op1;
	tl[OPERATOR_2] = op2;
	tl[OPERATOR_3] = op3;
	tl[OPERATOR_4] = op4;
}

/**
 * set Attack Rate to 4 operators.
 *
 * @param op1 parameter for operator1
 * @param op2 parameter for operator2
 * @param op3 parameter for operator3
 * @param op4 parameter for operator4
 */
void YM2203_Timbre::setAR(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4)
{
	ar[OPERATOR_1] = op1;
	ar[OPERATOR_2] = op2;
	ar[OPERATOR_3] = op3;
	ar[OPERATOR_4] = op4;
}

/**
 * set Decay Rate to 4 operators.
 *
 * @param op1 parameter for operator1
 * @param op2 parameter for operator2
 * @param op3 parameter for operator3
 * @param op4 parameter for operator4
 */
void YM2203_Timbre::setDR(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4)
{
	dr[OPERATOR_1] = op1;
	dr[OPERATOR_2] = op2;
	dr[OPERATOR_3] = op3;
	dr[OPERATOR_4] = op4;
}

/**
 * set Sustain Rate to 4 operators.
 *
 * @param op1 parameter for operator1
 * @param op2 parameter for operator2
 * @param op3 parameter for operator3
 * @param op4 parameter for operator4
 */
void YM2203_Timbre::setSR(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4)
{
	sr[OPERATOR_1] = op1;
	sr[OPERATOR_2] = op2;
	sr[OPERATOR_3] = op3;
	sr[OPERATOR_4] = op4;
}

/**
 * set Sustain Level to 4 operators.
 *
 * @param op1 parameter for operator1
 * @param op2 parameter for operator2
 * @param op3 parameter for operator3
 * @param op4 parameter for operator4
 */
void YM2203_Timbre::setSL(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4)
{
	sl[OPERATOR_1] = op1;
	sl[OPERATOR_2] = op2;
	sl[OPERATOR_3] = op3;
	sl[OPERATOR_4] = op4;
}

/**
 * set Release Rate to 4 operators.
 *
 * @param op1 parameter for operator1
 * @param op2 parameter for operator2
 * @param op3 parameter for operator3
 * @param op4 parameter for operator4
 */
void YM2203_Timbre::setRR(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4)
{
	rr[OPERATOR_1] = op1;
	rr[OPERATOR_2] = op2;
	rr[OPERATOR_3] = op3;
	rr[OPERATOR_4] = op4;
}

/**
 * set Detune to 4 operators.
 *
 * @param op1 parameter for operator1
 * @param op2 parameter for operator2
 * @param op3 parameter for operator3
 * @param op4 parameter for operator4
 */
void YM2203_Timbre::setDT(int8_t op1, int8_t op2, int8_t op3, int8_t op4)
{
	detune[OPERATOR_1] = op1;
	detune[OPERATOR_2] = op2;
	detune[OPERATOR_3] = op3;
	detune[OPERATOR_4] = op4;
}

/**
 * set Multiple to 4 operators.
 *
 * @param op1 parameter for operator1
 * @param op2 parameter for operator2
 * @param op3 parameter for operator3
 * @param op4 parameter for operator4
 */
void YM2203_Timbre::setML(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4)
{
	multiple[OPERATOR_1] = op1;
	multiple[OPERATOR_2] = op2;
	multiple[OPERATOR_3] = op3;
	multiple[OPERATOR_4] = op4;
}

/**
 * set Key Scale to 4 operators.
 *
 * @param op1 parameter for operator1
 * @param op2 parameter for operator2
 * @param op3 parameter for operator3
 * @param op4 parameter for operator4
 */
void YM2203_Timbre::setKS(uint8_t op1, uint8_t op2, uint8_t op3, uint8_t op4)
{
	keyScale[OPERATOR_1] = op1;
	keyScale[OPERATOR_2] = op2;
	keyScale[OPERATOR_3] = op3;
	keyScale[OPERATOR_4] = op4;
}
