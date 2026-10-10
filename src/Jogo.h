#pragma once

#include "Config.h"
#include "Fantasma.h"
#include "Labirinto.h"
#include "Mapas.h"
#include "Pacman.h"
#include "Sons.h"
#include "Tipos.h"

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

// Momentos de uma partida, na ordem em que normalmente acontecem
enum class Etapa {
    Pronto,         // "PRONTO!" antes de começar (início, depois de perder vida, nova fase)
    Jogando,
    Morrendo,       // Animação de morte do Pac-Man
    FaseConcluida,  // Labirinto piscando antes da próxima fase
    FimDeJogo       // Vitória ou game over: o jogador digita o nome para o ranking
};

// Uma partida: janela, mapa da fase atual, Pac-Man, fantasmas, itens e pontuação
class Jogo {
public:
    sf::RenderWindow janela;
    sf::Font fonte;                  // Carregada uma vez no construtor
    sf::RenderTexture texturaEscura; // Camada escura do modo Desafio, criada uma vez no construtor
    Labirinto labirinto;             // Imagem das paredes, montada a cada fase
    Sons sons;
    Pacman pacman;
    std::vector<std::unique_ptr<Fantasma>> fantasmas;
    sf::Clock relogioJogo; // No início do jogo
    sf::Texture texturaParede;       // Só para a colisão: as paredes visíveis vêm do Labirinto
    sf::Texture texturaPilula;
    sf::Texture texturaItem;
    sf::Texture texturaApple;
    sf::Texture texturaOrange;
    sf::Texture texturaBeer;
    sf::Texture texturaPilulaFortalecedora;
    TexturasFantasma texturasBlinky, texturasPinky, texturasInky, texturasClyde;
    TexturasEspeciais texturasEspeciais;
    EstadoJogo estadoJogo;
    Etapa etapa = Etapa::Pronto;
    float tempoEtapa = duracaoProntoInicial; // Segundos restantes da etapa atual
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

    std::string nomeDigitado;    // Nome sendo digitado no fim de jogo
    bool nomeConfirmado = false; // Enter: salva no ranking
    bool sairSemSalvar = false;  // Esc: volta ao menu sem salvar

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

    // Avança a partida conforme a etapa atual
    void atualizar(float deltaTempo);

    // Uma atualização da etapa Jogando: movimento, fantasmas, itens e colisões
    void atualizarJogando(float deltaTempo);

    void processarEventos();

    // Registra a partida no ranking com a mesma pontuação mostrada no HUD
    void finalizarJogo(const std::string& nomeJogador);

    // Cria os 4 fantasmas, um em cada célula da casa (valor 4 no mapa)
    void criarFantasmas(const std::vector<Posicao>& casa);

    void executar();

    bool verificarColisaoParede(const sf::FloatRect& objeto);

    void desenhar(sf::Clock& relogioJogo);

    // Pontos, fase, vidas (ícones), tempo, movimentos e som, abaixo do mapa
    void desenharHud();

    // Textos sobre o labirinto: "PRONTO!", fase concluída e o painel de fim de jogo
    void desenharMensagens();

    void dificuldade(NivelDificuldade dificuldade);

private:
    // Texto com contorno preto, centralizado horizontalmente no mapa
    void textoCentralizado(const std::string& texto, unsigned tamanho, float y, sf::Color cor);
};
