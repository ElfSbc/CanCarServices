#include "settings_mazda_cx5.h"
//#include "settings_volvo_xc70.h"

//#define DEBUG 

#include <SPI.h>
#include <mcp_can.h>

// pin settings
#define SPI_CS_PIN			            	10				// pin for CS CAN
#define PIN_TO_LOCK                 		8

#define BEEPER							3

#define LED_RED                       7
#define LED_GROUND                    6
#define LED_GREEN                      5
#define LED_BLUE                    4



class Door{
	private:
		bool state = false; // closed
	public:
		bool getState(){
			return state;
		      };
		void setState(bool newState){
			state = newState;
		      };
};

class Car{
	private:		
              char msgString[150];
                long unsigned int rxId;
                unsigned char len = 0;
                unsigned char rxBuf[8];
                MCP_CAN* CAN;
	public:
		float speed;
		int rpm;
		Door doorFrontLeft 	;
		Door doorFrontRight ;
		Door doorRearLeft 	;
		Door doorRearRight 	;
		
		Car ();		
		void canProcess(MCP_CAN *CAN);
};

Car :: Car (){
};

void Car :: canProcess(MCP_CAN *CAN)
		{

			CAN->readMsgBuf(&rxId, &len, rxBuf);
			if (rxId == CAN_DOOR_ID){	
				doorFrontLeft. setState	( bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_FRONT_LEFT_BIT   ));
				doorFrontRight.setState	( bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_FRONT_RIGHT_BIT  ));
				doorRearLeft.  setState	( bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_REAR_LEFT_BIT    ));
				doorRearRight. setState	( bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_REAR_RIGHT_BIT   ));

				#ifdef DEBUG
				sprintf(msgString, "- %d %d %d %d", 
					bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_FRONT_LEFT_BIT   ), 
					bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_FRONT_RIGHT_BIT  ), 
					bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_REAR_LEFT_BIT    ), 
					bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_REAR_RIGHT_BIT   )
				);
				Serial.println(msgString);
				#endif
                
			}
			if (rxId == CAN_SPEED_ID){
				speed = (float)( rxBuf[CAN_SPEED_BYTE_1] * 256 + rxBuf[CAN_SPEED_BYTE_2] ) / 100;
				#ifdef DEBUG
  				sprintf(msgString, "- speed: %d.%d", (int)speed , (int)(speed*100-trunc(speed)*100));
  				Serial.println(msgString);
				#endif				
			}	
		};

/////////////////////////////////////////////////////
Car car;

MCP_CAN CAN0(SPI_CS_PIN);                          // Set CS to pin 10
  
bool wasOpened = true;  // true if one of the doors was opened
bool wasTone = false;

char msgString[150];

/////////////////////////////////////////////////////


void setup()
{
  Serial.begin(115200);
  Serial.println("=======================OK========================\n");
  if(CAN0.begin(MCP_ANY, CAN_SPEED, CAN_FREQ) == CAN_OK) {
		#ifdef DEBUG
		Serial.print("CAN: [OK]\n");  
		#endif
	}
	else{
		#ifdef DEBUG
		Serial.print("CAN: [ERROR]\n");  
		#endif
	}
	CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.
	// TODO: нужно настроить фильтрацию пакетов. 
	// CAN0.init_Filt(0, 0, CAN_DOOR_ID);

	pinMode(PIN_TO_LOCK, OUTPUT);   
	digitalWrite(PIN_TO_LOCK, LOW);
	
	pinMode(BEEPER, OUTPUT);   
	digitalWrite(BEEPER, LOW);

	pinMode(LED_RED, OUTPUT);   
	digitalWrite(LED_RED, HIGH);

	pinMode(LED_GROUND, OUTPUT);   
	digitalWrite(LED_GROUND, HIGH);

	pinMode(LED_BLUE, OUTPUT);   
	digitalWrite(LED_BLUE, HIGH);

	pinMode(LED_GREEN, OUTPUT);   
	digitalWrite(LED_GREEN, HIGH);
}

void light_up(int s){
  digitalWrite(s, LOW);
}
void light_down(int s){
  digitalWrite(s, HIGH);
}


void loop()
{
	car.canProcess(&CAN0);

	// SERVISE (SPEED ALARM)
	// If speed greate than 80 Km/h)
	// Than Beep 
	//		and light	
	if (true){
		if (car.speed<80){
		  light_down(LED_RED);
		  wasTone = false;
		}
		
		if (car.speed>=80 and !wasTone){
		  light_up(LED_RED);   
		  tone(BEEPER,2000,300);
		  delay(300);
		  noTone(BEEPER);
		  wasTone=true;
		}
	}
  
	// SERVISE (CLOSE DOORS)
	// If speed is higher than 10 and one of the doors was opened then
	// than send 1 second to HIGH on PIN_TO_LOCK 
	// 		and make a beep sound on BEEPER
	if (true){
		wasOpened = wasOpened || car.doorFrontLeft.getState() || car.doorFrontRight.getState() || car.doorRearLeft.getState() || car.doorRearRight.getState();
		if (wasOpened & car.speed > 10 & !car.doorFrontLeft.getState() & !car.doorFrontRight.getState() & !car.doorRearLeft.getState() & !car.doorRearRight.getState()){
			tone(BEEPER, 1000,100);
      digitalWrite(PIN_TO_LOCK, HIGH);
			delay(300);              
			noTone(BEEPER);	
      digitalWrite(PIN_TO_LOCK, LOW);             		

			wasOpened = false;      
		}
	}
   
}
  
