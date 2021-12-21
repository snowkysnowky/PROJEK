#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h> //6.17.3
#include <MFRC522.h>
#include <Servo.h>
#include <NewPing.h>
//#include <LiquidCrystal_I2C.h>
//LiquidCrystal_I2C lcd(0x27, 16, 2);
int triger =2;
int echo =3;
int batas =400; //Maksimal 400 cm
int ir_awal = 0;
int ir_akhir = 0;
int count=0;
NewPing cm(triger,echo,batas);
String jenis;

// replace the MAC address below by the MAC address printed on a sticker on the Arduino Shield 2
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetClient client;

int    HTTP_PORT   = 80;
String HTTP_METHOD = "GET";
char   HOST_NAME[] = "192.168.0.111"; // change to your PC's IP address
String PATH_NAME   = "/argon/data-api.php";
String getData;

#define SS_PIN 9
#define RST_PIN 8
#define buzzer 7
#define outServo 5
Servo myservo;
int sensorIR=6;
int ledr=A0;
int ledg=A1;
int ledy=A2;





MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
int flag=0;
void setup() {
  Serial.begin(115200);
   
 // lcd.begin();
 // lcd.clear(); //Untuk Menghapus karakter pada LCD
// lcd.setCursor(0,0); lcd.print("ON");
 delay(200);
  myservo.attach(outServo); 
  myservo.write(0);
  pinMode (buzzer,OUTPUT);
  pinMode (ledr,OUTPUT);
  pinMode (ledy,OUTPUT);
  pinMode (ledg,OUTPUT);
  pinMode(sensorIR,INPUT); 
  while(!Serial);
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  //delay(3000);
  //START IP DHCP
  Serial.println("Konfigurasi DHCP, Silahkan Tunggu!");
  digitalWrite(ledy,HIGH);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP Gagal!");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet Tidak tereteksi :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Hubungkan kabel Ethernet!");
    }
    while (true) {delay(1);}
  }  
  //End DHCP 
  delay(5000); 
  Serial.print("IP address: ");
  Serial.println(Ethernet.localIP());  
  client.connect(HOST_NAME, HTTP_PORT);
  Serial.println("Siap Digunakan!");
  digitalWrite(ledr,HIGH);
  digitalWrite(ledy,LOW);
}

void loop() {
  //Baca data
 int ir_start=map(digitalRead(sensorIR),1,0,0,1);
 int ir_awal=map(digitalRead(sensorIR),1,0,0,1);
 int bacaJarak=cm.ping_cm();

 //LOGIKA GOLONGAN
 if (bacaJarak<=5){
 jenis =String ("Bis");
 }
 else{ jenis =String ("Mobil");
 }
   //Program yang akan dijalankan berulang-ulang
  if(flag==0){
  if ( ! mfrc522.PICC_IsNewCardPresent()) {return;}
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {return;}
  
  //Show UID on serial monitor 
  Serial.print("UID tag :");

  String uidString;
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     uidString.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "" : ""));
     uidString.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  uidString.toUpperCase();
  Serial.println(uidString);
  digitalWrite(buzzer,HIGH);
  delay(100);
  digitalWrite(buzzer,LOW);
  //POST TO WEB
 
    client.connect(HOST_NAME, HTTP_PORT);
    client.println(HTTP_METHOD + " " + PATH_NAME + 
                   "?rfid=" + String(uidString) + 
                   "&lokasi=Cibubur"+
                   "&golongan=" +String(jenis)+ 
                   //"&sensor1=" + String(sensor1) + 
                   //"&sensor2=" + String(sensor2) + 
                   " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println(); // end HTTP header
   }
  
    while(client.connected()) {
      if(client.available()){
        char endOfHeaders[] = "\r\n\r\n";
        client.find(endOfHeaders);
        getData = client.readString();
        getData.trim();
        //AMBIL DATA JSON
        const size_t capacity = JSON_OBJECT_SIZE(2) + 50; //cari dulu nilainya pakai Arduino Json 5 Asisten
        DynamicJsonDocument doc(capacity);
        //StaticJsonDocument<192> doc;
        DeserializationError error = deserializeJson(doc, getData);
      
        const char* rfid_dibaca  = doc ["rfid"];
        const char* status_dibaca  = doc["status"];
        
      
       //LOGIKA
       if(String(status_dibaca)== "Sukses"){
        Serial.print("----------------------------------------------------");
        Serial.println() ;
        Serial.println("Kartu Terdaftar!");
        Serial.print("Status    = ");Serial.println(status_dibaca);
        Serial.println("Transaksi Sukses");
        Serial.print("----------------------------------------------------"); 
        Serial.println();
        digitalWrite(ledg,HIGH);
        digitalWrite(ledr,LOW);
        flag=1;
        myservo.write(90);  
        buzzeroke();
       }
        if(String(status_dibaca)== "ID Belum Terdaftar"){
        buzzergagal();
        Serial.print("----------------------------------------------------"); 
        Serial.println();         
        Serial.println("Kartu Tidak Terdaftar");
        Serial.print("Status    = ");Serial.println(status_dibaca);
        Serial.print("----------------------------------------------------"); 
        Serial.println();
       }
        if(String(status_dibaca)== "Saldo Tidak Cukup" ){
        buzzergagal();
        Serial.print("----------------------------------------------------");     
        Serial.println();     
        Serial.println("Saldo Tidak Cukup");  
        Serial.print("Status    = ");Serial.println(status_dibaca);
        Serial.print("----------------------------------------------------"); 
        Serial.println();
       }
      } 
     }
   //LOGIKA SERVO TERTUTUP
   if(ir_awal==0 && ir_akhir==1 && flag==1){     
   myservo.write(0);
   digitalWrite(ledg,LOW);
   digitalWrite(ledr,HIGH);
   flag=0;
   }
   ir_akhir=ir_awal;
}


void buzzeroke(){
  digitalWrite(buzzer,HIGH);
  delay(100);
  digitalWrite(buzzer,LOW);
  delay(100);
  digitalWrite(buzzer,HIGH);
  delay(100);
  digitalWrite(buzzer,LOW);
  delay(100);
}

void buzzergagal(){
  digitalWrite(buzzer,HIGH);
  digitalWrite(ledy,HIGH);
  delay(1000);
  digitalWrite(ledy,LOW);
  digitalWrite(buzzer,LOW);
  delay(10);
}
