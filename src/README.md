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
    Comme la library est deprecated on passe par un autre lien:

    ```
    wget https://project-downloads.drogon.net/wiringpi-latest.deb
    sudo dpkg -i wiringpi-latest.deb
    ```
    
    Pour tester que c'est correctement installé:
    ```gpio -v```

**3. Configuration pour faire fonctioner la matrice**
