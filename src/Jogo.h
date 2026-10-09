#pragma once

#include "Config.h"
#include "Fantasma.h"
#include "Mapas.h"
#include "Pacman.h"
#include "Tipos.h"

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

// Uma partida: janela, mapa da fase atual, Pac-Man, fantasmas, itens e pontuação
class Jogo {
public:
    sf::RenderWindow janela;
    sf::Font fonte;                  // Carregada uma vez no construtor
    sf::RenderTexture texturaEscura; // Camada escura do modo Desafio, criada uma vez no construtor
    Pacman pacman;
    std::vector<std::unique_ptr<Fantasma>> fantasmas;
    sf::Clock relogioJogo; // No início do jogo
    sf::Texture texturaParede;
    sf::Texture texturaPilula;
    sf::Texture texturaItem;
    sf::Texture texturaApple;
    sf::Texture texturaOrange;
    sf::Texture texturaBeer;
    sf::Texture texturaPilulaFortalecedora;
    sf::Texture texturaBlinky, texturaPinky, texturaInky, texturaClyde;
    EstadoJogo estadoJogo;
    sf::Vector2f proximaDirecao;
    int pontos = 0;

    int maxVidas = 5; // Limite superior de vidas permitido
    float velocidadeFantasmaAtual = velocidadeFantasma; // Blocos por segundo; muda com a dificuldade
    Mapa mapa;
    std::vector<sf::Sprite> paredes;
    std::vector<sf::Sprite> pilulas;
    std::vector<sf::Sprite> item;
    std::vector<sf::Sprite> apple;
    std::vector<sf::Sprite> orange;
    std::vector<sf::Sprite> beer;
    std::vector<sf::Sprite> pilulasFortalecedoras;
    int faseAtual = 0;
    int movimentos = 0;
    NivelDificuldade dificuldadeAtual = Facil;
    ModoJogo modoAtual = Manual; // Inicialmente, o modo é Manual

    std::vector<Mapa> mapas = mapasPadrao();

    Jogo();

    bool todasAsFrutasColetadas();

    void inicializarFase(int indiceFase);

    void criarParedes();

    void criarPilulas();

    void definirPosicoesIniciais();

    void criarItem1();

    void criarItem2();

    int calcularPontuacao();

    Direcao converterParaDirecao(const sf::Vector2f& vetor);

    void atualizar(float deltaTempo);

    void processarEventos();

    // Registra a partida no ranking com a mesma pontuação mostrada no HUD
    void finalizarJogo(const std::string& nomeJogador);

    // Cria os 4 fantasmas, um em cada célula da casa (valor 4 no mapa)
    void criarFantasmas(const std::vector<Posicao>& casa);

    void executar();

    bool verificarColisaoParede(const sf::FloatRect& objeto);

    void exibirMensagemTransicao(const std::string& mensagem);

    void exibirMensagem(const std::string& mensagem);

    void desenhar(sf::Clock& relogioJogo);

    void dificuldade(NivelDificuldade dificuldade);

};
