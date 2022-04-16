# IOT-WEATHER-STATION
IOT WEATHER STATION

Composant principal utilise

![image](https://user-images.githubusercontent.com/100481752/163039460-736955d9-84e9-4e3e-8c3a-0fb7a97b7b88.png)

Ce projet consiste a l'aide d'un microcontroleur du type esp8266 et de l'editeur d'arduino de faire une station météo connectée.
L'esp8266 a été choisi pour son nombre de output pins et son ananlog pin afin de pouvoir y connecter des capteurs analogiques telle que la résistence lumineuse qui y est presente.
De plus sa configuration permet une connection avec le wifi et le rend donc un IOT accessible à l'aide d'une page web.

Dans un projet futur il est prévu de faire une deuxieme station cette fois destinée a l'interieur avec un ecran afin d'avoir les donnes et de pouvoir la controler sans devoir passer par un ordinateur.

Les composants utilises sont
-une sonde de température immersible pour avoir la température de l'eau 
-une sonde DHT 22 qui elle permet de capter la température ainsi que l'humiditée présente das l'air
-deux relais 3V afin de controler les lumieres au bords de la psicine 
-un module horloge qui permet de suivre l'evolution en fonction du temps et d'aboir l'horaire d'actualisation sur la page web.
-module pour batterie lipo 1S
-2 panneaux solaires ainsi qu'une batterie lipo 3.33V

Version nr1 du PCB
![ESP8266 3](https://user-images.githubusercontent.com/100481752/163677831-deb55a67-20fb-49ff-9623-dbc3a40bbcfd.jpg)

Version nr2 actuelle du PCB

![ESP8266 4](https://user-images.githubusercontent.com/100481752/163677942-a786b236-a562-4644-92dd-3f422a5d41e4.jpg)


Site Internet nr4
![ESP8266 2](https://user-images.githubusercontent.com/100481752/163677844-519bb92c-438b-40aa-b267-ea73e048722a.png)

