#include "grid.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "colors.h"
#include <string>
#include "client.h"
#include <future>
#include "raylib.h"
#include <curl/curl.h>

Grid::Grid()
{
    numRows = 4;
    score = 0;
    numCols = 4;
    cellSize = 200;
    Initialize();
    colors = GetCellColors();
    playerName = "Your_Name";
    errorJoin = false;
    isFetchingLeaderboard = false;
}

void Grid::Initialize()
{
    for (int row = 0; row < numRows; row++)
    {
        for (int column = 0; column < numCols; column++)
        {
            grid[row][column] = 0;
        }
    }
}

void Grid::SubmitScore()
{
    // Construire la requête HTTP
    std::string request = "http://bun.bun.ovh:8080/save_score?name=" + playerName + "&score=" + std::to_string(score);
    printf("Sending request: %s\n", request.c_str());

    // Envoyer la requête de manière asynchrone
    std::future<std::string> response = std::async(std::launch::async, sendRequest, request);

    // Attendre la réponse
    response.wait();

    // Vérifier la validité de la réponse
    if (!response.valid())
    {
        errorJoin = true; // Erreur si la réponse est invalide
        return;
    }

    // Récupérer la réponse sous forme de chaîne
    std::string response_str = response.get();

    // Vérifier si la réponse est "Score saved successfully."
    if (response_str != "Score saved successfully.")
    {
        errorJoin = true; // Erreur si le message attendu n'est pas reçu
    }
    else
    {
        errorJoin = false; // Réinitialiser l'erreur si tout est correct
    }
}

void Grid::FetchScoresFromServer()
{
    std::string request = "http://bun.bun.ovh:8080/scores";
    std::future<std::string> response = std::async(std::launch::async, sendRequest, request);

    response.wait(); // Attendre la réponse du serveur

    if (response.valid())
    {
        try
        {
            std::string responseStr = response.get();

            // Vérifier que la réponse commence et finit avec des accolades
            if (responseStr.front() != '{' || responseStr.back() != '}')
            {
                throw std::runtime_error("Invalid JSON response");
            }

            // Nettoyer et remplir le leaderboard
            leaderboard.clear();

            // Trouver le tableau "scores"
            std::size_t scoresStart = responseStr.find("\"scores\":");
            if (scoresStart == std::string::npos)
            {
                throw std::runtime_error("Missing 'scores' key in JSON");
            }

            // Trouver les crochets [ ]
            scoresStart = responseStr.find('[', scoresStart);
            std::size_t scoresEnd = responseStr.find(']', scoresStart);
            if (scoresStart == std::string::npos || scoresEnd == std::string::npos)
            {
                throw std::runtime_error("Invalid or missing 'scores' array");
            }

            std::string scoresArray = responseStr.substr(scoresStart + 1, scoresEnd - scoresStart - 1);

            // Diviser les entrées dans le tableau
            std::size_t entryStart = 0;
            while (entryStart < scoresArray.size())
            {
                // Trouver un objet { ... }
                entryStart = scoresArray.find('{', entryStart);
                std::size_t entryEnd = scoresArray.find('}', entryStart);
                if (entryStart == std::string::npos || entryEnd == std::string::npos)
                    break;

                std::string entry = scoresArray.substr(entryStart + 1, entryEnd - entryStart - 1);

                // Extraire le pseudo et le score
                PlayerScore player;
                std::size_t namePos = entry.find("\"name\":");
                std::size_t scorePos = entry.find("\"score\":");
                if (namePos == std::string::npos || scorePos == std::string::npos)
                {
                    throw std::runtime_error("Invalid entry in 'scores' array");
                }

                // Extraire le pseudo
                namePos = entry.find('"', namePos + 7) + 1; // Trouver le début du pseudo
                std::size_t nameEnd = entry.find('"', namePos);
                player.pseudo = entry.substr(namePos, nameEnd - namePos);

                // Extraire le score
                scorePos = entry.find(':', scorePos) + 1; // Trouver le début du score
                std::size_t scoreEnd = entry.find(',', scorePos);
                if (scoreEnd == std::string::npos)
                    scoreEnd = entry.size();
                player.score = std::stoi(entry.substr(scorePos, scoreEnd - scorePos));

                leaderboard.push_back(player);
                entryStart = entryEnd + 1; // Passer à l'entrée suivante
            }
        }
        catch (const std::exception &e)
        {
            // Gérer les erreurs lors de la récupération ou du parsing
            printf("Erreur lors du chargement des scores : %s\n", e.what());
        }
    }
    else
    {
        printf("Erreur : pas de réponse valide du serveur.\n");
    }
}

void Grid::Print()
{
    for (int row = 0; row < numRows; row++)
    {
        for (int column = 0; column < numCols; column++)
        {
            std::cout << grid[row][column] << " ";
        }
        std::cout << std::endl;
    }
}

void Grid::Draw()
{
    for (int row = 0; row < numRows; row++)
    {
        for (int column = 0; column < numCols; column++)
        {
            int cellValue = grid[row][column];

            // Dessiner le fond de la cellule
            DrawRectangle(column * cellSize + 10, row * cellSize + 200, cellSize - 10, cellSize - 10,
                          colors[(cellValue > 0) ? (int)(log(cellValue) / log(2)) : 0]);

            // Afficher le texte de la valeur de la cellule
            if (cellValue != 0)
            {
                std::string text = std::to_string(cellValue);

                // Mesurer la largeur et la hauteur du texte
                int textWidth = MeasureText(text.c_str(), 40); // Taille de police 40
                int textHeight = 40;                           // La hauteur de la police est généralement la taille donnée

                // Calculer les positions pour centrer le texte
                int textX = column * cellSize + (cellSize - textWidth) / 2 + 10;
                int textY = row * cellSize + 200 + (cellSize - textHeight) / 2;

                // Dessiner le texte centré dans la cellule
                DrawText(text.c_str(), textX, textY, 40, BLACK);
            }
        }
    }
}

void Grid::RandomApparition()
{
    std::vector<std::pair<int, int>> emptyCells;

    for (int row = 0; row < numRows; row++)
    {
        for (int column = 0; column < numCols; column++)
        {
            if (grid[row][column] == 0)
            {
                emptyCells.emplace_back(row, column);
            }
        }
    }

    if (emptyCells.empty())
    {
        std::cout << "Aucune case vide disponible !" << std::endl;
        return;
    }

    srand(static_cast<unsigned>(time(0)));
    int randomIndex = rand() % emptyCells.size();
    auto [row, col] = emptyCells[randomIndex];
    int value = (rand() % 10 < 9) ? 2 : 4; // 90% pour 2, 10% pour 4

    grid[row][col] = value;
    std::cout << "Ajouté " << value << " à la position (" << row << ", " << col << ")" << std::endl;
}

void Grid::MoveLeft()
{
    for (int row = 0; row < numRows; row++)
    {
        // Étape 1 : Déplacer toutes les valeurs vers la gauche
        for (int col = 0; col < numCols; col++)
        {
            for (int k = col + 1; k < numCols; k++)
            {
                if (grid[row][k] != 0 && grid[row][col] == 0)
                {
                    grid[row][col] = grid[row][k];
                    grid[row][k] = 0;
                    break;
                }
            }
        }

        // Étape 2 : Fusionner les cellules adjacentes identiques
        for (int col = 0; col < numCols - 1; col++)
        {
            if (grid[row][col] != 0 && grid[row][col] == grid[row][col + 1])
            {
                grid[row][col] *= 2;    // Fusionner les valeurs
                grid[row][col + 1] = 0; // Vider la cellule fusionnée
            }
        }

        // Étape 3 : Redéplacer toutes les valeurs vers la gauche après la fusion
        for (int col = 0; col < numCols; col++)
        {
            for (int k = col + 1; k < numCols; k++)
            {
                if (grid[row][k] != 0 && grid[row][col] == 0)
                {
                    grid[row][col] = grid[row][k];
                    grid[row][k] = 0;
                    break;
                }
            }
        }
    }
}

void Grid::MoveRight()
{
    for (int row = 0; row < numRows; row++)
    {
        // Étape 1 : Déplacer toutes les valeurs vers la droite
        for (int col = numCols - 1; col >= 0; col--)
        {
            for (int k = col - 1; k >= 0; k--)
            {
                if (grid[row][k] != 0 && grid[row][col] == 0)
                {
                    grid[row][col] = grid[row][k];
                    grid[row][k] = 0;
                    break;
                }
            }
        }

        // Étape 2 : Fusionner les cellules adjacentes identiques
        for (int col = numCols - 1; col > 0; col--)
        {
            if (grid[row][col] != 0 && grid[row][col] == grid[row][col - 1])
            {
                grid[row][col] *= 2;    // Fusionner les valeurs
                grid[row][col - 1] = 0; // Vider la cellule fusionnée
            }
        }

        // Étape 3 : Redéplacer toutes les valeurs vers la droite après la fusion
        for (int col = numCols - 1; col >= 0; col--)
        {
            for (int k = col - 1; k >= 0; k--)
            {
                if (grid[row][k] != 0 && grid[row][col] == 0)
                {
                    grid[row][col] = grid[row][k];
                    grid[row][k] = 0;
                    break;
                }
            }
        }
    }
}

void Grid::MoveUp()
{
    for (int col = 0; col < numCols; col++)
    {
        // Étape 1 : Déplacer toutes les valeurs vers le haut
        for (int row = 0; row < numRows; row++)
        {
            for (int k = row + 1; k < numRows; k++)
            {
                if (grid[k][col] != 0 && grid[row][col] == 0)
                {
                    grid[row][col] = grid[k][col];
                    grid[k][col] = 0;
                    break;
                }
            }
        }

        // Étape 2 : Fusionner les cellules adjacentes identiques
        for (int row = 0; row < numRows - 1; row++)
        {
            if (grid[row][col] != 0 && grid[row][col] == grid[row + 1][col])
            {
                grid[row][col] *= 2;    // Fusionner les valeurs
                grid[row + 1][col] = 0; // Vider la cellule fusionnée
            }
        }

        // Étape 3 : Redéplacer toutes les valeurs vers le haut après la fusion
        for (int row = 0; row < numRows; row++)
        {
            for (int k = row + 1; k < numRows; k++)
            {
                if (grid[k][col] != 0 && grid[row][col] == 0)
                {
                    grid[row][col] = grid[k][col];
                    grid[k][col] = 0;
                    break;
                }
            }
        }
    }
}

void Grid::MoveDown()
{
    for (int col = 0; col < numCols; col++)
    {
        // Étape 1 : Déplacer toutes les valeurs vers le bas
        for (int row = numRows - 1; row >= 0; row--)
        {
            for (int k = row - 1; k >= 0; k--)
            {
                if (grid[k][col] != 0 && grid[row][col] == 0)
                {
                    grid[row][col] = grid[k][col];
                    grid[k][col] = 0;
                    break;
                }
            }
        }

        // Étape 2 : Fusionner les cellules adjacentes identiques
        for (int row = numRows - 1; row > 0; row--)
        {
            if (grid[row][col] != 0 && grid[row][col] == grid[row - 1][col])
            {
                grid[row][col] *= 2;    // Fusionner les valeurs
                grid[row - 1][col] = 0; // Vider la cellule fusionnée
            }
        }

        // Étape 3 : Redéplacer toutes les valeurs vers le bas après la fusion
        for (int row = numRows - 1; row >= 0; row--)
        {
            for (int k = row - 1; k >= 0; k--)
            {
                if (grid[k][col] != 0 && grid[row][col] == 0)
                {
                    grid[row][col] = grid[k][col];
                    grid[k][col] = 0;
                    break;
                }
            }
        }
    }
}

Grid &Grid::operator=(const Grid &other)
{
    // Vérifier l'auto-affectation
    if (this != &other)
    {
        // Copier les tailles et autres variables
        numRows = other.numRows;
        numCols = other.numCols;
        cellSize = other.cellSize;
        colors = other.colors;

        // Copier la grille (tableau 2D)
        for (int i = 0; i < numRows; i++)
        {
            for (int j = 0; j < numCols; j++)
            {
                grid[i][j] = other.grid[i][j];
            }
        }
    }

    // Retourner *this pour permettre les affectations en chaîne
    return *this;
}

bool Grid::operator==(const Grid &other) const
{
    // Comparer les tailles des grilles
    if (numRows != other.numRows || numCols != other.numCols)
    {
        return false;
    }

    // Comparer chaque élément du tableau 2D
    for (int i = 0; i < numRows; i++)
    {
        for (int j = 0; j < numCols; j++)
        {
            if (grid[i][j] != other.grid[i][j])
            {
                return false; // Si une case est différente, les grilles ne sont pas égales
            }
        }
    }

    // Si on arrive ici, les grilles sont égales
    return true;
}

bool Grid::operator!=(const Grid &other) const
{
    // Si l'opérateur == retourne false, cela signifie que les grilles sont égales,
    // donc on retourne l'inverse de ==.
    return !(*this == other);
}

bool Grid::CheckLose()
{
    // Vérifier s'il reste une case vide
    for (int row = 0; row < numRows; row++)
    {
        for (int column = 0; column < numCols; column++)
        {
            if (grid[row][column] == 0)
            {
                return false; // Il reste une case vide, pas encore perdu
            }
        }
    }

    // Vérifier s'il existe des fusions possibles
    for (int row = 0; row < numRows; row++)
    {
        for (int column = 0; column < numCols; column++)
        {
            // Vérifier fusion possible vers la droite
            if (column + 1 < numCols && grid[row][column] == grid[row][column + 1])
            {
                return false;
            }

            // Vérifier fusion possible vers le bas
            if (row + 1 < numRows && grid[row][column] == grid[row + 1][column])
            {
                return false;
            }
        }
    }

    // Si aucune case vide et aucune fusion possible, la partie est perdue
    return true;
}

void Grid::DrawScore()
{
    score = 0;
    // Calculer le score
    for (int row = 0; row < numRows; row++)
    {
        for (int column = 0; column < numCols; column++)
        {
            score += grid[row][column];
        }
    }

    // Dessiner le fond blanc pour la zone du score
    DrawRectangle(0, 0, numCols * cellSize + 10, 190, WHITE);

    // Convertir le score en texte
    std::string scoreText = "Score: " + std::to_string(score);

    // Calculer la largeur du texte pour le centrer
    int textWidth = MeasureText(scoreText.c_str(), 100); // Taille de police 50
    int textX = (numCols * cellSize - textWidth) / 2;    // Position X centrée
    int textY = (190 - 100) / 2;                         // Position Y centrée dans la zone de 190px

    // Dessiner le texte du score
    DrawText(scoreText.c_str(), textX, textY, 100, BLACK);
}

std::pair<Rectangle, Rectangle> Grid::DrawEndPage()
{
    // Partie du haut : Texte "Perdu", score et champ pour le pseudo
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight() / 3, LIGHTGRAY);
    DrawText("PERDU", GetScreenWidth() / 2 - MeasureText("PERDU", 50) / 2, 50, 50, RED);
    DrawText(("Score: " + std::to_string(score)).c_str(),
             GetScreenWidth() / 2 - MeasureText(("Score: " + std::to_string(score)).c_str(), 30) / 2,
             120, 30, BLACK);

    // Champ pour le pseudo
    DrawText("Entrez votre pseudo:", 100, 180, 20, BLACK);
    Rectangle pseudoInputRect = {350, 170, 200, 30};
    DrawRectangleRec(pseudoInputRect, WHITE);
    DrawRectangleLinesEx(pseudoInputRect, 2, DARKGRAY);
    DrawText(playerName.c_str(), pseudoInputRect.x + 10, pseudoInputRect.y + 5, 20, BLACK);

    // Bouton pour valider
    Rectangle validateButtonRect = {350 + pseudoInputRect.width + 20, 170, 100, 30};
    DrawRectangleRec(validateButtonRect, LIGHTGRAY);
    DrawRectangleLinesEx(validateButtonRect, 2, DARKGRAY);
    DrawText("Valider", validateButtonRect.x + (validateButtonRect.width - MeasureText("Valider", 20)) / 2,
             validateButtonRect.y + (validateButtonRect.height - 20) / 2, 20, DARKGRAY);

    // Partie du milieu : Tableau des scores ou animation de chargement
    int middleStartY = pseudoInputRect.y + pseudoInputRect.height + 20; // Ajuster pour être juste sous "Entrez votre pseudo"
    int middleEndY = GetScreenHeight() - 150;                           // Laisser de la place pour le bouton "Restart"
    int middleHeight = middleEndY - middleStartY;

    DrawRectangle(0, middleStartY, GetScreenWidth(), middleHeight, ORANGE);

    if (isFetchingLeaderboard)
    {
        DrawText("Chargement des scores...", 100, middleStartY + 20, 20, DARKGRAY);

        // Dessiner une animation simple (par exemple, un quart de cercle tournant)
        static float angle = 0.0f;
        angle += 5.0f;
        DrawCircleSector({static_cast<float>(GetScreenWidth()) / 2, static_cast<float>(middleStartY + 80)},
                         30.0f, angle, angle + 270, 10, RED);
    }
    else if (leaderboard.empty())
    {
        DrawText("Aucun score disponible.", 100, middleStartY + 20, 20, DARKGRAY);
    }
    else
    {
        DrawText("Leaderboard:", 100, middleStartY + 20, 20, BLACK);

        // Calcul de la hauteur dynamique pour chaque entrée
        int startY = middleStartY + 50;
        int availableHeight = middleHeight - 50; // Espace restant après le titre
        int entryHeight = availableHeight / (leaderboard.size() > 0 ? leaderboard.size() : 1);

        // S'assurer que la hauteur des entrées est raisonnable
        entryHeight = std::min(entryHeight, 40); // Limite maximale pour éviter des entrées trop grandes
        for (size_t i = 0; i < leaderboard.size(); ++i)
        {
            std::string text = leaderboard[i].pseudo + ": " + std::to_string(leaderboard[i].score);
            DrawText(text.c_str(), 120, startY + i * entryHeight, 20, DARKGRAY);
        }
    }

    // Partie du bas : Bouton "Restart"
    Rectangle restartButtonRect = {
        static_cast<float>(GetScreenWidth()) / 2 - 100.0f,
        static_cast<float>(GetScreenHeight()) - 100.0f,
        200.0f, 50.0f};
    DrawRectangleRec(restartButtonRect, LIGHTGRAY);
    DrawRectangleLinesEx(restartButtonRect, 2, DARKGRAY);
    DrawText("Restart", restartButtonRect.x + (restartButtonRect.width - MeasureText("Restart", 30)) / 2,
             restartButtonRect.y + (restartButtonRect.height - 30) / 2, 30, DARKGRAY);

    return {validateButtonRect, restartButtonRect};
}

void Grid::HandlePlayerNameInput()
{
    int key = GetCharPressed();

    // Si une touche est pressée
    while (key > 0)
    {
        // Vérifiez si la touche est un caractère affichable (pas un espace) et si la longueur du pseudo est inférieure à 13
        if ((key >= 32 && key <= 126) && key != ' ' && playerName.length() < 13)
        {
            playerName += static_cast<char>(key);
        }

        // Reprend la lecture de la prochaine touche
        key = GetCharPressed();
    }

    // Gestion de la suppression avec BACKSPACE
    if (IsKeyPressed(KEY_BACKSPACE) && !playerName.empty())
    {
        playerName.pop_back();
    }
}
