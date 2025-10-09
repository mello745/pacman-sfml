🟡 Pac-Man — Projeto em C++ com SFML e Qt

Trabalho desenvolvido durante a disciplina de Algoritmos II, com o objetivo de aplicar Programação Orientada a Objetos (POO) e explorar o uso das bibliotecas SFML (Simple and Fast Multimedia Library) e Qt Framework para criação de interfaces e jogos 2D.

Sobre o Projeto

Este projeto é uma recriação do clássico Pac-Man, desenvolvida em C++, combinando duas tecnologias poderosas:

SFML, para renderização, movimentação e lógica de jogo;

Qt, para as telas de menu, vitória e game over.

O jogo implementa movimentação do Pac-Man, coleta de pellets, comportamento dos fantasmas, sistema de pontuação e telas interativas de fim de jogo.

Tecnologias Utilizadas
Tecnologia	Função
C++	Linguagem principal de desenvolvimento
SFML 2.5+	Biblioteca para gráficos, áudio e controle de eventos
Qt Framework (5 ou 6)	Interface gráfica, menus e diálogos
Programação Orientada a Objetos	Estruturação modular e reutilizável do código
Conceitos Aplicados

Durante o desenvolvimento foram aplicados os seguintes conceitos de POO:

Criação e relacionamento entre classes (Pacman, Ghost, Pinky, Pellet, Game, MainWindow, etc.);

Encapsulamento e abstração de comportamentos;

Herança e polimorfismo (ex: Pinky herdando de Ghost);



Funcionalidades Principais:

✅ Movimento suave do Pac-Man com animação 

✅ Fantasmas com IA básica (seguem e reagem ao Pac-Man)

✅ Sistema de pontuação e pellets colecionáveis

✅ Modos de jogo: normal, vitória e game over

✅ Interface visual com Qt (menus, tela de vitória e derrota)

✅ Sons e sprites animados

✅ Sistema de ranking e pontuação persistente (arquivo .txt)⚙️ Como Compilar e Executar

Requisitos

Compilador C++17+

Qt 5/6 instalado

SFML 2.5+ instalada

CMake (opcional)

Compilação (exemplo com g++ e SFML)
g++ src/*.cpp -o Pacman -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio `pkg-config --cflags --libs Qt5Widgets`



Autor

Gustavo Corrêa de Mello
Itajaí — SC
gustavocmello27@gmail.com

LinkedIn = www.linkedin.com/in/gustavo-correa-de-mello-772a8834a

GitHub = www.github.com/mello745

Licença

Este projeto foi desenvolvido para fins educacionais, como parte da disciplina Algoritmos II.
Você é livre para estudar, modificar e aprimorar o código com os devidos créditos.
