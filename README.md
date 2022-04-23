# IOT-WEATHER-STATION
IOT WEATHER STATION

Composant principal utilise
Esp 8266

![image](https://user-images.githubusercontent.com/100481752/163039460-736955d9-84e9-4e3e-8c3a-0fb7a97b7b88.png)

Ce projet consiste a l'aide d'un microcontroleur du type esp8266 et de l'editeur d'arduino de faire une station météo connectée.
L'esp8266 a été choisi pour son nombre de output pins et son ananlog pin afin de pouvoir y connecter des capteurs analogiques telle que la résistence lumineuse qui y est presente.
Un controleur de charge lipo a ete ajoute afin de pouvoir rendre le systeme totalement autonome sans le besoin d'avoir a tirer un cable.
De plus sa configuration permet une connection avec le wifi et le rend donc un IOT accessible à l'aide d'une page web.

Dans un projet futur il est prévu de faire une deuxieme station cette fois destinée a l'interieur avec un ecran afin d'avoir les donnes et de pouvoir la controler sans devoir passer par un ordinateur.

Les composants utilises sont
-une sonde de température immersible pour avoir la température de l'eau 
-une sonde DHT 22 qui elle permet de capter la température ainsi que l'humiditée présente das l'air
-deux relais 3V afin de controler les lumieres au bords de la psicine 
-un module horloge qui permet de suivre l'evolution en fonction du temps et d'aboir l'horaire d'actualisation sur la page web.
-module pour batterie lipo 1S
-2 panneaux solaires ainsi qu'une batterie lipo 3.33V

**Version nr1 du PCB
**
![Version nr 1 petit](https://user-images.githubusercontent.com/100481752/164914854-03cd3560-35c2-4889-9d1a-6df03ef7ed36.jpg)

**Version nr2 actuelle du PCB
**
![Version nr 2 petit](https://user-images.githubusercontent.com/100481752/164914864-561802e5-a83b-4984-b4ef-683b4d0e7c1b.jpg)

**Le tout assemble**

![Montage complet](https://user-images.githubusercontent.com/100481752/164914679-70ed3c28-faa0-4327-bbeb-ed4bfe7086b9.jpg)
**
La station meteo dans son milieu naturel**

![Dehors petit](https://user-images.githubusercontent.com/100481752/164914614-a89cea1f-6970-45ec-8343-6f6a54fde1cf.jpg)



**Site Internet nr4**
Ce site est la version nr4 tu site internet de l'IoT. On remarque une echelle de temperature pour symboliser du froid au chaud.
On retrouve egalement la temperature eterieur ainsi que de l'humidite. On pourrait donc peut etre y ajoouter un ecart relatif. Ceci pourrait a l'aide du stockage de donnes predire la temperature en fonction des temperatures eterieures. Pour finir on a deu boutons pour allumer et forcer le pompage ainsi que l'allumage des lumieres exterieures. 

![ESP8266 2](https://user-images.githubusercontent.com/100481752/163677844-519bb92c-438b-40aa-b267-ea73e048722a.png)

