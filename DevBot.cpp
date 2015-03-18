#include <DevBot.h>
#include <Servo.h>


#ifndef WIRING_H
  #include <Arduino.h>
#endif
#include "HardwareSerial.h"

DevBot robot;
volatile long lastDebounce=0;
volatile long debounceDelay=1;
volatile long ntime=0;
int lastButtonState=0, buttonState=0;
char command1[10];
char parameter1[10];

char comando[16];
void mudarModoPeloBotao() {

  if(millis()-lastDebounce>debounceDelay && ntime++>=100) {
    robot.ultimoModo=robot.modoAtual;
    robot.modoAtual = robot.modoAtual==robot.contadorModos-1 ? 0 : robot.modoAtual + 1;
    //robot.robot.motores.parar();
    ntime=0;
    robot.modoMudou=1;

    lastDebounce=millis();
  }
}
void mudarModoPeloBotaoSync() {
  int reading = digitalRead(2);

  if (reading != lastButtonState) {
    lastDebounce = millis();
      Serial.println(reading);
      Serial.println(millis() - lastDebounce);
  } 
  
  if ((millis() - lastDebounce) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      Serial.println("mudooooou");  
      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        robot.ultimoModo=robot.modoAtual;
        robot.modoAtual = robot.modoAtual==robot.contadorModos-1 ? 0 : robot.modoAtual + 1;
        //robot.robot.motores.parar();
        ntime=0;
        robot.modoMudou=1;
      }
    }
  }
  lastButtonState = reading;
}
void DevBot::loop() {
  #ifdef ARDUINO_LINUX 
    mudarModoPeloBotaoSync();
  #endif
  if(ultimoModo!=modoAtual) {
    robot.motores.parar();
    ultimoModo = modoAtual;
    beep(modoAtual+1);
    //modoMudou=1;
    modos[modoAtual].configurar();
  }
  modoMudou=0;
  modos[modoAtual].executar();
  //controle de servo manualmente

}






void limpaComando() {
  for(int x=0;x<15;x++) comando[x]='\0';
  for(int x=0;x<10;x++) {
    command1[x]='\0';
    parameter1[x]='\0';
  }

}
 
void split() {
  int cv=0;
  int cp=0;
  for(int x=0;x<15;x++) {
    if(comando[x]!='?' && comando[x]!='\0') {
      command1[cv++]=comando[x];
    } else {
      cp = comando[x]=='?' ? x : 0;
      break;
    }
  }

  if(cp>0) {
    for(int x=cp+1;x<15;x++) {
      parameter1[x-cp-1]=comando[x];
      if(comando[x]=='\0') break;
    }
  } 
}



long DevBot::sensorLuz() {
  return analogRead(portas.sensorLuz);
}
long DevBot::sensorTemperatura() {
  for(int x=0;x<5;x++) {
    analogRead(portas.sensorTemperatura);
    
  }
  return analogRead(portas.sensorTemperatura);
}      
int DevBot::recebeuComando(char*) {

  return 0;
}
void DevBot::mudarServo(int posicao) {
  if(!temServo) return;
  servo.attach(portas.servo);
  delay(50);
  servo.write(posicao);
  delay(250);
}

DevBot::DevBot() {
  modoAtual=0;
  contadorModos=0;
  ultimoModo=0;
  pinMode(2, INPUT);
  pinMode(portas.sensorDistancia1, OUTPUT);

  //para modelo não parallax
  //pinMode(portaSensorDistancia+1, INPUT);
  
}
void DevBot::iniciar() {
   #ifndef ARDUINO_LINUX
     attachInterrupt(0, mudarModoPeloBotao, CHANGE);
   #endif
   /* welcome devbot code */
      
   Serial.begin(115200);
   #ifdef ARDUINO_LINUX
     Serial1.begin(115200);
   #endif
   pinMode(portas.speaker, OUTPUT);
   beep(1);
}


void DevBot::beep(int ntime) {
  for(int x=0;x<ntime;x++) {
    digitalWrite(portas.speaker, HIGH);
    delay(100);
    digitalWrite(portas.speaker, LOW);
    delay(100);
  }

}
void DevBot::modo(int numeroModo, void(*modo1)()) {
  Modo novoModo(modo1);
  modos[numeroModo]=novoModo;
  contadorModos++;
}

void DevBot::checarMudancaModo() {
  /*if(operationMode==0 || operationMode==2) {
    if(Serial1.available()) { 
      byte lido;
      lastMode=mode;
      lido = Serial1.read(); 
      char c=(char) lido;
      mode = atoi(&c);
    }
  }
  if(operationMode==0 || operationMode==3) {
    for(int x=0;x<numberOfCheckChangeModeMethods;x++) {
      checkChangeModeMethods[x]();
    }
  }*/ 
}

void DevBot::esperar(long milis) { 
  modoMudou=0;
  if(modoAtual!=ultimoModo) {
    modoMudou=1;
    return;
  }
  if(milis<10) delay(milis);
  else {
    for(long x=0;x<milis/10;x++) {
      delay(10);
      #ifdef ARDUINO_LINUX 
        mudarModoPeloBotaoSync();
      #endif
      if(modoAtual!=ultimoModo) {
        modoMudou=1;
        return;
      }
    }
    delay(milis%10);
  }

}

byte DevBot::mudouModo() { return modoMudou;}

long DevBot::sensorDistancia1() {
  //COLOCAR IF por Tipo de Sensor e no futuro refatorar
  long cm=0;
  if(sensor==sensor_distancia_parallax) {
    pinMode(portas.sensorDistancia1, OUTPUT);

    digitalWrite(portas.sensorDistancia1, LOW);
    delayMicroseconds(2);
    digitalWrite(portas.sensorDistancia1, HIGH);
    delayMicroseconds(5);
    digitalWrite(portas.sensorDistancia1, LOW);
    pinMode(portas.sensorDistancia1, INPUT);
    long duration = pulseIn(portas.sensorDistancia1, HIGH);

    cm = (duration / 29) / 2;
  }
  return cm;
}



void DevBot::configurar(Bateria b) { 

}
void DevBot::configurar(Motor b) { 

}
void DevBot::configurar(Placa b) {
  placa = b;
  if(b==programME_v2) {
    portas.motor1A=7;
    portas.motor1B=8;
    portas.motor1PWM=(temServo? 3 : 10);
    portas.motor2A=12;
    portas.motor2B=13;
    portas.motor2PWM=11;
    portas.servo=6;
    portas.sensorLuz=3;
    portas.sensorTemperatura=2;
    portas.sensorDistancia1=(temServo ?     10 : 3);
    portas.speaker=4;  
    robot.motores.configurar(1, portas.motor1A,  
                       portas.motor1B,
	               portas.motor1PWM,
                       portas.motor2A,  
                       portas.motor2B,
	               portas.motor2PWM);
  }
}
void DevBot::configurar(Portas p) {
  portas = p;
  robot.motores.configurar(1, portas.motor1A,  
                     portas.motor1B,
                     portas.motor1PWM,
                     portas.motor2A,  
                     portas.motor2B,
	             portas.motor2PWM);

}
void DevBot::configurar(Sensor sensor1) { 
  sensor=sensor1;
}
void DevBot::configurar(Servos servo) { 
  temServo = 1;
  //se for program-me v2.0
  //vamos trocar as portas do PWM motor1 com a numero 3
  //por conta do problema do timer!
  if(placa==programME_v2) {
    portas.motor1PWM=3;
    portas.sensorDistancia1=10;
    robot.motores.configurar(1, portas.motor1A,  
                     portas.motor1B,
                     portas.motor1PWM,
                     portas.motor2A,  
                     portas.motor2B,
	             portas.motor2PWM);

  }
}


void DevBot::enviar(int x) { Serial1.println(x);}
void DevBot::enviar(char x) { Serial1.println(x);}
void DevBot::enviar(long x) { Serial1.println(x);}
void DevBot::enviar(char* x) { Serial1.println(x);}

/* 
 * Parte de recepção e envio de dados
 */

int DevBot::recebeuDados() {
  #ifdef ARDUINO_LINUX
    return Serial.available() + Serial1.available();
  #else 
    return Serial.available();
  #endif  
}

char* DevBot::receber() {
  int counter=0;
  limpaComando();
  #ifdef ARDUINO_LINUX
    if(Serial1.available()>0) {
      while(Serial1.available()>0) // loop through all but the last
      {
        char c = Serial1.read(); // receive byte as a character
        delay(1);
        comando[counter++]=c;
      }
      comando[counter]='\0';
      return comando;
    }
    else if(Serial.available()>0) {
      while(Serial.available()>0) // loop through all but the last
      {
        char c = Serial.read(); // receive byte as a character
        delay(1);
        comando[counter++]=c;
      }
      comando[counter]='\0';
      return comando;
    }
  #else  
    while(Serial.available()>0) // loop through all but the last
    {
      char c = Serial.read(); // receive byte as a character
      //delay(1);
      comando[counter++]=c;
    }
    comando[counter]='\0';
    return comando;
  #endif
}

void DevBot::controleRemoto() {
  if(!recebeuDados()) return;
  receber();
  split();

  
  if(strcmp("frente",command1)==0 || strcmp("fr",command1)==0 ) {
    robot.motores.frente(atoi(parameter1));
  }
  else if(strcmp("re",command1)==0 || strcmp("ré",command1)==0 ) {
    robot.motores.re(atoi(parameter1));
  }
  else if(strcmp("parar",command1)==0 || strcmp("pa",command1)==0 ) {
    robot.motores.parar();
  }
  else if(strcmp("esquerda",command1)==0 || strcmp("fe",command1)==0 ) {
    robot.motores.esquerda(atoi(parameter1));
  }
  else if(strcmp("direita",command1)==0 || strcmp("fd",command1)==0 ) {
    robot.motores.direita(atoi(parameter1));
  }
  else if(strcmp("reEsquerda",command1)==0 || strcmp("re",command1)==0 ) {
    robot.motores.reEsquerda(atoi(parameter1));
  }
  else if(strcmp("reDireita",command1)==0 || strcmp("rd",command1)==0 ) {
    robot.motores.reDireita(atoi(parameter1));
  }
  else if(strcmp("girar",command1)==0) { //aleatorio horário ou anti-horário
    robot.motores.girar(atoi(parameter1));
  }
  else if(strcmp("girarH",command1)==0 || strcmp("gh",command1)==0 ) { //aleatorio horário ou anti-horário
    robot.motores.girar(atoi(parameter1),0);
  }
  else if(strcmp("girarAH",command1)==0 || strcmp("ga",command1)==0 ) { //aleatorio horário ou anti-horário
    robot.motores.girar(atoi(parameter1),1);
  }
  else if(strcmp("aleatorio",command1)==0) { //aleatorio horário ou anti-horário
    robot.motores.movimentoAleatorio(atoi(parameter1));
  }
  else if(strcmp("sl",command1)==0) {
     Serial.print("l?");
     Serial.println(robot.sensorLuz());
     Serial.flush();
     #ifdef ARDUINO_LINUX     
       Serial1.print("l?");
       Serial1.println(robot.sensorLuz());
       Serial1.flush();
     #endif
  }
  else if(strcmp("st",command1)==0) {
     Serial.print("t?");
     Serial.println(robot.sensorTemperatura());
     Serial.flush();
     #ifdef ARDUINO_LINUX     
       Serial1.print("t?");
       Serial1.println(robot.sensorTemperatura());
       Serial1.flush();
     #endif
  }
  else if(strcmp("sd1",command1)==0) {
     Serial.print("d?");
     Serial.println(robot.sensorDistancia1());
     Serial.flush();
     #ifdef ARDUINO_LINUX     
       Serial1.print("d?");
       Serial1.println(robot.sensorDistancia1());
       Serial1.flush();
     #endif
  }
  else if(strcmp("servo",command1)==0) {
     mudarServo(atoi(parameter1));     
  } 
//38321548 
//38343200vigilancia sana


}



