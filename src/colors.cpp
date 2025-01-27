#include "colors.h"

const Color blanc = {255,255,255,255};
const Color deux = { 123,222,49,87};
const Color quatre = {196,222,49,87};
const Color huit = {222,187,49,87};
const Color seize = {51,222,49,87};
const Color trenteDeux = {207,222,123,87};
const Color soixanteQuatre = { 255, 222, 49, 87 };
const Color centVingtHuit = { 173, 222, 90, 87 };
const Color deuxCentCinquanteSix = { 222, 140, 49, 87 };
const Color cinqCentDouze = { 135, 222, 49, 87 };
const Color milleVingtQuatre = { 222, 222, 49, 150 };
const Color deuxMilleQuaranteHuit = { 200, 180, 49, 87 };

std::vector<Color> GetCellColors(){
    return {blanc,deux,quatre,huit,seize,trenteDeux,soixanteQuatre,centVingtHuit,deuxCentCinquanteSix,cinqCentDouze,milleVingtQuatre,deuxMilleQuaranteHuit};
}