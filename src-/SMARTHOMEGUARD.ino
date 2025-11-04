// Arduino sketch file for MPR project 
// SMART HOME GUARD: Home Security System with Bluetooth and App
#define ABLEPIN   21
#define OUTPIN    18   
#define PIRPIN    19
#define RESETPIN  4
#define ALARMPIN  16
#define DHTPIN    17
#define DHTTYPE   DHT22

unsigned long int pasttime = millis();

// ----- Door States -----
typedef enum {
  STATE_DISABLED = 0,
  STATE_DOOR_OPEN = 1,
  STATE_DOOR_CLOSED = 2,
  STATE_INTRUDER = 3
} DoorState;

// ----- Transition Types -----
typedef enum {
  TRANS_OPENED = 0,
  TRANS_CLOSED = 1,
  TRANS_OPEN_TO_CLOSE = 2,
  TRANS_CLOSE_TO_OPEN = 3,
  TRANS_OPEN_TO_INTRUDER = 4,
  TRANS_DISABLED = 5   
} DoorTransitions;

class DoorManagement {
  public:
    DoorManagement(int Doorenable, int Latchenable, int PIRpin, int RESETpin) {
      this->Doorenable = Doorenable;
      this->Latchenable = Latchenable;
      this->PIRpin = PIRpin;
      this->RESETpin = RESETpin;

      pinMode(this->Doorenable, INPUT);
      pinMode(this->Latchenable, INPUT);
      pinMode(this->PIRpin, INPUT);
      pinMode(this->RESETpin, INPUT);

      MyDoorState = STATE_DISABLED; 
      doorPinState = latchPinState = pirPinState = resetPinState = 0;
    }

    // ---- State Machine ----
    void checkdoorstatus() {
      switch (MyDoorState) {
        case STATE_DISABLED:
          if (doorPinState == 0) {
            MyDoorState = STATE_DISABLED;
            //Serial.println("The System is Disabled");
          } else {
            if (latchPinState == 0) {
              MyDoorState = STATE_DOOR_CLOSED;
              Serial.println("The System is Activated and Your Door is closed");
            } else {
              MyDoorState = STATE_DOOR_OPEN;
              Serial.println("The System is Activated but Your Door is not properly closed");
            }
          }
          break;

        case STATE_DOOR_OPEN:
          if (doorPinState == 0) {
            MyDoorState = STATE_DISABLED;
            Serial.println("The System is Disabled");
          } else if(resetPinState == 0){
            MyDoorState = STATE_DISABLED;
            Serial.println("You have reset the system");
          }
          else {
            if (latchPinState == 0) {
              MyDoorState = STATE_DOOR_CLOSED;
              Serial.println("The Door is closed");
            } else {
              if (pirPinState == 0) {
                MyDoorState = STATE_DOOR_OPEN;
                //Serial.println("Your Door is still open");
              } else {
                MyDoorState = STATE_INTRUDER;
                Serial.println("There is an Intruder!!! Intruder Detected.....");
              }
            }
          }
          break;

        case STATE_DOOR_CLOSED:
          if (doorPinState == 0) {
            MyDoorState = STATE_DISABLED;
            Serial.println("The System is Disabled");
          } else if(resetPinState == 0){
            MyDoorState = STATE_DISABLED;
            Serial.println("You have reset the system");
          }
          else {
            if (latchPinState == 0) {
              MyDoorState = STATE_DOOR_CLOSED;
              //Serial.println("The Door is still closed");
            } else {
              if (pirPinState == 0) {
                MyDoorState = STATE_DOOR_OPEN;
                Serial.println("Your Door is opened");
              } else {
                MyDoorState = STATE_INTRUDER;
                Serial.println("There is an Intruder!!! Intruder Detected.....");
              }
            }
          }
          break;

        case STATE_INTRUDER:
          if(doorPinState == 0){
            MyDoorState = STATE_DISABLED;
          } else if(resetPinState == 0){
            MyDoorState = STATE_DISABLED;
            Serial.println("You have reset the system");
          } else {
            MyDoorState = STATE_INTRUDER;
            Serial.println("There is an INTRUDER. Check the house immediately.....");
          }
          break;
      }
    }

    // ---- ISR Helper Functions ----
    void Doorenableread() {
      doorPinState = digitalRead(Doorenable);
    }

    void Latchenableread() {
      latchPinState = digitalRead(Latchenable);
    }

    void PIRread() {
      pirPinState = digitalRead(PIRpin);
    }
    
    void RESETread(){
      resetPinState = digitalRead(RESETpin);
    }

    void updateprevstate(){
      PrevDoorState = MyDoorState;
    }

  private:
    DoorState MyDoorState;
    DoorState PrevDoorState;
    int Doorenable;
    int Latchenable;
    int PIRpin;
    int RESETpin;
    volatile bool doorPinState;
    volatile bool latchPinState;
    volatile bool pirPinState;
    volatile bool resetPinState;
};

// ----------------
// Global instance
DoorManagement MyDoorManagement(ABLEPIN, OUTPIN, PIRPIN, RESETPIN);

// ---- Global ISR functions ----
void IRAM_ATTR isrDoor() {
  MyDoorManagement.Doorenableread();
}

void IRAM_ATTR isrLatch() {
  MyDoorManagement.Latchenableread();
}

void IRAM_ATTR isrPIR() {
  MyDoorManagement.PIRread();
}

void IRAM_ATTR isrReset() {
  MyDoorManagement.RESETread();
}

// ----------------
void setup() {
  Serial.begin(115200);
  Serial.println("System is initialized");
  MyDoorManagement.Doorenableread();
  MyDoorManagement.PIRread();
  MyDoorManagement.RESETread();
  MyDoorManagement.Latchenableread();
  MyDoorManagement.updateprevstate();
  MyDoorManagement.checkdoorstatus();
  // Attach interrupts (both edges)
  attachInterrupt(digitalPinToInterrupt(ABLEPIN), isrDoor, CHANGE);
  attachInterrupt(digitalPinToInterrupt(OUTPIN), isrLatch, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIRPIN), isrPIR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RESETPIN), isrReset, CHANGE);
}

void loop() {
  MyDoorManagement.updateprevstate();
  buffer(2000);
  MyDoorManagement.checkdoorstatus();
  buffer(3000);
  delay(100);
}

void buffer(unsigned long int time_interval){
  if(millis()-pasttime<=time_interval){

  }
  pasttime = millis();
}
