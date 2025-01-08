# <p align="center">ft_ping</p>

> Ce projet consiste à recoder une partie de la commande traceroute.
>
> Recoder la commande traceroute va être pour vous l'occasion d'approfondir vos connaissances dans le reseau (TCP/IP)
> en faisant le suivit des chemins qu'un paquet IP va emprunter entre deux machines.

## Options supportées

- [ ] `-i --interface DEVICE`: Spécifie une interface réseau avec laquelle opérer.
- [ ] `-f --first TTL`: Commence à partir de TTL (au lieu de commencer à 1).
- [ ] `-m --max-hops MAX_TTL`: Définit le nombre maximal de sauts (TTL), 30 par défaut. 
- [ ] `-n`: Ne résout pas les adresses IP en noms de domaine.
- [ ] `-q --queries NQUERIES`: Définit le nombre de sondes par saut. 3 par défaut.

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
