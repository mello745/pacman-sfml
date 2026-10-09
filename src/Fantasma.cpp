#include "Fantasma.h"

#include "Config.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <random>

Fantasma::Fantasma(sf::Texture& texture, Posicao startPos, Posicao scatterPos, const std::string& ghostName)
    : posicao(startPos), scatterTarget(scatterPos), name(ghostName),
    proximoBloco(static_cast<int>(startPos.x), static_cast<int>(startPos.y)) {
    sprite.setTexture(texture);
    sprite.setPosition(posicao.x * tamanhoBloco, posicao.y * tamanhoBloco);
}

void Fantasma::atualizarModo(float deltaTime) {
    tempoParaTrocarEstado -= deltaTime;
    if (tempoParaTrocarEstado <= 0) {
        estadoAtual = (estadoAtual == Seguir) ? Aleatorio : Seguir;
        tempoParaTrocarEstado = (estadoAtual == Seguir) ? duracaoSeguir : duracaoAleatorio;
    }
}

void Fantasma::update(const Posicao& pacmanPos, Direcao pacmanDir, bool assustado, float speed, float deltaTime, const Mapa& mapa) {
    atualizarModo(deltaTime);

    float restante = speed * deltaTime;
    while (restante > 0.0f) {
        if (posicao.x == proximoBloco.x && posicao.y == proximoBloco.y) {
            bool perseguir = (estadoAtual == Seguir) && !assustado;
            Posicao alvo = perseguir ? calcularAlvo(pacmanPos, pacmanDir) : posicao;
            escolherNovaDirecao(mapa, alvo, perseguir);
            if (direcao == parado) break; // Sem saída
            sf::Vector2f passoDirecao = converterDirecao(direcao);
            proximoBloco += sf::Vector2i(static_cast<int>(passoDirecao.x), static_cast<int>(passoDirecao.y));
        }

        float dx = proximoBloco.x - posicao.x;
        float dy = proximoBloco.y - posicao.y;
        float distancia = std::abs(dx) + std::abs(dy); // Sempre em um eixo só
        if (restante >= distancia) {
            posicao = { static_cast<float>(proximoBloco.x), static_cast<float>(proximoBloco.y) }; // Encaixa no centro
            restante -= distancia;
        }
        else {
            posicao.x += dx / distancia * restante;
            posicao.y += dy / distancia * restante;
            restante = 0.0f;
        }
    }

    sprite.setPosition(posicao.x * tamanhoBloco, posicao.y * tamanhoBloco);
}

void Fantasma::voltarParaCasa() {
    posicao = scatterTarget;
    proximoBloco = sf::Vector2i(static_cast<int>(scatterTarget.x), static_cast<int>(scatterTarget.y));
    direcao = parado;
    saiuDaBase = false;
    tempoAteSaida = 5.0f;
    sprite.setPosition(posicao.x * tamanhoBloco, posicao.y * tamanhoBloco);
}

bool Fantasma::verificarColisaoParede(const Posicao& posicao, const Mapa& mapa) {
    int x = static_cast<int>(std::round(posicao.x));
    int y = static_cast<int>(std::round(posicao.y));

    if (y < 0 || y >= mapa.size() || x < 0 || x >= mapa[0].size()) {
        return true; // Fora dos limites é considerado colisão
    }

    return mapa[y][x] == Parede;
}

Direcao Fantasma::direcaoParaAlvo(const Mapa& mapa, const Posicao& alvo) {
    const int linhas = static_cast<int>(mapa.size());
    const int colunas = static_cast<int>(mapa[0].size());
    sf::Vector2i inicio(static_cast<int>(std::round(posicao.x)), static_cast<int>(std::round(posicao.y)));

    std::vector<std::vector<bool>> visitado(linhas, std::vector<bool>(colunas, false));
    std::vector<std::vector<Direcao>> primeiroPasso(linhas, std::vector<Direcao>(colunas, parado));
    std::queue<sf::Vector2i> fila;
    visitado[inicio.y][inicio.x] = true;
    fila.push(inicio);

    sf::Vector2i melhor = inicio;
    float menorDistancia = std::hypot(inicio.x - alvo.x, inicio.y - alvo.y);

    while (!fila.empty()) {
        sf::Vector2i atual = fila.front();
        fila.pop();

        float distancia = std::hypot(atual.x - alvo.x, atual.y - alvo.y);
        if (distancia < menorDistancia) {
            menorDistancia = distancia;
            melhor = atual;
        }

        for (Direcao d : { cima, esquerda, baixo, direita }) {
            sf::Vector2f v = converterDirecao(d);
            sf::Vector2i proximo(atual.x + static_cast<int>(v.x), atual.y + static_cast<int>(v.y));
            if (verificarColisaoParede({ static_cast<float>(proximo.x), static_cast<float>(proximo.y) }, mapa) ||
                visitado[proximo.y][proximo.x]) {
                continue;
            }
            visitado[proximo.y][proximo.x] = true;
            primeiroPasso[proximo.y][proximo.x] = (atual == inicio) ? d : primeiroPasso[atual.y][atual.x];
            fila.push(proximo);
        }
    }

    return primeiroPasso[melhor.y][melhor.x];
}

void Fantasma::escolherNovaDirecao(const Mapa& mapa, const Posicao& alvo, bool perseguir) {
    static std::mt19937 gerador(std::random_device{}());

    if (perseguir) {
        Direcao caminho = direcaoParaAlvo(mapa, alvo);
        if (caminho != parado) {
            direcao = caminho;
            return;
        }
        // Já está no alvo: continua andando sem rumo
    }

    auto vizinho = [&](Direcao d) {
        sf::Vector2f v = converterDirecao(d);
        return Posicao{ posicao.x + v.x, posicao.y + v.y };
    };

    std::vector<Direcao> possiveis;
    for (Direcao d : { cima, esquerda, baixo, direita }) {
        if (d != direcaoOposta(direcao) && !verificarColisaoParede(vizinho(d), mapa)) {
            possiveis.push_back(d);
        }
    }

    if (possiveis.empty()) { // Beco sem saída: só resta voltar
        Direcao volta = direcaoOposta(direcao);
        direcao = (volta != parado && !verificarColisaoParede(vizinho(volta), mapa)) ? volta : parado;
        return;
    }

    std::uniform_int_distribution<size_t> sorteio(0, possiveis.size() - 1);
    direcao = possiveis[sorteio(gerador)];
}

void Fantasma::desenhar(sf::RenderWindow& janela) {
    janela.draw(sprite);
}

Posicao Blinky::calcularAlvo(const Posicao& pacmanPos, Direcao) {
    return pacmanPos; // Alvo direto: posição do Pac-Man
}

Posicao Pinky::calcularAlvo(const Posicao& pacmanPos, Direcao pacmanDir) {
    Posicao target = pacmanPos;
    switch (pacmanDir) {
    case cima: target.y -= 4; break;
    case baixo: target.y += 4; break;
    case esquerda: target.x -= 4; break;
    case direita: target.x += 4; break;
    default: break;
    }
    return target;
}

Posicao Inky::calcularAlvo(const Posicao& pacmanPos, Direcao pacmanDir) {
    Posicao target = pacmanPos;
    switch (pacmanDir) {
    case cima: target.y -= 2; break;
    case baixo: target.y += 2; break;
    case esquerda: target.x -= 2; break;
    case direita: target.x += 2; break;
    default: break;
    }
    target.x = 2 * target.x - blinkyPos.x;
    target.y = 2 * target.y - blinkyPos.y;
    return target;
}

Posicao Clyde::calcularAlvo(const Posicao& pacmanPos, Direcao) {
    return (posicao.distanciaAte(pacmanPos) > 8.0f) ? pacmanPos : scatterTarget;
}
