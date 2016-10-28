/**
 * Sample Music Program for FM Sound Shield
 */

/*GR-SAKURA Sketch Template Version: V1.02*/
#include <rxduino.h>

// to use the MML player
#include "YM2203_MMLplayer.h"

// sample music "Jingle Bells"
void music_JingleBells(void);

/**
 * initialize
 */
void setup()
{
	// initialize the MML player
	MMLplayer.begin();
	
	// initialize LEDs (for debug)
	pinMode(PIN_LED0,OUTPUT);
	pinMode(PIN_LED1,OUTPUT);
	pinMode(PIN_LED2,OUTPUT);
	pinMode(PIN_LED3,OUTPUT);
	
	delay(3000);
}

/**
 * main loop
 */
void loop()
{
	// play music
	music_JingleBells();

	delay(3000);
}

/**
 * sample music "Jingle Bells"
 */
void music_JingleBells(void)
{
	YM2203_Timbre tmbEBass, tmbZitar, tmbBell;
	char *note1[6], *note2[6], *note3[6], *note4[6], *note5[6], *note6[6];

	// E.BASS
	tmbEBass.algorithm = 2;
	tmbEBass.feedback  = 5;
	tmbEBass.opMask    = MASK_ALL;
	tmbEBass.setAR(31, 31, 31, 31);
	tmbEBass.setDR( 8, 14, 16, 12);
	tmbEBass.setSR( 0,  6,  3,  5);
	tmbEBass.setRR( 0,  9,  0,  8);
	tmbEBass.setSL( 3,  2,  2,  2);
	tmbEBass.setTL(34, 42, 20,  0);
	tmbEBass.setKS( 0,  0,  0,  0);
	tmbEBass.setML( 0,  8,  0,  1);
	tmbEBass.setDT( 3,  0,  7,  0);
	
	// ZITAR
	tmbZitar.algorithm = 0;
	tmbZitar.feedback  = 6;
	tmbZitar.opMask    = MASK_ALL;
	tmbZitar.setAR(18, 31, 31, 31);
	tmbZitar.setDR( 5,  5,  5, 10);
	tmbZitar.setSR( 3,  4,  3,  2);
	tmbZitar.setRR( 1,  1,  3,  5);
	tmbZitar.setSL( 2,  1,  2,  4);
	tmbZitar.setTL(30, 28, 35,  0);
	tmbZitar.setKS( 1,  1,  1,  0);
	tmbZitar.setML( 3,  2,  1,  1);
	tmbZitar.setDT( 7,  0,  0,  3);
	
	// BELL
	tmbBell.algorithm = 4;
	tmbBell.feedback  = 0;
	tmbBell.opMask    = MASK_ALL;
	tmbBell.setAR(31, 20, 31, 20);
	tmbBell.setDR(24, 23, 23, 23);
	tmbBell.setSR( 9,  8,  9,  8);
	tmbBell.setRR( 5,  5,  5,  5);
	tmbBell.setSL( 1,  1,  1,  1);
	tmbBell.setTL(11,  0, 11,  0);
	tmbBell.setKS( 0,  2,  0,  2);
	tmbBell.setML( 8,  2,  4,  2);
	tmbBell.setDT( 1,  5,  5,  1);
	
	// FM Ch-1: BELL -> ZITAR
	note1[0] = (char*)"L8Q8O5V8DDDDV9DDV10DDV11DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD";
	note1[1] = (char*)"V14Q7O4DBAGD4RDDBAGE4REE>C<BAF+4R>DDDC<AB4RDDBAGD4RDDBADE4REE>C<BA>DDDDEDC<AGR>D4";
	note1[2] = (char*)"O4BBB4BBB4B>D<G.A16B4RR>CCC.C16C<BBBBAAGAR>D4<BBB4BBB4B>D<G.A16B4RR>CCC.C16C<BBB";
	note1[3] = (char*)">DDC<AG4RR";
	note1[4] = (char*)">DDC<AGR>D4";
	note1[5] = (char*)">D4D4C4<A4G1.";

	// FM Ch-2: BELL
	note2[0] = (char*)"L8Q8O4V8>CCCCV9CCV10CCV11<BBBBBBBB>CCCCCCCC<BBBBBBBB>CCCCCCCC";
	note2[1] = (char*)"V13Q4O5RRRRRD16C+16D16C+16DRRRRRE16D+16E16D+16ERRRRRF+16F16F+16F16F+RRRRR"
	                  "D16D+16E16D+16DRRRRRD16C+16D16C+16DRRRRRE16D+16E16D+16ERRRRRF+16F16F+16F16F+RRRRRRD4";
	note2[2] = (char*)"V13Q8O5DDDDDDDDDDDDDDDDCCCCCCCCC+C+C+C+DDDDDDDDDDDDDDDDDDDDCCCCCCCC";
	note2[3] = (char*)"DDDDD4RR";
	note2[4] = (char*)"DDDDDRD4";
	note2[5] = (char*)"DDDDDDDDDDDDDDDDDDDDRDD4";

	// FM Ch-3: E.BASS
	note3[0] = (char*)"L8Q8O5V14O4Q8RRRRRRRRG4D4G4D4A4D4A4D4G4D4G4D4A4D4ADEF+";
	note3[1] = (char*)"O4G4D4G4D4G4AB>C4<G4>C4<G4A4D4A4D4GDEF+G4D4G4D4G4AB>C4<G4>C4<G4A4D4ADEF+G4D4";
	note3[2] = (char*)"O4G4D4G4D4G4D4GGAB>C4C4<G4G4A4A4DDEF+G4D4G4D4G4D4GGAG>C4C4<G4G4";
	note3[3] = (char*)"DDEF+GDEF+";
	note3[4] = (char*)"DDEF+G4D4";
	note3[5] = (char*)"A4D4A4D4G4D4G4D4G4D4GDG4";

	// SSG Ch-A
	note4[0] = (char*)"L8Q4O5V11RRRRRRRRRDRDRDRDRDRDRDRDRDRDRDRDRDRDRDRD";
	note4[1] = (char*)"V11Q4O5RDRDRDRDRDRDRERFRERERDRDRDRDRDRDRDRDRDRDRDRDRERERERERDRDRDRDDV12Q6RD4";
	note4[2] = (char*)"Q4O4RBRBRBRBRBRBRBRB>RCRC<RBRB>RC+RC+<RBRBRBRBRBRBRBRBRBRB>RCRC<RBRB";
	note4[3] = (char*)"RARABRRR";
	note4[4] = (char*)"RARABRA4";
	note4[5] = (char*)"RARARARARBRBRBRBRBRBV13R>DD4";

	// SSG Ch-B
	note5[0] = (char*)"L8Q4O4V10RRRRRRRRRBRBRBRB>RCRCRCRC<RBR8RBRB>RCRCRCRC";
	note5[1] = (char*)"V11Q4O4RBRBRBRBRBRB>RCRCRCRC<RARARARARBRBRBRBRBRBRBRB>RCRCRCRC<RARARARABV12Q6RA4";
	note5[2] = (char*)"Q4O4RGRGRGRGRGRGRGRGRGRGRGRGRARARGRGRGRGRGRGRGRGRGRGRGRGRGRG";
	note5[3] = (char*)"RF+RF+GRRR";
	note5[4] = (char*)"RF+RF+GR>D4";
	note5[5] = (char*)"RF+RF+RF+RF+RGRGRGRGRGRGV13RAB4";

	// SSG Ch-C
	note6[0] = (char*)"L8Q4O4V10RRRRRRRRRARARARARARARARARARARARARARARARA";
	note6[1] = (char*)"V11Q4O4RGRGRGRGRGRGRGRGRGRGRF+RF+RF+RF+RGRGRGRGRGRGRGRGRGRGRGRGRF+RF+RF+RF+GV12Q6RF+&F+";
	note6[2] = (char*)"V10O6RRA+32B32A+32B32A+32B32A+32B32RRA+32B32A+32B32A32B32A+32B32RRA+32B32A+32B32A+32B32A+32B32"
	                  "RRRRRRRRRRRRRRRRRRRRRR"
	                  "A+32B32A+32B32A+32B32A+32B32RRA+32B32A+32B32A+32B32A+32B32RRA+32B32A+32B32A+32B32A+32B32"
	                  "RRRRRRRRRRRR";
	note6[3] = (char*)"RRRRRRRR";
	note6[4] = (char*)"RRRRRRRR";
	note6[5] = (char*)"V13Q6O5D4D4E4F+4G1.Q4RF+G4";
	
	// sequence table:
	// Introduction(0) -> Verse&Bridge(1) -> Chorus1(2,3)
	// -> Verse&Bridge(1) -> Chorus2(2,4) -> Chorus3(2,5)
	const int seq_table[] ={0,1,2,3,1,2,4,2,5};
	
	// set tempo
	MMLplayer.setTempo(104);
	
	// set timbre
	MMLplayer.setTimbre(FM_CH1, &tmbBell);
	MMLplayer.setTimbre(FM_CH2, &tmbBell);
	MMLplayer.setTimbre(FM_CH3, &tmbEBass);
	
	int i,j;
	for(i=0;i<9;i++)
	{
		if(i==1){
			// change timbre
			MMLplayer.setTimbre(FM_CH1, &tmbZitar);
		}
		// set note
		j = seq_table[i];
		MMLplayer.setNote(FM_CH1,   note1[j]);
		MMLplayer.setNote(FM_CH2,   note2[j]);
		MMLplayer.setNote(FM_CH3,   note3[j]);
		MMLplayer.setNote(SSG_CH_A, note4[j]);
		MMLplayer.setNote(SSG_CH_B, note5[j]);
		MMLplayer.setNote(SSG_CH_C, note6[j]);
		
		// start to play, and wait for the end
		MMLplayer.playAndWait();
	}
}
