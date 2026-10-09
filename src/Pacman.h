#pragma once

#include "Tipos.h"

#include <SFML/Graphics.hpp>
#include <vector>

class Pacman {
public:
    sf::Sprite sprite;
    std::vector<sf::Texture> texturas;
    int vidas;
    float velocidade;
    bool fortalecido;
    float temporizadorFortalecimento;
    sf::Vector2f direcaoAtual;
    int frameAtual;
    float tempoEntreFrames;
    float temporizadorFrame;

    Pacman(float x, float y);

    void atualizarAnimacao(float deltaTempo);

    void moverAutomaticamente(const Mapa& mapa,
        const std::vector<sf::Sprite>& pilulas,
        const std::vector<sf::Sprite>& fantasmas, // Adicionando vetor de fantasmas
        float deltaTempo);

    void atualizarFortalecimento(float deltaTempo);

    void ativarFortalecimento();

    void desenhar(sf::RenderWindow& janela);
};
