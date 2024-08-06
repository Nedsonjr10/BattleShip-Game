# Projeto - Jogo Batalha Naval

## Disciplina:
Estruturas de Dados Lineares
## Professor:
Gibson Barbosa

## EXPLICAÇÃO DAS ESTRUTURAS QUE ESTÃO EM FORMATOS DE COMENTARIOS 

essas estrturas são as que a gente não  utiliza no codigo, mas implementamos por que foi solicitado no arquivo do projeto que deveria ter as estruturas ( inserir, remover e printar) as que a gente nao usa, deixamos em formato de comentario 

## Technologies used

C SDL/SDL2 TTF

## METODOS PARA RODAR O JOGO

--> O arquivo JOGO.C é o jogo sem a interface gráfica e pode ser rodado em qualquer terminal da linguagem C

--> E a pasta JOGOCOMINTERFACE, o jogo rodando com a interface apresentada, ele precisa ter o complilador de SDL2/TTF, no meu caso fui no replit por esse link https://replit.com/@yourusername/SDL2-For-C entra na pasta compiler-bash e mude no código 0 para 1, com isso, voce coloca o código e da run que ele irá rodar perfeitamente
## BREVE COMENTARIO DO JOGO

Batalha Naval é um jogo de estratégia para dois jogadores, disputado em um tabuleiro de 10x10. Cujo o objetivo é derrubar todos os navios do seu oponente para se sagrar vencedor.

## REGRAS DO JOGO

--- Tabuleiro e Navios:​

. O jogo é disputado em um tabuleiro de 10x10.​

. Cada jogador possui 4 navios distribuídos aleatoriamente no seu tabuleiro.​

---Objetivo:​

. O objetivo de cada jogador é destruir os 4 navios do oponente.​

---Turnos:​

. Os jogadores alternam seus turnos para realizar ataques.​

. Durante o seu turno, um jogador escolhe uma coordenada no tabuleiro do oponente para atacar. As coordenadas são denotadas por pares (x, y), onde x é a linha e y é a coluna.​

. Se a coordenada escolhida não foi atacada anteriormente, o jogador realiza um ataque.​

. Se a coordenada já foi atacada antes, o jogador tem direito a um novo ataque até que escolha uma coordenada ainda não atacada.​

---Ataque:​

. O resultado de um ataque é determinado aleatoriamente.​

. Se o ataque acertar um navio do oponente, o navio é afundado.​

. Se o ataque errar, não acontece nada e o turno passa para o próximo jogador.​

---Vitória:​

. O jogo continua até que um dos jogadores tenha seus 4 navios destruídos.​

. O primeiro jogador a destruir todos os navios do oponente vence a partida.​

​

## Authors

Diogo Herrera --> https://github.com/DiogoHerreraa

João Pedo Dias --> https://github.com/Joaoadiaas

Nedson Bomman --> https://github.com/Nedsonjr10
