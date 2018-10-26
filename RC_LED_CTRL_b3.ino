//	FenStar.co
//	RC_LED_CTRL Ver. b1 (beta 1)
//	2018-10-26 22:56:24 +0 1540594584 

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// defines
	//Define pins
	#define NEOPIX 3	//Pin to WS2812 Data
	#define HLIGHT 9	//Headlight FET
	#define RTURN 6		//Right Turn LED
	#define LTURN 5 	//Left Turn LED
	#define CH2 7		//Throt PWM
	#define CH3 8		//Turn sig PWM
	
	//define lighting vars
	#define PERI 1000 	//period of turn signal in ms
	#define DUTY 2 		//Fraction of turn signal to be on (inverse)
	#define BRIGHTS 127	//Headlight brightness at full
	#define RUNNING 64	//Headlight brightness at half
	#define TURNPWM 127	//Turn light brightness
	
	//Define pwm input states based on us returned from pulseIn
	#define THROT0 1515	//Throtal neutral position
	#define HIMED 1720	//High / medium headlight threshold in us
	#define MEDOF 1445	//Medium / off headlight threshold in us
	#define OFFOF 700	//Off / no-man headlight threshold in us
	#define SST1 230	//Thresholds for triggers to determine tirn signal pwm state
	#define SST2 160	// ^see above^
	#define SST3 110	// ^see above^
	
// end defines

//Set global vars

	//Setup WS2812 strip
	Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, NEOPIX, NEO_GRB + NEO_KHZ800);
	//timeing var
	unsigned long tme; 
	//WS2812 colur values
	uint32_t RRED = strip.Color(48, 0, 0);		//Back red running lights
	uint32_t REVR = strip.Color(146, 114, 99);	//White lights for reverse
	uint32_t BRED = strip.Color(128, 0, 0);		//Braking red lights
	uint32_t TURN = strip.Color(128, 64, 0);	//Yellow turn signal
	uint32_t OFFF = strip.Color(0, 0, 0);		//LED off

	//UC1 int THRSTRT;								//If grabbing throttle at start, uncomment this and all "//UC1" lines and delete the next line
	int THRSTRT = THROT0;						//If using static throttle you can simply replace all instances of "THRSTRT" with "THROT0" and delete this line, or juste leave it as is.
  
	int WAIT=5;
	//end global vars
  
  
void setup() 
{
	//init WS2812
	strip.begin();
	strip.show(); //Is this needed here? I don't know.
	//end WS2812
	
	//Setup Headlight fet
	pinMode(HLIGHT, OUTPUT);
	
	//Setup turn LEDs
	pinMode(RTURN, OUTPUT);
	pinMode(LTURN, OUTPUT);
	digitalWrite(RTURN, LOW);
	digitalWrite(LTURN, LOW);
	//end turn LEDs
	
	//Calibrate Throt based on start condition
	//UC1 THRSTRT = pulseIn(CH2, HIGH, 20000);		//If grabbing throttle at start, uncomment this and all "//UC1" lines and delete the "int THRSTRT = THROT0;" line
	
}

void loop() 
{
	//Get PWM sigs
	int Chan2 = pulseIn(CH2, HIGH, 20000);
	int Chan3 = pulseIn(CH3, HIGH, 20000);
	//Get time
	tme = millis();
	//reset front turn LEDs
	digitalWrite(RTURN, LOW);
	digitalWrite(LTURN, LOW);
	
	//Modify base LEDs (Front brightnes as well as run/brake/reverse)
	FstSta(Chan2);
  
	//Modify turn LEDs after checking timeing
	SecSta(Chan3,tme);
	
	if (Chan3 == 0)
		NoConn();
	
	//Push strip lighting
	strip.show();
	delay(WAIT);

  
}

void FstSta(int Throt)				//Sets weather thr backlights are in running, brake, or reverse mode based on the throttle (CH2)
{
	if (Throt >= THRSTRT+15)
	{
		Reverse();
	}
		
	else if (Throt <= THRSTRT-15)
	{
		Runn();
	}
	else
	{
		Stop();
	}
}

void SecSta(int Lightlevel, int bob)			//Sets brightness of hedlights and turn signals based on CH3 value
{
	int SecondState=0;
	if (Lightlevel >=HIMED)
	{
		analogWrite(HLIGHT, BRIGHTS);
		SecondState = Lightlevel -HIMED;
	}
	else if (Lightlevel >=MEDOF)
	{
		analogWrite(HLIGHT, RUNNING);
		SecondState = Lightlevel -MEDOF;
	}
	else //turn it all off
	{
		analogWrite(HLIGHT, 0);
		AllOff();
		SecondState = Lightlevel -OFFOF;
	}
	
	if (bob%PERI <= PERI/DUTY) //Blink turn lights during on cycle
	{
		if(SecondState>=SST1)//1950
		{
			TurR();
		}
		else if (SecondState>=SST2)//1880
		{
			TurR();
			TurL();
		}
		else if (SecondState>=SST3) //1830
		{
			//Do Nothing
		}
		else //1720
		{
			TurL();
		}
	}
}

void Runn()							//Sets running lights (right an left side of back lights to dim red)
{
    strip.setPixelColor(0, RRED);
    strip.setPixelColor(1, RRED);
    
    strip.setPixelColor(2, OFFF);
    strip.setPixelColor(3, OFFF);

    strip.setPixelColor(4, OFFF);
    strip.setPixelColor(5, OFFF);
    
    strip.setPixelColor(6, RRED);
    strip.setPixelColor(7, RRED);
}

void TurR()							//Sets right turn signal (right 2 back lights to yellow)
{
    strip.setPixelColor(0, TURN);
    strip.setPixelColor(1, TURN);
    analogWrite(RTURN, TURNPWM);
}

void TurL()							//Sets left turn signal (left 2 back lights to yellow)
{
    strip.setPixelColor(6, TURN);
    strip.setPixelColor(7, TURN);
    analogWrite(LTURN, TURNPWM);
}

void Stop()							//Sets brake lights (center 4 back lights to bright red)
{
    strip.setPixelColor(0, RRED);
    strip.setPixelColor(1, OFFF);
    
    strip.setPixelColor(2, BRED);
    strip.setPixelColor(3, BRED);

    strip.setPixelColor(4, BRED);
    strip.setPixelColor(5, BRED);
    
    strip.setPixelColor(6, OFFF);
    strip.setPixelColor(7, RRED);
}

void Reverse() 						//Sets back-up lights (center 4 back lights to white)
{
    strip.setPixelColor(0, RRED);
    strip.setPixelColor(1, OFFF);
    
    strip.setPixelColor(2, REVR);
    strip.setPixelColor(3, REVR);

    strip.setPixelColor(4, REVR);
    strip.setPixelColor(5, REVR);
    
    strip.setPixelColor(6, OFFF);
    strip.setPixelColor(7, RRED);
}

void AllOff()						//Turns off all back lights
{
	strip.setPixelColor(0, OFFF);
    strip.setPixelColor(1, OFFF);
    
    strip.setPixelColor(2, OFFF);
    strip.setPixelColor(3, OFFF);

    strip.setPixelColor(4, OFFF);
    strip.setPixelColor(5, OFFF);
    
    strip.setPixelColor(6, OFFF);
    strip.setPixelColor(7, OFFF);
}

void NoConn()						//Sets pattern to indacate no signal from transmitter
{
	strip.setPixelColor(0, 0,64,0);
    strip.setPixelColor(1, 0,64,64);
    
    strip.setPixelColor(2, 64,0,64);
    strip.setPixelColor(3, OFFF);

    strip.setPixelColor(4, OFFF);
    strip.setPixelColor(5, 64,0,64);
    
    strip.setPixelColor(6, 0,64,64);
    strip.setPixelColor(7, 0,64,0);
}