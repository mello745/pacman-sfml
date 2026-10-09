#include "Pacman.h"

#include "Config.h"
#include "Recursos.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <string>

Pacman::Pacman(float x, float y)
    : vidas(vidasIniciais), velocidade(velocidadePacman), fortalecido(false),
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
    const std::vector<sf::Vector2i>& blocosComPilula,
    const std::vector<sf::Vector2i>& blocosFantasmas,
    float deltaTempo) {
    float restante = velocidadeAtual() * deltaTempo; // Pixels a andar neste frame

    while (restante > 0.0f) {
        sf::Vector2f posicao = sprite.getPosition();

        // No canto de um bloco (posição múltipla do tamanho do bloco): decide para onde ir.
        // Também cobre o início da fase e o renascimento, quando o Pac-Man é colocado direto num bloco.
        if (std::fmod(posicao.x, static_cast<float>(tamanhoBloco)) == 0.0f &&
            std::fmod(posicao.y, static_cast<float>(tamanhoBloco)) == 0.0f) {
            sf::Vector2i atual(static_cast<int>(posicao.x) / tamanhoBloco, static_cast<int>(posicao.y) / tamanhoBloco);
            Direcao d = escolherDirecaoIA(mapa, atual, blocosComPilula, blocosFantasmas);
            if (d == parado) break;
            direcaoAtual = converterDirecao(d);
            proximoBloco = atual + sf::Vector2i(static_cast<int>(direcaoAtual.x), static_cast<int>(direcaoAtual.y));
        }

        sf::Vector2f destino(static_cast<float>(proximoBloco.x * tamanhoBloco), static_cast<float>(proximoBloco.y * tamanhoBloco));
        float distancia = std::abs(destino.x - posicao.x) + std::abs(destino.y - posicao.y); // Sempre em um eixo só
        if (restante >= distancia) {
            sprite.setPosition(destino); // Encaixa exatamente no bloco
            restante -= distancia;
        }
        else {
            sprite.move(direcaoAtual * restante);
            restante = 0.0f;
        }
    }
}

Direcao Pacman::escolherDirecaoIA(const Mapa& mapa, sf::Vector2i atual,
    const std::vector<sf::Vector2i>& blocosComPilula,
    const std::vector<sf::Vector2i>& blocosFantasmas) {
    const int linhas = static_cast<int>(mapa.size());
    const int colunas = static_cast<int>(mapa[0].size());
    auto livre = [&](sf::Vector2i b) {
        return b.x >= 0 && b.y >= 0 && b.x < colunas && b.y < linhas && mapa[b.y][b.x] != Parede;
    };
    auto distanciaFantasmas = [&](sf::Vector2i b) {
        int menor = std::numeric_limits<int>::max();
        for (const auto& f : blocosFantasmas) menor = std::min(menor, std::abs(f.x - b.x) + std::abs(f.y - b.y));
        return menor;
    };

    // Blocos perigosos: perto de um fantasma (a não ser com o Pac-Man fortalecido)
    std::vector<std::vector<bool>> perigo(linhas, std::vector<bool>(colunas, false));
    if (!fortalecido) {
        for (int y = 0; y < linhas; ++y)
            for (int x = 0; x < colunas; ++x)
                perigo[y][x] = distanciaFantasmas({ x, y }) <= distanciaSeguraIA;
    }

    std::vector<std::vector<bool>> temPilula(linhas, std::vector<bool>(colunas, false));
    for (const auto& b : blocosComPilula) {
        if (b.x >= 0 && b.y >= 0 && b.x < colunas && b.y < linhas) temPilula[b.y][b.x] = true;
    }

    // Busca em largura até a pílula mais próxima, sem passar por blocos perigosos
    std::vector<std::vector<bool>> visitado(linhas, std::vector<bool>(colunas, false));
    std::vector<std::vector<Direcao>> primeiroPasso(linhas, std::vector<Direcao>(colunas, parado));
    std::queue<sf::Vector2i> fila;
    visitado[atual.y][atual.x] = true;
    fila.push(atual);

    while (!fila.empty()) {
        sf::Vector2i b = fila.front();
        fila.pop();
        if (b != atual && temPilula[b.y][b.x]) {
            return primeiroPasso[b.y][b.x];
        }
        for (Direcao d : { cima, esquerda, baixo, direita }) {
            sf::Vector2f v = converterDirecao(d);
            sf::Vector2i proximo(b.x + static_cast<int>(v.x), b.y + static_cast<int>(v.y));
            if (!livre(proximo) || visitado[proximo.y][proximo.x] || perigo[proximo.y][proximo.x]) continue;
            visitado[proximo.y][proximo.x] = true;
            primeiroPasso[proximo.y][proximo.x] = (b == atual) ? d : primeiroPasso[b.y][b.x];
            fila.push(proximo);
        }
    }

    // Nenhuma pílula alcançável em segurança: foge para o vizinho mais longe dos fantasmas
    Direcao melhor = parado;
    int maiorDistancia = blocosFantasmas.empty() ? 0 : distanciaFantasmas(atual);
    for (Direcao d : { cima, esquerda, baixo, direita }) {
        sf::Vector2f v = converterDirecao(d);
        sf::Vector2i proximo(atual.x + static_cast<int>(v.x), atual.y + static_cast<int>(v.y));
        if (livre(proximo) && distanciaFantasmas(proximo) > maiorDistancia) {
            maiorDistancia = distanciaFantasmas(proximo);
            melhor = d;
        }
    }
    return melhor;
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

float Pacman::velocidadeAtual() const {
    return velocidade * multiplicadorTurbo;
}

void Pacman::ativarTurbo(float multiplicador, float duracao) {
    if (tempoTurbo <= 0.0f || multiplicador > multiplicadorTurbo) {
        multiplicadorTurbo = multiplicador;
    }
    tempoTurbo = duracao;
}

void Pacman::atualizarTurbo(float deltaTempo) {
    if (tempoTurbo > 0.0f) {
        tempoTurbo -= deltaTempo;
        if (tempoTurbo <= 0.0f) {
            multiplicadorTurbo = 1.0f; // Acabou o efeito
        }
    }
}

void Pacman::desenhar(sf::RenderWindow& janela) {
    janela.draw(sprite);
}
