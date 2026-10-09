#pragma once

#include "Tipos.h"

#include <SFML/Graphics.hpp>
#include <vector>

class Pacman {
public:
    sf::Sprite sprite;
    std::vector<sf::Texture> texturas;
    int vidas;
    float velocidade;                // Velocidade base, em pixels por segundo
    float multiplicadorTurbo = 1.0f; // Efeito temporário do energético e da cerveja
    float tempoTurbo = 0.0f;         // Segundos restantes do efeito
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

    // Velocidade efetiva (base x turbo), em pixels por segundo
    float velocidadeAtual() const;

    // Multiplica a velocidade por alguns segundos. Não acumula: vale o maior multiplicador ativo
    void ativarTurbo(float multiplicador, float duracao);

    void atualizarTurbo(float deltaTempo);

    void desenhar(sf::RenderWindow& janela);
};
