#include "crow.h"
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <mutex>

// Structure pour stocker les scores
struct ScoreEntry {
    std::string name;
    int score;
};

// Mutex pour protéger les opérations sur le fichier
std::mutex fileMutex;

// Chemin vers le fichier des scores
const std::string scoreFile = "scores.txt";

// Fonction pour charger les scores depuis le fichier
std::vector<ScoreEntry> loadScores() {
    std::lock_guard<std::mutex> lock(fileMutex);
    std::vector<ScoreEntry> scores;
    std::ifstream file(scoreFile);
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string name;
        int score;
        if (iss >> name >> score) {
            scores.push_back({name, score});
        }
    }

     // Trier les scores en ordre décroissant
    std::sort(scores.begin(), scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
        return b.score < a.score; // Comparaison pour ordre décroissant
    });

    // Limiter à 10 scores maximum
    if (scores.size() > 10) {
        scores.resize(10);
    }

    return scores;
}

// Fonction pour sauvegarder un score dans le fichier
void saveScore(const std::string& name, int score) {
    //vérifier si ce joueur a déjà un score
    auto scores = loadScores();
    for (auto& entry : scores) {
        if (entry.name == name) {
            if (entry.score < score) {
                entry.score = score;
            }
            return;
        }
    }
    std::lock_guard<std::mutex> lock(fileMutex);
    std::ofstream file(scoreFile, std::ios::app); // Append mode
    if (file.is_open()) {
        file << name << " " << score << "\n";
    }
}

// Point d'entrée principal
int main() {
    crow::SimpleApp app;

    // Route pour récupérer les scores : GET /scores
    CROW_ROUTE(app, "/scores").methods("GET"_method)([]() {
    auto scores = loadScores();
    crow::json::wvalue result;

    // Construire une liste de scores
    std::vector<crow::json::wvalue> scoreList;
    for (const auto& entry : scores) {
        crow::json::wvalue scoreItem;
        scoreItem["name"] = entry.name;
        scoreItem["score"] = entry.score;
        scoreList.push_back(scoreItem);
    }

    result["scores"] = std::move(scoreList); // Ajouter la liste au résultat
    printf("Scores fetched\n");
    return crow::response(result);
});

    // Route pour sauvegarder un score : GET /save_score?name=thename&score=lescore
    CROW_ROUTE(app, "/save_score").methods("GET"_method)([](const crow::request& req) {
        auto name = req.url_params.get("name");
        auto score = req.url_params.get("score");
        printf("name: %s, score: %s\n", name, score);

        if (!name || !score) {
            return crow::response(400, "Missing 'name' or 'score' parameter.");
        }

        try {
            int scoreValue = std::stoi(score);
            saveScore(name, scoreValue);
            return crow::response(200, "Score saved successfully.");
        } catch (...) {
            return crow::response(400, "Invalid score value.");
        }
    });

    // Lancer le serveur sur le port 8080
    app.port(8080).multithreaded().run();

    return 0;
}
