/* Bu kod aşağıdaki kaynaktan alınıp değişikliğe uğramıştır. 
https://howtomechatronics.com/projects/diy-arduino-rc-transmitter/ */
#include <eRCaGuy_PPM_Writer.h> // PPM sinyalini oluşturan kütüphane
const int n = 4; // kanal sayısı, drone projesi için 4 olmalı
int analogPin[n] = {A0, A1, A2, A3}; // eğer n = 4 ise A2 ve A3'ü de ekleyin
byte kanal[n];
const byte channel[4] = {0, 1, 2, 3}; // Gabriel'ın PPM kütüphanesine erişecek kanalları tanımla
// The channel us values below are for Radiomaster TX16S radio (remote) controller
const unsigned int MAX_CHANNEL_VAL = 2000, MIN_CHANNEL_VAL = 1000, MIDDLE_CHANNEL_VAL = 1500;
unsigned int PPMsignal[4] = {MIN_CHANNEL_VAL, MIDDLE_CHANNEL_VAL, MIDDLE_CHANNEL_VAL, MIDDLE_CHANNEL_VAL};

void setup() {
  PPMWriter.setChannelVal(channel[0], MIN_CHANNEL_VAL*2);
  for (int i = 1; i < n; i++)
    PPMWriter.setChannelVal(channel[i], MIDDLE_CHANNEL_VAL*2);
  PPMWriter.begin();
}

void loop() {
  for (int i=0; i<n; i++)
      kanal[i] = map(analogRead(analogPin[i]), 0, 1023, 0, 255);
  for (int i = 0; i < n; i++)
      PPMsignal[i] = map(kanal[i], 0, 255, MIN_CHANNEL_VAL, MAX_CHANNEL_VAL);
  for (int i = 0; i < n; i++)
    PPMWriter.setChannelVal(channel[i], PPMsignal[i]*2);
}
