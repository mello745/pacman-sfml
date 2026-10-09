#include <SFML/Graphics.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <algorithm>
#include <random>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <SFML/System.hpp> // Para usar sf::Vector2i
#include <memory>
#include <cmath>
#include <limits>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <optional>

using namespace std;
using namespace chrono;

const int tamanhoBloco = 18;
const int larguraJanela = 340;
const int alturaJanela = 500;
const int numLinhas = alturaJanela / tamanhoBloco;
const int numColunas = larguraJanela / tamanhoBloco;
const int vidasIniciais = 3;
const float tempoFantasmaVulneravel = 5.0f;
const float escalaPacman = 1.0f;
const float velocidadeFantasma = 2.0f; // Blocos por segundo
const float dtMaximo = 1.0f / 20.0f;   // Maior passo de tempo por frame (em segundos)

const std::string pastaAssets = "assets/";
const std::string caminhoFonte = pastaAssets + "fonts/pixel.ttf";

// Carrega uma textura de assets/. Se o arquivo não existir, usa um quadrado
// colorido do tamanho de um bloco para o jogo continuar rodando.
void carregarTextura(sf::Texture& textura, const std::string& caminho, sf::Color corReserva = sf::Color::Magenta) {
    if (!textura.loadFromFile(pastaAssets + caminho)) {
        std::cerr << "Aviso: textura nao encontrada (" << caminho << "), usando substituta." << std::endl;
        sf::Image imagem;
        imagem.create(tamanhoBloco, tamanhoBloco, corReserva);
        textura.loadFromImage(imagem);
    }
}

// Escala o sprite para ocupar exatamente um bloco do mapa, seja qual for o tamanho da imagem
void ajustarAoBloco(sf::Sprite& sprite) {
    sf::Vector2u tamanho = sprite.getTexture()->getSize();
    sprite.setScale(static_cast<float>(tamanhoBloco) / tamanho.x, static_cast<float>(tamanhoBloco) / tamanho.y);
}

enum EstadoFantasma { Aleatorio, Seguir };
enum EstadoJogo { Jogando, Vitoria, GameOver };
enum Direcao { parado, cima, baixo, esquerda, direita };
enum NivelDificuldade { Facil, Medio, Dificil, Desafio };
enum ModoJogo { Manual, IA};

sf::Vector2f converterDirecao(Direcao dir) {
    switch (dir) {
    case cima: return { 0, -1 };
    case baixo: return { 0, 1 };
    case esquerda: return { -1, 0 };
    case direita: return { 1, 0 };
    default: return { 0, 0 };
    }
}

Direcao direcaoOposta(Direcao dir) {
    switch (dir) {
    case cima: return baixo;
    case baixo: return cima;
    case esquerda: return direita;
    case direita: return esquerda;
    default: return parado;
    }
}

struct Posicao {
    float x, y;

    float distanciaAte(const Posicao& other) const {
        return std::sqrt(std::pow(x - other.x, 2) + std::pow(y - other.y, 2));
    }
};

struct Node {
    int x, y;
    float g, h;
    Node* parent;

    Node(int x, int y, float g, float h, Node* parent = nullptr)
        : x(x), y(y), g(g), h(h), parent(parent) {}

    float f() const { return g + h; }

    bool operator>(const Node& other) const {
        return f() > other.f();
    }
};

class PathFinder {
public:
    static std::vector<sf::Vector2i> buscarCaminho(const std::vector<std::vector<int>>& mapa,
        sf::Vector2i inicio, sf::Vector2i destino) {
        std::priority_queue<Node, std::vector<Node>, std::greater<>> abertos;
        std::unordered_set<int> fechados;

        auto key = [&](int x, int y) { return y * mapa[0].size() + x; };

        auto heuristica = [&](int x1, int y1, int x2, int y2) {
            return static_cast<float>(abs(x1 - x2) + abs(y1 - y2)); // Distância de Manhattan
            };

        abertos.emplace(inicio.x, inicio.y, 0, heuristica(inicio.x, inicio.y, destino.x, destino.y));

        while (!abertos.empty()) {
            Node atual = abertos.top();
            abertos.pop();

            // Se destino alcançado
            if (atual.x == destino.x && atual.y == destino.y) {
                return reconstruirCaminho(&atual);
            }

            // Marca o nó atual como fechado
            fechados.insert(key(atual.x, atual.y));

            // Vizinhos (cima, baixo, esquerda, direita)
            for (auto& vizinho : getVizinhos(atual.x, atual.y, mapa)) {
                if (fechados.count(key(vizinho.x, vizinho.y)) || mapa[vizinho.y][vizinho.x] == 0) {
                    continue;
                }

                float gNovo = atual.g + 1;
                float hNovo = heuristica(vizinho.x, vizinho.y, destino.x, destino.y);

                abertos.emplace(vizinho.x, vizinho.y, gNovo, hNovo, new Node(atual.x, atual.y, gNovo, hNovo, nullptr));
            }
        }

        return {}; // Nenhum caminho encontrado
    }

private:
    static std::vector<sf::Vector2i> getVizinhos(int x, int y, const std::vector<std::vector<int>>& mapa) {
        std::vector<sf::Vector2i> vizinhos;
        int dx[] = { 0, 0, -1, 1 };
        int dy[] = { -1, 1, 0, 0 };

        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (nx >= 0 && nx < mapa[0].size() && ny >= 0 && ny < mapa.size()) {
                vizinhos.emplace_back(nx, ny);
            }
        }

        return vizinhos;
    }

    static std::vector<sf::Vector2i> reconstruirCaminho(Node* no) {
        std::vector<sf::Vector2i> caminho;
        while (no) {
            caminho.emplace_back(no->x, no->y);
            no = no->parent;
        }
        std::reverse(caminho.begin(), caminho.end());
        return caminho;
    }
};

struct jogador {
    string nome;
    int pontos;
    time_t tempo;
};

std::vector<jogador> ranking;

const std::string arquivoRanking = "ranking.txt";

// Acrescenta UMA partida ao final do arquivo (cada linha: nome pontos tempo)
void salvarNoRanking(const jogador& partida) {
    std::ofstream file(arquivoRanking, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Erro ao salvar o ranking!" << std::endl;
        return;
    }
    file << partida.nome << " " << partida.pontos << " " << partida.tempo << std::endl;
}

// Lê o nome no console. Espaços viram "_" porque o arquivo separa os campos por espaço
std::string lerNomeJogador() {
    std::cout << "Digite o nome do jogador: ";
    std::string nome;
    std::getline(std::cin >> std::ws, nome);
    nome.erase(nome.find_last_not_of(" \t\r") + 1);
    std::replace_if(nome.begin(), nome.end(), [](unsigned char c) { return std::isspace(c); }, '_');
    return nome.empty() ? "Jogador" : nome;
}

void carregarRanking() {
    ranking.clear();
    std::ifstream file(arquivoRanking);
    if (!file.is_open()) {
        return; // Ainda não há partidas salvas
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
}

void exibirRanking() {
    carregarRanking();
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

                string info = "Pos: " + to_string(index + 1) + " | " +
                    "Jogador: " + ranking[index].nome + " | " +
                    "Pontos: " + to_string(ranking[index].pontos) + " | " +
                    "Data: " + data.str();

                text.setString(info);
                text.setPosition((window.getSize().x - text.getLocalBounds().width) / 2, 150 + i * 30);
                window.draw(text);
            }
        }

        window.display();
    }
}

class Pacman {
public:
    sf::Sprite sprite;
    std::vector<sf::Texture> texturas;
    std::vector<sf::Sprite> paredes;
    int vidas;
    float velocidade;
    bool fortalecido;
    float temporizadorFortalecimento;
    sf::Vector2f direcaoAtual;
    int frameAtual;
    float tempoEntreFrames;
    float temporizadorFrame;
    sf::Vector2f direcao;
    float velocidadeTemporaria;
    sf::Clock clock;
    float deltaTempo = clock.restart().asSeconds();
    Direcao dir = parado;
    sf::Vector2f movimento = converterDirecao(dir) * velocidade * deltaTempo;

    Pacman(float x, float y)
        : vidas(vidasIniciais), velocidade(90.0f), fortalecido(false),
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

    void atualizarAnimacao(float deltaTempo) {
        temporizadorFrame += deltaTempo;
        if (temporizadorFrame >= tempoEntreFrames) {
            temporizadorFrame = 0.0f;
            frameAtual = (frameAtual + 1) % texturas.size(); // Alterna entre todos os frames
            sprite.setTexture(texturas[frameAtual]);
        }
    }

    void mover(float deltaTempo) {
        sf::Vector2f movimentoLocal = direcao / static_cast<float>(tamanhoBloco) * velocidadeTemporaria * deltaTempo;
        sprite.move(movimentoLocal);
    }

    void moverAutomaticamente(const std::vector<std::vector<int>>& mapa,
        const std::vector<sf::Sprite>& pilulas,
        const std::vector<sf::Sprite>& fantasmas, // Adicionando vetor de fantasmas
        float deltaTempo) {
        // Obter posição atual do Pac-Man no grid
        sf::Vector2i posicaoPacman(
            static_cast<int>(sprite.getPosition().x / tamanhoBloco),
            static_cast<int>(sprite.getPosition().y / tamanhoBloco)
        );

        // Verificar se Pac-Man está no centro de um bloco
        sf::Vector2f posicaoAtual = sprite.getPosition();
        bool noCentroDoBloco =
            static_cast<int>(posicaoAtual.x) % tamanhoBloco == 0 &&
            static_cast<int>(posicaoAtual.y) % tamanhoBloco == 0;

        if (noCentroDoBloco) {
            // Encontrar a pílula mais próxima
            sf::Vector2i destino = posicaoPacman;
            float menorDistancia = std::numeric_limits<float>::max();

            for (const auto& pilula : pilulas) {
                sf::Vector2i posicaoPilula(
                    static_cast<int>(pilula.getPosition().x / tamanhoBloco),
                    static_cast<int>(pilula.getPosition().y / tamanhoBloco)
                );

                float distancia = std::hypot(
                    posicaoPacman.x - posicaoPilula.x,
                    posicaoPacman.y - posicaoPilula.y
                );

                if (distancia < menorDistancia) {
                    menorDistancia = distancia;
                    destino = posicaoPilula;
                }
            }

            // Avaliar todas as direções válidas e encontrar a melhor
            sf::Vector2i melhorDirecao = { 0, 0 };
            menorDistancia = std::numeric_limits<float>::max();
            std::vector<sf::Vector2i> direcoes = {
                {0, -1},  // Cima
                {0, 1},   // Baixo
                {-1, 0},  // Esquerda
                {1, 0}    // Direita
            };

            for (const auto& direcao : direcoes) {
                sf::Vector2i novaPosicao = posicaoPacman + direcao;

                // Verificar se a nova posição está dentro do mapa
                if (novaPosicao.y < 0 || novaPosicao.y >= static_cast<int>(mapa.size()) ||
                    novaPosicao.x < 0 || novaPosicao.x >= static_cast<int>(mapa[0].size())) {
                    continue;
                }

                // Verificar se a nova posição não é uma parede
                if (mapa[novaPosicao.y][novaPosicao.x] == 1) {
                    continue;
                }

                // Verificar se a nova posição não é ocupada por um fantasma
                bool colidiuComFantasma = false;
                for (const auto& fantasma : fantasmas) {
                    sf::FloatRect rectFantasma = fantasma.getGlobalBounds();
                    if (rectFantasma.contains(novaPosicao.x * tamanhoBloco, novaPosicao.y * tamanhoBloco)) {
                        colidiuComFantasma = true;
                        break;
                    }
                }

                if (colidiuComFantasma) {
                    continue; // Evitar a direção que leva ao fantasma
                }

                // Calcular a distância até o destino (pílula)
                float distancia = std::hypot(
                    destino.x - novaPosicao.x,
                    destino.y - novaPosicao.y
                );

                // Escolher a direção que minimiza a distância e é válida
                if (distancia < menorDistancia) {
                    menorDistancia = distancia;
                    melhorDirecao = direcao;
                }
            }

            // Atualizar a direção apenas se uma direção válida for encontrada
            if (melhorDirecao != sf::Vector2i(0, 0)) {
                direcaoAtual = sf::Vector2f(melhorDirecao.x, melhorDirecao.y);
            }
            else {
                std::cout << "Nenhuma direção válida encontrada.\n";
            }
        }

        // Mover Pac-Man na direção atual
        sf::Vector2f movimento = direcaoAtual * (velocidade * deltaTempo);
        sf::FloatRect novaPosicaoPacman = sprite.getGlobalBounds();
        novaPosicaoPacman.left += movimento.x;
        novaPosicaoPacman.top += movimento.y;

        // Verificar colisão antes de mover
        sf::Vector2i posicaoGridNova(
            static_cast<int>(novaPosicaoPacman.left / tamanhoBloco),
            static_cast<int>(novaPosicaoPacman.top / tamanhoBloco)
        );

        // Verificar se a posição nova está válida e não contém paredes
        if (posicaoGridNova.y >= 0 && posicaoGridNova.y < static_cast<int>(mapa.size()) &&
            posicaoGridNova.x >= 0 && posicaoGridNova.x < static_cast<int>(mapa[0].size()) &&
            mapa[posicaoGridNova.y][posicaoGridNova.x] != 1) {

            // Verificar se a nova posição colide com algum fantasma
            for (const auto& fantasma : fantasmas) {
                if (fantasma.getGlobalBounds().contains(novaPosicaoPacman.left, novaPosicaoPacman.top)) {
                    std::cout << "Colisão com fantasma detectada. Movimento bloqueado.\n";
                    direcaoAtual = sf::Vector2f(0.0f, 0.0f); // Parar em caso de colisão com fantasma
                    return;
                }
            }

            sprite.move(movimento);
        }
        else {
            std::cout << "Colisão com parede detectada. Movimento bloqueado.\n";
            direcaoAtual = sf::Vector2f(0.0f, 0.0f); // Parar em caso de colisão
        }
    }

    bool verificarColisaoParede(const sf::FloatRect& novaPosicaoPacman) {
        for (const auto& parede : paredes) {
            if (novaPosicaoPacman.intersects(parede.getGlobalBounds())) {
                return true; // Colisão detectada
            }
        }
        return false; // Sem colisão
    }

    void atualizarFortalecimento(float deltaTempo) {
        if (fortalecido) {
            temporizadorFortalecimento -= deltaTempo;
            if (temporizadorFortalecimento <= 0) {
                fortalecido = false;
                sprite.setColor(sf::Color::White); // Volta à cor normal
            }
        }
    }

    void ativarFortalecimento() {
        fortalecido = true;
        temporizadorFortalecimento = tempoFantasmaVulneravel;
        sprite.setColor(sf::Color::Green); // Indica fortalecimento visualmente
    }

    void desenhar(sf::RenderWindow& janela) {
        janela.draw(sprite);
    }
};

class Fantasma {
public:
    static constexpr float duracaoAleatorio = 7.0f; // Segundos andando sem rumo
    static constexpr float duracaoSeguir = 20.0f;   // Segundos perseguindo o Pac-Man

    sf::Sprite sprite;
    EstadoFantasma estadoAtual = Aleatorio;
    Posicao posicao;              // Em blocos (x = coluna, y = linha)
    Posicao scatterTarget;
    std::string name;
    float detectionRange;
    bool saiuDaBase = false;
    float tempoParaTrocarEstado = duracaoAleatorio;
    Direcao direcao = parado;
    sf::Vector2i proximoBloco;    // Bloco para onde o fantasma está andando
    float tempoAteSaida = 0.0f;

    Fantasma(sf::Texture& texture, Posicao startPos, Posicao scatterPos, const std::string& ghostName, float range)
        : posicao(startPos), scatterTarget(scatterPos), name(ghostName), detectionRange(range),
        proximoBloco(static_cast<int>(startPos.x), static_cast<int>(startPos.y)) {
        sprite.setTexture(texture);
        sprite.setPosition(posicao.x * tamanhoBloco, posicao.y * tamanhoBloco);
    }

    virtual Posicao calcularAlvo(const Posicao& pacmanPos, Direcao pacmanDir) = 0;

    // Alterna entre andar sem rumo e perseguir o Pac-Man
    void atualizarModo(float deltaTime) {
        tempoParaTrocarEstado -= deltaTime;
        if (tempoParaTrocarEstado <= 0) {
            estadoAtual = (estadoAtual == Seguir) ? Aleatorio : Seguir;
            tempoParaTrocarEstado = (estadoAtual == Seguir) ? duracaoSeguir : duracaoAleatorio;
        }
    }

    // Anda bloco a bloco: só escolhe uma nova direção ao chegar no centro de um bloco.
    // Com o Pac-Man fortalecido (assustado = true), anda sem rumo em vez de perseguir.
    void update(const Posicao& pacmanPos, Direcao pacmanDir, bool assustado, float speed, float deltaTime, const std::vector<std::vector<int>>& mapa) {
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

    // Comido pelo Pac-Man: volta para a casa e espera 5 s antes de sair de novo
    void voltarParaCasa() {
        posicao = scatterTarget;
        proximoBloco = sf::Vector2i(static_cast<int>(scatterTarget.x), static_cast<int>(scatterTarget.y));
        direcao = parado;
        saiuDaBase = false;
        tempoAteSaida = 5.0f;
        sprite.setPosition(posicao.x * tamanhoBloco, posicao.y * tamanhoBloco);
    }

    bool verificarColisaoParede(const Posicao& posicao, const std::vector<std::vector<int>>& mapa) {
        int x = static_cast<int>(std::round(posicao.x));
        int y = static_cast<int>(std::round(posicao.y));

        if (y < 0 || y >= mapa.size() || x < 0 || x >= mapa[0].size()) {
            return true; // Fora dos limites é considerado colisão
        }

        return mapa[y][x] <= 0; // Valores menores ou iguais a 0 são paredes
    }

    // Primeiro passo do menor caminho (busca em largura) até o bloco livre mais próximo do alvo.
    // Retorna "parado" se o fantasma já está nesse bloco.
    Direcao direcaoParaAlvo(const std::vector<std::vector<int>>& mapa, const Posicao& alvo) {
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

    // Escolhe a direção no centro de um bloco.
    // Perseguindo: segue o menor caminho até o alvo.
    // Senão: sorteia entre os vizinhos livres, sem voltar para trás (a não ser em beco sem saída).
    void escolherNovaDirecao(const std::vector<std::vector<int>>& mapa, const Posicao& alvo, bool perseguir) {
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

    void desenhar(sf::RenderWindow& janela) {
        janela.draw(sprite);
    }
};

class Blinky : public Fantasma {
public:
    Blinky(sf::Texture& texture, Posicao startPos, Posicao scatterPos, float range)
        : Fantasma(texture, startPos, scatterPos, "Blinky", 1.0f) {}
    Posicao calcularAlvo(const Posicao& pacmanPos, Direcao) override {
        return pacmanPos; // Alvo direto: posição do Pac-Man
    }
};

class Pinky : public Fantasma {
public:
    Pinky(sf::Texture& texture, Posicao startPos, Posicao scatterPos, float range)
        : Fantasma(texture, startPos, scatterPos, "Pinky", 2.0f) {}
    Posicao calcularAlvo(const Posicao& pacmanPos, Direcao pacmanDir) override {
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
};

class Inky : public Fantasma {
public:
    Posicao blinkyPos;

    Inky(sf::Texture& texture, Posicao startPos, Posicao scatterPos, float range)
        : Fantasma(texture, startPos, scatterPos, "Inky", 3.0f), blinkyPos(startPos) {}
    Posicao calcularAlvo(const Posicao& pacmanPos, Direcao pacmanDir) override {
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
};

class Clyde : public Fantasma {
public:
    Clyde(sf::Texture& texture, Posicao startPos, Posicao scatterPos, float range)
        : Fantasma(texture, startPos, scatterPos, "Clyde", 4.0f) {}
    Posicao calcularAlvo(const Posicao& pacmanPos, Direcao) override {
        float distancia = std::sqrt(std::pow(pacmanPos.x - posicao.x, 2) + std::pow(pacmanPos.y - posicao.y, 2));
        return (distancia > 8.0f) ? pacmanPos : scatterTarget;
    }
};

class Jogo {
public:
    sf::RenderWindow janela;
    sf::Font fonte;                  // Carregada uma vez no construtor
    sf::RenderTexture texturaEscura; // Camada escura do modo Desafio, criada uma vez no construtor
    std::vector<std::unique_ptr<Pacman>> pacmans;
    Pacman pacman;
    std::vector<std::unique_ptr<Fantasma>> fantasmas;
    std::vector<sf::Sprite> spritesFantasmas;
    sf::Clock relogioJogo; // No início do jogo
    sf::RectangleShape parede;
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
    sf::Vector2i calcularDestinoFuga(const sf::Vector2i& posicaoFantasma, const sf::Vector2i& posicaoPacman) {
        int dx = posicaoFantasma.x - posicaoPacman.x;
        int dy = posicaoFantasma.y - posicaoPacman.y;

        return sf::Vector2i(posicaoFantasma.x + dx, posicaoFantasma.y + dy);
    }
    int pontos = 0;

    int maxVidas = 5; // Limite superior de vidas permitido
    // Definindo um tipo para o mapa
    std::vector<std::vector<int>> mapa;
    std::vector<sf::Sprite> paredes;
    std::vector<sf::Sprite> pilulas;
    std::vector<sf::Sprite> item;
    std::vector<sf::Sprite> apple;
    std::vector<sf::Sprite> orange;
    std::vector<sf::Sprite> beer;
    std::vector<sf::Sprite> pilulasFortalecedoras;
    float tempoInvulneravel = 2.0f; // 2 segundos de invulnerabilidade
    bool invulneravel = true;
    int fase = 0;
    int faseAtual = 0;
    int movimentos = 0;
    NivelDificuldade dificuldadeAtual;
    bool saiuDaBase = false;
    float tempoAteSaida = 5.0f;
    ModoJogo modoAtual = Manual; // Inicialmente, o modo é Manual

    std::vector<std::vector<std::vector<int>>> mapas = {
    { // Fase 1
              {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
              {0, 8, 2, 2, 2, 2, 2, 0, 2, 2, 2, 0, 2, 2, 2, 2, 2, 9, 0},
              {0, 2, 0, 0, 0, 0, 2, 0, 0, 3, 0, 0, 2, 0, 0, 0, 0, 2, 0},
              {0, 2, 0, 2, 2, 0, 2, 0, 2, 2, 2, 0, 2, 0, 6, 2, 0, 2, 0},
              {0, 2, 0, 2, 0, 0, 2, 0, 1, 0, 1, 0, 2, 0, 0, 2, 0, 2, 0},
              {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0},
              {0, 2, 0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 2, 0},
              {0, 2, 0, 2, 6, 2, 1, 1, 0, 1, 0, 2, 2, 2, 2, 2, 0, 2, 0},
              {0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0},
              {0, 2, 2, 2, 2, 1, 2, 1, 1, 3, 1, 1, 2, 1, 2, 2, 2, 7, 0},
              {0, 0, 0, 0, 2, 0, 2, 0, 1, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0},
              {0, 2, 2, 0, 2, 0, 2, 0, 1, 4, 1, 0, 2, 0, 2, 0, 2, 2, 0},
              {0, 6, 2, 0, 2, 0, 2, 0, 4, 4, 4, 0, 2, 0, 2, 0, 2, 2, 0},
              {0, 2, 2, 0, 2, 0, 2, 0, 1, 1, 1, 0, 2, 0, 2, 0, 2, 2, 0},
              {0, 2, 0, 0, 2, 0, 2, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0, 2, 0},
              {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0},
              {0, 2, 0, 1, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 1, 0, 2, 0},
              {0, 2, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 2, 0},
              {0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0},
              {0, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 6, 0},
              {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    { // Fase 2
              {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
              {0, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 0},
              {0, 2, 0, 0, 0, 0, 2, 0, 0, 8, 0, 0, 2, 0, 0, 0, 0, 2, 0},
              {0, 2, 0, 1, 1, 0, 2, 0, 1, 1, 1, 0, 2, 0, 8, 1, 0, 2, 0},
              {0, 2, 0, 1, 0, 0, 2, 0, 1, 0, 1, 0, 2, 0, 0, 1, 0, 2, 0},
              {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0},
              {0, 2, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 2, 0},
              {0, 2, 0, 1, 9, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 2, 0},
              {0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0},
              {0, 2, 2, 2, 2, 0, 2, 1, 1, 3, 1, 1, 2, 0, 2, 2, 2, 2, 0},
              {0, 0, 0, 0, 2, 0, 7, 0, 0, 1, 0, 0, 2, 0, 2, 0, 0, 0, 0},
              {0, 2, 2, 0, 2, 0, 2, 0, 1, 4, 1, 0, 2, 0, 7, 0, 2, 2, 0},
              {0, 6, 2, 0, 2, 0, 2, 0, 4, 4, 4, 0, 2, 0, 2, 0, 2, 2, 0},
              {0, 2, 2, 0, 2, 0, 2, 0, 0, 0, 0, 0, 2, 0, 2, 0, 2, 2, 0},
              {0, 2, 0, 0, 2, 0, 7, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0, 2, 0},
              {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0},
              {0, 2, 0, 1, 9, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 1, 0, 2, 0},
              {0, 2, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 2, 0},
              {0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0},
              {0, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 7, 0},
              {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    { //Fase 3
              {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
              {0, 7, 2, 2, 2, 2, 2, 0, 2, 2, 2, 0, 2, 2, 2, 2, 2, 6, 0},
              {0, 2, 0, 0, 0, 0, 2, 0, 0, 3, 0, 0, 2, 0, 0, 0, 0, 2, 0},
              {0, 2, 0, 0, 0, 0, 2, 0, 1, 1, 1, 0, 2, 0, 0, 0, 0, 2, 0},
              {0, 2, 0, 0, 0, 0, 2, 0, 1, 0, 1, 0, 2, 0, 0, 0, 0, 2, 0},
              {0, 2, 2, 2, 2, 2, 2, 2, 9, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0},
              {0, 2, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 2, 0},
              {0, 2, 0, 1, 6, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 2, 0},
              {0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0},
              {0, 2, 2, 2, 2, 0, 2, 1, 1, 3, 1, 1, 2, 0, 2, 2, 2, 7, 0},
              {0, 0, 0, 0, 2, 0, 2, 0, 0, 1, 0, 0, 2, 0, 2, 0, 0, 0, 0},
              {0, 2, 2, 0, 2, 0, 2, 0, 1, 4, 1, 0, 2, 0, 2, 0, 2, 2, 0},
              {0, 6, 2, 0, 2, 0, 2, 0, 4, 4, 4, 0, 2, 0, 2, 0, 2, 2, 0},
              {0, 2, 2, 0, 2, 0, 2, 0, 0, 0, 0, 0, 2, 0, 2, 0, 2, 2, 0},
              {0, 2, 0, 0, 2, 0, 2, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0, 2, 0},
              {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0},
              {0, 2, 0, 1, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 1, 0, 2, 0},
              {0, 2, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 2, 0},
              {0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0},
              {0, 6, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 8, 0},
              {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    }
    };

    Jogo() : janela(sf::VideoMode(larguraJanela, alturaJanela), "Pac-Man"), pacman(0, 0), estadoJogo(Jogando) {
        srand(static_cast<unsigned>(time(0)));
        janela.setFramerateLimit(60);

        if (!fonte.loadFromFile(caminhoFonte)) {
            std::cerr << "Erro ao carregar a fonte." << std::endl;
        }
        texturaEscura.create(larguraJanela, alturaJanela);

        // Parede: bloco azul sólido gerado em código, no tamanho exato de um bloco
        sf::Image imagemParede;
        imagemParede.create(tamanhoBloco, tamanhoBloco, sf::Color(33, 33, 222));
        texturaParede.loadFromImage(imagemParede);

        carregarTextura(texturaPilula, "img/item/dot.png");
        carregarTextura(texturaItem, "img/item/cherry.png", sf::Color::Red);
        carregarTextura(texturaApple, "img/item/apple.png", sf::Color::Green);
        carregarTextura(texturaOrange, "img/item/redbull.png", sf::Color::Cyan);
        carregarTextura(texturaBeer, "img/item/beer.png", sf::Color(255, 165, 0));
        carregarTextura(texturaPilulaFortalecedora, "img/item/pellet.png");

        carregarTextura(texturaBlinky, "img/ghost/blinky/d1.png", sf::Color::Red);
        carregarTextura(texturaPinky, "img/ghost/pinky/d1.png", sf::Color(255, 184, 255));
        carregarTextura(texturaInky, "img/ghost/inky/d1.png", sf::Color::Cyan);
        carregarTextura(texturaClyde, "img/ghost/clyde/d1.png", sf::Color(255, 184, 82));

        inicializarFase(0);
        proximaDirecao = sf::Vector2f(1.0f, 0.0f);
    }

    bool todasAsFrutasColetadas() {
        return pilulas.empty(); // Verifica se não há mais frutas
    }

    void inicializarFase(int indiceFase) {
        if (indiceFase >= 0 && indiceFase < mapas.size()) {
            mapa = mapas[indiceFase];

            // Remove os objetos da fase anterior antes de criar os da nova
            paredes.clear();
            pilulas.clear();
            pilulasFortalecedoras.clear();
            item.clear();
            apple.clear();
            orange.clear();
            beer.clear();

            definirPosicoesIniciais();
            criarParedes();
            criarPilulas();
            criarItem1();
            criarItem2();
        }
        else {
            std::cerr << "Fase inválida: " << indiceFase << std::endl;
        }
    }

    void criarParedes() {
        for (int y = 0; y < mapa.size(); ++y) {
            for (int x = 0; x < mapa[y].size(); ++x) {
                if (mapa[y][x] == 0) { // 1 indica uma parede
                    sf::Sprite paredeSprite;
                    paredeSprite.setTexture(texturaParede);
                    paredeSprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
                    paredes.push_back(paredeSprite);
                }
            }
        }
    }

    void criarPilulas() {
        for (int y = 0; y < mapa.size(); ++y) {
            for (int x = 0; x < mapa[y].size(); ++x) {
                if (mapa[y][x] == 2) {
                    sf::Sprite pilulaSprite;
                    pilulaSprite.setTexture(texturaPilula);
                    pilulaSprite.setPosition(x * tamanhoBloco + tamanhoBloco / 4, y * tamanhoBloco + tamanhoBloco / 4);
                    pilulaSprite.setScale(1.0f, 1.0f);
                    pilulas.push_back(pilulaSprite);
                }
                else if (mapa[y][x] == 3) {
                    sf::Sprite pilulaFortalecedoraSprite;
                    pilulaFortalecedoraSprite.setTexture(texturaPilulaFortalecedora);
                    pilulaFortalecedoraSprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
                    pilulasFortalecedoras.push_back(pilulaFortalecedoraSprite);
                }
            }
        }
    }

    void definirPosicoesIniciais() {
        std::vector<Posicao> casaDosFantasmas;

        // Percorre o mapa para encontrar as posições do Pac-Man e dos fantasmas
        for (int y = 0; y < mapa.size(); ++y) {
            for (int x = 0; x < mapa[y].size(); ++x) {
                if (mapa[y][x] == 5) { // Pac-Man
                    pacman.sprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
                }
                else if (mapa[y][x] == 4) { // Casa dos fantasmas
                    casaDosFantasmas.push_back({ static_cast<float>(x), static_cast<float>(y) });
                }
            }
        }

        criarFantasmas(casaDosFantasmas);
    }

    void criarItem1() {
        for (int y = 0; y < mapa.size(); ++y) {
            for (int x = 0; x < mapa[y].size(); ++x) {
                if (mapa[y][x] == 6) {
                    sf::Sprite itemSprite;
                    itemSprite.setTexture(texturaItem);
                    itemSprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
                    ajustarAoBloco(itemSprite);
                    item.push_back(itemSprite);
                }
                else if (mapa[y][x] == 7) {
                    sf::Sprite appleSprite;
                    appleSprite.setTexture(texturaApple);
                    appleSprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
                    ajustarAoBloco(appleSprite);
                    apple.push_back(appleSprite);
                }
            }
        }
    }

    void criarItem2() {
        for (int y = 0; y < mapa.size(); ++y) {
            for (int x = 0; x < mapa[y].size(); ++x) {
                if (mapa[y][x] == 8) {
                    sf::Sprite orangeSprite;
                    orangeSprite.setTexture(texturaOrange);
                    orangeSprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
                    ajustarAoBloco(orangeSprite);
                    orange.push_back(orangeSprite);
                }
                else if (mapa[y][x] == 9) {
                    sf::Sprite beerSprite;
                    beerSprite.setTexture(texturaBeer);
                    beerSprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
                    ajustarAoBloco(beerSprite);
                    beer.push_back(beerSprite);
                }
            }
        }
    }

    int calcularPontuacao() {
        return pontos - (movimentos * 2); // Penaliza 2 pontos por movimento
    }

    Direcao converterParaDirecao(const sf::Vector2f& vetor) {
        if (vetor == sf::Vector2f(0, -1)) return cima;
        if (vetor == sf::Vector2f(0, 1)) return baixo;
        if (vetor == sf::Vector2f(-1, 0)) return esquerda;
        if (vetor == sf::Vector2f(1, 0)) return direita;
        return parado; // Caso nenhuma direção seja identificada
    }

    void jogarComIA() {
        Jogo jogo;
        jogo.pacman.vidas = 3;  // Configuração inicial do Pac-Man
        jogo.dificuldade(Medio);  // Ajuste a dificuldade se necessário

        sf::Clock relogio;

        while (jogo.janela.isOpen() && jogo.estadoJogo == Jogando) {
            sf::Time dt = relogio.restart();

            jogo.atualizar(dt.asSeconds());
            pacman.moverAutomaticamente(mapa, pilulas, spritesFantasmas, dt.asSeconds());  // Passa deltaTime como argumento
            jogo.desenhar(relogio);
        }

        if (jogo.estadoJogo == Vitoria) {
            jogo.exibirMensagem("Parabéns! Você venceu!");
        }
        else if (jogo.estadoJogo == GameOver) {
            jogo.exibirMensagem("Game Over! Tente novamente.");
        }
    }

    void atualizar(float deltaTempo) {
        if (estadoJogo != Jogando) return;

        float tempoDecorrido = relogioJogo.getElapsedTime().asSeconds();

        if (todasAsFrutasColetadas()) {
            exibirMensagemTransicao("Fase " + std::to_string(faseAtual + 1) + " Concluída!");
            faseAtual++;
            if (faseAtual < mapas.size()) {
                inicializarFase(faseAtual);
            }
            else {
                estadoJogo = Vitoria;
            }
        }

        // Atualizações gerais
        pacman.atualizarAnimacao(deltaTempo);
        pacman.atualizarFortalecimento(deltaTempo);

        if (modoAtual == IA) {
            pacman.moverAutomaticamente(mapa, pilulas, spritesFantasmas, deltaTempo); // Chama o movimento automático
        }
        else if (modoAtual == Manual) {
            // Controle manual
            sf::Vector2f movimentoTentativo = proximaDirecao * pacman.velocidade * deltaTempo;
            sf::FloatRect novaPosicaoTentativa = pacman.sprite.getGlobalBounds();
            novaPosicaoTentativa.left += movimentoTentativo.x;
            novaPosicaoTentativa.top += movimentoTentativo.y;

            if (!verificarColisaoParede(novaPosicaoTentativa)) {
                pacman.direcaoAtual = proximaDirecao; // Apenas altera a direção se não houver colisão
            }

            // Tentar mover na direção atual
            sf::Vector2f movimentoPacman = pacman.direcaoAtual * pacman.velocidade * deltaTempo;
            sf::FloatRect novaPosicaoPacman = pacman.sprite.getGlobalBounds();
            novaPosicaoPacman.left += movimentoPacman.x;
            novaPosicaoPacman.top += movimentoPacman.y;

            if (!verificarColisaoParede(novaPosicaoPacman)) {
                pacman.sprite.move(movimentoPacman);
            }
        }
  
        // Atualiza cada fantasma UMA vez por frame
        Direcao direcaoPacman = converterParaDirecao(pacman.direcaoAtual);
        sf::FloatRect corpoPacman = pacman.sprite.getGlobalBounds();
        Posicao pacmanPos = { // Centro do Pac-Man, em blocos
            (corpoPacman.left + corpoPacman.width / 2) / tamanhoBloco - 0.5f,
            (corpoPacman.top + corpoPacman.height / 2) / tamanhoBloco - 0.5f
        };

        for (auto& fantasma : fantasmas) {
            if (!fantasma->saiuDaBase) {
                fantasma->tempoAteSaida -= deltaTempo;
                if (fantasma->tempoAteSaida > 0.0f) continue; // Ainda esperando na casa
                fantasma->saiuDaBase = true;
            }

            // Inky calcula o alvo a partir da posição atual do Blinky (sempre o primeiro da lista)
            if (auto* inky = dynamic_cast<Inky*>(fantasma.get())) {
                inky->blinkyPos = fantasmas[0]->posicao;
            }

            fantasma->update(pacmanPos, direcaoPacman, pacman.fortalecido, velocidadeFantasma, deltaTempo, mapa);
        }

        // Verificar colisões entre Pac-Man e pilulas
        for (auto it = pilulas.begin(); it != pilulas.end(); ) {
            if (pacman.sprite.getGlobalBounds().intersects(it->getGlobalBounds())) {
                pontos += 10; // Adiciona pontos ao jogador
                it = pilulas.erase(it); // Remove a fruta do vetor
            }
            else {
                ++it; // Avança o iterador se a fruta não foi coletada
            }
        }

        //Verificar colisoes entre o pacman e as cherrys
        for (auto it = item.begin(); it != item.end();) {
            if (pacman.sprite.getGlobalBounds().intersects(it->getGlobalBounds())) {
                pontos += pontos * 2; // Duplica a quantidade de pontos
                it = item.erase(it); // Remove o item da lista e avança o iterador 
            }
            else {
                ++it;
            }
        }

        for (auto it = apple.begin(); it != apple.end();) {
            if (pacman.sprite.getGlobalBounds().intersects(it->getGlobalBounds())) {
                pacman.vidas++; // Incrementa uma vida
                if (pacman.vidas > maxVidas) {
                    pacman.vidas = maxVidas; // Garante que o número de vidas não exceda o limite
                }
                it = apple.erase(it); // Remove o item da lista
            }
            else {
                ++it; // Avança o iterador
            }
        }

        //Verificar colisoes entre o pacman e as cherrys
        for (auto it = orange.begin(); it != orange.end();) {
            if (pacman.sprite.getGlobalBounds().intersects(it->getGlobalBounds())) {
                pacman.velocidade += pacman.velocidade * 1.5f; // Duplica a quantidade de pontos
                it = orange.erase(it); // Remove o item da lista e avança o iterador 
            }
            else {
                ++it;
            }
        }

        //Verificar colisoes entre o pacman e as cherrys
        for (auto it = beer.begin(); it != beer.end();) {
            if (pacman.sprite.getGlobalBounds().intersects(it->getGlobalBounds())) {
                pontos = pontos / 2; // Duplica a quantidade de pontos
                pacman.velocidade = pacman.velocidade * 2;
                it = beer.erase(it); // Remove o item da lista e avança o iterador 
            }
            else {
                ++it;
            }
        }

        // Verificar colisões entre Pac-Man e pilulas fortalecedoras 
        for (auto it = pilulasFortalecedoras.begin(); it != pilulasFortalecedoras.end();) {
            if (pacman.sprite.getGlobalBounds().intersects(it->getGlobalBounds())) {
                pacman.ativarFortalecimento();
                pontos += 50; // Adiciona pontos ao coletar uma pilula fortalecedora
                it = pilulasFortalecedoras.erase(it);
            }
            else {
                ++it;
            }
        }

        // Verificar colisões entre Pac-Man e fantasmas
        bool perdeuVida = false;
        for (auto& fantasma : fantasmas) {
            if (pacman.sprite.getGlobalBounds().intersects(fantasma->sprite.getGlobalBounds())) {
                if (pacman.fortalecido) {
                    // Pac-Man come o fantasma
                    fantasma->voltarParaCasa(); // Volta para a casa e espera 5 s
                    pontos += 200; // Adiciona pontos por comer o fantasma
                }
                else {
                    perdeuVida = true;
                    break;
                }
            }
        }

        // Tratado fora do loop: definirPosicoesIniciais() recria o vetor de
        // fantasmas, o que não pode acontecer enquanto ele está sendo percorrido
        if (perdeuVida) {
            pacman.vidas--;
            if (pacman.vidas <= 0) {
                estadoJogo = GameOver;
            }
            else {
                definirPosicoesIniciais(); // Reinicializa as posições do jogo
            }
        }
    }

    void processarEventos() {
        sf::Event evento;
        while (janela.pollEvent(evento)) {
            if (evento.type == sf::Event::Closed)
                janela.close();

            if (evento.type == sf::Event::KeyPressed) {
                switch (evento.key.code) {
                case sf::Keyboard::Up:
                    proximaDirecao = sf::Vector2f(0.f, -1.f); // Direção para cima
                    movimentos++; // Contabiliza o movimento
                    break;
                case sf::Keyboard::Down:
                    proximaDirecao = sf::Vector2f(0.f, 1.f); // Direção para baixo
                    movimentos++; // Contabiliza o movimento
                    break;
                case sf::Keyboard::Left:
                    proximaDirecao = sf::Vector2f(-1.f, 0.f); // Direção para a esquerda
                    movimentos++; // Contabiliza o movimento
                    break;
                case sf::Keyboard::Right:
                    proximaDirecao = sf::Vector2f(1.f, 0.f); // Direção para a direita
                    movimentos++; // Contabiliza o movimento
                    break;
                default:
                    break;
                }
            }
        }
    }

    // Registra a partida no ranking com a mesma pontuação mostrada no HUD
    void finalizarJogo(const std::string& nomeJogador) {
        jogador partida;
        partida.nome = nomeJogador;
        partida.pontos = std::max(0, calcularPontuacao());
        partida.tempo = time(nullptr); // Salva o tempo atual

        salvarNoRanking(partida);
    }

    // Cria os 4 fantasmas, um em cada célula da casa (valor 4 no mapa)
    void criarFantasmas(const std::vector<Posicao>& casa) {
        fantasmas.clear();

        if (casa.size() < 4) {
            std::cerr << "Mapa invalido: a casa dos fantasmas precisa de 4 celulas (valor 4)." << std::endl;
            return;
        }

        // Criar fantasmas com tempos de saída diferentes
        fantasmas.push_back(std::make_unique<Blinky>(texturaBlinky, casa[0], Posicao{ 9, 11 }, 2.0f));
        fantasmas.back()->tempoAteSaida = 2.0f; // Sai após 2 segundos

        fantasmas.push_back(std::make_unique<Pinky>(texturaPinky, casa[1], Posicao{ 10, 12 }, 5.0f));
        fantasmas.back()->tempoAteSaida = 4.0f; // Sai após 4 segundos

        fantasmas.push_back(std::make_unique<Inky>(texturaInky, casa[2], Posicao{ 8, 12 }, 10.0f));
        fantasmas.back()->tempoAteSaida = 6.0f; // Sai após 6 segundos

        fantasmas.push_back(std::make_unique<Clyde>(texturaClyde, casa[3], Posicao{ 9, 12 }, 15.0f));
        fantasmas.back()->tempoAteSaida = 8.0f; // Sai após 8 segundos
    }

    void executar() {
        sf::Clock relogio;

        while (janela.isOpen() && estadoJogo == Jogando) {
            // Limita o passo de tempo: depois de uma pausa (ex.: tela de transição de fase)
            // o dt seria de segundos, e os personagens dariam um salto atravessando paredes
            float dt = std::min(relogio.restart().asSeconds(), dtMaximo);
            processarEventos();
            atualizar(dt);
            desenhar(relogioJogo);
            if (estadoJogo != Jogando) break;
        }

        // Quando o jogo termina, salva o ranking
        if (estadoJogo == Vitoria || estadoJogo == GameOver) {
            // Toda partida entra no ranking, mesmo de um jogador que já jogou antes
            finalizarJogo(lerNomeJogador());

            // Exibir mensagem de vitória ou derrota
            if (estadoJogo == Vitoria) {
                exibirMensagem("Parabéns! Você venceu!");
            }
            else {
                exibirMensagem("Game Over! Tente novamente.");
            }
        }
    }

    bool verificarColisaoParede(const sf::FloatRect& objeto) {
        for (const auto& parede : paredes) {
            if (objeto.intersects(parede.getGlobalBounds())) {
                return true;
            }
        }

        // Verificar se o fantasma está fora do grid do mapa
        if (objeto.left < 0 || objeto.top < 0 ||
            objeto.left + objeto.width > larguraJanela ||
            objeto.top + objeto.height > alturaJanela) {
            return true;
        }

        return false;
    }

    bool verificarColisaoFantasmas(const Fantasma& fantasmaAtual) {
        for (const auto& outroFantasma : fantasmas) {
            if (outroFantasma.get() != &fantasmaAtual) {
                if (fantasmaAtual.sprite.getGlobalBounds().intersects(outroFantasma->sprite.getGlobalBounds())) {
                    return true;
                }
            }
        }
        return false;
    }

    void exibirMensagemTransicao(const std::string& mensagem) {
        sf::RenderWindow janelaTransicao(sf::VideoMode(600, 300), "Transição");
        janelaTransicao.setFramerateLimit(60);
        sf::Text textoMensagem(mensagem, fonte, 30);
        textoMensagem.setFillColor(sf::Color::White);
        textoMensagem.setPosition(50, 100);

        sf::Clock relogio;
        while (janelaTransicao.isOpen() && relogio.getElapsedTime().asSeconds() < 3.0f) {
            // Processa os eventos para o Windows não marcar a janela como "Não respondendo"
            sf::Event evento;
            while (janelaTransicao.pollEvent(evento)) {
                if (evento.type == sf::Event::Closed) janelaTransicao.close();
            }

            janelaTransicao.clear();
            janelaTransicao.draw(textoMensagem);
            janelaTransicao.display();
        }
        janelaTransicao.close();
    }

    void exibirMensagem(const std::string& mensagem) {
        sf::RenderWindow janelaMensagem(sf::VideoMode(600, 300), "Mensagem");
        janelaMensagem.setFramerateLimit(60);
        sf::Text textoMensagem(mensagem, fonte, 24);
        textoMensagem.setFillColor(sf::Color::White);
        textoMensagem.setPosition(50, 80);

        while (janelaMensagem.isOpen()) {
            sf::Event evento;
            while (janelaMensagem.pollEvent(evento)) {
                if (evento.type == sf::Event::Closed || evento.type == sf::Event::KeyPressed) {
                    janelaMensagem.close();
                }
            }

            janelaMensagem.clear();
            janelaMensagem.draw(textoMensagem);
            janelaMensagem.display();
        }
    }

    void desenhar(sf::Clock& relogioJogo) {
        janela.clear();

        // Elementos do jogo
        for (const auto& parede : paredes)
            janela.draw(parede);
        for (const auto& pilula : pilulas)
            janela.draw(pilula);
        for (const auto& pilulaFortalecedora : pilulasFortalecedoras)
            janela.draw(pilulaFortalecedora);
        for (const auto& itemSprite : item)
            janela.draw(itemSprite);
        for (const auto& appleSprite : apple)
            janela.draw(appleSprite);
        for (const auto& orangeSprite : orange)
            janela.draw(orangeSprite);
        for (const auto& beerSprite : beer)
            janela.draw(beerSprite);

        pacman.desenhar(janela);
        for (const auto& fantasma : fantasmas)
            fantasma->desenhar(janela);

        // Modo Desafio: escurece tudo, menos um círculo de luz ao redor do Pac-Man
        if (dificuldadeAtual == Desafio) {
            sf::RectangleShape camadaEscura(sf::Vector2f(larguraJanela, alturaJanela));
            camadaEscura.setFillColor(sf::Color(0, 0, 0, 220)); // Escuro semitransparente

            sf::CircleShape luz(30.0f); // Define o raio da luz
            luz.setOrigin(luz.getRadius(), luz.getRadius());
            luz.setPosition(pacman.sprite.getPosition() + sf::Vector2f(tamanhoBloco / 2, tamanhoBloco / 2));
            luz.setFillColor(sf::Color(0, 0, 0, 0)); // Transparente no centro
            luz.setOutlineThickness(200.0f);
            luz.setOutlineColor(sf::Color(255, 255, 255, 255)); // Mantém o escuro ao redor do círculo

            // Reaproveita a textura criada no construtor: só redesenha o conteúdo
            texturaEscura.clear();
            texturaEscura.draw(camadaEscura);
            texturaEscura.draw(luz, sf::BlendMultiply);
            texturaEscura.display();

            janela.draw(sf::Sprite(texturaEscura.getTexture()));
        }

        // Desenha a pontuação, vidas e tempo de jogo
        sf::Text textoPontuacao("Pontos: " + std::to_string(calcularPontuacao()), fonte, 20);
        textoPontuacao.setPosition(8, 370);
        janela.draw(textoPontuacao);

        sf::Text textoVidas("Vidas: " + std::to_string(pacman.vidas), fonte, 20);
        textoVidas.setPosition(8, 390);
        janela.draw(textoVidas);

        float tempoDecorrido = relogioJogo.getElapsedTime().asSeconds();
        sf::Text textoTempo("Tempo: " + std::to_string(static_cast<int>(tempoDecorrido)) + "s", fonte, 20);
        textoTempo.setPosition(8, 410);
        janela.draw(textoTempo);

        sf::Text textoMovimentos("Movimentos: " + std::to_string(movimentos), fonte, 20);
        textoMovimentos.setPosition(8, 430);
        janela.draw(textoMovimentos);

        janela.display();
    }

    void dificuldade(NivelDificuldade dificuldade) {
        dificuldadeAtual = dificuldade; // Define o estado atual da dificuldade
        switch (dificuldade) {
        case Facil:
            pacman.velocidade = 45.0f;
            pacman.vidas = 100;
            maxVidas = 100;
            break;
        case Medio:
            pacman.velocidade = 35.0f;
            pacman.vidas = 2;
            maxVidas = 3;
            pontos += pontos + 15;
            break;
        case Dificil:
            pacman.velocidade = 50.0f;
            pacman.vidas = 1;
            maxVidas = 1;
            pontos += pontos + 30;
            break;
        case Desafio:
            pacman.velocidade = 50.0f;
            pacman.vidas = 1;
            maxVidas = 1;
            pontos += pontos + 50;
            break;
        default:
            std::cout << "Nível de dificuldade inválido!" << std::endl;
            break;
        }
    }

    void rodar() {
        sf::Clock relogio;

        while (janela.isOpen()) {
            sf::Event evento;
            while (janela.pollEvent(evento)) {
                if (evento.type == sf::Event::Closed) {
                    janela.close();
                }

                if (evento.type == sf::Event::KeyPressed) {
                    switch (evento.key.code) {
                    case sf::Keyboard::Up:    proximaDirecao = sf::Vector2f(0, -1); break;
                    case sf::Keyboard::Down:  proximaDirecao = sf::Vector2f(0, 1); break;
                    case sf::Keyboard::Left:  proximaDirecao = sf::Vector2f(-1, 0); break;
                    case sf::Keyboard::Right: proximaDirecao = sf::Vector2f(1, 0); break;
                    default: break;
                    }
                }
            }

            float deltaTempo = relogio.restart().asSeconds();
            atualizar(deltaTempo);
            desenhar(relogioJogo);

            if (estadoJogo != Jogando) {
                break; // Encerrar o loop de jogo se o estado não for Jogando
            }
        }
    }
};

// Tela de escolha de dificuldade. Retorna vazio se o jogador fechar a janela ou apertar Esc
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

enum class OpcaoMenu { Jogar, IA, Ranking, Sair };

// Tela inicial. Só mostra as opções e retorna a escolhida; quem age é o main()
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

// Função principal: volta ao menu depois de cada partida ou tela, até o jogador escolher Sair
int main()
{
    while (true) {
        switch (menu()) {
        case OpcaoMenu::Jogar: {
            std::optional<NivelDificuldade> dificuldade = escolherDificuldade();
            if (dificuldade) {
                Jogo jogo;
                jogo.dificuldade(*dificuldade);
                jogo.executar();
            }
            break;
        }
        case OpcaoMenu::IA: {
            Jogo jogo;
            jogo.modoAtual = IA; // Pac-Man controlado pelo computador
            jogo.dificuldade(Facil);
            jogo.executar();
            break;
        }
        case OpcaoMenu::Ranking:
            exibirRanking();
            break;
        case OpcaoMenu::Sair:
            return 0;
        }
    }
}
//Criar novos mapas
