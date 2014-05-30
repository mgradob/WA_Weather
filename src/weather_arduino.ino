
#include<stdlib.h>

//Xbee
char buffout[80];
char buffin[80];
int sum, recibir, sensor1, sensor2, sensor3;
int array_bien=0;

//Variables generales
float velocidad, velocidad2, humedad, temp; 
int radiacion;
int s1=10, s2=9, s3=8;
unsigned long highCounter = 0;
unsigned long tiempoanterior=0;
long tiempo=60000; //Tiempo que durará contando los pulsos (milisegundos)

//Variables anemometro
int pulsePin = 22;  //Pin para los pulsos del anemometro
int pulse = 0;
int lastPulse = LOW;
int totalpulsos; //Variable donde se guarda el total de pulsos
unsigned int velocidad_ent;
unsigned int velocidad_dec;
float velocidad_ant = 0;

//Variables radiacion solar

int PYR = A0;
unsigned int radiacionh;
unsigned int radiacionl;

//Variables VP3
unsigned int humedad_ent;
unsigned int humedad_dec;
unsigned int temp_ent;
unsigned int temp_dec;
float humedad_ant = 0;
float temp_ant = 0;

//--------Kalman variables
#define alpha 0.1
int radiacion_ant = 0;

int led = 13;


void serialEvent(){
digitalWrite(led,HIGH);
if (Serial.available() > 0) {
    Serial.readBytes(buffin,22);
     while(!array_bien){
        
        if(!(buffin[0] == 0x7E)){
          for(int k=0; k<22; k++){
          buffin[k] = buffin[k+1];
          }}
          else{
           array_bien = 1;
          }
      
      } 
    
   if(buffin[0] == 0x7E && buffin[15] == 0x43){
  
                buffout[0]=0x7E;//API mode start delimeter
		buffout[1]=0x00;//Data length MSB
		buffout[2]=0x17;//Data length LSB
		buffout[3]=0x10;//API ID TX Request 16 bit address
		buffout[4]=0x00;//Frame ID, if equals to 0x00 disable response frame
		
                buffout[5]=0x00;//Destination Address MSB
                buffout[6]=0x13;
                buffout[7]=0xA2;
                buffout[8]=0x00;
                buffout[9]=0x40;
                buffout[10]=buffin[9];
                buffout[11]=buffin[10];
                buffout[12]=buffin[11];//Destination Address LSB
		
                buffout[13]=0xFF;//Reserved
		buffout[14]=0xFE;//Reserved
		buffout[15]=0x00;
		buffout[16]=0x00;
		
                buffout[17]=0x1F;//Identificador de nodo (31)
                buffout[18]=radiacionh;//Radiacion solar MSB
                buffout[19]=radiacionl;//Radiacion solar LSB
                buffout[20]=humedad_ent;//Humedad entero
                buffout[21]=humedad_dec;//Humedad decimal
                buffout[22]=temp_ent;//Temperatura entero
                buffout[23]=temp_dec;//Temperatura decimal
                buffout[24]=velocidad_ent;//Velocidad viento entero
                buffout[25]=velocidad_dec;//Velocidad viento decimal


for(int i=3;i<26;i++){

sum+=buffout[i];


}
buffout[26]=0xFF-lowByte(sum); //Checksum
sum=0;

  
    
            Serial.write(buffout[0]);
            Serial.write(buffout[1]);
            Serial.write(buffout[2]);
            Serial.write(buffout[3]);
            Serial.write(buffout[4]);
            Serial.write(buffout[5]);
            Serial.write(buffout[6]);
            Serial.write(buffout[7]);
            Serial.write(buffout[8]);
            Serial.write(buffout[9]);
            Serial.write(buffout[10]);
            Serial.write(buffout[11]);
            Serial.write(buffout[12]);
            Serial.write(buffout[13]);
            Serial.write(buffout[14]);
            Serial.write(buffout[15]);
            Serial.write(buffout[16]);
            Serial.write(buffout[17]);
            Serial.write(buffout[18]);
            Serial.write(buffout[19]);
            Serial.write(buffout[20]);
            Serial.write(buffout[21]);
            Serial.write(buffout[22]);
            Serial.write(buffout[23]);
            Serial.write(buffout[24]);
            Serial.write(buffout[25]);
            Serial.write(buffout[26]);
            
            
   }
          
   
   
            
}
digitalWrite(led, LOW);
}

 

  

void readPYR(){
  
  digitalWrite(s2, LOW);//Energizar sensor
  delay(20);
  radiacion = (4.88281*analogRead(PYR))*5;//Realizar lectura y convertirla a radiacion solar
  digitalWrite(s2, HIGH);//Desenergizar sensor
  
  radiacion = (radiacion*alpha) + (1-alpha)*radiacion_ant;//kalman
  radiacion_ant = radiacion;
   
   //Dividir radiacion en 2 bytes
  radiacionh = highByte(radiacion);
  radiacionl = lowByte(radiacion);


}
 
void readWind(){

  digitalWrite(s3, LOW);//Energizar sensor (anemometro permanece encendido todo el tiempo)
  //Contador de pulsos
  pulse = digitalRead(pulsePin);
  if (pulse != lastPulse) { 
    
    
    lastPulse = pulse;
    
    if (pulse == HIGH) {
      highCounter++; 
      }
  } 
  
 
}

void readVP3(){
/*      */
    
  digitalWrite(s2, LOW);//Energizar sensor
    float valor;
    char a[4], b[4];
    char datos[16];
  if (Serial1.available()) {  //Esperar y leer datos provenientes del sensor 
  Serial1.readBytes(datos,16);
 
 //Guardar vapor en arreglo "a"
  a[0]=datos[1];
  a[1]=datos[2];
  a[2]=datos[3];
  a[3]=datos[4];
  
 //Guardar temperatura en arreglo "b" 
  b[0]=datos[6];
  b[1]=datos[7];
  b[2]=datos[8];
  b[3]=datos[9];
  
  temp = atof(b); //Convertir arreglo "b" a variable flotante
 
  temp = (temp*alpha)+(1-alpha)*temp_ant; // kalman
  temp_ant = temp;
  
  
  humedad = (atof(a)/pow(2.7182, ((17.502*temp)/(240.97+temp)))*100);//Convertir arreglo "b" a flotante y obtener humedad relativa
 
  humedad = (humedad*alpha)+(1-alpha)*humedad_ant; // kalman
  humedad_ant = humedad;
  
  digitalWrite(s2, HIGH);//Desenergizar sensor
  }
  
 /* else{ //En caso de que el sensor sea desconectado
  humedad = 0;
  temp = 0;
  }*/
  
  //Dividir humedad en 2 bytes, 1 para enteros y 1 para decimales
  humedad_ent = humedad;
  humedad_dec = (humedad - humedad_ent)*100;
  
  //Dividir temperatura en 2 bytes, 1 para enteros y 1 para decimales y establecer bit mas significativo para signo
  if(temp<0){
  temp_ent = abs(temp) + 128;                                   
  temp_dec = ((abs(temp) - 128) - temp_ent)*100;
  }
  else{
  temp_ent = temp;
  temp_dec = (temp - temp_ent)*100;
  }
  
  
   /*Serial.print("Humedad: ");
   Serial.print(humedad);
   Serial.print(" Temperatura: ");
   Serial.println(temp);*/

}


void setup() {
   pinMode(pulsePin, INPUT);
   pinMode(s1, OUTPUT);
   pinMode(s2, OUTPUT);
   pinMode(s3, OUTPUT);
   pinMode(led, OUTPUT);
 
   digitalWrite(pulsePin, HIGH);  //Habilitar resistencia de pull up 
   Serial.begin(9600);//Puerto para comunicacion por xbee
   Serial1.begin(1200);//Puerto de comunicacion con sensor digital
   
}
 
void loop() {
  
  
   unsigned long tiempoactual=millis();
  readWind();
  if(tiempoactual - tiempoanterior > tiempo) {
   
    tiempoanterior = tiempoactual;   
    totalpulsos=highCounter;
    highCounter=0;  //Reset del contador cada vez que "tiempo" se cumple
    //velocidad = (totalpulsos*(2.25/(tiempo/1000)))/3.6;
    velocidad= 1.006*(totalpulsos/60); //Formula alternativa
  
    velocidad = (velocidad*alpha)+(1-alpha)*velocidad_ant; // kalman
    velocidad_ant = velocidad;
    
    velocidad_ent = velocidad;
    velocidad_dec = (velocidad - velocidad_ent)*100;
   
    
    
   /* Serial.print(velocidad2);
    Serial.println(" m/s");*/
  
  readPYR();   
  readVP3();
  
  
  
 
  /*  Serial.print("Velocidad viento: ");
    Serial.print(velocidad);
    Serial.println(" m/s ");
    
   Serial.print("Humedad: ");
   Serial.print(humedad);
   Serial.println(" %");
   Serial.print("Temperatura: ");
   Serial.print(temp);
   Serial.println(" °C");
   
   Serial.print("Radiacion solar: ");
   Serial.print(radiacion);
   Serial.println(" W/m2");
   
   Serial.println("");
   Serial.println("");*/
    
}}
