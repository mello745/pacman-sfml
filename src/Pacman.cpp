#include "Pacman.h"

#include "Config.h"
#include "Recursos.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <string>

Pacman::Pacman(float x, float y)
    : vidas(vidasIniciais), velocidade(90.0f), fortalecido(false),
    temporizadorFortalecimento(0.0f), frameAtual(0), tempoEntreFrames(0.1f), temporizadorFrame(0.0f) {

    // Carregar texturas (3 frames: 0.png, 1.png, 2.png)
    for (int i = 0; i < 3; ++i) {
        sf::Texture texture;
        carregarTextura(texture, "img/pacman/" + std::to_string(i) + ".png", sf::Color::Yellow);
        texturas.push_back(texture);
    }

    sprite.setTexture(texturas[0]);
    sprite.setScale(escalaPacman, escalaPacman);
    sprite.setPosition(x, y);
    direcaoAtual = sf::Vector2f(1.0f, 0.0f); // Inicialmente movendo para a direita
}

void Pacman::atualizarAnimacao(float deltaTempo) {
    temporizadorFrame += deltaTempo;
    if (temporizadorFrame >= tempoEntreFrames) {
        temporizadorFrame = 0.0f;
        frameAtual = (frameAtual + 1) % texturas.size(); // Alterna entre todos os frames
        sprite.setTexture(texturas[frameAtual]);
    }
}

void Pacman::moverAutomaticamente(const Mapa& mapa,
    const std::vector<sf::Sprite>& pilulas,
    const std::vector<sf::Sprite>& fantasmas, // Adicionando vetor de fantasmas
    float deltaTempo) {
    // Obter posição atual do Pac-Man no grid
    sf::Vector2i posicaoPacman(
        static_cast<int>(sprite.getPosition().x / tamanhoBloco),
        static_cast<int>(sprite.getPosition().y / tamanhoBloco)
    );

    // Verificar se Pac-Man está no centro de um bloco
    sf::Vector2f posicaoAtual = sprite.getPosition();
    bool noCentroDoBloco =
        static_cast<int>(posicaoAtual.x) % tamanhoBloco == 0 &&
        static_cast<int>(posicaoAtual.y) % tamanhoBloco == 0;

    if (noCentroDoBloco) {
        // Encontrar a pílula mais próxima
        sf::Vector2i destino = posicaoPacman;
        float menorDistancia = std::numeric_limits<float>::max();

        for (const auto& pilula : pilulas) {
            sf::Vector2i posicaoPilula(
                static_cast<int>(pilula.getPosition().x / tamanhoBloco),
                static_cast<int>(pilula.getPosition().y / tamanhoBloco)
            );

            float distancia = std::hypot(
                posicaoPacman.x - posicaoPilula.x,
                posicaoPacman.y - posicaoPilula.y
            );

            if (distancia < menorDistancia) {
                menorDistancia = distancia;
                destino = posicaoPilula;
            }
        }

        // Avaliar todas as direções válidas e encontrar a melhor
        sf::Vector2i melhorDirecao = { 0, 0 };
        menorDistancia = std::numeric_limits<float>::max();
        std::vector<sf::Vector2i> direcoes = {
            {0, -1},  // Cima
            {0, 1},   // Baixo
            {-1, 0},  // Esquerda
            {1, 0}    // Direita
        };

        for (const auto& direcao : direcoes) {
            sf::Vector2i novaPosicao = posicaoPacman + direcao;

            // Verificar se a nova posição está dentro do mapa
            if (novaPosicao.y < 0 || novaPosicao.y >= static_cast<int>(mapa.size()) ||
                novaPosicao.x < 0 || novaPosicao.x >= static_cast<int>(mapa[0].size())) {
                continue;
            }

            // Verificar se a nova posição não é uma parede
            if (mapa[novaPosicao.y][novaPosicao.x] == Parede) {
                continue;
            }

            // Verificar se a nova posição não é ocupada por um fantasma
            bool colidiuComFantasma = false;
            for (const auto& fantasma : fantasmas) {
                sf::FloatRect rectFantasma = fantasma.getGlobalBounds();
                if (rectFantasma.contains(novaPosicao.x * tamanhoBloco, novaPosicao.y * tamanhoBloco)) {
                    colidiuComFantasma = true;
                    break;
                }
            }

            if (colidiuComFantasma) {
                continue; // Evitar a direção que leva ao fantasma
            }

            // Calcular a distância até o destino (pílula)
            float distancia = std::hypot(
                destino.x - novaPosicao.x,
                destino.y - novaPosicao.y
            );

            // Escolher a direção que minimiza a distância e é válida
            if (distancia < menorDistancia) {
                menorDistancia = distancia;
                melhorDirecao = direcao;
            }
        }

        // Atualizar a direção apenas se uma direção válida for encontrada
        if (melhorDirecao != sf::Vector2i(0, 0)) {
            direcaoAtual = sf::Vector2f(melhorDirecao.x, melhorDirecao.y);
        }
        else {
            std::cout << "Nenhuma direção válida encontrada.\n";
        }
    }

    // Mover Pac-Man na direção atual
    sf::Vector2f movimento = direcaoAtual * (velocidade * deltaTempo);
    sf::FloatRect novaPosicaoPacman = sprite.getGlobalBounds();
    novaPosicaoPacman.left += movimento.x;
    novaPosicaoPacman.top += movimento.y;

    // Verificar colisão antes de mover
    sf::Vector2i posicaoGridNova(
        static_cast<int>(novaPosicaoPacman.left / tamanhoBloco),
        static_cast<int>(novaPosicaoPacman.top / tamanhoBloco)
    );

    // Verificar se a posição nova está válida e não contém paredes
    if (posicaoGridNova.y >= 0 && posicaoGridNova.y < static_cast<int>(mapa.size()) &&
        posicaoGridNova.x >= 0 && posicaoGridNova.x < static_cast<int>(mapa[0].size()) &&
        mapa[posicaoGridNova.y][posicaoGridNova.x] != Parede) {

        // Verificar se a nova posição colide com algum fantasma
        for (const auto& fantasma : fantasmas) {
            if (fantasma.getGlobalBounds().contains(novaPosicaoPacman.left, novaPosicaoPacman.top)) {
                std::cout << "Colisão com fantasma detectada. Movimento bloqueado.\n";
                direcaoAtual = sf::Vector2f(0.0f, 0.0f); // Parar em caso de colisão com fantasma
                return;
            }
        }

        sprite.move(movimento);
    }
    else {
        std::cout << "Colisão com parede detectada. Movimento bloqueado.\n";
        direcaoAtual = sf::Vector2f(0.0f, 0.0f); // Parar em caso de colisão
    }
}

void Pacman::atualizarFortalecimento(float deltaTempo) {
    if (fortalecido) {
        temporizadorFortalecimento -= deltaTempo;
        if (temporizadorFortalecimento <= 0) {
            fortalecido = false;
            sprite.setColor(sf::Color::White); // Volta à cor normal
        }
    }
}

void Pacman::ativarFortalecimento() {
    fortalecido = true;
    temporizadorFortalecimento = tempoFantasmaVulneravel;
    sprite.setColor(sf::Color::Green); // Indica fortalecimento visualmente
}

void Pacman::desenhar(sf::RenderWindow& janela) {
    janela.draw(sprite);
}
