/*
 * main.c
 */
#include <DSP28x_Project.h>
#include "DSP_KB.h"
#include "Lab9.h"
#include "FPU.h"
#include <math.h>
void fft_init();

RFFT_F32_STRUCT fft;


#define FFT_SIZE 256
#define FFT_STAGES 8  //log base 2 (FFT_size)

//#pragma DATA_SECTION(sampleBuffer, "INBUFA");
static float sampleBuffer[FFT_SIZE];
static float outBuffer[FFT_SIZE];
float TwiddleBuffer[FFT_SIZE];
static float MagBuffer[FFT_SIZE/2];
static float SumSpectrum[FFT_SIZE/2];

static int sampleCount = 0;
static int sampleBufferFull = 0; //variable used to denote whether the buffer has been filled

interrupt void sampleGet_ISR(void);
void initBuffers();


unsigned long ISRvalue = &sampleGet_ISR;


int main(void) {
	
	DisableDog();
	CPUinit();
	EALLOW;
	outputEnable();
	LCDinit();
	LCDclear();
	initADC();
	DAC_init();
//	SRAMwrite(0);
//	SRAMaddress = 0x260000; //shouldn't need SRAM here
	fft_init();
	initBuffers();

	timerINIT(ISRvalue, samplingRate);




	while(1){
		if(sampleBufferFull){
			fft.InBuf = &sampleBuffer[0];
			int i;
			for(i = 0;i<FFT_SIZE;i++){
				outBuffer[i] = 0;
			}
			for(i=0;i<FFT_SIZE/2;i++){
				MagBuffer[i] = 0;
			}
			RFFT_f32(&fft);
			//fft.MagBuf = &sampleBuffer[0];
			RFFT_f32_mag(&fft);

			sampleBufferFull = 0;
			EINT;
		}
		else{
			//do nothing
		}

	}
	return 0;
}

void fft_init(){
	fft.InBuf = &sampleBuffer[0];
	fft.OutBuf = &outBuffer[0];
	fft.CosSinBuf = &TwiddleBuffer[0];
	fft.FFTSize = FFT_SIZE;
	fft.FFTStages = FFT_STAGES;
	fft.MagBuf = &MagBuffer[0];
	RFFT_f32_sincostable(&fft);

}

interrupt void test_ISR(void){
	unsigned int input = ADC_get();
	DAC_set(input);
}

interrupt void sampleGet_ISR(void){
	unsigned int input = ADC_get();
	DAC_set(input);
	sampleBuffer[sampleCount] = (float)input;

	sampleCount++;
	if(sampleCount == FFT_SIZE){
		sampleCount = 0;
		sampleBufferFull = 1;
		DINT; //disables interrupts, if doesn't work can send to dummy interrupt vector
	}

}

void initBuffers(){
	int i,k;
	for(i = 0; i<FFT_SIZE;i++){
		sampleBuffer[i] = 0;
	}

	for(k = 0; i<FFT_SIZE/2;i++){
			SumSpectrum[i] = 0;
	}

}


