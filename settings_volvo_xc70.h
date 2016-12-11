// Volvo xc-70 

// we should set x2 of real speed
// if real speed is 125KBPS we should set CAN_250KBPS
#define CAN_SPEED			            	CAN_1000KBPS		// can speed settings
#define CAN_FREQ			            	MCP_16MHZ		// can frequency

#define CAN_DOOR_ID			            	0x3EE			// can packet id for doors
#define CAN_DOOR_BYTE			          	2
#define CAN_DOOR_FRONT_LEFT_BIT		  		0
#define CAN_DOOR_FRONT_RIGHT_BIT	  		1
#define CAN_DOOR_REAR_LEFT_BIT		  		2
#define CAN_DOOR_REAR_RIGHT_BIT		  		3

#define CAN_SPEED_ID                  0x1D1     // can packet id for Speed
#define CAN_SPEED_BYTE_1              6
#define CAN_SPEED_BYTE_2              7