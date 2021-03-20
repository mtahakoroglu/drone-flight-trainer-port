/* Bu kod aşağıdaki kaynaktan alınıp değişikliğe uğramıştır. 
https://howtomechatronics.com/projects/diy-arduino-rc-transmitter/ */
#include <Wire.h> // Arduino ile MPU6050 arasındaki haberleşme protokolü olan I2C'nin kullanımına izin veren kütüphane
#include <eRCaGuy_PPM_Writer.h> // PPM sinyalini oluşturan kütüphane
const int MPU = 0x68; // MPU6050 I2C address
/* sensör koordinat sistemine göre okunan ham veriler.
accX, accY ve accZ değişkenlerinin birimi m/s^2
gyroX, gyroY ve gyroZ değişkenlerinin birimi derece/s */
float accX, accY, accZ, gyroX, gyroY, gyroZ; // ortogonal üç eksendeki ham veriler
float accAngleX, accAngleY, gyroAngleX, gyroAngleY; // sadece ivmemetre ve ciroskop kullanılarak hesaplanan açılar
float accErrorX, accErrorY, gyroErrorX, gyroErrorY; // açılardaki hatalar - bu kodda kullanmak zorunda değilsiniz
/* Aşağıda tanımladığımız elapsedTime değişkenini gyro verilerinin nümerik 
integralini almada kullanıyoruz. Ayrıca currentTime bize yakalanan zaman verisi 
olarak hizmet ediyor. */
float elapsedTime, currentTime, previousTime; // sırasıyla geçenZaman, şimdikiZaman, öncekiZaman
int c = 0;
struct IMU {
  float angles[2]; // angleX, angleY
  unsigned long timeStamp; // zamanı yakalıyoruz
};
IMU imu;
#define pb 6 // joystick'in SW isimli bacağı, push-button digital giriş olarak D6 bacağına bağlanmalı
bool switchTurn = true;
const int n = 4; // kanal sayısı, drone projesi için 4 olmalı
int analogPin[n] = {A0, A1, A2, A3}; // eğer n = 4 ise A2 ve A3'ü de ekleyin
byte kanal[n];
const byte channel[4] = {0, 1, 2, 3}; // Gabriel'ın PPM kütüphanesine erişecek kanalları tanımla
const unsigned int MAX_CHANNEL_VAL = 1900;
const unsigned int MIN_CHANNEL_VAL = 1100;
const unsigned int MIDDLE_CHANNEL_VAL = 1500;
unsigned int PPMsignal[4] = {MIN_CHANNEL_VAL, MIDDLE_CHANNEL_VAL, MIDDLE_CHANNEL_VAL, MIDDLE_CHANNEL_VAL};

void setup() {
  Serial.begin(57600);
  initialize_MPU6050();
  //calculate_IMU_error(); // Call this function if you need to get the IMU error values for your module
  pinMode(pb, INPUT_PULLUP); // Activate the Arduino internal pull-up resistor
  PPMWriter.setChannelVal(channel[0], MIN_CHANNEL_VAL*2);
  for (int i = 1; i < n; i++)
    PPMWriter.setChannelVal(channel[i], MIDDLE_CHANNEL_VAL*2);
  PPMWriter.begin();
}

void loop() {
  read_IMU(); 
  if (!digitalRead(pb)) // push-button anahtara basıldığında LOW oluyor, default değeri HIGH
  {
    delay(50); // bu çok da arzu ettiğimiz bir metod değil ama TOGGLE SWITCH'imiz yoksa bu yolla yapalım
    switchTurn = !switchTurn; // kanal sinyallerini üretme sırası kimde: Joystick mi yoksa MPU6050 mi?
  }
  for (int i=2; i<n; i++)
      kanal[i] = map(analogRead(analogPin[i]), 0, 1023, 0, 255);
  if (switchTurn == true) // sıra joystick'de
  {
    for (int i=0; i<2; i++)
      kanal[i] = map(analogRead(analogPin[i]), 0, 1023, 0, 255);
  }
  else // sıra hareket sensörü olan MPU6050'de
  {
    //for (int i=0; i<2; i++)
      //kanal[i] = map(imu.angles[i], -90, 90, 0, 255);
      kanal[0] = map(imu.angles[1], -60, 60, 0, 255);
      kanal[1] = map(imu.angles[0], 60, -60, 0, 255);
  }
  for (int i = 0; i < n; i++)
      PPMsignal[i] = map(kanal[i], 0, 255, MIN_CHANNEL_VAL, MAX_CHANNEL_VAL);
  for (int i = 0; i < n; i++)
    PPMWriter.setChannelVal(channel[i], PPMsignal[i]*2);
  Serial.print(switchTurn); Serial.print(' '); Serial.print(imu.angles[0]); Serial.print(' '); Serial.print(imu.angles[1]); Serial.print(' '); 
  Serial.print(digitalRead(pb)); Serial.print(' '); Serial.print(kanal[0]); Serial.print(' '); Serial.print(kanal[1]); Serial.print(' '); Serial.print(kanal[2]);
  Serial.print(' '); Serial.print(kanal[3]); Serial.print(' '); Serial.println(imu.timeStamp);;
  //Serial.write('h'); Serial.write((byte*)(&imu), sizeof(imu));
}

/* IMU'dan açıyı elde etmek için read_IMU() fonksiyonunu çağırıyoruz. */
void read_IMU() {
  // === ivmemetre verisini oku === //
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  // For a range of +-8g, we need to divide the raw values by 4096, according to the datasheet
  accX = (Wire.read() << 8 | Wire.read()) / 4096.0; // x-eksenindeki değeri oku
  accY = (Wire.read() << 8 | Wire.read()) / 4096.0; // y-eksenindeki değeri oku
  accZ = (Wire.read() << 8 | Wire.read()) / 4096.0; // z-eksenindeki değeri oku
  // sadece ivmemetre verisini kullanarak açıyı hesapla
  accAngleX = (atan(accY / sqrt(pow(accX, 2) + pow(accZ, 2))) * 180 / PI) + 1.15; // accErrorX ~(-1.15) See the calculate_IMU_error() custom function for more details
  accAngleY = (atan(-1 * accX / sqrt(pow(accY, 2) + pow(accZ, 2))) * 180 / PI) - 0.52; // accErrorX ~(0.5)
  // === ciroskop verisini oku === //
  previousTime = currentTime; // eski şimdikiZaman şimdi oluyor öncekiZaman
  imu.timeStamp = millis(); // yakaladığımız zamanı seri portla başka yerlere de gönderebiliriz
  currentTime = float(imu.timeStamp); // yakalanan zaman şimdikiZaman oluyor
  elapsedTime = (currentTime - previousTime) / 1000; // geçenZaman, birimi ms olduğundan 1000'e bölünerek saniye birimine çevriliyor
  Wire.beginTransmission(MPU);
  Wire.write(0x43); // Gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 4, true); // Read 4 registers total, each axis value is stored in 2 registers
  gyroX = (Wire.read() << 8 | Wire.read()) / 32.8; // For a 1000dps range we have to divide first the raw value by 32.8, according to the datasheet
  gyroY = (Wire.read() << 8 | Wire.read()) / 32.8;
  gyroX = gyroX + 1.85; //// gyroErrorX ~(-1.85)
  gyroY = gyroY - 0.15; // gyroErrorY ~(0.15)
  // nümerik integral ile sadece ciroskop verisini kullanarak açıyı hesapla
  gyroAngleX = gyroX * elapsedTime; // derece/s --> derece
  gyroAngleY = gyroY * elapsedTime; // derece/s --> derece
  /* Tamamlayıcı süzgeç (Complementary filter) ile ciroskop ve ivmemetre ile ayrı ayrı 
  bulunan açıların ağırlıklı ortalaması alınarak daha doğru bir açının bulunması. */
  imu.angles[0]= 0.98 * (imu.angles[0] + gyroAngleX) + 0.02 * accAngleX;
  imu.angles[1] = 0.98 * (imu.angles[1] + gyroAngleY) + 0.02 * accAngleY;
}

/* MPU6050 hareket sensörünü başlangıçta aktif hale getir. */
void initialize_MPU6050() {
  Wire.begin();                      // Initialize communication
  Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire.endTransmission(true);        //end the transmission
  // Configure Accelerometer
  Wire.beginTransmission(MPU);
  Wire.write(0x1C);                  //Talk to the ACCEL_CONFIG register
  Wire.write(0x10);                  //Set the register bits as 00010000 (+/- 8g full scale range)
  Wire.endTransmission(true);
  // Configure Gyro
  Wire.beginTransmission(MPU);
  Wire.write(0x1B);                   // Talk to the GYRO_CONFIG register (1B hex)
  Wire.write(0x10);                   // Set the register bits as 00010000 (1000dps full scale)
  Wire.endTransmission(true);
}

/* Hareket sensörü ile bulunan açıların hatalarının hesaplanması - Bu fonksiyonu çağırmasak da olur. */
void calculate_IMU_error() {
  // We can call this funtion in the setup section to calculate the accelerometer and gury data error. 
  // From here we will get the error values used in the above equations printed on the Serial Monitor.
  // Note that we should place the IMU flat in order to get the proper values, so that we then can the correct values
  // Read accelerometer values 200 times
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    accX = (Wire.read() << 8 | Wire.read()) / 4096.0 ;
    accY = (Wire.read() << 8 | Wire.read()) / 4096.0 ;
    accZ = (Wire.read() << 8 | Wire.read()) / 4096.0 ;
    // Sum all readings
    accErrorX = accErrorX + ((atan((accY) / sqrt(pow((accX), 2) + pow((accZ), 2))) * 180 / PI));
    accErrorY = accErrorY + ((atan(-1 * (accX) / sqrt(pow((accY), 2) + pow((accZ), 2))) * 180 / PI));
    c++;
  }
  //Divide the sum by 200 to get the error value
  accErrorX = accErrorX / 200;
  accErrorY = accErrorY / 200;
  c = 0;
  // Read gyro values 200 times
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 4, true);
    gyroX = Wire.read() << 8 | Wire.read();
    gyroY = Wire.read() << 8 | Wire.read();
    // Sum all readings
    gyroErrorX = gyroErrorX + (gyroX / 32.8);
    gyroErrorY = gyroErrorY + (gyroY / 32.8);
    c++;
  }
  //Divide the sum by 200 to get the error value
  gyroErrorX = gyroErrorX / 200;
  gyroErrorY = gyroErrorY / 200;
  // Print the error values on the Serial Monitor
  Serial.print("accErrorX: ");
  Serial.println(accErrorX);
  Serial.print("accErrorY: ");
  Serial.println(accErrorY);
  Serial.print("gyroErrorX: ");
  Serial.println(gyroErrorX);
  Serial.print("gyroErrorY: ");
  Serial.println(gyroErrorY);
}
