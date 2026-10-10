#pragma once

#include <SFML/Graphics/Color.hpp>
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
const int distanciaSeguraIA = 2;       // Modo IA: blocos de distância que o Pac-Man mantém dos fantasmas
const float dtMaximo = 1.0f / 20.0f;   // Maior passo de tempo por frame (em segundos)

// Animações e telas (em segundos)
const float tempoQuadroFantasma = 0.15f;  // Troca de quadro dos fantasmas
const float tempoQuadroPacman = 0.06f;    // Troca de quadro da boca do Pac-Man
const float avisoFimFortalecimento = 2.0f; // Nos últimos segundos do fortalecimento, os fantasmas piscam
const float velocidadeOlhos = 6.0f;       // Blocos por segundo dos olhos voltando para a casa
const float esperaAposComido = 5.0f;      // Tempo na casa depois de voltar como olhos
const float duracaoProntoInicial = 4.2f;  // "PRONTO!" do início (duração da música de abertura)
const float duracaoPronto = 2.0f;         // "PRONTO!" depois de perder vida ou trocar de fase
const float duracaoMorte = 1.5f;          // Animação de morte do Pac-Man
const float duracaoFaseConcluida = 2.0f;  // Labirinto piscando ao terminar a fase
const int tamanhoMaximoNome = 12;         // Caracteres do nome no ranking

const sf::Color corParede(33, 33, 222);
const sf::Color corDestaque(255, 220, 0); // Amarelo dos textos em destaque

const std::string pastaAssets = "assets/";
const std::string caminhoFonte = pastaAssets + "fonts/pixel.ttf";
