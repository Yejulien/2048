# 2048 Multijoueur en C++

Projet pour le cours IN204

## 1. Analyse d'usage du logiciel (Use Case Analysis)

### Acteurs identifiés :

- **Joueur** : Interagit avec le jeu via l'interface utilisateur.
- **Système** : Gère la logique du jeu, l'affichage et la communication réseau.

### Cas d'utilisation principaux :

- Démarrer une nouvelle partie.
- Effectuer des déplacements (haut, bas, gauche, droite).
- Affichage du score en temps réel.
- Vérification de l'état de la partie (victoire/défaite).
- Communication des scores avec le serveur.
- Classement en ligne.

### Cas d'utilisation secondaires (évolutifs) :

- Implémentation de niveaux de difficulté.
- Mode de jeu en multijoueur interactif

---

## 2. Analyse fonctionnelle du logiciel

### Modules / Fonctionnalités principales :

#### Front-end (Interface graphique)

- Gestion de la grille 4x4.
- Gestion des entrées utilisateur (clavier).
- Affichage des scores.
- Logique de jeu.

#### Back-end (Serveur)

- Gestion des connexions clients.
- Stockage et récupération des scores.


## 3. Installation et exécution

### Prérequis

#### Front-end (Client)

Librairies requises :

- `raylib`
- `curl`

#### Back-end (Serveur)

Librairies requises :

- `crow`

### Instructions de compilation et d'exécution

#### Front-end (Client)

1. Ouvrir un terminal et se placer à la racine du projet.
2. Exécuter la commande suivante pour ouvrir le projet dans VS Code :
   ```sh
   code main.code-workspace
   ```
3. Appuyer sur la touche `F5` pour exécuter le jeu.

#### Back-end (Serveur)

1. Compiler le serveur avec la commande :
   ```sh
   g++ back.cpp -o server
   ```
2. Lancer le serveur :
   ```sh
   ./server
   ```

---


