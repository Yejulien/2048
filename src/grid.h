#pragma once
#include <vector>
#include <raylib.h>
#include <string>
#include <future>

struct PlayerScore {
    std::string pseudo;
    int score;
};

class Grid{
    public:
        Grid();
        void Initialize(); //initialise la grille
        void Print(); //permet si besoin d'afficher l'état de la grille
        void Draw(); //permet de dessiner la grille
        void RandomApparition(); //fait apparaitre aléatoirement un nouvel élément
        void MoveLeft(); //fait bouger la grille à gauche
        void MoveRight(); //fait bouger la grille à droite
        void MoveUp(); //fait bouger la grille en haut
        void MoveDown(); //fait bouger la gille en bas
        int grid[4][4]; 
        Grid& operator=(const Grid& other); // permet de créer la grille_temp qui vérifie si quelque chose a bien été modifié lors du mouvement
        bool operator==(const Grid& other) const;
        bool operator!=(const Grid& other) const;
        bool CheckLose(); //vérifie si la partie est perdu
        void DrawScore(); //
        std::pair<Rectangle, Rectangle> DrawEndPage();
        void SubmitScore();
        void FetchScoresFromServer();
        int score;
        void HandlePlayerNameInput();
        bool errorJoin;
        void UpdateLeaderboard();
        bool isFetchingLeaderboard;
        std::vector<PlayerScore> leaderboard;

    private:
        int numRows;
        int numCols;
        int cellSize;
        std::vector<Color> colors;
        std::string playerName;
};