#include "Telas.h"

#include "Config.h"

#include <SFML/Graphics.hpp>
#include <iostream>

std::optional<NivelDificuldade> escolherDificuldade() {
    sf::RenderWindow window(sf::VideoMode(600, 500), "DIFICULDADE");
    window.setFramerateLimit(60);
    sf::Font font;

    if (!font.loadFromFile(caminhoFonte)) {
        std::cout << "Erro ao carregar a fonte!" << std::endl;
        return std::nullopt;
    }

    sf::Text titulo("DIFICULDADE", font, 40);
    sf::Text subtitulo("Escolha a dificuldade", font, 10);
    sf::Text opcao1("FACIL", font, 25);
    sf::Text opcao2("MEDIO", font, 25);
    sf::Text opcao3("DIFICIL", font, 25);
    sf::Text desafio("DESAFIO", font, 25);

    titulo.setPosition((window.getSize().x - titulo.getLocalBounds().width) / 2, 75);
    subtitulo.setPosition((window.getSize().x - subtitulo.getLocalBounds().width) / 2, 150);
    opcao1.setPosition((window.getSize().x - opcao1.getLocalBounds().width) / 2, 200);
    opcao2.setPosition((window.getSize().x - opcao2.getLocalBounds().width) / 2, 270);
    opcao3.setPosition((window.getSize().x - opcao3.getLocalBounds().width) / 2, 340);
    desafio.setPosition((window.getSize().x - desafio.getLocalBounds().width) / 2, 410);

    titulo.setFillColor(sf::Color::White);
    subtitulo.setFillColor(sf::Color::Cyan);
    opcao1.setFillColor(sf::Color::Green);
    opcao2.setFillColor(sf::Color::Yellow);
    opcao3.setFillColor(sf::Color::Red);
    desafio.setFillColor(sf::Color::Magenta);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                return std::nullopt; // Volta ao menu sem escolher
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f clique(event.mouseButton.x, event.mouseButton.y);
                if (opcao1.getGlobalBounds().contains(clique)) return Facil;
                if (opcao2.getGlobalBounds().contains(clique)) return Medio;
                if (opcao3.getGlobalBounds().contains(clique)) return Dificil;
                if (desafio.getGlobalBounds().contains(clique)) return Desafio;
            }
        }

        window.clear();
        window.draw(titulo);
        window.draw(subtitulo);
        window.draw(opcao1);
        window.draw(opcao2);
        window.draw(opcao3);
        window.draw(desafio);
        window.display();
    }
    return std::nullopt;
}

OpcaoMenu menu()
{
    sf::RenderWindow window(sf::VideoMode(850, 600), "MENU");
    window.setFramerateLimit(60);
    sf::Font font;

    if (!font.loadFromFile(caminhoFonte))
    {
        std::cerr << "Erro ao carregar a fonte!" << std::endl;
        return OpcaoMenu::Sair;
    }

    sf::Text titulo("WELCOME", font, 20);
    sf::Text titulo2("PACMAN", font, 40);
    sf::Text jogar("Jogar", font, 25);
    sf::Text ia("IA", font, 25);
    sf::Text ranking("Ranking", font, 25);
    sf::Text sair("Sair", font, 25);

    titulo.setPosition((window.getSize().x - titulo.getLocalBounds().width) / 2, 75);
    titulo2.setPosition((window.getSize().x - titulo2.getLocalBounds().width) / 2, 100);
    jogar.setPosition((window.getSize().x - jogar.getLocalBounds().width) / 2, 200);
    ia.setPosition((window.getSize().x - ia.getLocalBounds().width) / 2, 270);
    ranking.setPosition((window.getSize().x - ranking.getLocalBounds().width) / 2, 340);
    sair.setPosition((window.getSize().x - sair.getLocalBounds().width) / 2, 410);

    titulo.setFillColor(sf::Color::Yellow);
    titulo2.setFillColor(sf::Color::Yellow);
    jogar.setFillColor(sf::Color::White);
    ia.setFillColor(sf::Color::White);
    ranking.setFillColor(sf::Color::White);
    sair.setFillColor(sf::Color::White);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) return OpcaoMenu::Sair;

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2f clique(event.mouseButton.x, event.mouseButton.y);
                if (jogar.getGlobalBounds().contains(clique)) return OpcaoMenu::Jogar;
                if (ia.getGlobalBounds().contains(clique)) return OpcaoMenu::IA;
                if (ranking.getGlobalBounds().contains(clique)) return OpcaoMenu::Ranking;
                if (sair.getGlobalBounds().contains(clique)) return OpcaoMenu::Sair;
            }
        }

        // Desenha a cada frame, e não só quando chega um evento
        window.clear();
        window.draw(titulo);
        window.draw(titulo2);
        window.draw(jogar);
        window.draw(ia);
        window.draw(ranking);
        window.draw(sair);
        window.display();
    }
    return OpcaoMenu::Sair;
}
