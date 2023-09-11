#include <config.h>//memasukkan library yg diperlukan
#include <ds3231.h> //~
#include "ds3231.h" //~
#include <Wire.h>  //~
#include <Servo.h>  //memasukkan library yg diperlukan


#define BUFF_MAX 256 //ukuran buffer maksimum

Servo myservo1; //memberi nama masing-masing servo
Servo myservo2; //~
Servo myservo3; //~
Servo myservo4; //~
Servo myservo5; //memberi nama masing-masing servo

uint8_t waktu_memasak = 2;       // deklarasi penahan waktu memasak antara 2 waktu berturut-turut
unsigned long prev = 1000, interval = 1000; // informasi waktu refresh milisecond(ms)
void waktu_angkat_selanjutnya(void) //mendeklarasikan variabel program
{
    struct ts t; //membuat tipe data sendiri
    unsigned char angkat_menit; //deklarasi tipe data unsigned char mengkodekan angka dari 0-255 untuk konsistensi 
    DS3231_get(&t); //memanggil tipe data yang dibuat
    angkat_menit = (t.min + waktu_memasak); // kalkulasi waktu angkat masakkan selanjutnya aktif
    if (angkat_menit > 59) { //jika memenuhi kondisi berikut
        angkat_menit -= 60;  //jika memenuhi kondisi berikut
    }
    // flags adalah pengecekan data waktu yang akan diambil dari kalender 
    // untuk mengaktifkan atau mematikan data waktu
    // A2M2 (minutes) (0 untuk aktif, 1 untuk nonaktif)
    // A2M3 (hour)    (0 untuk aktif, 1 untuk nonaktif) 
    // A2M4 (day)     (0 untuk aktif, 1 untuk nonaktif)
    // DY/DT          (satu minggu == 1/satu bulan == 0)
    uint8_t flags[4] = { 0, 1, 1, 1 };
    // hanya mengambil data menit dan detik, jam dan hari tidak diikut sertakan
    DS3231_set_a2(angkat_menit, 0, 0, flags);
    // mengaktifkan set data waktu
    DS3231_set_creg(DS3231_CONTROL_INTCN | DS3231_CONTROL_A2IE);
}

void setup() //menyiapkan program yang akan diperlukan divoid loop
{
    Serial.begin(9600); //membangun komunikasi serial dan arduino dengan perangkat lain
    Wire.begin(); //membangun komunikasi serial dan arduino dengan perangkat lain
    DS3231_init(DS3231_CONTROL_INTCN); //menunjukkan kontrol interupsi untuk menonaktifkan sementara
    DS3231_clear_a2f(); //membersihkan data waktu aktif 
    waktu_angkat_selanjutnya(); //waktu set data waktu berikutnya
    
    myservo1.attach(8); //kanan kiri
    myservo2.attach(9); //naik turun
    myservo2.attach(10); //maju mundur
    myservo4.attach(11);// //capit(capit lepas)
    myservo5.attach(7); //peniris(buka tutup)
    
}
void loop() //fungsi mengeksekusi program
{
    char buff[BUFF_MAX];  //mendeklarasikan variabel buffer 
    unsigned long now = millis(); //mendeklarasikan variabel waktu (millis adalah delay program menyeluruh)
    struct ts t; //membuat tipe data sendiri
    
    // mengambil data waktu sekarang dan membuat kondisi
    if ((now - prev > interval) && (Serial.available() <= 0)) {
        DS3231_get(&t); //mendapatkan data waktu

        // menampilkan data waktu
        snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", t.year,
             t.mon, t.mday, t.hour, t.min, t.sec); 
        Serial.println(buff); //mencetak data waktu

        // menampilkan a2 debug informasi
        DS3231_get_a2(&buff[0], 59); //mendapatkan waktu menit dan detik
        Serial.println(buff); //mencetak data waktu

        if (DS3231_triggered_a2()) { //jika kondisi waktu a2 terpenuhi
            Serial.println(" -> program peniris telah berjalan");
            myservo2.write(160);      //ke bawah
            delay(2000);
            myservo1.write(10);      //ke kanan
            delay(2000);
            myservo3.write(180);      //maju
            delay(2000);
            myservo4.write(150);      //buka capit
            delay(2000);
            myservo4.write(80);      //capit
            delay(1000);
            myservo2.write(80);      //ke atas (posisi awal)
            delay(1000);
            myservo3.write(80);      //mundur
            delay(1000);
            myservo1.write(90);      //ke kiri (posisi awal)
            delay(1000);
            myservo4.write(150);      //buka capit
            delay(2000);
            waktu_angkat_selanjutnya(); //membuat set waktu yang baru
            // membersihkan set waktu sebelumnya
            DS3231_clear_a2f();          
        }
        prev = now; //kembali ke program awal dibuat sekarang
    }
}
