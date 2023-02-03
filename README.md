# Tableau-led



Code pour contrôler une matrice de 128*128 pixels avec un RaspberyPi:
- Affiche des GIF
- Modifie la couleur en fonction de la distance entre les 3 capteurs ultrasons et le spectateur
- Une interface Web pour que les utilisateurs puissent changer d'animation, modifier la vitesse et dessiner sur le tableau

Ce projet utilise cette bibliothèque très bien faite qui contient plein d'exemples et de documentation :-)
https://github.com/hzeller/rpi-rgb-led-matrix

## Voici un exemple d'affichage

<img src="https://user-images.githubusercontent.com/47485034/171824266-2df660cf-6212-4783-8a58-a7bc5dc3c4a0.jpg" width="200" />

# Installation

**1. Mise à jour du RaspberryPi**
```
sudo apt update -y && sudo apt upgrade -y
```

**2. Installation de WiringPi**

Cette [bibliothèque](https://github.com/WiringPi/WiringPi) permet de contrôler les GPIO facilement en C++
Comme la bibliothèque est obsolète on passe par un autre lien

```
wget https://project-downloads.drogon.net/wiringpi-latest.deb
sudo dpkg -i wiringpi-latest.deb
```

Pour tester que s'est correctement installé:
`gpio -v`

**3. Installation deq librairies nécessaires**

```sudo apt-get install libboost-all-dev libgraphicsmagick++-dev libwebp-dev nlohmann-json-dev rapidjson-dev git -y```

**4. Configuration pour faire fonctioner la Matrice**
a. Désactivation du son

  `sudo nano /boot/config.txt` 
  
  Remplacer `dtparam=audio=on` par `dtparam=audio=off`
    
b. Désinstallation des logiciels qui peuvent rentrer en conflit avec la librairie des matrices

   ```sudo apt-get remove bluez bluez-firmware pi-bluetooth triggerhappy pigpio```

**5. Téléchagement du programme**

  ```git clone https://github.com/xmow49/tableau-led/```

**6. Redémarrage**

  ```sudo reboot```
  
## Compilation
La compilation se fait avec make.
Dans le dossier src, faite `make` puis attendre la fin de la compilation.

## Utilisation
Pour lancer le programme, executer le fichier `matrix` présent dans le dossier src. Ajouter les paths des gifs comme arguments

`sudo ./matrix ./gifs/loading.gif ./gifs/1.gif ./gifs/2.gif ./gifs/3.gif ./gifs/4.gif` 

Le premier gif sera affiché comme écran de chargement lors du démmarage.

## Installation du service
Le service permet de démmarer/arreter le programme avec un commande et de démmarrer le programme tous seul au boot du PI.

1. Insallation du service

```
cd service
./setup-service.sh
```

Maintenant vous pouvez démmarer les programme avec:

```
sudo systemctl start matrix
sudo systemctl start server-matrix
```

Et les arreter avec

```
sudo systemctl stop matrix
sudo systemctl stop server-matrix
```


