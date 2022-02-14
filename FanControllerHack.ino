#include <PWM.h>

class FanController {
  private:
  
    byte ServerPin;
    byte ServerPin2;
    byte FanPin;
    byte SensePin;
    byte FeedBckpin;
    byte SensePinSlave;
    byte FanFequencyController;

    int CtrlFrequency = 33000;
    int FeedBackDefaultFrequency = 400;

    int DeadFanCount = 0;

    int RefreshCommand = 25;
    int ConsecutiveUpdateRequests = 0;

    int FanFeedBackConversion = 16;

    int C_FACTOR = 9;

    int CurrentFeedBackFrequency = 400;
    
    bool IsDead = 0;

    int CurrentCommand = 25;

  public:

    

    FanController(byte ServerPin, byte ServerPin2, byte FanPin, byte SensePin, byte SensePinSlave, byte FeedBckpin, byte FanFequencyController) {
      if (ServerPin != ServerPin2){
        this->ServerPin = ServerPin;
        this->ServerPin2 = ServerPin2;
        pinMode(ServerPin, INPUT);
        pinMode(ServerPin2, INPUT);
      }
      else{
        this->ServerPin = ServerPin;
        this->ServerPin2 = -1;
        pinMode(ServerPin, INPUT);
      }
      
      this->FanFequencyController = FanFequencyController;

      this->FanPin = FanPin;
      this->SensePin = SensePin;
      this->SensePinSlave = SensePinSlave;
      this->FeedBckpin = FeedBckpin;
      
      pinMode(FeedBckpin, INPUT);
      pinMode(SensePin, INPUT_PULLUP);
      pinMode(SensePinSlave, INPUT_PULLUP);

      
   
    }

    void SetupSignals(){
      if (FanFequencyController != -1) {
        SetPinFrequencySafe(FanFequencyController, FeedBackDefaultFrequency);
        pwmWrite(FanFequencyController, 127);
      }   
      SetPinFrequencySafe(FanPin, CtrlFrequency);
      pwmWrite(FanPin, CurrentCommand * C_FACTOR);
    }

    
    void MonitorFansStatus(){
      
      unsigned long duration = pulseIn(SensePin, HIGH, 100000);
      unsigned long duration2 = pulseIn(SensePinSlave, HIGH, 100000);
      
      Serial.print(CurrentCommand);Serial.print("\t");Serial.println(CurrentFeedBackFrequency);
      
      if ( duration == 0 || duration2==0 ){
        if ( IsDead == 0 ){
          DeadFanCount = DeadFanCount+1;
        }
      }
      else if ( duration != 0 && duration2 != 0 ){
        DeadFanCount=0;
        if ( IsDead==1 ){
          pinMode(FeedBckpin, INPUT);
          IsDead=0;
        }
      }

      if ( DeadFanCount >=20 && IsDead == 0 ){
        IsDead = 1;
        pinMode(FeedBckpin, OUTPUT);
        digitalWrite(FeedBckpin, LOW);
      }
      
    }
    
    void UpdateFan() {
      
      unsigned long servercommand = 0;
      if (ServerPin2 == -1){
        servercommand = pulseIn(ServerPin, HIGH, 100000);
      }
      else{
        unsigned long Temp1 = pulseIn(ServerPin, HIGH, 100000);
        unsigned long Temp2 = pulseIn(ServerPin2, HIGH, 100000);
        if (Temp1 >= Temp2){
          servercommand = Temp1;
        }
        else{
          servercommand = Temp2;
        }
      }

      
      if ( servercommand!=0 ){

        RefreshCommand = ((RefreshCommand*2) + servercommand) /3;
        int Diff = CurrentCommand - servercommand;
    
        if ( abs(Diff) >= 3 ){
          ConsecutiveUpdateRequests = ConsecutiveUpdateRequests+1; 
        }
        else if ( ConsecutiveUpdateRequests != 0){
          ConsecutiveUpdateRequests=0;
        }
        
        if (ConsecutiveUpdateRequests >= 10 ){
      
          CurrentCommand = RefreshCommand;
          ConsecutiveUpdateRequests=0;
          UpdateFanSpeed();
          if (FanFequencyController != -1) {
            CurrentFeedBackFrequency = FanFeedBackConversion * CurrentCommand;
            SetPinFrequencySafe(FanFequencyController, CurrentFeedBackFrequency);
            pwmWrite(FanFequencyController, 127);
          }
        }
      }
    }
    
  void UpdateFanSpeed(){
    
    if ( CurrentCommand*C_FACTOR < 30 ){
      pwmWrite(FanPin, 30);
    }
    else if ( CurrentCommand*C_FACTOR > 200){
      pwmWrite(FanPin, 200);
    }
    else{
      pwmWrite(FanPin, CurrentCommand*C_FACTOR);
    }
  }

  void SetConversionFactor(int C_FACTOR){
    if ( C_FACTOR < 5 ){
      C_FACTOR = 5;
    }
    else if ( C_FACTOR > 9 ){
      C_FACTOR = 9;
    }
    this->C_FACTOR = C_FACTOR;
    UpdateFanSpeed();
  }
};
