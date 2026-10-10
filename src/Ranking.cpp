#include "Ranking.h"

#include "Config.h"
#include "Janela.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

const std::string arquivoRanking = "ranking.txt";

void salvarNoRanking(const jogador& partida) {
    std::ofstream file(arquivoRanking, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Erro ao salvar o ranking!" << std::endl;
        return;
    }
    file << partida.nome << " " << partida.pontos << " " << partida.tempo << std::endl;
}

std::vector<jogador> carregarRanking() {
    std::vector<jogador> ranking;
    std::ifstream file(arquivoRanking);
    if (!file.is_open()) {
        return ranking; // Ainda não há partidas salvas
    }

    std::string nome;
    int pontos;
    time_t tempo;
    while (file >> nome >> pontos >> tempo) {
        if (nome.empty() || pontos < 0 || tempo < 0) {
            std::cerr << "Erro nos dados do ranking: "
                << "nome=" << nome << ", pontos=" << pontos << ", tempo=" << tempo << std::endl;
            continue;
        }
        ranking.push_back({ nome, pontos, tempo });
    }
    return ranking;
}

void exibirRanking() {
    std::vector<jogador> ranking = carregarRanking();
    std::stable_sort(ranking.begin(), ranking.end(), [](const jogador& a, const jogador& b) {
        return a.pontos > b.pontos;
        });

    const unsigned largura = 850, altura = 600;
    sf::RenderWindow window;
    abrirJanela(window, { largura, altura }, "Ranking");
    sf::Font font;
    if (!font.loadFromFile(caminhoFonte)) {
        std::cerr << "Erro ao carregar a fonte!" << std::endl;
        return;
    }

    // Colunas da tabela
    const float xPos = 70, xNome = 160, xPontos = 560, xData = 600;
    const float yCabecalho = 105, yPrimeiraLinha = 140, alturaLinha = 27;
    const int linhasVisiveis = 14;
    int primeiraLinhaVisivel = 0;

    const sf::Color cinza(130, 130, 130);
    const sf::Color coresPodio[3] = { sf::Color(255, 215, 0), sf::Color(200, 200, 210), sf::Color(205, 127, 50) };

    auto escrever = [&](const std::string& texto, unsigned tamanho, float x, float y, sf::Color cor, bool alinharDireita = false) {
        sf::Text t(texto, font, tamanho);
        t.setFillColor(cor);
        float px = alinharDireita ? x - t.getLocalBounds().width : x;
        t.setPosition(std::round(px), std::round(y));
        window.draw(t);
        return t.getLocalBounds().width;
    };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (tratarEventoDeJanela(window, event, { largura, altura }, "Ranking")) continue;
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                window.close(); // Volta ao menu
            }

            // Controle de rolagem (teclado e roda do mouse)
            int rolar = 0;
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Up) rolar = -1;
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Down) rolar = 1;
            if (event.type == sf::Event::MouseWheelScrolled) rolar = event.mouseWheelScroll.delta > 0 ? -1 : 1;
            if (rolar < 0 && primeiraLinhaVisivel > 0) primeiraLinhaVisivel--;
            if (rolar > 0 && primeiraLinhaVisivel + linhasVisiveis < static_cast<int>(ranking.size())) primeiraLinhaVisivel++;
        }

        window.clear();

        float larguraTitulo = sf::Text("RANKING", font, 40).getLocalBounds().width;
        escrever("RANKING", 40, (largura - larguraTitulo) / 2.0f, 30, corDestaque);

        if (ranking.empty()) {
            float l = sf::Text("Nenhuma partida registrada ainda", font, 20).getLocalBounds().width;
            escrever("Nenhuma partida registrada ainda", 20, (largura - l) / 2.0f, 200, sf::Color::White);
        }
        else {
            escrever("POS", 14, xPos, yCabecalho, cinza);
            escrever("NOME", 14, xNome, yCabecalho, cinza);
            escrever("PONTOS", 14, xPontos, yCabecalho, cinza, true);
            escrever("DATA", 14, xData, yCabecalho, cinza);
        }

        for (int i = 0; i < linhasVisiveis; i++) {
            int index = primeiraLinhaVisivel + i;
            if (index >= static_cast<int>(ranking.size())) break;
            float y = yPrimeiraLinha + i * alturaLinha;

            // Linhas alternadas com fundo levemente azulado
            if (index % 2 == 0) {
                sf::RectangleShape fundo({ largura - 100.0f, alturaLinha });
                fundo.setPosition(50, y - 4);
                fundo.setFillColor(sf::Color(18, 18, 50));
                window.draw(fundo);
            }

            // Formatar tempo como data legível (dd/mm/aaaa hh:mm)
            std::ostringstream data;
            data << std::put_time(std::localtime(&ranking[index].tempo), "%d/%m/%Y %H:%M");

            // No arquivo os espaços do nome viram "_"; na tela voltam a ser espaços
            std::string nome = ranking[index].nome;
            std::replace(nome.begin(), nome.end(), '_', ' ');

            sf::Color cor = index < 3 ? coresPodio[index] : sf::Color::White;
            escrever(std::to_string(index + 1), 18, xPos, y, cor);
            escrever(nome, 18, xNome, y, cor);
            escrever(std::to_string(ranking[index].pontos), 18, xPontos, y, cor, true);
            escrever(data.str(), 18, xData, y, cinza);
        }

        // Rodapé: posição da rolagem e atalhos
        if (static_cast<int>(ranking.size()) > linhasVisiveis) {
            int ultima = std::min(primeiraLinhaVisivel + linhasVisiveis, static_cast<int>(ranking.size()));
            escrever(std::to_string(primeiraLinhaVisivel + 1) + "-" + std::to_string(ultima) + " de " + std::to_string(ranking.size()),
                12, largura - 50.0f, 545, cinza, true);
        }
        float l = sf::Text("SETAS ou RODA DO MOUSE rolam   ESC volta   F11 tela cheia", font, 12).getLocalBounds().width;
        escrever("SETAS ou RODA DO MOUSE rolam   ESC volta   F11 tela cheia", 12, (largura - l) / 2.0f, 568, sf::Color(110, 110, 110));

        window.display();
    }
}
