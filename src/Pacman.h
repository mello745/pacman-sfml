#pragma once

#include "Tipos.h"

#include <SFML/Graphics.hpp>
#include <vector>

class Pacman {
public:
    sf::Sprite sprite;
    std::vector<sf::Texture> texturas;      // Boca: 0 fechada, 1 meio aberta, 2 aberta
    std::vector<sf::Texture> texturasMorte; // Animação de morte (11 quadros)
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
    int passoAnimacao = 0;           // Posição na sequência de quadros 0-1-2-1
    bool movendo = false;            // A boca só anima enquanto o Pac-Man anda (o Jogo atualiza)
    float rotacao = 0.0f;            // Última direção desenhada, em graus

    Pacman(float x, float y);

    // Abre e fecha a boca (0-1-2-1...) enquanto o Pac-Man está se movendo
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

    // Desenha centralizado no bloco e virado para a direção do movimento.
    // Só a imagem gira: o sprite usado na colisão continua igual.
    void desenhar(sf::RenderTarget& alvo);

    // Desenha a animação de morte; progresso vai de 0 (início) a 1 (fim)
    void desenharMorte(sf::RenderTarget& alvo, float progresso);
};
