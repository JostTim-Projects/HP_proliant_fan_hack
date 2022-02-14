#include <PWM.h>
#include <FanController.h>

const byte FanFriedCHA = 15;

const byte FanFriedCPU = 14;


const byte sensor1 = 8;
const byte sensor2 = 7;
const byte sensor3 = 4;
const byte sensor4 = 2;

const byte FanCtl1 = 9;
const byte FanCtl2 = 10;

const byte ServerInput1 = 5;
const byte ServerInput2 = 12;
const byte ServerInput3 = 13;

const byte FeedBackSense = 3;

const byte Nothing = -1;


const byte Button = 6;

unsigned long Debounce = 0;
int CONVERSION_FACTOR = 9;
int directionUpdate = -1;

FanController CpuFANS = FanController(ServerInput1, ServerInput2, FanCtl1, sensor1, sensor2, FanFriedCPU, FeedBackSense);

FanController ChassisFANS = FanController(ServerInput3,ServerInput3, FanCtl2, sensor3, sensor4, FanFriedCHA, Nothing);

void setup()
{
  //initialize all timers except for 0, to save time keeping functions

  InitTimersSafe();

  CpuFANS.SetupSignals();
  ChassisFANS.SetupSignals();
  
  pinMode(Button,INPUT);

  Serial.begin(115200);// Initialisation de la communication
}




void loop()
{
  CpuFANS.MonitorFansStatus();
  ChassisFANS.MonitorFansStatus();
  CpuFANS.UpdateFan();
  ChassisFANS.UpdateFan();  
  
  if ( digitalRead(Button) == LOW && millis() > Debounce + 200 ){
    
    CONVERSION_FACTOR = CONVERSION_FACTOR + directionUpdate;
    
    if ( CONVERSION_FACTOR >= 9 || CONVERSION_FACTOR <= 5 ) { 
      directionUpdate = - directionUpdate;
    }
    CpuFANS.SetConversionFactor(CONVERSION_FACTOR);
    ChassisFANS.SetConversionFactor(CONVERSION_FACTOR);
    Debounce=millis();
  }
  
}
