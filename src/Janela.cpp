#include "Janela.h"

#include <algorithm>
#include <cmath>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

bool telaCheia = false; // Vale para todas as janelas enquanto o jogo estiver aberto

// Área da tela disponível para o conteúdo da janela (sem a barra de tarefas e a barra de título)
sf::IntRect areaLivre() {
#ifdef _WIN32
    RECT trabalho;
    if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &trabalho, 0)) {
        int moldura = GetSystemMetrics(SM_CYCAPTION) + 2 * GetSystemMetrics(SM_CYFRAME) + 2 * GetSystemMetrics(SM_CXPADDEDBORDER);
        int bordaLateral = 2 * GetSystemMetrics(SM_CXFRAME) + 2 * GetSystemMetrics(SM_CXPADDEDBORDER);
        return { trabalho.left, trabalho.top,
            (trabalho.right - trabalho.left) - bordaLateral, (trabalho.bottom - trabalho.top) - moldura };
    }
#endif
    sf::VideoMode tela = sf::VideoMode::getDesktopMode();
    return { 0, 0, static_cast<int>(tela.width) - 20, static_cast<int>(tela.height) - 100 };
}

// Escala que cabe no espaço. Se dá para usar um múltiplo inteiro (2x, 3x...) perdendo pouco
// espaço, usa ele: assim a pixel art fica nítida, sem pixels de tamanhos diferentes.
float escalaQueCabe(sf::Vector2f espaco, sf::Vector2u tamanhoVirtual) {
    float cabe = std::min(espaco.x / tamanhoVirtual.x, espaco.y / tamanhoVirtual.y);
    float inteira = std::floor(cabe);
    if (inteira >= 2.0f && inteira / cabe >= 0.85f) return inteira;
    return cabe;
}

// Faz o conteúdo virtual ocupar o centro da janela, na escala que cabe, com faixas pretas ao redor
void ajustarProporcao(sf::RenderWindow& janela, sf::Vector2u tamanhoVirtual) {
    sf::Vector2f real(janela.getSize());
    float escala = escalaQueCabe(real, tamanhoVirtual);
    float largura = tamanhoVirtual.x * escala / real.x;
    float altura = tamanhoVirtual.y * escala / real.y;

    sf::View vista(sf::FloatRect(0.0f, 0.0f, static_cast<float>(tamanhoVirtual.x), static_cast<float>(tamanhoVirtual.y)));
    vista.setViewport({ (1.0f - largura) / 2.0f, (1.0f - altura) / 2.0f, largura, altura });
    janela.setView(vista);
}

} // namespace

void abrirJanela(sf::RenderWindow& janela, sf::Vector2u tamanhoVirtual, const std::string& titulo) {
    if (telaCheia) {
        janela.create(sf::VideoMode::getDesktopMode(), titulo, sf::Style::Fullscreen);
    }
    else {
        sf::IntRect livre = areaLivre();
        float escala = std::max(0.5f, escalaQueCabe(sf::Vector2f(static_cast<float>(livre.width), static_cast<float>(livre.height)), tamanhoVirtual));
        sf::Vector2u tamanho(static_cast<unsigned>(std::round(tamanhoVirtual.x * escala)),
            static_cast<unsigned>(std::round(tamanhoVirtual.y * escala)));

        janela.create(sf::VideoMode(tamanho.x, tamanho.y), titulo, sf::Style::Default);

        // Centraliza na área livre (a posição é a da moldura, um pouco acima e à esquerda do conteúdo)
        int x = livre.left + (livre.width - static_cast<int>(tamanho.x)) / 2;
        int y = livre.top + std::max(0, (livre.height - static_cast<int>(tamanho.y)) / 2);
        janela.setPosition({ std::max(livre.left, x), y });
    }

    janela.setFramerateLimit(60);
    ajustarProporcao(janela, tamanhoVirtual);
}

bool tratarEventoDeJanela(sf::RenderWindow& janela, const sf::Event& evento,
    sf::Vector2u tamanhoVirtual, const std::string& titulo) {
    if (evento.type == sf::Event::Resized) {
        ajustarProporcao(janela, tamanhoVirtual);
        return true;
    }
    if (evento.type == sf::Event::KeyPressed && evento.key.code == sf::Keyboard::F11) {
        telaCheia = !telaCheia;
        abrirJanela(janela, tamanhoVirtual, titulo);
        return true;
    }
    return false;
}
