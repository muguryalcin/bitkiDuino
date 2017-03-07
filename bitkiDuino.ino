//KUTUPHANELER
#include <Wire.h>
#include <SFE_BMP180.h>
#include "DHT.h" //DHT Kütüphanesini dahil ediyoruz

//DEGISKENLER
#define NEM_1 A0 //1.Nem Sensörünün pini
#define NEM_2 A1 //2. Nem sensörünün pini
#define DHTPIN 2 //DHT sensörünün pini
#define DHTTYPE DHT22 //DHT sensörünün türü
#define ALTITUDE 100 //İstanbuldaki rakım değeri
#define motor_pin 9

int sulanma_suresi = 4;
int aradaki_bekleme = 7;
int ortalama_nem = 0;
int nem_1_deger = 0;
int nem_2_deger = 0;
int calisma_siniri = 400;
int kritik_sinir = 100;
int yapilanTahmin = 0;
unsigned long eskiZaman=0;
unsigned long yeniZaman;
unsigned long defZaman =7200000;
int devirSayisi = 1;
/*
 Eğer yapılan tahmin 0 ise tahmin yapılmamıştır veya yağış yoktur.
 Eğer yapılan tahmin 1 ise yağmur yağacaktır.
 Eğer yapılan tahmin 2 ise kar yağacaktır.
*/
DHT dht(DHTPIN,DHTTYPE);
SFE_BMP180 bmp180;

void setup() {
  // Serial Ekranı Çalıştır
  Serial.begin(9600);
  dht.begin();
  pinMode(motor_pin , OUTPUT);
  bmp180.begin();
  tahminYap();
}

void loop() {
  yeniZaman=millis();
  delay(2000);
  float derece = dht.readTemperature(); // Dereceyi Ölçme
  float nem = dht.readHumidity(); //Nemi Ölçme
  float basinc = basincOku(); //Basıncı Ölçme
  if (isnan(derece) || isnan(nem)) { //DHT'den veri okunuyor mu ?
    Serial.println("DHT sensöründen veri okunamadı !");
    return;
  }
  if(!bmp180.begin()){
    Serial.println("BMP180 sensöründen veri okunamadı !");//BMP'den veri okunuyor mu ?
  }
  
  if(yapilanTahmin == 0){
    Serial.println("Yagis Olmayacak");
  }else if(yapilanTahmin == 1){
    Serial.println("Yagmur Yagabilir");
  }else if(yapilanTahmin == 2){
    Serial.println("Kar Yagabilir");
  }
  Serial.print("Havadaki Nem: %"); 
  Serial.println(nem); //Nemi yazdırma
  Serial.print("Derece: "); 
  Serial.print(derece); //Sıcaklığı yazdırma
  Serial.println(" *C ");
  Serial.print("Basinc : ");
  Serial.println(basinc);
  Serial.println(yeniZaman);
  
  // Nem Sensörlerinin değerini okuma
  nem_1_deger = 1024 - analogRead(NEM_1);
  Serial.print("1. Toprak Nem Sensoru : ");
  Serial.println(nem_1_deger);

  nem_2_deger = 1024 - analogRead(NEM_2);
  Serial.print("2. Toprak Nem Sensoru : ");
  Serial.println(nem_2_deger);

  ortalama_nem = (nem_1_deger + nem_2_deger) / 2;
  Serial.print("Toprak Ortalama  Nem : ");
  Serial.println(ortalama_nem);
  Serial.println("--------------------");

  if(yeniZaman-eskiZaman >= defZaman){
      tahminYap();
      Serial.println("Tahmin yapılıyor");
      Serial.println("Yeni tahmin bundan sonra goruntulenecek");
      yeniZaman = millis();
      eskiZaman = yeniZaman;
      devirSayisi++;
  }
  if(yeniZaman <= 1000 && yeniZaman >= 0 ){
    devirSayisi = 1;
  }
  if(ortalama_nem < calisma_siniri){
    if(ortalama_nem <= kritik_sinir){
      digitalWrite(motor_pin,HIGH);
      delay(sulanma_suresi*1000);
      digitalWrite(motor_pin,LOW);
      delay(aradaki_bekleme*1000);
    }else if(yapilanTahmin == 0 && ortalama_nem < calisma_siniri){
      digitalWrite(motor_pin,HIGH);
      delay(sulanma_suresi*1000);
      digitalWrite(motor_pin,LOW);
      delay(aradaki_bekleme*1000);
    }
  }
}

float basincOku()
{
  char status;
  double T,P,p0,a;

  status = bmp180.startTemperature();
  if (status != 0)
  {
    delay(status);
    status = bmp180.getTemperature(T);
    if (status != 0)
    { 
      status = bmp180.startPressure(3);
      if (status != 0)
      {
        delay(status);
        status = bmp180.getPressure(P,T);
        if (status != 0)
        {
          p0 = bmp180.sealevel(P,ALTITUDE);       
          return p0;
        }
      }
    }
  }
}

void tahminYap(){
  float derece = dht.readTemperature(); // Dereceyi Ölçme
  float nem = dht.readHumidity(); //Nemi Ölçme
  float basinc = basincOku(); //Basıncı Ölçme
  if(nem == 100.0 && basinc <= 1013.00){
    if(derece <= -4.00){
      yapilanTahmin = 2;
    }else if(derece > -4.00){
      yapilanTahmin = 1;
    }
  }else{
    yapilanTahmin = 0;
  }
}

