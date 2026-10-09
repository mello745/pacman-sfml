#include "Ranking.h"

#include "Config.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cctype>
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

std::string lerNomeJogador() {
    std::cout << "Digite o nome do jogador: ";
    std::string nome;
    std::getline(std::cin >> std::ws, nome);
    nome.erase(nome.find_last_not_of(" \t\r") + 1);
    std::replace_if(nome.begin(), nome.end(), [](unsigned char c) { return std::isspace(c); }, '_');
    return nome.empty() ? "Jogador" : nome;
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
    std::sort(ranking.begin(), ranking.end(), [](const jogador& a, const jogador& b) {
        return a.pontos > b.pontos;
        });

    sf::RenderWindow window(sf::VideoMode(1200, 800), "Ranking");
    window.setFramerateLimit(60);
    sf::Font font;
    if (!font.loadFromFile(caminhoFonte)) {
        std::cerr << "Erro ao carregar a fonte!" << std::endl;
        return;
    }

    sf::Text titulo("======== RANKING ========", font, 30);
    titulo.setPosition((window.getSize().x - titulo.getLocalBounds().width) / 2, 75);

    sf::Text text("", font, 20);
    int posicao = 1;

    // Variáveis para controle de rolagem
    const int linhasVisiveis = 20; // Número de linhas que podem ser exibidas na tela
    int primeiraLinhaVisivel = 0; // Índice da primeira linha visível

    window.clear();
    window.draw(titulo);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                window.close(); // Volta ao menu
            }

            // Controle de rolagem
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    if (primeiraLinhaVisivel > 0) {
                        primeiraLinhaVisivel--; // Rolando para cima
                    }
                }
                else if (event.key.code == sf::Keyboard::Down) {
                    if (primeiraLinhaVisivel + linhasVisiveis < static_cast<int>(ranking.size())) {
                        primeiraLinhaVisivel++; // Rolando para baixo
                    }
                }
            }
        }

        // Desenhar as linhas do ranking visíveis
        window.clear(); // Limpa a janela a cada iteração
        window.draw(titulo);

        if (ranking.empty()) {
            text.setString("Nenhuma partida registrada ainda");
            text.setPosition((window.getSize().x - text.getLocalBounds().width) / 2, 150);
            window.draw(text);
        }

        for (int i = 0; i < linhasVisiveis; i++) {
            int index = primeiraLinhaVisivel + i;
            if (index < ranking.size()) { // Verifica se o índice está dentro dos limites
                // Formatar tempo como data legível (dd/mm/aaaa hh:mm)
                std::ostringstream data;
                data << std::put_time(std::localtime(&ranking[index].tempo), "%d/%m/%Y %H:%M");

                std::string info = "Pos: " + std::to_string(index + 1) + " | " +
                    "Jogador: " + ranking[index].nome + " | " +
                    "Pontos: " + std::to_string(ranking[index].pontos) + " | " +
                    "Data: " + data.str();

                text.setString(info);
                text.setPosition((window.getSize().x - text.getLocalBounds().width) / 2, 150 + i * 30);
                window.draw(text);
            }
        }

        window.display();
    }
}
