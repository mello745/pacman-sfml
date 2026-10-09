#pragma once

#include <string>

// Tamanhos, velocidades e caminhos usados em todo o jogo

const int tamanhoBloco = 18;
const int colunasMapa = 19;
const int linhasMapa = 21;
const int larguraJanela = colunasMapa * tamanhoBloco; // 342: exatamente a largura do mapa
const int alturaMapa = linhasMapa * tamanhoBloco;     // 378: o HUD fica logo abaixo
const int alturaJanela = 500;
const int vidasIniciais = 3;
const float tempoFantasmaVulneravel = 5.0f;
const float escalaPacman = 1.0f;
const float velocidadeFantasma = 2.0f; // Blocos por segundo (o padrão; muda com a dificuldade)
const float velocidadePacman = 60.0f;  // Pixels por segundo (cerca de 3,3 blocos/s)
const float duracaoTurbo = 5.0f;       // Segundos de efeito do energético e da cerveja
const float dtMaximo = 1.0f / 20.0f;   // Maior passo de tempo por frame (em segundos)

const std::string pastaAssets = "assets/";
const std::string caminhoFonte = pastaAssets + "fonts/pixel.ttf";
