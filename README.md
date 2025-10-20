# Awalé

## Sommaire

- [Installation](#installation)
	- [Prérequis](#prérequis)
	- [Clonage](#clonage)
	- [Compilation basique](#compilation-basique)
	- [Options de compilation](#options-de-compilation)
	- [Exécution](#exécution)
- [Manuel utilisateur](#manuel-utilisateur)
- [Ressources](#ressources)

## Installation

### Prérequis

Avant de compiler et d'exécuter le programme, assurez-vous d'avoir :

- Un compilateur C (`gcc` ou `clang`)
- `make`

### Clonage

```sh
git clone https://github.com/Yaacoub/4IF-PR.git
cd 4IF-PR
```

### Compilation basique

Compiler en mode par défaut :
```sh
make
```

Nettoyer les fichiers objets et l'exécutable :
```sh
make clean
```

### Options de compilation

Le Makefile proposé expose les variables suivantes que vous pouvez redéfinir en ligne de commande :
 - `CC` : compilateur C (`gcc` par défaut)
 - `CFLAGS` : options de compilation (`-Wall -Wextra -std=c11 -O2 -g` par défaut)

Compiler en mode debug :

```sh
make CFLAGS="-Wall -Wextra -std=c11 -O0 -g"
```

Utiliser `clang` au lieu de `gcc` :

```sh
make CC=clang
```

### Exécution

Après compilation :

```sh
./awale
```

## Manuel utilisateur

...

## Ressources

- [Awalé](https://fr.wikipedia.org/wiki/Awalé), Wikipedia

