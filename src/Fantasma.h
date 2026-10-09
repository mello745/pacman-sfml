#pragma once

#include "Tipos.h"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

// Classe base dos fantasmas. Cada fantasma concreto define o próprio alvo em calcularAlvo().
class Fantasma {
public:
    static constexpr float duracaoAleatorio = 7.0f; // Segundos andando sem rumo
    static constexpr float duracaoSeguir = 20.0f;   // Segundos perseguindo o Pac-Man

    sf::Sprite sprite;
    EstadoFantasma estadoAtual = Aleatorio;
    Posicao posicao;              // Em blocos (x = coluna, y = linha)
    Posicao scatterTarget;
    std::string name;
    bool saiuDaBase = false;
    float tempoParaTrocarEstado = duracaoAleatorio;
    Direcao direcao = parado;
    sf::Vector2i proximoBloco;    // Bloco para onde o fantasma está andando
    float tempoAteSaida = 0.0f;

    Fantasma(sf::Texture& texture, Posicao startPos, Posicao scatterPos, const std::string& ghostName);

    virtual Posicao calcularAlvo(const Posicao& pacmanPos, Direcao pacmanDir) = 0;

    // Alterna entre andar sem rumo e perseguir o Pac-Man
    void atualizarModo(float deltaTime);

    // Anda bloco a bloco: só escolhe uma nova direção ao chegar no centro de um bloco.
    // Com o Pac-Man fortalecido (assustado = true), anda sem rumo em vez de perseguir.
    void update(const Posicao& pacmanPos, Direcao pacmanDir, bool assustado, float speed, float deltaTime, const Mapa& mapa);

    // Comido pelo Pac-Man: volta para a casa e espera 5 s antes de sair de novo
    void voltarParaCasa();

    bool verificarColisaoParede(const Posicao& posicao, const Mapa& mapa);

    // Primeiro passo do menor caminho (busca em largura) até o bloco livre mais próximo do alvo.
    // Retorna "parado" se o fantasma já está nesse bloco.
    Direcao direcaoParaAlvo(const Mapa& mapa, const Posicao& alvo);

    // Escolhe a direção no centro de um bloco.
    // Perseguindo: segue o menor caminho até o alvo.
    // Senão: sorteia entre os vizinhos livres, sem voltar para trás (a não ser em beco sem saída).
    void escolherNovaDirecao(const Mapa& mapa, const Posicao& alvo, bool perseguir);

    void desenhar(sf::RenderWindow& janela);
};

class Blinky : public Fantasma {
public:
    Blinky(sf::Texture& texture, Posicao startPos, Posicao scatterPos)
        : Fantasma(texture, startPos, scatterPos, "Blinky") {}
    Posicao calcularAlvo(const Posicao& pacmanPos, Direcao) override;
};

class Pinky : public Fantasma {
public:
    Pinky(sf::Texture& texture, Posicao startPos, Posicao scatterPos)
        : Fantasma(texture, startPos, scatterPos, "Pinky") {}
    Posicao calcularAlvo(const Posicao& pacmanPos, Direcao pacmanDir) override;
};

class Inky : public Fantasma {
public:
    Posicao blinkyPos;

    Inky(sf::Texture& texture, Posicao startPos, Posicao scatterPos)
        : Fantasma(texture, startPos, scatterPos, "Inky"), blinkyPos(startPos) {}
    Posicao calcularAlvo(const Posicao& pacmanPos, Direcao pacmanDir) override;
};

class Clyde : public Fantasma {
public:
    Clyde(sf::Texture& texture, Posicao startPos, Posicao scatterPos)
        : Fantasma(texture, startPos, scatterPos, "Clyde") {}
    Posicao calcularAlvo(const Posicao& pacmanPos, Direcao) override;
};
