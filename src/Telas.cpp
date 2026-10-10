#include "Telas.h"

#include "Config.h"
#include "Recursos.h"

#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

const unsigned larguraTela = 850, alturaTela = 600;

struct Opcao {
    std::string texto;
    std::string descricao; // Mostrada embaixo quando a opção está selecionada
    sf::Color cor;         // Cor da opção selecionada
};

// Pac-Man e os 4 fantasmas atravessando a tela, como enfeite do menu
class Desfile {
public:
    Desfile() {
        for (int i = 0; i < 3; ++i) carregarTextura(pacman[i], "img/pacman/" + std::to_string(i) + ".png", sf::Color::Yellow);
        const std::string nomes[4] = { "blinky", "pinky", "inky", "clyde" };
        for (int f = 0; f < 4; ++f)
            for (int q = 0; q < 2; ++q)
                carregarTextura(fantasmas[f][q], "img/ghost/" + nomes[f] + "/r" + std::to_string(q + 1) + ".png", sf::Color::White);
    }

    void desenhar(sf::RenderTarget& alvo, float segundos, float y) const {
        const float escala = 2.0f, espaco = 44.0f, largura = larguraTela + 6 * espaco;
        float x = std::fmod(segundos * 120.0f, largura) - 5 * espaco; // Entra pela esquerda e sai pela direita
        int boca = static_cast<int>(segundos * 12) % 4;
        const int sequencia[4] = { 0, 1, 2, 1 };

        sf::Sprite p(pacman[sequencia[boca]]);
        p.setScale(escala, escala);
        p.setPosition(x + 4 * espaco, y);
        alvo.draw(p);

        int quadro = static_cast<int>(segundos * 7) % 2;
        for (int f = 0; f < 4; ++f) {
            sf::Sprite s(fantasmas[f][quadro]);
            s.setScale(escala, escala);
            s.setPosition(x + (3 - f) * espaco - 10.0f, y);
            alvo.draw(s);
        }
    }

    sf::Texture pacman[3];
    sf::Texture fantasmas[4][2];
};

void desenharCentralizado(sf::RenderTarget& alvo, const sf::Font& fonte, const std::string& texto,
    unsigned tamanho, float y, sf::Color cor) {
    sf::Text t(texto, fonte, tamanho);
    t.setFillColor(cor);
    t.setPosition(std::round((larguraTela - t.getLocalBounds().width) / 2.0f), y);
    alvo.draw(t);
}

// Tela de opções no estilo do jogo: logo (ou título), lista navegável com o mouse (passar por cima
// destaca, clicar escolhe) ou com o teclado (setas + Enter).
// Retorna o índice escolhido, ou -1 se a janela for fechada (ou Esc, fora do menu principal).
int telaDeOpcoes(const std::string& tituloJanela, const std::string& titulo,
    const std::vector<Opcao>& opcoes, bool menuPrincipal) {
    sf::RenderWindow janela(sf::VideoMode(larguraTela, alturaTela), tituloJanela);
    janela.setFramerateLimit(60);

    sf::Font fonte;
    if (!fonte.loadFromFile(caminhoFonte)) {
        std::cerr << "Erro ao carregar a fonte!" << std::endl;
        return -1;
    }

    sf::Texture logo;
    bool temLogo = menuPrincipal && logo.loadFromFile(pastaAssets + "img/pacman_logo.png");
    logo.setSmooth(true);
    Desfile desfile;
    sf::Clock relogio;

    const float yOpcoes = menuPrincipal ? 270.0f : 170.0f, espaco = 62.0f;
    const sf::Color cinza(150, 150, 150);
    int selecionada = 0;

    // O Windows às vezes manda um "mouse se moveu" sem movimento (por exemplo, quando a janela
    // ganha foco). Só uma posição nova do mouse muda a seleção, para não desfazer a do teclado.
    sf::Vector2i ultimoMouse = sf::Mouse::getPosition(janela);

    // Área clicável de cada opção: a faixa inteira da linha, não só as letras
    auto opcaoEm = [&](float x, float y) {
        for (int i = 0; i < static_cast<int>(opcoes.size()); ++i) {
            float topo = yOpcoes + i * espaco - 8.0f;
            if (x >= 200 && x <= larguraTela - 200 && y >= topo && y < topo + espaco - 10.0f) return i;
        }
        return -1;
    };

    while (janela.isOpen()) {
        sf::Event evento;
        while (janela.pollEvent(evento)) {
            if (evento.type == sf::Event::Closed) return -1;

            if (evento.type == sf::Event::KeyPressed) {
                switch (evento.key.code) {
                case sf::Keyboard::Up:
                    selecionada = (selecionada + static_cast<int>(opcoes.size()) - 1) % static_cast<int>(opcoes.size());
                    break;
                case sf::Keyboard::Down:
                    selecionada = (selecionada + 1) % static_cast<int>(opcoes.size());
                    break;
                case sf::Keyboard::Enter:
                case sf::Keyboard::Space:
                    return selecionada;
                case sf::Keyboard::Escape:
                    if (!menuPrincipal) return -1; // No menu principal, Esc não fecha o jogo
                    break;
                default:
                    break;
                }
            }

            if (evento.type == sf::Event::MouseMoved) {
                sf::Vector2i agora(evento.mouseMove.x, evento.mouseMove.y);
                if (agora != ultimoMouse) {
                    ultimoMouse = agora;
                    int i = opcaoEm(static_cast<float>(agora.x), static_cast<float>(agora.y));
                    if (i >= 0) selecionada = i;
                }
            }

            if (evento.type == sf::Event::MouseButtonPressed && evento.mouseButton.button == sf::Mouse::Left) {
                int i = opcaoEm(static_cast<float>(evento.mouseButton.x), static_cast<float>(evento.mouseButton.y));
                if (i >= 0) return i;
            }
        }

        float segundos = relogio.getElapsedTime().asSeconds();
        janela.clear();

        // Logo (menu principal) ou título
        if (temLogo) {
            sf::Sprite s(logo);
            float escala = 560.0f / logo.getSize().x;
            s.setScale(escala, escala);
            s.setPosition((larguraTela - 560.0f) / 2.0f, 30.0f);
            janela.draw(s);
            desfile.desenhar(janela, segundos, 200.0f);
        }
        else {
            desenharCentralizado(janela, fonte, titulo, 44, 60.0f, corDestaque);
        }

        // Opções; a selecionada ganha cor e o Pac-Man ao lado
        for (int i = 0; i < static_cast<int>(opcoes.size()); ++i) {
            bool ativa = i == selecionada;
            sf::Text t(opcoes[i].texto, fonte, 30);
            t.setFillColor(ativa ? opcoes[i].cor : cinza);
            float x = std::round((larguraTela - t.getLocalBounds().width) / 2.0f);
            float y = yOpcoes + i * espaco;
            t.setPosition(x, y);
            janela.draw(t);

            if (ativa) {
                const int sequencia[4] = { 0, 1, 2, 1 };
                sf::Sprite cursor(desfile.pacman[sequencia[static_cast<int>(segundos * 12) % 4]]);
                cursor.setScale(2.0f, 2.0f);
                cursor.setPosition(x - 50.0f, y + 2.0f);
                janela.draw(cursor);
            }
        }

        desenharCentralizado(janela, fonte, opcoes[selecionada].descricao, 16, 525.0f, sf::Color(200, 200, 200));
        desenharCentralizado(janela, fonte, menuPrincipal ? "SETAS + ENTER ou MOUSE" : "SETAS + ENTER ou MOUSE   ESC volta",
            12, 568.0f, sf::Color(110, 110, 110));
        janela.display();
    }
    return -1;
}

} // namespace

std::optional<NivelDificuldade> escolherDificuldade() {
    const std::vector<Opcao> opcoes = {
        { "FACIL", "5 vidas - fantasmas lentos (2,0 blocos/s)", sf::Color::Green },
        { "MEDIO", "3 vidas - fantasmas a 2,4 blocos/s - bonus de 15 pontos", corDestaque },
        { "DIFICIL", "2 vidas - fantasmas a 2,8 blocos/s - bonus de 30 pontos", sf::Color::Red },
        { "DESAFIO", "1 vida - tela escura - bonus de 50 pontos", sf::Color::Magenta },
    };
    const NivelDificuldade niveis[] = { Facil, Medio, Dificil, Desafio };

    int escolha = telaDeOpcoes("DIFICULDADE", "DIFICULDADE", opcoes, false);
    if (escolha < 0) return std::nullopt; // Volta ao menu sem escolher
    return niveis[escolha];
}

OpcaoMenu menu()
{
    const std::vector<Opcao> opcoes = {
        { "JOGAR", "Escolha a dificuldade e jogue com as setas", corDestaque },
        { "MODO IA", "Assista o Pac-Man jogar sozinho", sf::Color::Cyan },
        { "RANKING", "As melhores pontuacoes", sf::Color(255, 184, 255) },
        { "SAIR", "Ate a proxima!", sf::Color(255, 184, 82) },
    };
    const OpcaoMenu resultado[] = { OpcaoMenu::Jogar, OpcaoMenu::IA, OpcaoMenu::Ranking, OpcaoMenu::Sair };

    int escolha = telaDeOpcoes("MENU", "PAC-MAN", opcoes, true);
    return escolha < 0 ? OpcaoMenu::Sair : resultado[escolha];
}
