#include "Tipos.h"

sf::Vector2f converterDirecao(Direcao dir) {
    switch (dir) {
    case cima: return { 0, -1 };
    case baixo: return { 0, 1 };
    case esquerda: return { -1, 0 };
    case direita: return { 1, 0 };
    default: return { 0, 0 };
    }
}

Direcao direcaoOposta(Direcao dir) {
    switch (dir) {
    case cima: return baixo;
    case baixo: return cima;
    case esquerda: return direita;
    case direita: return esquerda;
    default: return parado;
    }
}
