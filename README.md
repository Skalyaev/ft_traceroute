# <p align="center">ft_traceroute</p>

> Ce projet consiste à recoder une partie de la commande traceroute.
>
> Recoder la commande traceroute va être pour vous l'occasion d'approfondir vos connaissances dans le reseau (TCP/IP)
> en faisant le suivit des chemins qu'un paquet IP va emprunter entre deux machines.

## Options supportées

- [x] `-F --dont-fragment`: Définit le bit DF dans le paquet IP.
- [x] `-f --first TTL`: Commence à partir de TTL (au lieu de commencer à 1).
- [x] `-i --interface DEVICE`: Spécifie une interface réseau avec laquelle opérer.
- [x] `-m --max-hops MAX_TTL`: Définit le nombre maximal de sauts (TTL), 30 par défaut.
- [x] `-N --sim-queries NQUERIES`: Définit le nombre de sondes simultanées. 16 par défaut.
- [x] `-n`: Ne résout pas les adresses IP en noms de domaine.
- [x] `-p --port PORT`: Définit le port de destination.
- [x] `-w --wait TIME`: Définit le temps d'attente pour chaque sonde en secondes. 5 par défaut.
- [x] `-q --queries NQUERIES`: Définit le nombre de sondes par saut. 3 par défaut.

## Install

```bash
mkdir -p ~/.local/src
mkdir -p ~/.local/bin

apt update -y
apt install -y git
apt install -y make
apt install -y gcc
```

```bash
cd ~/.local/src
git clone https://github.com/Skalyaev/ft_traceroute.git
cd ft_traceroute && make

ln -s $PWD/ft_traceroute ~/.local/bin/ft_traceroute
export PATH=~/.local/bin:$PATH

ft_traceroute --help
```
