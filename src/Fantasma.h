#pragma once

#include "Tipos.h"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

// Sprites de um fantasma andando: [direção][quadro], direção na ordem de indiceDirecao()
struct TexturasFantasma {
    sf::Texture andando[4][2];
};

// Sprites iguais para todos os fantasmas
struct TexturasEspeciais {
    sf::Texture assustado[2]; // Azul, com o Pac-Man fortalecido
    sf::Texture piscando[2];  // Branco, nos últimos segundos do fortalecimento
    sf::Texture olhos[4];     // Só os olhos, voltando para a casa depois de comido
};

// Carrega os 8 sprites de assets/img/ghost/<pasta>/ (u1, u2, d1, d2, l1, l2, r1, r2)
void carregarTexturasFantasma(TexturasFantasma& texturas, const std::string& pasta, sf::Color corReserva);

// Carrega os sprites de assustado (ghost/nerf) e de olhos (ghost/dead)
void carregarTexturasEspeciais(TexturasEspeciais& texturas);

// Classe base dos fantasmas. Cada fantasma concreto define o próprio alvo em calcularAlvo().
class Fantasma {
public:
    static constexpr float duracaoAleatorio = 7.0f; // Segundos andando sem rumo
    static constexpr float duracaoSeguir = 20.0f;   // Segundos perseguindo o Pac-Man

    sf::Sprite sprite;            // Posição e colisão; a textura muda a cada frame em desenhar()
    EstadoFantasma estadoAtual = Aleatorio;
    Posicao posicao;              // Em blocos (x = coluna, y = linha)
    Posicao scatterTarget;        // Bloco da casa para onde volta quando é comido
    std::string name;
    bool saiuDaBase = false;
    bool comido = false;          // Virou olhos e está voltando para a casa
    float tempoParaTrocarEstado = duracaoAleatorio;
    Direcao direcao = parado;
    sf::Vector2i proximoBloco;    // Bloco para onde o fantasma está andando
    float tempoAteSaida = 0.0f;
    int quadro = 0;               // Quadro atual da animação (0 ou 1)
    float tempoAnimacao = 0.0f;

    Fantasma(const TexturasFantasma& texturas, const TexturasEspeciais& especiais,
        Posicao startPos, Posicao scatterPos, const std::string& ghostName);
    virtual ~Fantasma() = default;

    virtual Posicao calcularAlvo(const Posicao& pacmanPos, Direcao pacmanDir) = 0;

    // Alterna entre andar sem rumo e perseguir o Pac-Man
    void atualizarModo(float deltaTime);

    // Avança a animação (chamar todo frame, inclusive com o fantasma parado na casa)
    void animar(float deltaTime);

    // Anda bloco a bloco: só escolhe uma nova direção ao chegar no centro de um bloco.
    // Com o Pac-Man fortalecido (assustado = true), anda sem rumo em vez de perseguir.
    // Comido, ignora tudo isso e volta para a casa pelo menor caminho, mais rápido.
    void update(const Posicao& pacmanPos, Direcao pacmanDir, bool assustado, float speed, float deltaTime, const Mapa& mapa);

    // Comido pelo Pac-Man: vira olhos que voltam para a casa e, ao chegar, esperam antes de sair
    void voltarParaCasa();

    bool verificarColisaoParede(const Posicao& posicao, const Mapa& mapa);

    // Primeiro passo do menor caminho (busca em largura) até o bloco livre mais próximo do alvo.
    // Retorna "parado" se o fantasma já está nesse bloco.
    Direcao direcaoParaAlvo(const Mapa& mapa, const Posicao& alvo);

    // Escolhe a direção no centro de um bloco.
    // Perseguindo: segue o menor caminho até o alvo.
    // Senão: sorteia entre os vizinhos livres, sem voltar para trás (a não ser em beco sem saída).
    void escolherNovaDirecao(const Mapa& mapa, const Posicao& alvo, bool perseguir);

    // Escolhe o sprite (normal, assustado, piscando ou olhos) e desenha.
    // tempoRestante: segundos que faltam do fortalecimento do Pac-Man.
    void desenhar(sf::RenderTarget& alvo, bool assustado, float tempoRestante);

private:
    const TexturasFantasma* texturas;
    const TexturasEspeciais* especiais;
};

class Blinky : public Fantasma {
public:
    Blinky(const TexturasFantasma& t, const TexturasEspeciais& e, Posicao startPos, Posicao scatterPos)
        : Fantasma(t, e, startPos, scatterPos, "Blinky") {}
    Posicao calcularAlvo(const Posicao& pacmanPos, Direcao) override;
};

class Pinky : public Fantasma {
public:
    Pinky(const TexturasFantasma& t, const TexturasEspeciais& e, Posicao startPos, Posicao scatterPos)
        : Fantasma(t, e, startPos, scatterPos, "Pinky") {}
    Posicao calcularAlvo(const Posicao& pacmanPos, Direcao pacmanDir) override;
};

class Inky : public Fantasma {
public:
    Posicao blinkyPos;

    Inky(const TexturasFantasma& t, const TexturasEspeciais& e, Posicao startPos, Posicao scatterPos)
        : Fantasma(t, e, startPos, scatterPos, "Inky"), blinkyPos(startPos) {}
    Posicao calcularAlvo(const Posicao& pacmanPos, Direcao pacmanDir) override;
};

class Clyde : public Fantasma {
public:
    Clyde(const TexturasFantasma& t, const TexturasEspeciais& e, Posicao startPos, Posicao scatterPos)
        : Fantasma(t, e, startPos, scatterPos, "Clyde") {}
    Posicao calcularAlvo(const Posicao& pacmanPos, Direcao) override;
};
