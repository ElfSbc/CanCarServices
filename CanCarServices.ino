#include <SPI.h>
#include <mcp_can.h>


#define DEBUG 

#define SPI_CS_PIN			            	10				// pin for CS CAN
#define CAN_SPEED			            	CAN_1000KBPS		// can speed settings
#define CAN_FREQ			            	MCP_16MHZ		// can frequency

//MAZDA cx-5
#define CAN_DOOR_ID			            	0x43E			// can packet id for doors
#define CAN_DOOR_BYTE			          	4
#define CAN_DOOR_FRONT_LEFT_BIT		  		5
#define CAN_DOOR_FRONT_RIGHT_BIT	  		4
#define CAN_DOOR_REAR_LEFT_BIT		  		3
#define CAN_DOOR_REAR_RIGHT_BIT		  		2

//MAZDA CX-5
//#define CAN_SPEED_ID			          	0x217			// can packet id for Speed
//#define CAN_SPEED_BYTE_1			       	4
//#define CAN_SPEED_BYTE_2             	5

//VOLVO XC-70
#define CAN_SPEED_ID                  0x1D1     // can packet id for Speed
#define CAN_SPEED_BYTE_1              6
#define CAN_SPEED_BYTE_2              7



#define PIN_TO_LOCK                 		8

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
//				Serial.println(msgString);
				#endif
                
			}
			if (rxId == CAN_SPEED_ID){
				speed = (float)( rxBuf[CAN_SPEED_BYTE_1] * 256 + rxBuf[CAN_SPEED_BYTE_2] ) / 100;
				#ifdef DEBUG
  				sprintf(msgString, "- speed: %d.%d | %d %d", (int)speed , (int)(speed*100-trunc(speed)*100), rxBuf[CAN_SPEED_BYTE_1],rxBuf[CAN_SPEED_BYTE_2]);
  				Serial.println(msgString);
				#endif				
			}	
		};

/////////////////////////////////////////////////////
Car mazda;

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

  pinMode(LED_RED, OUTPUT);   
  digitalWrite(LED_RED, HIGH);

  pinMode(LED_GROUND, OUTPUT);   
  digitalWrite(LED_GROUND, HIGH);

  pinMode(LED_BLUE, OUTPUT);   
  digitalWrite(LED_BLUE, HIGH);

  pinMode(LED_GREEN, OUTPUT);   
  digitalWrite(LED_GREEN, HIGH);
}

void up(int l){
  digitalWrite(l, LOW);
}
void down(int l){
  digitalWrite(l, HIGH);
}


void loop()
{
	  mazda.canProcess(&CAN0);

	  wasOpened = wasOpened || mazda.doorFrontLeft.getState() || mazda.doorFrontRight.getState() || mazda.doorRearLeft.getState() || mazda.doorRearRight.getState();

    

    if (mazda.speed<10){
      down(LED_GREEN);
      down(LED_BLUE);
      down(LED_RED);
      wasTone = false;
    }
    if (mazda.speed>=10 and mazda.speed<20){
      up(LED_GREEN);
      down(LED_BLUE);
      down(LED_RED);
      wasTone = false;      
    }
    if (mazda.speed>=20 and mazda.speed<30){
      down(LED_GREEN);
      up(LED_BLUE);
      down(LED_RED);
      wasTone = false;
    }
    if (mazda.speed>=30 and !wasTone){
      down(LED_GREEN);
      down(LED_BLUE);
      up(LED_RED);      
      tone(3,1000,500);
      delay(500);
      noTone(3);
      wasTone=true;
    }


    
    
	  if (wasOpened & mazda.speed > 20 & !mazda.doorFrontLeft.getState() & !mazda.doorFrontRight.getState() & !mazda.doorRearLeft.getState() & !mazda.doorRearRight.getState()){
			#ifdef DEBUG
			sprintf(msgString, "+ %d %d %d %d | %f", mazda.doorFrontLeft.getState(), mazda.doorFrontRight.getState(), mazda.doorRearLeft.getState(), mazda.doorRearRight.getState(), mazda.speed/100);
			Serial.println(msgString);
			#endif
   
			tone(PIN_TO_LOCK, 1000,100);
			delay(100);              
			noTone(PIN_TO_LOCK);			
			//delay(100);              // wait for a second
			//      digitalWrite(PIN_TO_LOCK, HIGH);
			//      delay(1000);
			//      digitalWrite(PIN_TO_LOCK, LOW);      
			//      delay(1000);
			wasOpened = false;      
		}
   
}
  
