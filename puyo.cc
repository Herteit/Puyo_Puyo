#include <SFML/Graphics.hpp>
#include <cassert>
#include <random>
#include <math.h>
#include <iostream>
using namespace std;
using namespace sf;

//on définit les constantes hors du main pour ne pas avoir à les appeler à chaque fois qu'elles sont utilisées dans un programme
const int NBCOLORS = 5;
const int HEIGHT = 600;
const int WIDTH = 900;
const int FALLSPEED = 10;

const int SIZEPUYO = 1/30 * WIDTH, WIDTHMAT = 6, HEIGHTMAT = 12, WAITINGPOSX = WIDTH /2 - SIZEPUYO, WAITINGPOSY = 100;

const char VOID = 'v', WHITE = 'w', RED = 'r', YELLOW = 'y', BLUE = 'b', GREEN = 'g', PURPLE = 'p';

const float DELAY = 0.2f;

struct Pos {
	int x ;
	int y ;
};

struct Block {
	bool exist;
	char color;
	int groupID;
};

struct BlockFall {
	int orient ;
	char color1 ;
	char color2 ; 
	Pos posMat ; 
	int speed ; 
};

struct Player {
	BlockFall bf1;
	BlockFall bf2 ;
	Block blocks[WIDTHMAT][HEIGHTMAT] ;
	int score ;
};

struct Game {
	Player p1;
	Player p2; 
};

//cette structure est là pour avoir une fonction Math::random (bon pour l'instant elle marche pas)
struct Math {
  static float random() {
    static mt19937 engine(time(nullptr));
    uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(engine);
  }
};

//initBlockFall
void initBlockFall(BlockFall& bf1, const BlockFall& bf2) {
	bf1.orient = 2;
	bf1.color1 = bf2.color1;
	bf1.color2 = bf2.color2;
	bf1.posMat.x = WIDTHMAT /2 -1;
	bf1.posMat.y = 0;
	bf1.speed = bf2.speed;
}

char switchColor (int nb) {
	char color ;
	switch (nb) {
		case 0 :
			color = YELLOW;
		break;
		case 1 : 
			color = RED ; 
		break ; 
		case 2 : 
			color = BLUE ; 
		break ; 
		case 3 : 
			color = GREEN ;
		break ;
		case 4 :
			color = PURPLE ;
		break ;
		default : 
			color = VOID ;
		break ;	
	}
	return color;
}

int random0toNb (int nb) {
	return (nb+1) * Math::random();
}

//randBlockFall
BlockFall randBlockFall() {
    BlockFall randBlock;
	randBlock.orient = 3;
	randBlock.color1 = switchColor(random0toNb(NBCOLORS));
	randBlock.color2 = switchColor(random0toNb(NBCOLORS));
	randBlock.speed = FALLSPEED;
    return randBlock;
}


void startTour(Player& p1) {
	initBlockFall(p1.bf1, p1.bf2);
	p1.bf2 = randBlockFall();
	p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y].color = p1.bf1.color1;
	p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y].exist = true;
}

bool blockTest(const Player& player, int i, int j) {
    assert(0 <= i && i < WIDTHMAT);
    assert(0 <= j && j < HEIGHTMAT);
	return !player.blocks[i][j].exist;
}

void doGravityOnAll (Player& player) {
	int i, j, k;
	for (i = 0; i < WIDTHMAT -1; i++) {
		for (j = 0; j<HEIGHTMAT-1;j++){
			if (blockTest(player, i+1, j)){
				for (k = j; k>=0; k--) {
					player.blocks[i+1][j].exist = player.blocks[i][j].exist ;
					player.blocks[i+1][j].color = player.blocks[i][j].color ;
					player.blocks[i][j].exist = false ;
					player.blocks[i][j].color = VOID;
				}
			}
		}
	}
}

bool continueFall(Block blocks[WIDTHMAT][HEIGHTMAT], BlockFall bf) {
	bool test;
	switch(bf.orient){
		case 0 : 
			test = ((blockTest(blocks, bf.posMat.x, bf.posMat.y + 1)) || (blockTest(blocks, bf.posMat.x - 1, bf.posMat.y +1)) || (bf.posMat.y < HEIGHTMAT - 1));
		break;
		case 2 : 
			test = ((blockTest(blocks, bf.posMat.x, bf.posMat.y + 1)) || (blockTest(blocks , bf.posMat.x + 1, bf.posMat.y + 1)) || (bf.posMat.y < HEIGHTMAT -1));
		break;
		case 1 : 
			test = ((blockTest(blocks, bf.posMat.x, bf.posMat.y + 1)) || (bf.posMat.y < HEIGHTMAT -1));
		break;
		case 3 : 
			test = ((blockTest(blocks, bf.posMat.x, bf.posMat.y +2)) || (bf.posMat.y +1 < HEIGHTMAT -1));
		break;
	}
	return test;
}

void blockDown (Player p1) {
	switch (p1.bf1.orient) {
		case 0 : { 
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y - 1].color = p1.bf1.color2;
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y - 1].exist = true;
		} break;
		case 2 : {
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y + 1].color = p1.bf1.color2;
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y + 1].exist = true;
		} break;
		case 1 : {
			p1.blocks[p1.bf1.posMat.x + 1][p1.bf1.posMat.y].color = p1.bf1.color2;
			p1.blocks[p1.bf1.posMat.x + 1][p1.bf1.posMat.y].exist = true;
		} break;
		case 3 : {
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y + 1].color = p1.bf1.color2;
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y + 1].exist = true;
		} break;
	}
}

//AttribuerGroupe - Anton

//checkAllChains - Anton not finito
void checkAllChains(Block mat[WIDTHMAT][HEIGHTMAT]) {
	int baseGroupId = 1;
	for (int i = 0; i < WIDTHMAT; i++) {
		for (int j = 0; j < HEIGHTMAT; j++) {

		}
	}
}

bool blockAtStart (Block blocks[WIDTHMAT][HEIGHTMAT]) {
	return (blockTest(blocks,0,(WIDTHMAT/2 -1)) || blockTest(blocks,0,WIDTHMAT/2));
}

int countNbBlocksEqualID (Block mat[WIDTHMAT][HEIGHTMAT], int ID) {
	int nb = 0; 
	for (int i = 0 ; i < WIDTHMAT ; i++) {
		for (int j = 0 ; j < HEIGHTMAT ; j++) {
			if (mat[i][j].groupID == ID) {
				nb ++ ;
			}
		}
	}
	return nb;
}

void setMalusOnPlayer (Player p1, int reps) {
	int nb ; 
	for (int i = 0 ; i < reps ; i++) {
		nb = random0toNb(WIDTHMAT);
		if (p1.blocks[0][nb].exist == false) {
			p1.blocks[i][0].color = WHITE ;
			p1.blocks[i][0].exist = true ; 
			doGravityOnAll(p1);		
		}
	}
}

void resetBlocksForID (Block block[WIDTHMAT][HEIGHTMAT], int ID) {
	for (int i = 0 ; i < WIDTHMAT ; i++) {
		for (int j = 0 ; j < HEIGHTMAT ; j++) {
			if (block[i][j].groupID == ID && block[i][j].color != WHITE) {
				block[i][j].exist = false ;
				block[i][j].color = VOID ; 
				if (block[i][j+1].color == WHITE) {
					block[i][j+1].exist = false ; 
					block[i][j+1].color = VOID ; 
				}
				if (block[i][j-1].color == WHITE) {
					block[i][j-1].exist = false ; 
					block[i][j-1].color = VOID ; 
				}
				if (block[i-1][j].color == WHITE) {
					block[i-1][j].exist = false ; 
					block[i-1][j].color = VOID ; 
				}
				if (block[i+1][j].color == WHITE) {
					block[i+1][j].exist = false ; 
					block[i+1][j].color = VOID ; 
				}
			}
		}
	}
}

int destroyBlock (Block mat[WIDTHMAT][HEIGHTMAT]) {
	int nbrChain = 0 ; 
	for (int k = 1 ; k < WIDTHMAT*HEIGHTMAT ; k++ ) {
		if (countNbBlocksEqualID(mat, k)>3) {
			resetBlocksForID(mat, k) ; 
			nbrChain ++ ; 
		}
	} 
	for (int i = 0 ; i < WIDTHMAT ; i++) {
		for (int j = 0 ; j < HEIGHTMAT ; j++) {
			mat[i][j].groupID = 0 ; 
		}
	}
	return nbrChain ; 
}

int main() {

	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Puyo Puyo");
	bool doStartTourPlayer1 = true, doStartTourPlayer2 = true, gameOver = false, up = false, left = false, right=false, down=false; 
	int penaltyReps1, penaltyReps2, nbCombinations; 
	Game game ;
	sf::Clock clock; 

    float delay = 0;
	
	//boucle principale 
	while (window.isOpen()) {
		//actions du joueur
		sf::Event event ; 
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Left) {
					left = true;
				}
				if (event.key.code == sf::Keyboard::Right) {
					right = true;
				}
				if (event.key.code == sf::Keyboard::Up) {
					up = true;
				}
				if (event.key.code == sf::Keyboard::Down) {
					down = true;
				}
			}
			if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::Left) {
					left = false;
				}
				if (event.key.code == sf::Keyboard::Right) {
					right = false;
				}
				if (event.key.code == sf::Keyboard::Up) {
					up = false;
				}
				if (event.key.code == sf::Keyboard::Down) {
					down = false;
				}
			}

		}
		
		//mise jour de l'état du jeu 
		float dt = clock.restart().asSeconds();
		
        delay += dt;
        
        if (delay > DELAY) {
            // TODO: faire tomber les pièces d'une case
            
            delay -= DELAY;
        }
        
		
		//1er joueur
		if (doStartTourPlayer1) {
			startTour(game.p1);
			doStartTourPlayer1 = false;
		}
		
		//actions du joueur (j'ai la flemme de les faire)
		
		//la gravité sur le block qui tombe qui du coup n'est pas dogravityonall parce que *affichage*
		
		if (!continueFall(game.p1.blocks, game.p1.bf1)) {
			blockDown(game.p1);
			doGravityOnAll(game.p1);
			do {
				checkAllChains(game.p1.blocks);
				nbCombinations = destroyBlock(game.p1.blocks);
				doGravityOnAll(game.p1); 
				if (nbCombinations > 0) {
					penaltyReps2 += pow(2, nbCombinations);
				}
			} while (nbCombinations > 0) ;
			if (penaltyReps1 > 0) {
				setMalusOnPlayer(game.p1, penaltyReps1);
				penaltyReps1 = 0; 
			}
			if (!blockAtStart(game.p1.blocks)) {
				doStartTourPlayer1 = true; 
			} else {
				gameOver = true;
			}
		
		}
		
		//2e joueur
		if (doStartTourPlayer2) {
			startTour(game.p2);
			doStartTourPlayer2 = false;
		}
		
		//actions du joueur 2 (j'ai la flemme de les faire)
		
		//la gravité sur le block qui tombe qui du coup n'est pas dogravityonall parce que *affichage*
		
		if (!continueFall(game.p2.blocks, game.p2.bf1)) {
			blockDown(game.p2);
			doGravityOnAll(game.p2);
			do {
				checkAllChains(game.p2.blocks);
				nbCombinations = destroyBlock(game.p2.blocks);
				doGravityOnAll(game.p2); 
				if (nbCombinations > 0) {
					penaltyReps1 += pow(2, nbCombinations);
				}
			} while (nbCombinations > 0) ;
			if (penaltyReps2 > 0) {
				setMalusOnPlayer(game.p2, penaltyReps2);
				penaltyReps2 = 0; 
			}
			if (!blockAtStart(game.p2.blocks)) {
				doStartTourPlayer2 = true; 
			} else {
				gameOver = true;
			}
		
		}

	}
	return 0 ;
}


