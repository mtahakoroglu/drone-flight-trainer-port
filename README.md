# Hazır ve Elle Yapılan Kumanda ve Hareket Sensörü ile Bir Dronun Sanal ve Gerçek Ortamlarda Manuel Kontrolü
## Özet

Bu çalışmada bilgisayar ortamında dinamikleri modellenmiş dört pervaneli bir dronun (i.e., quadcopter) manuel kontrolü için tasarlanan 4-kanallı bir RF alıcı-verici sisteminin prototipi geliştirilmiştir. Kumanda tarafındaki iki joystick tarafından üretilen dört kanal sinyali Arduino mikrodenetleyicisine analog girişlerden [0-5]V aralığında alınırken [0-1023] tam sayı aralığına dönüştürülmüştür. Kablosuz iletişimde hızı artırmak amacıyla belirtilen aralık [0-255] aralığına tekrardan dönüştürülmüş, elde edilen bu sinyaller NRF24L01 kablosuz modülü aracılığıyla ilerleyen zamanlarda dron üzerinde yer alması planlanan halihazırda ise ikinci bir Arduino – NRF24L01 kablosuz modülü olan alıcı sistemine gönderilmiştir. Burada tekrardan elde edilen [0-255] aralığındaki dört kanal sinyali simülasyon ortamında dronu uçurmak için kullanılacak olan PPM sinyaline dönüştürülmüştür. MATLAB ortamında gerçek-zamanda PPM sinyalini gözlemlemek maksadıyla alıcıdan bilgisayara seri port ile kanal sinyalleri gönderilmiştir. Bu çalışmada NRF24L01 kablosuz modülüyle yapılan iletişim oldukça popüler olan XBee kablosuz modülleriyle de yapılarak test edilmiştir. İlerleyen zamanda dronun simülasyon ortamındaki kontrolünün yanısıra fiziksel bir dronun kontrolü de hedefler arasındadır. Çalışmanın bir başka hedefi ise hazır bir RF alıcı-verici modülü ile bağlanmış fiziksel bir drona, hazır modülün TRAINER girişi üzerinden elle tasarlanan kumanda sinyalini bağlayıp dronu iki kişi tarafından kontrol etmektir.

## Abstract
In this study, we designed a succesful prototype to perform as a 4-channel RF transmitter-receiver system that is going to be used to control a quadcopter type of drone that flies in virtual and physical environments. All the mapping starting from reading the potentiometers of two joysticks of the transmitter system into Arduino, i.e., [0-5]V  [0-1023], and then another mini-mapping for speed purposes, i.e., [0-1023]  [0-255], and subsequently forming PPM signals in the range of [1100-1900]us from the channel signals after the wireless communication takes place is elaborately described in this report. Arduinos are used as signal processing devices while NRF24L01 and XBee wireless modules are tested for the communication. Additionally, received channel signals are sent to MATLAB via serial port in order to graphically monitor the PPM signal in real-time. The first step is going to be applying the generated PPM signal to a drone flying in a virtual environemnt while flying a physical drone with a hand-made radio transmitter is next step. Lastly, an off-the-shelf radio transmitter will be employed as the master while the hand-made transmitter functions as the slave and the combined system is going to be used to fly a drone with two users.

Potansiyometreye **ayarlı direnç** dendiğinden daha önce bahsetmiştik. Bu devre elemanının;
* Üç bacağı olduğunu, 
* İlk ve son bacaklarına Arduino'nun Vcc (5V) ve GND pinlerinin bağlandığını, 
* Orta bacağının Arduino'nun analog girişlerinden bir tanesine (e.g., A0) bağlandığını, 
* Analogdan dijitale çevirici (Analog to Digital Converter - ADC) aracılığıyla [0-5V] aralığında sürekli (continuous) değer alabilen analog bir sinyalin, [0-1023] aralığında bir tam sayıya (integer) dönüştürülerek alındığını, 

görmüştük. Bir potansiyometreyi ve yukarıda listelenen Arduino bağlantılarını Şekil 1'de görebilirsiniz.

<img src="https://www.direnc.net/22k-potansiyometre-potansiyometre-spike-55641-56-B.jpg" alt="Potansiyometre" height="240"/> <img src="figures/potansiyometre_arduino_bağlantılar.jpg" alt="Arduino ile potansiyometre arasındaki bağlantılar." height="240"/>

*Şekil 1: Potansiyometre ([1]'in izni ile) ve Arduino-potansiyometre bağlantıları.*
## Joystick
İçinde iki tane potansiyometre barındıran bir joystick'in resmini Şekil 2'de görebilirsiniz. Her bir potansiyometrenin üretmiş olduğu sinyale *kanal sinyali* diyeceğiz. Kendi-kendini dengeleyen robot projesinde manuel kontrol için **iki** kanala (ileri-geri ve kendi ekseni etrafında dönme (i.e., spin)), dron projesinde ise minimum **dört** kanala (i.e., yukarı-aşağı (i.e., *throttle*), kendi ekseni etrafında dönme (i.e., *rudder* veya *yaw*), sağa-sola (*aileron* veya *roll*) ve ileri-geri (*elevator* veya *pitch*)) ihtiyaç duyulmaktadır.

<img src="https://market.projehocam.com/wp-content/uploads/joystick.jpg" alt="Potansiyometre" height="240"/>

*Şekil 2: İki eksenli joystick ([2]'nin izni ile). VRx ve VRy olarak isimlendirilen çıkışlar Arduino'nun analog girişlerine (e.g., A0 ve A1) bağlanmaktadır.*

Şekil 2'de gösterilen iki-kanallı joystick'in beşinci bacağı **SW** olarak isimlendirilmiş olup *switch* anlamında *anahtar* vazifesi görmektedir. Joystick'in üzerine basılmadığında **HIGH** durumunda olan bu bacak basıldığı zaman **LOW** duruma geçer ve bu sinyal Arduino'ya dijital giriş olarak alınabilir<sup>1</sup>. Bu şekilde istenildiği zaman joystick'e basarak Arduino'ya işaret gönderip kanal sinyalleri için hareket sensörüne geçeceğimizi bildirebiliriz. Burada gözden kaçırılmaması gereken bir nokta VRx ve VRy bacaklarından Arduino'ya *her zaman* sinyallerin okunduğudur. Eğer joystick'e basılıp da kontrol hareket sensörüne geçse bile joystick sinyallerini Arduino'ya almaya devam ediyor olacağız. Ancak robota veya drona gönderilecek olan kanal sinyalleri joystick'den gitmiyor olabilir.

## Dipnotlar
<sup>1</sup> Kullanılan joystick'e göre **SW** girişinin default değeri değişebilir, bu yüzden multimetre veya Arduino seri port ile push-button'a basılmadığında oluşan değeri test ediniz.

## Kaynaklar
[1] https://www.direnc.net/22k-potansiyometre-potansiyometre-spike-55641-56-B.jpg</br>
[2] https://market.projehocam.com/wp-content/uploads/joystick.jpg</br>