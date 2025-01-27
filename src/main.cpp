#include <raylib.h>
#include "grid.h"

int main() 
{
    Color mainColor ={222,208,49,87};
    InitWindow(810, 1000 , "2048");

    SetTargetFPS(60);

    //initialisation de la grille
    Grid grid = Grid();
    Grid grid_temp;
    grid.Initialize();
    grid.RandomApparition();
    grid.RandomApparition();
    bool lose=false;
    std::pair<Rectangle, Rectangle> buttons;



    while(WindowShouldClose()==false){

        BeginDrawing();
        ClearBackground(mainColor);

        if(!lose){

            grid.DrawScore();

        if (IsKeyPressed(KEY_LEFT)) {
            grid.MoveLeft(); // Déplace la grille vers la gauche
            if(grid_temp!=grid){
                grid.RandomApparition();
            }
        }

        if (IsKeyPressed(KEY_RIGHT)) {
            grid.MoveRight(); // Déplace la grille vers la gauche
            if(grid_temp!=grid){
                grid.RandomApparition();
            }
        }

        if (IsKeyPressed(KEY_UP)) {
            grid.MoveUp(); // Déplace la grille vers la gauche
            if(grid_temp!=grid){
                grid.RandomApparition();
            }
        }

        if (IsKeyPressed(KEY_DOWN)) {
            grid.MoveDown(); // Déplace la grille vers la gauche
            if(grid_temp!=grid){
                grid.RandomApparition();
            }
        }

        grid.Draw();


        lose = grid.CheckLose();
        grid_temp=grid;
        }
        else{
            grid.HandlePlayerNameInput();
            if (grid.leaderboard.empty()) {
                grid.FetchScoresFromServer();
            }

            buttons = grid.DrawEndPage();
            Vector2 mousePosition = GetMousePosition();

            // Vérifier le clic sur le bouton "Valider"
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePosition, buttons.first)) {
                // Fonction liée au bouton "Valider"
                grid.SubmitScore(); 
                grid.FetchScoresFromServer();

                
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePosition, buttons.second)) {
                // Réinitialiser le jeu
                grid = Grid();
                grid.Initialize();
                grid.RandomApparition();
                grid.RandomApparition();
                lose = false;
            }

        }
        EndDrawing();
    }
    CloseWindow();
}