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
    sf::Vector2i proximoBloco;       // Modo IA: bloco para onde está andando
    int frameAtual;
    float tempoEntreFrames;
    float temporizadorFrame;

    Pacman(float x, float y);

    void atualizarAnimacao(float deltaTempo);

    // Modo IA: anda bloco a bloco; em cada bloco escolhe a direção com escolherDirecaoIA()
    void moverAutomaticamente(const Mapa& mapa,
        const std::vector<sf::Vector2i>& blocosComPilula,
        const std::vector<sf::Vector2i>& blocosFantasmas,
        float deltaTempo);

    // Primeiro passo do menor caminho até a pílula mais próxima, evitando os blocos perto dos
    // fantasmas. Se não houver caminho seguro, foge para o vizinho mais longe deles.
    Direcao escolherDirecaoIA(const Mapa& mapa, sf::Vector2i atual,
        const std::vector<sf::Vector2i>& blocosComPilula,
        const std::vector<sf::Vector2i>& blocosFantasmas);

    void atualizarFortalecimento(float deltaTempo);

    void ativarFortalecimento();

    // Velocidade efetiva (base x turbo), em pixels por segundo
    float velocidadeAtual() const;

    // Multiplica a velocidade por alguns segundos. Não acumula: vale o maior multiplicador ativo
    void ativarTurbo(float multiplicador, float duracao);

    void atualizarTurbo(float deltaTempo);

    void desenhar(sf::RenderWindow& janela);
};
