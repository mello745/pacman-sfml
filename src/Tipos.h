#pragma once

#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <vector>

enum EstadoFantasma { Aleatorio, Seguir };
enum EstadoJogo { Jogando, Vitoria, GameOver };
enum Direcao { parado, cima, baixo, esquerda, direita };
enum NivelDificuldade { Facil, Medio, Dificil, Desafio };
enum ModoJogo { Manual, IA};

// O que cada número dos mapas significa
enum Celula {
    Parede = 0,
    Vazio = 1,               // Corredor sem item
    Pilula = 2,
    PilulaFortalecedora = 3,
    CasaFantasma = 4,        // Onde os fantasmas nascem (o mapa precisa de 4 células)
    InicioPacman = 5,
    Cereja = 6,
    Maca = 7,
    Energetico = 8,
    Cerveja = 9
};

using Mapa = std::vector<std::vector<int>>; // [linha][coluna]; cada valor é uma Celula

struct Posicao {
    float x, y;

    float distanciaAte(const Posicao& other) const {
        return std::sqrt(std::pow(x - other.x, 2) + std::pow(y - other.y, 2));
    }
};

sf::Vector2f converterDirecao(Direcao dir);
Direcao direcaoOposta(Direcao dir);
