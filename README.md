Welcome to the Practica-Mirel-Marineata wiki!
Tema 1.

Tema 1 consta in crearea de firmware pentru un device de iluminat cu led-uri neopixel.
Tehnologii folosite:
1. Esp32-S3 development board.
2. Neopixel led stripe.
3. PlatformIO.
4. Arduino.

Software-ul va fi creat astfel incat:
1. Sa permita conectarea la internet prin Wifi (credentialele vor fi hardcodate in aplicatie pentru inceput)
2. Sa permita setarea tuturor led-urilor cu aceeasi culoare folosind o comanda live prin protocol MQTT.
3. Sa permita setarea unor actiuni dinamice cu urmatoarele caracteristici (stocate in RAM):
    * O actiune poate fi de tip "fade" sau "instant" in cazul unei actiuni "fade" output-ul va varia liniar intre culoarea initiala si culoarea finala definita in actiune intr-un timp dt. In cazul unei actiuni de tip "instant" output-ul va varia brusc dupa trecerea timpului dt. 
4. Sa permita setarea unor actiuni inlantuite denumite scene.
5. Sa permita scrierea scenelor in memoria Flash, citirea lor dupa un id unic si trigger-ul acestora prin MQTT.
6. Sa se conecteze la un server NTP pentru a putea porni scenele la o data/ora predefinita. 
7. Sa permita update over the air (OTA). 
