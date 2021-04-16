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
const int SIZEPUYO = 1/30 * WIDTH;
const int WIDTHMAT = 6;
const int HEIGHTMAT = 12; 
const int WAITINGPOSX = WIDTH /2 - SIZEPUYO;
const int WAITINGPOSY = 100;
const int LEFT = 0;
const int UP = 1;
const int RIGHT = 2;
const int DOWN = 3; 

const char VOID = 'v';
const char WHITE = 'w';
const char RED = 'r';
const char YELLOW = 'y';
const char BLUE = 'b';
const char GREEN = 'g';
const char PURPLE = 'p';

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

//cette structure est là pour avoir une fonction Math::random
struct Math {
  static float random() {
    static mt19937 engine(time(nullptr));
    uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(engine);
  }
};

//initBlockFall
void initBlockFall(BlockFall& bf1, const BlockFall& bf2) {
	bf1.orient = RIGHT;
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
	randBlock.orient = DOWN;
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


void doGravityOnBlockFall (Player& player){
	assert(0 <= player.bf1.posMat.y);
	assert(player.bf1.posMat.y < HEIGHTMAT - 1);
	player.bf1.posMat.y ++;
	player.blocks[player.bf1.posMat.y][player.bf1.posMat.y].exist = true ;
	player.blocks[player.bf1.posMat.x][player.bf1.posMat.y].color = player.bf1.color1 ;
	player.blocks[player.bf1.posMat.y][player.bf1.posMat.y-1].exist = false ;
	player.blocks[player.bf1.posMat.x][player.bf1.posMat.y-1].color = VOID;
}

void doGravityOnAll (Player& player) {
	int i, j, k;
	for (i = 0; i < WIDTHMAT -1; i++) {
		for (j = 0; j<HEIGHTMAT-1;j++){
			if (!player.blocks[i][j+1].exist){
				for (k = j; k>=0; k--) {
					assert(k>=0);
					assert(k<HEIGHTMAT-1);
					player.blocks[i][k+1].exist = player.blocks[i][k].exist ;
					player.blocks[i][k+1].color = player.blocks[i][k].color ;
					player.blocks[i][k].exist = false ;
					player.blocks[i][k].color = VOID;
				}
			}
		}
	}
}

bool continueFall(const Player& player) {
	bool test = false;
	switch(player.bf1.orient){
		case LEFT : 
			assert(0 <= player.bf1.posMat.x - 1);
			assert(WIDTHMAT> player.bf1.posMat.x - 1);
			assert(0 <= player.bf1.posMat.y + 1);
			if (player.bf1.posMat.y < HEIGHTMAT - 1){
				test = (!player.blocks[player.bf1.posMat.x][player.bf1.posMat.y + 1].exist || !player.blocks[player.bf1.posMat.x - 1][player.bf1.posMat.y + 1].exist);
			}
		break;
		case RIGHT : 
			assert(0 <= player.bf1.posMat.x + 1);
			assert(WIDTHMAT> player.bf1.posMat.x + 1);
			assert(0 <= player.bf1.posMat.y + 1);
			if (player.bf1.posMat.y < HEIGHTMAT - 1){
				test = (!player.blocks[player.bf1.posMat.x][player.bf1.posMat.y + 1].exist || !player.blocks[player.bf1.posMat.x + 1][player.bf1.posMat.y + 1].exist);
			}
		break;
		case UP : 
			assert(0 <= player.bf1.posMat.y + 1);
			if (player.bf1.posMat.y < HEIGHTMAT - 1){
				test = (!player.blocks[player.bf1.posMat.x][player.bf1.posMat.y + 1].exist);
			}
		break;
		case DOWN : 
			assert(0 <= player.bf1.posMat.y + 1);
			assert(HEIGHTMAT> player.bf1.posMat.y + 2);
			if (player.bf1.posMat.y + 1 < HEIGHTMAT - 1){
				test = (!player.blocks[player.bf1.posMat.x][player.bf1.posMat.y + 2].exist);
			}
		break;
	}
	return test;
}

void blockDown (Player p1) {
	switch (p1.bf1.orient) {
		case LEFT : { 
			assert(p1.bf1.posMat.x-1>=0);
			assert(p1.bf1.posMat.x-1<HEIGHTMAT);
			p1.blocks[p1.bf1.posMat.x-1][p1.bf1.posMat.y].color = p1.bf1.color2;
			p1.blocks[p1.bf1.posMat.x-1][p1.bf1.posMat.y].exist = true;
		} break;
		case RIGHT : {
			assert(p1.bf1.posMat.x+1>=0);
			assert(p1.bf1.posMat.x+1<HEIGHTMAT);
			p1.blocks[p1.bf1.posMat.x+1][p1.bf1.posMat.y].color = p1.bf1.color2;
			p1.blocks[p1.bf1.posMat.x+1][p1.bf1.posMat.y].exist = true;
		} break;
		case UP : {
			assert(p1.bf1.posMat.y-1>=0);
			assert(p1.bf1.posMat.y-1<HEIGHTMAT);
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y-1].color = p1.bf1.color2;
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y-1].exist = true;
		} break;
		case DOWN : {
			assert(p1.bf1.posMat.y+1>=0);
			assert(p1.bf1.posMat.y+1<HEIGHTMAT);
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y+1].color = p1.bf1.color2;
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y+1].exist = true;
		} break;
	}
}

//AttribuerGroupe -  a verifié le telechargement de l'ide sur mon ordi portable est toujours pas fini 
int AttribuerGroupe ( Block mat[WIDTHMAT][HEIGHTMAT] , Position posBlock ,  int groupID ){
	int x, y
	int longueurChaine=0
	for (int i = 0; i < 4; i++){
		if (i%2 == 0){
			x = posBlock.x +i -1
			y = posBlock.y
		} else {
			x = posBlock.x
			y = posBlock.y +i -2
		}
		if (x >= 0 && x < longueur (2 , Mat ) && y >= 0 && y < longueur (1 , Mat )) && y >= 0 && y < longueur (1 , Mat ))  {
			if ( Mat[y][x].exist ) {
				if ( Mat[posBlock.y][posBlock.x].couleur == Mat[y][x].couleur && Mat[y][x].groupID == 0) {
					Mat[y][x].groupID = groupID
					Position pos = new Position ()
					pos.x = x
					pos.y = y
					longueurChaine += (1 + AttribuerGroupe ( Mat ,pos , groupID ))
				}
			}
		}
	return longueurChaine ;
}

//checkAllChains , a verifié le telechargement de l'ide sur mon ordi portable est toujours pas fini 
void checkAllChains ( Block mat[WIDTHMAT][HEIGHTMAT]) {
	int baseGroupId = 1;
	for (int i = 0; i < WIDTHMAT; i++) {
		for (int j = 0; j < HEIGHTMAT; j++) {
			if ((mat[i][j].exist && mat[i][j].groupID == 0) {
				Position pos = new Position ();
				pos.x = j;
				pos.y = i;
				int longueur = AttribuerGroupe (mat ,pos ,baseGroupId ) ;
				if (longueur > 0) {
				baseGroupId += 1 ;
				}
			}
		}
	}
}

bool blockAtStart (const Player& player) {
	return (!player.blocks[0][WIDTHMAT/2 - 1].exist || !player.blocks[0][WIDTHMAT/2].exist);
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
		nb = random0toNb(WIDTHMAT-1);
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
				if (j+1 < HEIGHTMAT){
					if (block[i][j+1].color == WHITE) {
						block[i][j+1].exist = false ; 
						block[i][j+1].color = VOID ; 
					}
				}
				if (j-1 > 0){
					if (block[i][j-1].color == WHITE) {
						block[i][j-1].exist = false ; 
						block[i][j-1].color = VOID ; 
					}
				}
				if (i-1 > 0){
					if (block[i-1][j].color == WHITE) {
						block[i-1][j].exist = false ; 
						block[i-1][j].color = VOID ; 
					}
				}
				if (i+1 < WIDTHMAT){
					if (block[i+1][j].color == WHITE) {
						block[i+1][j].exist = false ; 
						block[i+1][j].color = VOID ; 
					}
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
	bool doStartTourPlayer1 = true;
	bool doStartTourPlayer2 = true; 
	bool gameOver = false;
	bool up = false;
	bool left = false;
	bool right=false;
	bool down=false; 
	int penaltyReps1;
	int penaltyReps2;
	int nbCombinations; 
	Game game ;
	sf::Clock clock; 

	float delayPlayer1 = 0;
	float delayPlayer2 = 0;
	
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
		delayPlayer1 += dt;
		delayPlayer2 += dt; 
        
		
		//1er joueur
		if (doStartTourPlayer1) {
			startTour(game.p1);
			doStartTourPlayer1 = false;
		}
		
		//actions du joueur (j'ai la flemme de les faire)
		
		if (delayPlayer1 > DELAY) {	
			doGravityOnBlockFall(game.p1);
			delayPlayer1 -= DELAY;
		}
		
		if (!continueFall(game.p1)) {
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
			if (!blockAtStart(game.p1)) {
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
		
		if (delayPlayer2 > DELAY) {		
			doGravityOnBlockFall(game.p2);
			delayPlayer2 -= DELAY;
		}
		
		if (!continueFall(game.p2)) {
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
			if (!blockAtStart(game.p2)) {
				doStartTourPlayer2 = true; 
			} else {
				gameOver = true;
			}
		
		}

	}
	return 0 ;
}


