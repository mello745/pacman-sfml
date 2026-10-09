#include "Jogo.h"

#include "Config.h"
#include "Ranking.h"
#include "Recursos.h"

#include <algorithm>
#include <iostream>

Jogo::Jogo() : janela(sf::VideoMode(larguraJanela, alturaJanela), "Pac-Man"), pacman(0, 0), estadoJogo(Jogando) {
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

bool Jogo::todasAsFrutasColetadas() {
    return pilulas.empty(); // Verifica se não há mais frutas
}

void Jogo::inicializarFase(int indiceFase) {
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

void Jogo::criarParedes() {
    for (int y = 0; y < mapa.size(); ++y) {
        for (int x = 0; x < mapa[y].size(); ++x) {
            if (mapa[y][x] == Parede) {
                sf::Sprite paredeSprite;
                paredeSprite.setTexture(texturaParede);
                paredeSprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
                paredes.push_back(paredeSprite);
            }
        }
    }
}

void Jogo::criarPilulas() {
    for (int y = 0; y < mapa.size(); ++y) {
        for (int x = 0; x < mapa[y].size(); ++x) {
            if (mapa[y][x] == Pilula) {
                sf::Sprite pilulaSprite;
                pilulaSprite.setTexture(texturaPilula);
                pilulaSprite.setPosition(x * tamanhoBloco + tamanhoBloco / 4, y * tamanhoBloco + tamanhoBloco / 4);
                pilulaSprite.setScale(1.0f, 1.0f);
                pilulas.push_back(pilulaSprite);
            }
            else if (mapa[y][x] == PilulaFortalecedora) {
                sf::Sprite pilulaFortalecedoraSprite;
                pilulaFortalecedoraSprite.setTexture(texturaPilulaFortalecedora);
                pilulaFortalecedoraSprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
                pilulasFortalecedoras.push_back(pilulaFortalecedoraSprite);
            }
        }
    }
}

void Jogo::definirPosicoesIniciais() {
    std::vector<Posicao> casaDosFantasmas;

    // Percorre o mapa para encontrar as posições do Pac-Man e dos fantasmas
    for (int y = 0; y < mapa.size(); ++y) {
        for (int x = 0; x < mapa[y].size(); ++x) {
            if (mapa[y][x] == InicioPacman) {
                pacman.sprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
            }
            else if (mapa[y][x] == CasaFantasma) {
                casaDosFantasmas.push_back({ static_cast<float>(x), static_cast<float>(y) });
            }
        }
    }

    criarFantasmas(casaDosFantasmas);
}

void Jogo::criarItem1() {
    for (int y = 0; y < mapa.size(); ++y) {
        for (int x = 0; x < mapa[y].size(); ++x) {
            if (mapa[y][x] == Cereja) {
                sf::Sprite itemSprite;
                itemSprite.setTexture(texturaItem);
                itemSprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
                ajustarAoBloco(itemSprite);
                item.push_back(itemSprite);
            }
            else if (mapa[y][x] == Maca) {
                sf::Sprite appleSprite;
                appleSprite.setTexture(texturaApple);
                appleSprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
                ajustarAoBloco(appleSprite);
                apple.push_back(appleSprite);
            }
        }
    }
}

void Jogo::criarItem2() {
    for (int y = 0; y < mapa.size(); ++y) {
        for (int x = 0; x < mapa[y].size(); ++x) {
            if (mapa[y][x] == Energetico) {
                sf::Sprite orangeSprite;
                orangeSprite.setTexture(texturaOrange);
                orangeSprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
                ajustarAoBloco(orangeSprite);
                orange.push_back(orangeSprite);
            }
            else if (mapa[y][x] == Cerveja) {
                sf::Sprite beerSprite;
                beerSprite.setTexture(texturaBeer);
                beerSprite.setPosition(x * tamanhoBloco, y * tamanhoBloco);
                ajustarAoBloco(beerSprite);
                beer.push_back(beerSprite);
            }
        }
    }
}

int Jogo::calcularPontuacao() {
    return pontos - (movimentos * 2); // Penaliza 2 pontos por movimento
}

Direcao Jogo::converterParaDirecao(const sf::Vector2f& vetor) {
    if (vetor == sf::Vector2f(0, -1)) return cima;
    if (vetor == sf::Vector2f(0, 1)) return baixo;
    if (vetor == sf::Vector2f(-1, 0)) return esquerda;
    if (vetor == sf::Vector2f(1, 0)) return direita;
    return parado; // Caso nenhuma direção seja identificada
}

void Jogo::atualizar(float deltaTempo) {
    if (estadoJogo != Jogando) return;

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

void Jogo::processarEventos() {
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

void Jogo::finalizarJogo(const std::string& nomeJogador) {
    jogador partida;
    partida.nome = nomeJogador;
    partida.pontos = std::max(0, calcularPontuacao());
    partida.tempo = time(nullptr); // Salva o tempo atual

    salvarNoRanking(partida);
}

void Jogo::criarFantasmas(const std::vector<Posicao>& casa) {
    fantasmas.clear();

    if (casa.size() < 4) {
        std::cerr << "Mapa invalido: a casa dos fantasmas precisa de 4 celulas (valor 4)." << std::endl;
        return;
    }

    // Criar fantasmas com tempos de saída diferentes
    fantasmas.push_back(std::make_unique<Blinky>(texturaBlinky, casa[0], Posicao{ 9, 11 }));
    fantasmas.back()->tempoAteSaida = 2.0f; // Sai após 2 segundos

    fantasmas.push_back(std::make_unique<Pinky>(texturaPinky, casa[1], Posicao{ 10, 12 }));
    fantasmas.back()->tempoAteSaida = 4.0f; // Sai após 4 segundos

    fantasmas.push_back(std::make_unique<Inky>(texturaInky, casa[2], Posicao{ 8, 12 }));
    fantasmas.back()->tempoAteSaida = 6.0f; // Sai após 6 segundos

    fantasmas.push_back(std::make_unique<Clyde>(texturaClyde, casa[3], Posicao{ 9, 12 }));
    fantasmas.back()->tempoAteSaida = 8.0f; // Sai após 8 segundos
}

void Jogo::executar() {
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

bool Jogo::verificarColisaoParede(const sf::FloatRect& objeto) {
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

void Jogo::exibirMensagemTransicao(const std::string& mensagem) {
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

void Jogo::exibirMensagem(const std::string& mensagem) {
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

void Jogo::desenhar(sf::Clock& relogioJogo) {
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

void Jogo::dificuldade(NivelDificuldade dificuldade) {
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
