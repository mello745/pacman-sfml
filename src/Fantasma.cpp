#include "Fantasma.h"

#include "Config.h"
#include "Recursos.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <random>

namespace {

// Índice das texturas por direção: 0 cima, 1 baixo, 2 esquerda, 3 direita (parado usa "baixo")
int indiceDirecao(Direcao d) {
    switch (d) {
    case cima: return 0;
    case esquerda: return 2;
    case direita: return 3;
    default: return 1;
    }
}

} // namespace

void carregarTexturasFantasma(TexturasFantasma& texturas, const std::string& pasta, sf::Color corReserva) {
    const char letras[4] = { 'u', 'd', 'l', 'r' };
    for (int d = 0; d < 4; ++d) {
        for (int q = 0; q < 2; ++q) {
            carregarTextura(texturas.andando[d][q],
                "img/ghost/" + pasta + "/" + letras[d] + std::to_string(q + 1) + ".png", corReserva);
        }
    }
}

void carregarTexturasEspeciais(TexturasEspeciais& texturas) {
    // nerf1x são os azuis; nerf0x, os brancos
    carregarTextura(texturas.assustado[0], "img/ghost/nerf/nerf11.png", sf::Color::Blue);
    carregarTextura(texturas.assustado[1], "img/ghost/nerf/nerf12.png", sf::Color::Blue);
    carregarTextura(texturas.piscando[0], "img/ghost/nerf/nerf01.png", sf::Color::White);
    carregarTextura(texturas.piscando[1], "img/ghost/nerf/nerf02.png", sf::Color::White);
    const char letras[4] = { 'u', 'd', 'l', 'r' };
    for (int d = 0; d < 4; ++d) {
        carregarTextura(texturas.olhos[d], std::string("img/ghost/dead/") + letras[d] + ".png", sf::Color::White);
    }
}

Fantasma::Fantasma(const TexturasFantasma& texturas, const TexturasEspeciais& especiais,
    Posicao startPos, Posicao scatterPos, const std::string& ghostName)
    : posicao(startPos), scatterTarget(scatterPos), name(ghostName),
    proximoBloco(static_cast<int>(startPos.x), static_cast<int>(startPos.y)),
    texturas(&texturas), especiais(&especiais) {
    sprite.setTexture(texturas.andando[indiceDirecao(parado)][0]);
    sprite.setPosition(posicao.x * tamanhoBloco, posicao.y * tamanhoBloco);
}

void Fantasma::atualizarModo(float deltaTime) {
    tempoParaTrocarEstado -= deltaTime;
    if (tempoParaTrocarEstado <= 0) {
        estadoAtual = (estadoAtual == Seguir) ? Aleatorio : Seguir;
        tempoParaTrocarEstado = (estadoAtual == Seguir) ? duracaoSeguir : duracaoAleatorio;
    }
}

void Fantasma::animar(float deltaTime) {
    tempoAnimacao += deltaTime;
    if (tempoAnimacao >= tempoQuadroFantasma) {
        tempoAnimacao -= tempoQuadroFantasma;
        quadro = 1 - quadro;
    }
}

void Fantasma::update(const Posicao& pacmanPos, Direcao pacmanDir, bool assustado, float speed, float deltaTime, const Mapa& mapa) {
    atualizarModo(deltaTime);

    float restante = (comido ? velocidadeOlhos : speed) * deltaTime;
    while (restante > 0.0f) {
        if (posicao.x == proximoBloco.x && posicao.y == proximoBloco.y) {
            if (comido) {
                // Olhos: ao chegar na casa, volta a ser fantasma e espera para sair de novo
                if (posicao.x == scatterTarget.x && posicao.y == scatterTarget.y) {
                    comido = false;
                    saiuDaBase = false;
                    tempoAteSaida = esperaAposComido;
                    direcao = parado;
                    break;
                }
                direcao = direcaoParaAlvo(mapa, scatterTarget);
                if (direcao == parado) break;
                sf::Vector2f passo = converterDirecao(direcao);
                proximoBloco += sf::Vector2i(static_cast<int>(passo.x), static_cast<int>(passo.y));
                continue;
            }

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
    comido = true; // Continua andando, agora como olhos, até a casa (ver update)
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

void Fantasma::desenhar(sf::RenderTarget& alvo, bool assustado, float tempoRestante) {
    const sf::Texture* textura;
    if (comido) {
        textura = &especiais->olhos[indiceDirecao(direcao)];
    }
    else if (assustado) {
        // Nos últimos segundos, alterna entre azul e branco 5 vezes por segundo
        bool branco = tempoRestante < avisoFimFortalecimento && static_cast<int>(tempoRestante * 5) % 2 == 0;
        textura = branco ? &especiais->piscando[quadro] : &especiais->assustado[quadro];
    }
    else {
        textura = &texturas->andando[indiceDirecao(direcao)][quadro];
    }
    sprite.setTexture(*textura);
    alvo.draw(sprite);
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
