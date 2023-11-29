#include <Arduino.h>
#include <QTRSensors.h>
#include <EEPROM.h>

QTRSensors qtr;
const uint8_t sensorCount = 8;
uint16_t sensorValues[sensorCount];

//Motor setup
uint8_t motor_pins[4] = {5,6,9,10};
//default
uint8_t MOTOR_SPEED = 100;
uint8_t MAX_SPEED = 150;
//PID values
uint16_t maxError = 3500;
double lastError = 0;
const int goal = 3500;

//EEPROM
float kP = 0.00f;
float kD = 0.00f;
float kI = 0.00f;

int I = 0;

//startup flag
bool flag = 0;

//encoder impulses
volatile int count_l = 0;
volatile int count_r = 0;


void calibration(void){

    digitalWrite(LED_BUILTIN,HIGH);
    for (uint16_t i = 0; i < 400; i++)
    {
        qtr.calibrate();
    }
    digitalWrite(LED_BUILTIN, LOW); 

    //Printing raw values (max and min)
    for (uint8_t i = 0; i < sensorCount; i++)
    {
        Serial1.print(qtr.calibrationOn.minimum[i]);
        Serial1.print(' ');
    }
    Serial1.println();

    for (uint8_t i = 0; i < sensorCount; i++)
    {
        Serial1.print(qtr.calibrationOn.maximum[i]);
        Serial1.print(' ');
    }
    Serial1.println();
    Serial1.println();
    delay(1000);

}

void sendEncoders(){
    Serial1.print("r");
    Serial1.print(count_r);
    Serial1.print(" ");
    Serial1.print(count_l);
    Serial1.print("\n");
    count_l = 0;
    count_r = 0;
}


void adjustPIDparams(float KP, float KD = 0, float KI = 0){
    //Write to an eeprom addres
    EEPROM.put(0,KP);
    EEPROM.put(4,KD);
    EEPROM.put(8,KI);
    //write from addres to variables
    EEPROM.get(0,kP);
    EEPROM.get(4,kD);
    EEPROM.get(8,kI);

}


String btCommunication(void){
    String Str = "NoData";
    while(Serial1.available()){
           
        Str = Serial1.readString();    
    
    }
    return Str;  
}


void pdControl(uint16_t *s_values,const int goal,double l_error,int max_speed){

    while(true){
        if(digitalRead(3)==LOW){
            digitalWrite(2,LOW);
            break;
        }
        uint16_t position = qtr.readLineBlack(s_values);
        int currentError = goal - position;
        I = I+currentError;
        int adjustment = kP*currentError+kD*(currentError-l_error)+kI*I;
        lastError = currentError;
        analogWrite(5,constrain(max_speed-adjustment, 0, max_speed));
        analogWrite(9,constrain(max_speed+adjustment, 0, max_speed));
        sendEncoders();
    }
}

void motorStartup(int max_speed){

    for(uint8_t i = 0; i<max_speed; i++){
        analogWrite(5,i);
        analogWrite(9,i);
        delay(10);
    }
}

void getPID(){

    Serial1.print("kP: ");
    Serial1.print(kP,3);
    Serial1.print(" kD: ");
    Serial1.print(kD,3);
    Serial1.print(" kI: ");
    Serial1.print(kI,3);
    Serial1.println();
}

void setSpeed(uint8_t newMaxSpeed, uint8_t newMotorSpeed){
    EEPROM.put(20,newMotorSpeed);
    EEPROM.put(24,newMaxSpeed);
}

void getSpeed(){
    uint8_t max=0, motor=0;
    EEPROM.get(20,motor);
    EEPROM.get(24,max);
    Serial1.print("Motor speed: ");
    Serial1.print(max);
    Serial1.print(" Max speed: ");
    Serial1.print(motor);
    Serial1.println();
}

void setDefault(){
    //default speed
    EEPROM.put(20,150);
    EEPROM.put(24,100);
    //default PID variables
    EEPROM.put(0,5.64);
    EEPROM.put(4,0.07);
    EEPROM.put(8,0.26);
    EEPROM.get(0,kP);
    EEPROM.get(4,kD);
    EEPROM.get(8,kI);
    
}

void counter_r(){

  if(count_r==600)
    count_r=0;
  count_r++;
}


void counter_l(){

  if(count_l==600)
    count_l=0;
  count_l++;
}


void executeCommand(String str){
    String command;
    if(!(str=="NoData")){
        String bufor;
        int flag = 0;
        
        float P = 0.00f;
        float D = 0.00f;
        float I = 0.00f;
        
        for(int i = 0; i<str.length(); i++){

            if(str[i]==' '&&flag==0){     
                flag+=1;
                command = bufor;
                bufor = "";
            }
            else if(str[i]==' '&&flag==1){
                flag+=1;
                P = bufor.toFloat();
                bufor = "";
            }
            else if(str[i]==' '&&flag==2){
                flag+=1;
                D = bufor.toFloat();
                bufor = "";
            }
            else if(str[i]==' '&&flag==3){
                flag+=1;
                I = bufor.toFloat();
                bufor = "";
            }

            if(!(str[i]==' ')){
                bufor+=str[i];
            } 
        }

        if(command == "writePID"){
            adjustPIDparams(P,D,I);   
        }
        else if(command == "calibrate"){
            calibration();
        }
        else if(command == "start"){
            motorStartup(MOTOR_SPEED);
            pdControl(sensorValues,goal,lastError,MAX_SPEED);
        }
        else if(command == "getPID"){
            getPID();
        }
        else if(command == "setSpeed"){
            setSpeed(round(P),round(D));
        }
        else if(command == "getSpeed"){
            getSpeed();
        }
        else if(command == "setDefault"){
            setDefault();
        }
        else if(command == "getEncoder"){
            sendEncoders();
        }
        else{
            Serial1.println("No such command!");
        }
    }
     
}

void setup() {
  
  EEPROM.get(0,kP);
  EEPROM.get(4,kD);
  EEPROM.get(8,kI);

  MOTOR_SPEED = EEPROM.read(20);
  MAX_SPEED= EEPROM.read(24);
  
  //Encoders
  pinMode(11,INPUT);
  pinMode(12,INPUT);
  pinMode(7,INPUT);
  pinMode(8,INPUT);

  attachInterrupt(digitalPinToInterrupt(7),counter_l,CHANGE);
  attachInterrupt(digitalPinToInterrupt(8),counter_l,CHANGE);
  attachInterrupt(digitalPinToInterrupt(11),counter_r,CHANGE);
  attachInterrupt(digitalPinToInterrupt(12),counter_r,CHANGE);
  
  Serial.begin(9600);

  //Bluetooth setup
  Serial1.begin(9600); 
  
  //motor setup 
  for(uint8_t i = 0; i<sizeof(motor_pins); i++){
    pinMode(motor_pins[i],OUTPUT);
  }
  //DRV8833 SLP pin initialization
  pinMode(2,OUTPUT); 
  digitalWrite(2,HIGH);

  //button to stop PID
  pinMode(3,INPUT_PULLUP);

  //rotation direction
  digitalWrite(6,LOW);
  digitalWrite(10,LOW);

  //sensors setup
  QTRReadMode::On;
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){A7, A6, A5, A4, A3, A2, A1, A0}, sensorCount);
  qtr.setEmitterPin(4);
  pinMode(LED_BUILTIN, OUTPUT);

}

void loop() {
  String instruction = btCommunication();
  executeCommand(instruction);
 }



