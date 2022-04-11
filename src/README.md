# Backend C++

Le code matrix permet de controler la matrice, lire la valeur des capteur, receptioner les ordre de la page web.

## Installation

**1. Mise à jour du RaspberryPi**
```
sudo apt update -y
sudo apt upgrade -y
```

**2. Installation de WiringPi**

Cette [library](https://github.com/WiringPi/WiringPi) permet de controler les GPIO facilement en C++
Comme la library est deprecated on passe par un autre lien

```
wget https://project-downloads.drogon.net/wiringpi-latest.deb
sudo dpkg -i wiringpi-latest.deb
```

Pour tester que c'est correctement installé:
`gpio -v`

**3. Installation de Boost**
`sudo apt-get install libboost-all-dev` 


**4. Configuration pour faire fonctioner la matrice**
a. Désactivation du son
    ```sudo nano /boot/config.txt```
    Remplacer `dtparam=audio=on` par `dtparam=audio=off`
    
b. Désinstallation de logiciel qui peuvent rentrer en conflit avec la library des matrices
    `sudo apt-get remove bluez bluez-firmware pi-bluetooth triggerhappy pigpio`

c. Installation de library nécessaires
    `sudo apt-get install libgraphicsmagick++-dev libwebp-dev nlohmann-json-dev -y`

## Compilation
La compilation se fait avec make.
Dans le dossier src, faites `make` puis attendre la fin de la compilation.

## Utilisation
Pour lancer le programme, executez le fichier `matrix` présent dans le dossier src. Ajouter le path des gifs comme arguments
`sudo ./matrix 1.gif 2.gif 3.gif 4.gif` 