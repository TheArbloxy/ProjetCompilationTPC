# Projet_Compilation_TPC
Projet de compilation sur le language TPC, un sous-ensemble du langage C.

## Description
Ce projet consiste à construire un compilateur sur le TPC, un sous-ensemble du C, en utilisant le langage NASM x86_64, et un analyseur syntaxique écrit en Yacc / Bison. Il peut être utilisé pour compiler des programmes simples avec une syntaxe réduite du C, et de comprendre comment un compilateur et langage de bas niveau fonctionne en détail.

Le TPC possède une partie du syntaxe du C : variables, fonctions, boucles, conditions, structures. Il doit y avoir un main dans chaque fichier tpc. Il possède aussi quatre fonctions built-in : getint, getchar, putint, putchar, qui sont des entrées / sorties de caractères / entiers.

## Pour commencer

### Requis
* NASM x86-64 sur Linux
* Yacc / Bison
* Python3

### Exécution
Pour compiler le programme de compilation:
```
make
```
Pour compiler le fichier NASM obtenu:
```
make run
```
Pour effacer les fichiers crées par le makefile:
```
make clean
```
Options disponibles:
* Aide `-h`
* Affichage arbre abstrait `-t`
* Affichage table symboles `-s`

Exécution du programme de compilation depuis la racine du projet:
```
./bin/tpcc [OPTIONS...] < fichier.tpc
```
Exécution du fichier NASM depuis la racine du projet:
```
./bin/prog
```

Lancement du script de déploiement des tests:
```
python3 ./src/tests.py
```

## Auteurs
* Phan Tran
* Karl Moukheiber

## Versions
* 0.1
    * Version initiale
 
## Licence
Le projet est sous la licence MIT - voir le fichier LICENCE.md pour plus de détails.
