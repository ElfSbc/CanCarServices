//#include "settings_mazda_cx5.h"
#include "settings_volvo_xc70.h"

//#define DEBUG
#define SERVICE_DISPLAY_STATE
#define SERVICE_SPEED_ALARM
#define SERVICE_CLOSE_DOORS



#include <SPI.h>
#include <mcp_can.h>
#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  // I2C / TWI 


// pin settings
#define SPI_CS_PIN			            	10				// pin for CS CAN
#define PIN_TO_LOCK                 		5

#define BEEPER							9
#define BEEPER_GROUND       6

#define LED_RED                       8
#define LED_GROUND                    7
//#define LED_GREEN                   5
//#define LED_BLUE                    4



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
		bool canProcess(MCP_CAN *CAN);
};

Car :: Car (){
};

bool Car :: canProcess(MCP_CAN *CAN)
		{
      bool is_changed;
      is_changed=false;

			CAN->readMsgBuf(&rxId, &len, rxBuf);


        #ifdef DEBUG          
          if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
            sprintf(msgString, "E: %.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
          else
            sprintf(msgString, "S: %.3lX       DLC: %1d  Data:", rxId, len);
          
          Serial.print(msgString);
          
          for(byte i = 0; i<len; i++){
            sprintf(msgString, " %.2X", rxBuf[i]);
            Serial.print(msgString);
          }
          for(byte i = len; i<8; i++){
          sprintf(msgString, " 00");
          Serial.print(msgString);
          }
          Serial.println();
         #endif

      
			if (rxId == CAN_DOOR_ID){	
        bool fl = !bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_FRONT_LEFT_BIT   );
        bool fr = !bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_FRONT_RIGHT_BIT  );
        bool rl = !bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_REAR_LEFT_BIT    );
        bool rr = !bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_REAR_RIGHT_BIT   );
        if ((fl != doorFrontLeft. getState()) || (fr != doorFrontRight. getState())  || rl !=doorRearLeft. getState() || rr != doorRearRight. getState()){
            is_changed=true;
    				doorFrontLeft. setState	(fl);
    				doorFrontRight.setState	(fr);
    				doorRearLeft.  setState	(rl);
    				doorRearRight. setState	(rr);
        }

				#ifdef DEBUG
				sprintf(msgString, "- %d %d %d %d", 
					bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_FRONT_LEFT_BIT   ), 
					bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_FRONT_RIGHT_BIT  ), 
					bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_REAR_LEFT_BIT    ), 
					bitRead ( rxBuf[CAN_DOOR_BYTE], CAN_DOOR_REAR_RIGHT_BIT   )
				);
				Serial.println(msgString);
//        Serial.println("\n");
				#endif
                
			}
			if (rxId == CAN_SPEED_ID){
        float speedd = (float)( float(rxBuf[CAN_SPEED_BYTE_1] * 256 + rxBuf[CAN_SPEED_BYTE_2]) ) / (float)100;
        
				if (speedd != speed){
            is_changed=true;				
            speed = speedd;
    				#ifdef DEBUG
      				sprintf(msgString, "- speed: %d.%d", (int)speed , (int)(speed*1000-trunc(speed)*1000));
      				Serial.println(msgString);
    				#endif				
         }
			}	
      return is_changed;
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

  /*
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_unifont);
    u8g.drawStr( 0, 22, "Mazda CX-5");
  } while( u8g.nextPage() );
*/
  
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
  
  pinMode(  BEEPER_GROUND, OUTPUT);   
  digitalWrite(  BEEPER_GROUND, LOW);

	pinMode(LED_RED, OUTPUT);   
	digitalWrite(LED_RED, HIGH);

	pinMode(LED_GROUND, OUTPUT);   
	digitalWrite(LED_GROUND, HIGH);

//	pinMode(LED_BLUE, OUTPUT);   
//	digitalWrite(LED_BLUE, HIGH);

//	pinMode(LED_GREEN, OUTPUT);   
//	digitalWrite(LED_GREEN, HIGH);
}

void light_up(int s){
  digitalWrite(s, LOW);
}
void light_down(int s){
  digitalWrite(s, HIGH);
}


void loop()
{
  
	if (car.canProcess(&CAN0)){
      
    	// SERVISE (SPEED ALARM)
    	// If speed greate than 80 Km/h)
    	// Than Beep 
    	//		and light	
    	#ifdef SERVICE_SPEED_ALARM
    		if (car.speed<80){
    		  light_down(LED_RED);
    		  wasTone = false;
    		}
    		
    		if (car.speed>=80 and !wasTone){
    		  light_up(LED_RED);   
    		  tone(BEEPER,2000,200);
    		  delay(200);
    		  noTone(BEEPER);
    		  wasTone=true;
    		}
    	#endif
      
    	// SERVISE (CLOSE DOORS)
      #ifdef SERVICE_CLOSE_DOORS`
      	// If speed is higher than 10 and one of the doors was opened then
      	// than send 1 second to HIGH on PIN_TO_LOCK 
      	// 		and make a beep sound on BEEPER

        //Serial.println("aaaaa");
    		wasOpened = wasOpened || car.doorFrontLeft.getState() || car.doorFrontRight.getState() || car.doorRearLeft.getState() || car.doorRearRight.getState();
    		if (wasOpened & car.speed > 7 & !car.doorFrontLeft.getState() & !car.doorFrontRight.getState() & !car.doorRearLeft.getState() & !car.doorRearRight.getState()){
          Serial.println("dddd");
          //digitalWrite(PIN_TO_LOCK, HIGH);
          tone(BEEPER, 1000,100);
    			delay(100);                   
          noTone(BEEPER);       
          //digitalWrite(PIN_TO_LOCK, LOW);             		   
    
    			wasOpened = false;      
    		}
    	#endif
    
    
      //DISPLAY STATE
      #ifdef SERVICE_DISPLAY_STATE 
    //      car.doorRearLeft.setState(true);
    //      car.speed=88.89;
          u8g.setColorIndex(255);     // white
        
          u8g.firstPage();  
          do {
            u8g.setFont(u8g_font_unifont);
            sprintf(msgString, "Speed: %d.%d",  (int)car.speed , (int)((float)car.speed*10.0-(float)trunc((float)car.speed)*10.0)
            );
            u8g.drawStr( 0, 15, msgString);
            sprintf(msgString, "   %d %d",  
                      car.doorFrontLeft.getState(),
                      car.doorFrontRight.getState()
            );
            u8g.drawStr( 0, 30, msgString);
            sprintf(msgString, "   %d %d",  
                      car.doorRearLeft.getState(),
                      car.doorRearRight.getState()              
            );
            u8g.drawStr( 0, 45, msgString);        
          } while( u8g.nextPage() );
      #endif
	}
   
}
  
