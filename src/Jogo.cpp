#include "Jogo.h"

#include "Config.h"
#include "Janela.h"
#include "Ranking.h"
#include "Recursos.h"

#include <algorithm>
#include <cctype>
#include <iostream>

Jogo::Jogo() : pacman(0, 0), estadoJogo(Jogando) {
    // Desenha sempre em 342x500 "virtuais"; a janela amplia para a tela do jogador
    abrirJanela(janela, { larguraJanela, alturaJanela }, "Pac-Man");

    if (!fonte.loadFromFile(caminhoFonte)) {
        std::cerr << "Erro ao carregar a fonte." << std::endl;
    }
    texturaEscura.create(larguraJanela, alturaJanela);

    // Parede (só para a colisão): bloco do tamanho exato de um bloco do mapa
    sf::Image imagemParede;
    imagemParede.create(tamanhoBloco, tamanhoBloco, corParede);
    texturaParede.loadFromImage(imagemParede);

    carregarTextura(texturaPilula, "img/item/dot.png");
    carregarTextura(texturaItem, "img/item/cherry.png", sf::Color::Red);
    carregarTextura(texturaApple, "img/item/maca.png", sf::Color::Green);
    carregarTextura(texturaOrange, "img/item/energetico.png", sf::Color::Cyan);
    carregarTextura(texturaBeer, "img/item/cerveja.png", sf::Color(255, 165, 0));
    carregarTextura(texturaPilulaFortalecedora, "img/item/pellet.png");

    carregarTexturasFantasma(texturasBlinky, "blinky", sf::Color::Red);
    carregarTexturasFantasma(texturasPinky, "pinky", sf::Color(255, 184, 255));
    carregarTexturasFantasma(texturasInky, "inky", sf::Color::Cyan);
    carregarTexturasFantasma(texturasClyde, "clyde", sf::Color(255, 184, 82));
    carregarTexturasEspeciais(texturasEspeciais);

    inicializarFase(0);
    proximaDirecao = sf::Vector2f(1.0f, 0.0f);
}

// A fase termina quando todas as pílulas foram comidas, inclusive as fortalecedoras
bool Jogo::todasAsFrutasColetadas() {
    return pilulas.empty() && pilulasFortalecedoras.empty();
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
        labirinto.montar(mapa);
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
    // As duas imagens têm 8x8 px; +5 px centraliza no bloco de 18 px
    const float centralizar = (tamanhoBloco - 8) / 2.0f;
    for (int y = 0; y < mapa.size(); ++y) {
        for (int x = 0; x < mapa[y].size(); ++x) {
            if (mapa[y][x] == Pilula) {
                sf::Sprite pilulaSprite;
                pilulaSprite.setTexture(texturaPilula);
                pilulaSprite.setPosition(x * tamanhoBloco + centralizar, y * tamanhoBloco + centralizar);
                pilulas.push_back(pilulaSprite);
            }
            else if (mapa[y][x] == PilulaFortalecedora) {
                sf::Sprite pilulaFortalecedoraSprite;
                pilulaFortalecedoraSprite.setTexture(texturaPilulaFortalecedora);
                pilulaFortalecedoraSprite.setPosition(x * tamanhoBloco + centralizar, y * tamanhoBloco + centralizar);
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
    // Os fantasmas e a boca do Pac-Man animam em todas as etapas, menos no fim de jogo
    if (etapa != Etapa::FimDeJogo) {
        for (auto& fantasma : fantasmas) fantasma->animar(deltaTempo);
    }

    switch (etapa) {
    case Etapa::Pronto:
        tempoEtapa -= deltaTempo;
        if (tempoEtapa <= 0.0f) etapa = Etapa::Jogando;
        break;

    case Etapa::Jogando:
        atualizarJogando(deltaTempo);
        break;

    case Etapa::Morrendo:
        tempoEtapa -= deltaTempo;
        if (tempoEtapa <= 0.0f) {
            if (pacman.vidas <= 0) {
                estadoJogo = GameOver;
                etapa = Etapa::FimDeJogo;
            }
            else {
                definirPosicoesIniciais(); // Reinicializa as posições do jogo
                etapa = Etapa::Pronto;
                tempoEtapa = duracaoPronto;
            }
        }
        break;

    case Etapa::FaseConcluida:
        tempoEtapa -= deltaTempo;
        if (tempoEtapa <= 0.0f) {
            faseAtual++;
            if (faseAtual < mapas.size()) {
                inicializarFase(faseAtual);
                etapa = Etapa::Pronto;
                tempoEtapa = duracaoPronto;
            }
            else {
                estadoJogo = Vitoria;
                etapa = Etapa::FimDeJogo;
            }
        }
        break;

    case Etapa::FimDeJogo:
        break; // Espera o nome (ver processarEventos)
    }
}

void Jogo::atualizarJogando(float deltaTempo) {
    // Atualizações gerais
    pacman.atualizarFortalecimento(deltaTempo);
    pacman.atualizarTurbo(deltaTempo);
    sf::Vector2f posicaoAntes = pacman.sprite.getPosition();

    if (modoAtual == IA) {
        // Converte pílulas e fantasmas (fora da casa) em blocos do mapa para a IA
        auto blocoDe = [](sf::Vector2f pixels) {
            return sf::Vector2i(static_cast<int>(pixels.x) / tamanhoBloco, static_cast<int>(pixels.y) / tamanhoBloco);
        };
        std::vector<sf::Vector2i> blocosComPilula;
        for (const auto& p : pilulas) blocosComPilula.push_back(blocoDe(p.getPosition()));
        for (const auto& p : pilulasFortalecedoras) blocosComPilula.push_back(blocoDe(p.getPosition()));

        std::vector<sf::Vector2i> blocosFantasmas;
        for (const auto& f : fantasmas) {
            if (f->saiuDaBase && !f->comido) {
                blocosFantasmas.emplace_back(static_cast<int>(std::round(f->posicao.x)), static_cast<int>(std::round(f->posicao.y)));
            }
        }

        pacman.moverAutomaticamente(mapa, blocosComPilula, blocosFantasmas, deltaTempo);
    }
    else if (modoAtual == Manual) {
        // Controle manual
        sf::Vector2f movimentoTentativo = proximaDirecao * pacman.velocidadeAtual() * deltaTempo;
        sf::FloatRect novaPosicaoTentativa = pacman.sprite.getGlobalBounds();
        novaPosicaoTentativa.left += movimentoTentativo.x;
        novaPosicaoTentativa.top += movimentoTentativo.y;

        if (!verificarColisaoParede(novaPosicaoTentativa)) {
            pacman.direcaoAtual = proximaDirecao; // Apenas altera a direção se não houver colisão
        }

        // Tentar mover na direção atual
        sf::Vector2f movimentoPacman = pacman.direcaoAtual * pacman.velocidadeAtual() * deltaTempo;
        sf::FloatRect novaPosicaoPacman = pacman.sprite.getGlobalBounds();
        novaPosicaoPacman.left += movimentoPacman.x;
        novaPosicaoPacman.top += movimentoPacman.y;

        if (!verificarColisaoParede(novaPosicaoPacman)) {
            pacman.sprite.move(movimentoPacman);
        }
    }

    // A boca só abre e fecha enquanto o Pac-Man anda
    pacman.movendo = (pacman.sprite.getPosition() != posicaoAntes);
    pacman.atualizarAnimacao(deltaTempo);

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

        fantasma->update(pacmanPos, direcaoPacman, pacman.fortalecido, velocidadeFantasmaAtual, deltaTempo, mapa);
    }

    // Verificar colisões entre Pac-Man e pilulas
    for (auto it = pilulas.begin(); it != pilulas.end(); ) {
        if (pacman.sprite.getGlobalBounds().intersects(it->getGlobalBounds())) {
            pontos += 10; // Adiciona pontos ao jogador
            sons.tocarSeLivre(Som::Comer);
            it = pilulas.erase(it); // Remove a fruta do vetor
        }
        else {
            ++it; // Avança o iterador se a fruta não foi coletada
        }
    }

    // Cereja: +100 pontos
    for (auto it = item.begin(); it != item.end();) {
        if (pacman.sprite.getGlobalBounds().intersects(it->getGlobalBounds())) {
            pontos += 100;
            sons.tocar(Som::Item);
            it = item.erase(it); // Remove o item da lista e avança o iterador
        }
        else {
            ++it;
        }
    }

    // Maçã: +1 vida (até o máximo da dificuldade)
    for (auto it = apple.begin(); it != apple.end();) {
        if (pacman.sprite.getGlobalBounds().intersects(it->getGlobalBounds())) {
            pacman.vidas++; // Incrementa uma vida
            if (pacman.vidas > maxVidas) {
                pacman.vidas = maxVidas; // Garante que o número de vidas não exceda o limite
            }
            sons.tocar(Som::VidaExtra);
            it = apple.erase(it); // Remove o item da lista
        }
        else {
            ++it; // Avança o iterador
        }
    }

    // Energético: velocidade x2,5 por alguns segundos
    for (auto it = orange.begin(); it != orange.end();) {
        if (pacman.sprite.getGlobalBounds().intersects(it->getGlobalBounds())) {
            pacman.ativarTurbo(2.5f, duracaoTurbo);
            sons.tocar(Som::Item);
            it = orange.erase(it); // Remove o item da lista e avança o iterador
        }
        else {
            ++it;
        }
    }

    // Cerveja: perde metade dos pontos, mas fica 2x mais rápido por alguns segundos
    for (auto it = beer.begin(); it != beer.end();) {
        if (pacman.sprite.getGlobalBounds().intersects(it->getGlobalBounds())) {
            pontos = pontos / 2;
            pacman.ativarTurbo(2.0f, duracaoTurbo);
            sons.tocar(Som::Item);
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
            sons.tocarSeLivre(Som::Comer);
            it = pilulasFortalecedoras.erase(it);
        }
        else {
            ++it;
        }
    }

    // Verificar colisões entre Pac-Man e fantasmas (olhos voltando para a casa não contam)
    bool perdeuVida = false;
    for (auto& fantasma : fantasmas) {
        if (fantasma->comido) continue;
        if (pacman.sprite.getGlobalBounds().intersects(fantasma->sprite.getGlobalBounds())) {
            if (pacman.fortalecido) {
                // Pac-Man come o fantasma
                fantasma->voltarParaCasa(); // Vira olhos que voltam para a casa
                pontos += 200; // Adiciona pontos por comer o fantasma
                sons.tocar(Som::ComerFantasma);
            }
            else {
                perdeuVida = true;
                break;
            }
        }
    }

    // Tratado fora do loop: ao fim da animação de morte, definirPosicoesIniciais() recria o
    // vetor de fantasmas, o que não pode acontecer enquanto ele está sendo percorrido
    if (perdeuVida) {
        pacman.vidas--;
        sons.pararTodos();
        sons.tocar(Som::Morte);
        etapa = Etapa::Morrendo;
        tempoEtapa = duracaoMorte;
        return;
    }

    if (todasAsFrutasColetadas()) {
        sons.pararTodos();
        etapa = Etapa::FaseConcluida;
        tempoEtapa = duracaoFaseConcluida;
    }
}

void Jogo::processarEventos() {
    sf::Event evento;
    while (janela.pollEvent(evento)) {
        if (evento.type == sf::Event::Closed)
            janela.close();

        // Redimensionar e F11 (tela cheia)
        if (tratarEventoDeJanela(janela, evento, { larguraJanela, alturaJanela }, "Pac-Man")) continue;

        // Fim de jogo: o teclado serve para digitar o nome
        if (etapa == Etapa::FimDeJogo) {
            if (evento.type == sf::Event::TextEntered) {
                char32_t c = evento.text.unicode;
                bool permitido = c < 128 && (std::isalnum(static_cast<int>(c)) || c == ' ' || c == '-' || c == '_');
                if (permitido && nomeDigitado.size() < tamanhoMaximoNome) {
                    nomeDigitado += static_cast<char>(c);
                }
            }
            else if (evento.type == sf::Event::KeyPressed) {
                if (evento.key.code == sf::Keyboard::BackSpace && !nomeDigitado.empty()) nomeDigitado.pop_back();
                else if (evento.key.code == sf::Keyboard::Enter) nomeConfirmado = true;
                else if (evento.key.code == sf::Keyboard::Escape) sairSemSalvar = true;
            }
            continue;
        }

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
            case sf::Keyboard::M:
                Sons::alternarMudo();
                if (Sons::mudo()) sons.pararTodos();
                break;
            default:
                break;
            }
        }
    }
}

void Jogo::finalizarJogo(const std::string& nomeJogador) {
    // Espaços viram "_" porque o arquivo do ranking separa os campos por espaço
    std::string nome = nomeJogador;
    nome.erase(0, nome.find_first_not_of(' '));
    nome.erase(nome.find_last_not_of(' ') + 1);
    std::replace(nome.begin(), nome.end(), ' ', '_');

    jogador partida;
    partida.nome = nome.empty() ? "Jogador" : nome;
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
    fantasmas.push_back(std::make_unique<Blinky>(texturasBlinky, texturasEspeciais, casa[0], Posicao{ 9, 11 }));
    fantasmas.back()->tempoAteSaida = 2.0f; // Sai após 2 segundos

    fantasmas.push_back(std::make_unique<Pinky>(texturasPinky, texturasEspeciais, casa[1], Posicao{ 10, 12 }));
    fantasmas.back()->tempoAteSaida = 4.0f; // Sai após 4 segundos

    fantasmas.push_back(std::make_unique<Inky>(texturasInky, texturasEspeciais, casa[2], Posicao{ 8, 12 }));
    fantasmas.back()->tempoAteSaida = 6.0f; // Sai após 6 segundos

    fantasmas.push_back(std::make_unique<Clyde>(texturasClyde, texturasEspeciais, casa[3], Posicao{ 9, 12 }));
    fantasmas.back()->tempoAteSaida = 8.0f; // Sai após 8 segundos
}

void Jogo::executar() {
    sf::Clock relogio;
    sons.tocar(Som::Inicio);

    while (janela.isOpen() && !nomeConfirmado && !sairSemSalvar) {
        // Limita o passo de tempo: depois de uma pausa (ex.: janela arrastada) o dt seria de
        // segundos, e os personagens dariam um salto atravessando paredes
        float dt = std::min(relogio.restart().asSeconds(), dtMaximo);
        processarEventos();
        atualizar(dt);
        desenhar(relogioJogo);
    }

    // Toda partida terminada entra no ranking, a não ser que o jogador saia com Esc
    if (nomeConfirmado) {
        finalizarJogo(nomeDigitado);
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

void Jogo::desenhar(sf::Clock& relogioJogo) {
    janela.clear();
    float segundos = relogioJogo.getElapsedTime().asSeconds();

    // Labirinto: azul; ao concluir a fase, pisca em branco
    bool piscarLabirinto = etapa == Etapa::FaseConcluida && static_cast<int>(tempoEtapa * 4) % 2 == 0;
    labirinto.desenhar(janela, piscarLabirinto ? sf::Color::White : corParede);

    // Elementos do jogo (as pílulas fortalecedoras piscam)
    for (const auto& pilula : pilulas)
        janela.draw(pilula);
    if (static_cast<int>(segundos * 4) % 2 == 0 || etapa != Etapa::Jogando) {
        for (const auto& pilulaFortalecedora : pilulasFortalecedoras)
            janela.draw(pilulaFortalecedora);
    }
    for (const auto& itemSprite : item)
        janela.draw(itemSprite);
    for (const auto& appleSprite : apple)
        janela.draw(appleSprite);
    for (const auto& orangeSprite : orange)
        janela.draw(orangeSprite);
    for (const auto& beerSprite : beer)
        janela.draw(beerSprite);

    // Pac-Man e fantasmas (os fantasmas somem durante a morte e a troca de fase)
    if (etapa == Etapa::Morrendo) {
        pacman.desenharMorte(janela, 1.0f - tempoEtapa / duracaoMorte);
    }
    else {
        pacman.desenhar(janela);
    }
    if (etapa != Etapa::Morrendo && etapa != Etapa::FaseConcluida) {
        for (const auto& fantasma : fantasmas)
            fantasma->desenhar(janela, pacman.fortalecido, pacman.temporizadorFortalecimento);
    }

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

    desenharMensagens();
    desenharHud();
    janela.display();
}

void Jogo::textoCentralizado(const std::string& texto, unsigned tamanho, float y, sf::Color cor) {
    sf::Text t(texto, fonte, tamanho);
    t.setFillColor(cor);
    t.setOutlineColor(sf::Color::Black);
    t.setOutlineThickness(2.0f);
    t.setPosition(std::round((larguraJanela - t.getLocalBounds().width) / 2.0f), y);
    janela.draw(t);
}

void Jogo::desenharMensagens() {
    // A linha 15 do mapa é um corredor em todas as fases: os avisos ficam ali, como no arcade
    const float linhaAviso = 15 * tamanhoBloco - 1.0f;

    if (etapa == Etapa::Pronto) {
        if (faseAtual > 0) textoCentralizado("FASE " + std::to_string(faseAtual + 1), 16, 9 * tamanhoBloco, sf::Color::Cyan);
        textoCentralizado("PRONTO!", 18, linhaAviso, corDestaque);
    }
    else if (etapa == Etapa::FaseConcluida) {
        textoCentralizado("FASE CONCLUIDA!", 18, linhaAviso, corDestaque);
    }
    else if (etapa == Etapa::FimDeJogo) {
        // Painel no centro do labirinto
        sf::RectangleShape painel({ 300.0f, 200.0f });
        painel.setPosition((larguraJanela - 300.0f) / 2.0f, 90.0f);
        painel.setFillColor(sf::Color(0, 0, 0, 230));
        painel.setOutlineColor(corParede);
        painel.setOutlineThickness(3.0f);
        janela.draw(painel);

        bool venceu = estadoJogo == Vitoria;
        textoCentralizado(venceu ? "VITORIA!" : "GAME OVER", 26, 105.0f, venceu ? corDestaque : sf::Color::Red);
        textoCentralizado("PONTOS: " + std::to_string(std::max(0, calcularPontuacao())), 16, 145.0f, sf::Color::White);
        textoCentralizado("DIGITE SEU NOME", 14, 180.0f, sf::Color(170, 170, 170));

        // Caixa de texto com cursor piscando
        sf::RectangleShape caixa({ 220.0f, 30.0f });
        caixa.setPosition((larguraJanela - 220.0f) / 2.0f, 202.0f);
        caixa.setFillColor(sf::Color::Black);
        caixa.setOutlineColor(sf::Color::White);
        caixa.setOutlineThickness(2.0f);
        janela.draw(caixa);

        bool cursor = static_cast<int>(relogioJogo.getElapsedTime().asSeconds() * 2) % 2 == 0;
        sf::Text nome(nomeDigitado + (cursor ? "_" : ""), fonte, 16);
        nome.setFillColor(corDestaque);
        nome.setPosition(caixa.getPosition().x + 8.0f, 207.0f);
        janela.draw(nome);

        textoCentralizado("ENTER salva   ESC sai", 12, 252.0f, sf::Color(170, 170, 170));
    }
}

void Jogo::desenharHud() {
    const float topo = static_cast<float>(alturaMapa);
    const sf::Color cinza(170, 170, 170);

    auto escrever = [&](const std::string& texto, unsigned tamanho, float x, float y, sf::Color cor, bool alinharDireita = false) {
        sf::Text t(texto, fonte, tamanho);
        t.setFillColor(cor);
        float px = alinharDireita ? x - t.getLocalBounds().width : x;
        t.setPosition(std::round(px), std::round(y));
        janela.draw(t);
    };

    const float esquerda = 10.0f, direita = larguraJanela - 10.0f;

    // Linha 1: rótulos; linha 2: valores
    escrever("PONTOS", 14, esquerda, topo + 6, cinza);
    escrever(std::to_string(calcularPontuacao()), 22, esquerda, topo + 22, sf::Color::White);
    escrever("FASE", 14, direita, topo + 6, cinza, true);
    escrever(std::to_string(faseAtual + 1) + "/" + std::to_string(mapas.size()), 22, direita, topo + 22, sf::Color::White, true);

    // Vidas como ícones do Pac-Man
    sf::Sprite icone(pacman.texturas[1]);
    for (int i = 0; i < pacman.vidas; ++i) {
        icone.setPosition(esquerda + i * 20.0f, topo + 60);
        janela.draw(icone);
    }

    // Efeitos ativos
    if (pacman.tempoTurbo > 0.0f) {
        escrever("TURBO " + std::to_string(static_cast<int>(std::ceil(pacman.tempoTurbo))) + "s", 14, direita, topo + 60, sf::Color::Cyan, true);
    }
    else if (pacman.fortalecido) {
        escrever("PODER " + std::to_string(static_cast<int>(std::ceil(pacman.temporizadorFortalecimento))) + "s", 14, direita, topo + 60, sf::Color(80, 120, 255), true);
    }

    // Linha de baixo: tempo, movimentos e som
    std::string tempo = "TEMPO " + std::to_string(static_cast<int>(relogioJogo.getElapsedTime().asSeconds())) + "s";
    escrever(tempo + "   MOV " + std::to_string(movimentos), 12, esquerda, topo + 94, cinza);
    escrever(Sons::mudo() ? "M: SOM OFF" : "M: SOM ON", 12, direita, topo + 94, cinza, true);
}

// O Pac-Man anda sempre na mesma velocidade; o que muda é a velocidade dos fantasmas e as vidas.
// Médio, Difícil e Desafio começam com um bônus de pontos.
void Jogo::dificuldade(NivelDificuldade dificuldade) {
    dificuldadeAtual = dificuldade;
    pacman.velocidade = velocidadePacman;

    switch (dificuldade) {
    case Facil:
        velocidadeFantasmaAtual = 2.0f;
        pacman.vidas = 5;
        break;
    case Medio:
        velocidadeFantasmaAtual = 2.4f;
        pacman.vidas = 3;
        pontos = 15;
        break;
    case Dificil:
        velocidadeFantasmaAtual = 2.8f;
        pacman.vidas = 2;
        pontos = 30;
        break;
    case Desafio: // Como o Difícil, mas com 1 vida e a tela escura
        velocidadeFantasmaAtual = 2.8f;
        pacman.vidas = 1;
        pontos = 50;
        break;
    }

    maxVidas = pacman.vidas; // A maçã recupera vidas só até o valor inicial
}
