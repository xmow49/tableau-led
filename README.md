# Tableau-led



Code pour contoler une matrice de 128*128 pixels avec un RaspberyPi:
- Affiche des GIF
- Modifie la couleur en fonction de la distance devant les 3 capteurs ultrasons
- Une interface Web pour que les utilisateurs puisent changer de GIF, modifier la vitesse et dessiner dessus

Ce projet utilise cette librarie très bien faite qui contient plein d'exemples et de documentations :)
https://github.com/hzeller/rpi-rgb-led-matrix

## Voici un exemple d'affichage

<img src="https://user-images.githubusercontent.com/47485034/171824266-2df660cf-6212-4783-8a58-a7bc5dc3c4a0.jpg" width="200" />

# Installation

**1. Mise à jour du RaspberryPi**
```
sudo apt update -y & sudo apt upgrade -y
```

**2. Installation de WiringPi**

Cette [library](https://github.com/WiringPi/WiringPi) permet de contrôler les GPIO facilement en C++
Comme la library est obsolete on passe par un autre lien

```
wget https://project-downloads.drogon.net/wiringpi-latest.deb
sudo dpkg -i wiringpi-latest.deb
```

Pour tester que c'est correctement installé:
`gpio -v`

**3. Installation de library nécessaires**

```sudo apt-get install libboost-all-dev libgraphicsmagick++-dev libwebp-dev nlohmann-json-dev git -y```

**4. Configuration pour faire fonctioner la Matrice**
a. Désactivation du son

  `sudo nano /boot/config.txt` 
  
  Remplacer `dtparam=audio=on` par `dtparam=audio=off`
    
b. Désinstallation de logiciels qui peuvent rentrer en conflit avec la library des matrices

   ```sudo apt-get remove bluez bluez-firmware pi-bluetooth triggerhappy pigpio```

**5. Téléchagement du programme**

  ```git clone https://github.com/xmow49/tableau-led/```

**6. Redémarrage**

  ```sudo reboot```
  
## Compilation
La compilation se fait avec make.
Dans le dossier src, faites `make` puis attendre la fin de la compilation.

## Utilisation
Pour lancer le programme, executez le fichier `matrix` présent dans le dossier src. Ajouter les paths des gifs comme arguments

`sudo ./matrix ./gifs/loading.gif ./gifs/1.gif ./gifs/2.gif ./gifs/3.gif ./gifs/4.gif` 

Le 1er gif sera affiché comme écran de chargement lors du démmarage.
