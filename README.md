## IOT-Weather-Station 


#### Le projet:
Ce projet consiste à l'aide d'un microcontrôleur du type esp8266 et de l'éditeur Arduino de faire une station météo connectée.
L'esp8266 a été choisi pour son nombre d'outputs pins et son analog pin afin de pouvoir y connecter des capteurs analogiques tels que la résistance lumineuse qui y est présente.
Un contrôleur de charge lipo a été ajouté afin de pouvoir rendre le système totalement autonome sans le besoin d'avoir à tirer un câble.
De plus, sa configuration permet une connexion avec le wifi et le rend donc un IoT accessible à l'aide d'une page web.

Dans un projet futur, il est prévu de faire une deuxième station, cette fois destinée à l'intérieur avec un écran afin d'avoir les donnes et de pouvoir la contrôler sans devoir passer par un ordinateur.


Le composant principal utilise dans ce projet est l'esp 8266:                                                     
Voici son schématique et la configuration utilisé

![image](https://user-images.githubusercontent.com/100481752/163039460-736955d9-84e9-4e3e-8c3a-0fb7a97b7b88.png)

# Les composants

Les composants utilisés sont
- une sonde de température immersible pour avoir la température de l'eau 
- une sonde DHT 22 qui elle permet de capter la température ainsi que l'humidité présente dans l'air
- deux relais 3V afin de contrôler les lumières aux bords de la piscine 
- un module horloge qui permet de suivre l'évolution en fonction du temps et d'avoir l'horaire d'actualisation sur la page web.
- module pour batterie lipo 1S
- 2 panneaux solaires ainsi qu'une batterie lipo 3.33V

A ajouter pour le futur:
- un voltmetre
- un ecran sur la station qui affiche la connectivitee

## **Version nr1 du PCB**

![Version nr 1 petit](https://user-images.githubusercontent.com/100481752/164914854-03cd3560-35c2-4889-9d1a-6df03ef7ed36.jpg)

### <ins> Version nr2 actuelle du PCB :</ins>  

![Version nr 2 petit](https://user-images.githubusercontent.com/100481752/164914864-561802e5-a83b-4984-b4ef-683b4d0e7c1b.jpg)

### <ins>Le tout assemble :</ins> 

![Montage complet](https://user-images.githubusercontent.com/100481752/164914679-70ed3c28-faa0-4327-bbeb-ed4bfe7086b9.jpg)

### <ins>La station meteo dans son milieu naturel :</ins>

![Dehors petit](https://user-images.githubusercontent.com/100481752/164914614-a89cea1f-6970-45ec-8343-6f6a54fde1cf.jpg)

## **Site Internet nr4 :**

Ce site est la version nr4 tu site internet de l'IoT. On remarque une échelle de température pour symboliser du froid au chaud.
On retrouve également la température extérieure ainsi que de l'humidité. On pourrait donc peut-être y ajouter un écart relatif. Ceci pourrait à l'aide du stockage de donnes prédire la température en fonction des températures extérieures. Pour finir on a deux boutons pour allumer et forcer le pompage ainsi que l'allumage des lumières extérieures. 

![ESP8266 2](https://user-images.githubusercontent.com/100481752/163677844-519bb92c-438b-40aa-b267-ea73e048722a.png)

