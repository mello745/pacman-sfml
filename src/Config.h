#pragma once

#include <string>

// Tamanhos, velocidades e caminhos usados em todo o jogo

const int tamanhoBloco = 18;
const int larguraJanela = 340;
const int alturaJanela = 500;
const int vidasIniciais = 3;
const float tempoFantasmaVulneravel = 5.0f;
const float escalaPacman = 1.0f;
const float velocidadeFantasma = 2.0f; // Blocos por segundo
const float dtMaximo = 1.0f / 20.0f;   // Maior passo de tempo por frame (em segundos)

const std::string pastaAssets = "assets/";
const std::string caminhoFonte = pastaAssets + "fonts/pixel.ttf";
