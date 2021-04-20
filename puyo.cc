#include <SFML/Graphics.hpp>
#include <cassert>
#include <random>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
using namespace std;
using namespace sf;

//on définit les constantes hors du main pour ne pas avoir à les appeler à chaque fois qu'elles sont utilisées dans un programme
const int NBCOLORS = 5;
const int HEIGHT = 600;
const int WIDTH = 900;
const int FALLSPEED = 10;
const int SIZEPUYO = WIDTH/30;
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



void initBlockFall(BlockFall& bf1, const BlockFall& bf2) {
	bf1.orient = RIGHT;
	bf1.color1 = bf2.color1;
	bf1.color2 = bf2.color2;
	bf1.posMat.x = WIDTHMAT /2 -1;
	bf1.posMat.y = 0;
	bf1.speed = bf2.speed;
	printf ("initBlockFall \n");
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



BlockFall randBlockFall() {
    BlockFall randBlock;
	randBlock.orient = DOWN;
	randBlock.color1 = switchColor(random0toNb(1));
	randBlock.color2 = switchColor(random0toNb(1));
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
	printf ("doGravOnBF \n");
	printf ("begin doGravityOnBlockFall y : %d \n", player.bf1.posMat.y);
	assert(0 <= player.bf1.posMat.y);
	assert(player.bf1.posMat.y < HEIGHTMAT - 1);
	player.bf1.posMat.y ++;
	player.blocks[player.bf1.posMat.x][player.bf1.posMat.y].exist = true ;
	player.blocks[player.bf1.posMat.x][player.bf1.posMat.y].color = player.bf1.color1 ;
	player.blocks[player.bf1.posMat.x][player.bf1.posMat.y-1].exist = false ;
	player.blocks[player.bf1.posMat.x][player.bf1.posMat.y-1].color = VOID;
	printf ("doGravityOnBlockFall y : %d \n", player.bf1.posMat.y);
}



void doGravityOnAll (Player& player) {
	int i, j, k;
	for (i = 0; i < WIDTHMAT ; i++) {
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
				test = (!player.blocks[player.bf1.posMat.x][player.bf1.posMat.y + 1].exist && !player.blocks[player.bf1.posMat.x - 1][player.bf1.posMat.y + 1].exist);
			}
		break;
		case RIGHT : 
			assert(0 <= player.bf1.posMat.x + 1);
			assert(WIDTHMAT> player.bf1.posMat.x + 1);
			assert(0 <= player.bf1.posMat.y + 1);
			if (player.bf1.posMat.y < HEIGHTMAT - 1){
				
				if (!player.blocks[player.bf1.posMat.x][player.bf1.posMat.y + 1].exist ) {
					printf ("1er");
				}
				if (!player.blocks[player.bf1.posMat.x + 1][player.bf1.posMat.y + 1].exist) {
					printf ("2e");
				}				
				
				
				test = (!player.blocks[player.bf1.posMat.x][player.bf1.posMat.y + 1].exist && !player.blocks[player.bf1.posMat.x + 1][player.bf1.posMat.y + 1].exist);
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
	printf ("continueFall: %d %d \n", player.bf1.posMat.x, player.bf1.posMat.y);
	if (test)
		printf ("block peut continuer\n");
	return test;
}



void blockDown (Player& p1) {
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
			printf("blockfall implanté \n");
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



int AttribuerGroupe ( Block mat[WIDTHMAT][HEIGHTMAT] , Pos posBlock ,  int groupID ){
	int x;
	int y;
	int longueurChaine=0;
	for (int i = 0; i < 4; i++){
		if (i%2 == 0){
			x = posBlock.x +i -1;
			y = posBlock.y;
		} else {
			x = posBlock.x;
			y = posBlock.y +i -2;
		}
		if (x >= 0 && x < WIDTHMAT && y >= 0 && y < HEIGHTMAT)  {
			if ( mat[x][y].exist ) {
				if ( mat[posBlock.x][posBlock.y].color == mat[x][y].color && mat[x][y].groupID == 0) {
					mat[x][y].groupID = groupID;
					Pos pos ;
					pos.x = x;
					pos.y = y;
					longueurChaine += (1 + AttribuerGroupe ( mat ,pos , groupID ));
				}
			}
		}
	}
	return longueurChaine ;
}



void checkAllChains ( Block mat[WIDTHMAT][HEIGHTMAT]) {
	int baseGroupId = 1;
	for (int i = 0; i < WIDTHMAT; i++) {
		for (int j = 0; j < HEIGHTMAT; j++) {
			if (mat[i][j].exist && mat[i][j].groupID == 0) {
				Pos pos;
				pos.x = i;
				pos.y = j;
				int longueur = AttribuerGroupe (mat ,pos ,baseGroupId ) ;
				if (longueur > 0) {
				baseGroupId += 1 ;
				}
			}
		}
	}
}



bool blockAtStart (const Player& player) {
	return (player.blocks[WIDTHMAT/2 - 1][0].exist || player.blocks[WIDTHMAT/2][0].exist);
}



int countNbBlocksEqualID (Block mat[WIDTHMAT][HEIGHTMAT], int ID) {
	int nb = 0; 
	for (int i = 0 ; i < WIDTHMAT ; i++) {
		for (int j = 0 ; j < HEIGHTMAT ; j++) {
			if (mat[i][j].groupID == ID && mat[i][j].color != VOID && mat[i][j].color != WHITE) {
				nb ++ ;
			}
		}
	}
	return nb;
}



void setMalusOnPlayer (Player& p1, int reps) {
	int nb ; 
	for (int i = 0 ; i < reps ; i++) {
		nb = random0toNb(WIDTHMAT-1);
		if (p1.blocks[nb][0].exist == false) {
			p1.blocks[nb][0].color = WHITE ;
			p1.blocks[nb][0].exist = true ; 
		}
		doGravityOnAll(p1);	
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



void mat2S(Player p) {
	//determination emplacement 2nd partie du bf
	Pos pos;
	pos.x = p.bf1.posMat.x;
	pos.y = p.bf1.posMat.y;
	switch (p.bf1.orient) {
		case RIGHT : {
			pos.x = p.bf1.posMat.x + 1;
			break;
		}
		case LEFT : {
			pos.x = p.bf1.posMat.x - 1;
			break;
		}
		case UP : {
			pos.y = p.bf1.posMat.y - 1;
			break;
		}
		case DOWN : {
			pos.y = p.bf1.posMat.y + 1;
			break;
		}
	}

	//affichage de la matrice avec la 2nd partie du bf
	for (int i = 0; i < WIDTHMAT; i++) {
		printf ("[");
		for (int j = 0; j < HEIGHTMAT; j++) {
			if (pos.x == i && pos.y == j) {
				printf ("%c", p.bf1.color2);
			} else {
				printf ("%c", p.blocks[i][j].color);
			}
			if (j!=HEIGHTMAT-1) {
				printf ("\t");
			}
		}
		printf ("]\n");
	}
}



void left(Player& p1) {
	int dep = false;
	//on verifie si c'est possible
	switch (p1.bf1.orient) { 
		case LEFT : {
			if (!p1.blocks[p1.bf1.posMat.x - 2][p1.bf1.posMat.y].exist && p1.bf1.posMat.x - 2 >= 0){
				dep = true;
			}
		} break;
		case RIGHT :
		case UP : {
			if (!p1.blocks[p1.bf1.posMat.x - 1][p1.bf1.posMat.y].exist && p1.bf1.posMat.x - 1 >= 0){
				dep = true;
			}
		} break;
		case DOWN : {
			if (!p1.blocks[p1.bf1.posMat.x - 1][p1.bf1.posMat.y].exist && p1.bf1.posMat.x - 1 >= 0 && !p1.blocks[p1.bf1.posMat.x - 1][p1.bf1.posMat.y + 1].exist){
				dep = true;
			}
		} break;
	}
	//et on le déplace
	if (dep) {
		p1.bf1.posMat.y = p1.bf1.posMat.y - 1 ;
		p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y].color = p1.bf1.color1;
		p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y].exist = true; 
		p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y + 1].color = VOID;
		p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y + 1].exist = false;
	}
}



void right(Player& p1) {
	int dep = false;
	//on verifie si c'est possible
	switch (p1.bf1.orient) { 
		case RIGHT : {
			if (!p1.blocks[p1.bf1.posMat.x + 2][p1.bf1.posMat.y].exist && p1.bf1.posMat.x + 2 < WIDTHMAT){
				dep = true;
			}
		} break;
		case LEFT :
		case UP : {
			if (!p1.blocks[p1.bf1.posMat.x + 1][p1.bf1.posMat.y].exist && p1.bf1.posMat.x + 1 < WIDTHMAT){
				dep = true;
			}
		} break;
		case DOWN : {
			if (!p1.blocks[p1.bf1.posMat.x + 1][p1.bf1.posMat.y].exist && p1.bf1.posMat.x + 1 < WIDTHMAT && !p1.blocks[p1.bf1.posMat.x + 1][p1.bf1.posMat.y + 1].exist){
				dep = true;
			}
		} break;
	}
	//et on le déplace
	if (dep) {
		p1.bf1.posMat.y = p1.bf1.posMat.y + 1 ;
		p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y].color = p1.bf1.color1;
		p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y].exist = true; 
		p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y - 1].color = VOID;
		p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y - 1].exist = false;
	}
}



void actionsJoueur(Player& p1, bool& left, bool& right, bool& up, bool& down) {
	if (left) {
		if (!p1.blocks[p1.bf1.posMat.x - 1][p1.bf1.posMat.y].exist){
			p1.bf1.orient = LEFT;
			left = false;
		}
	}
	
	if (right) {
		if (!p1.blocks[p1.bf1.posMat.x + 1][p1.bf1.posMat.y].exist){
			p1.bf1.orient = RIGHT;
			right = false;
		}
	}
	
	if (up) {
		if (!p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y - 1].exist){
			p1.bf1.orient = UP;
			up = false;
		}
	}
	
	if (down) {
		if (!p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y + 1].exist){
			p1.bf1.orient = DOWN;
			down = false;
		}
	}
}



void startGame(Game& game){
	//initialisation de la matrice	
	for (int i = 0; i < WIDTHMAT; i++) {
		for (int j = 0; j < HEIGHTMAT; j++) {
			game.p1.blocks[i][j].color = VOID;
			game.p1.blocks[i][j].exist = false;
			game.p2.blocks[i][j].color = VOID;
			game.p2.blocks[i][j].exist = false;
		}
	}
	//initialisation bf
	game.p1.bf2=randBlockFall();
	game.p2.bf2=game.p1.bf2;
}



Color getColor (char color){
	switch (color) {
		case YELLOW :
			return Color::Yellow;
		break;
		case RED : 
			return Color::Red; 
		break ; 
		case BLUE : 
			return Color::Blue ; 
		break ; 
		case GREEN : 
			return Color::Green ;
		break ;
		case PURPLE :
			return Color::Magenta;
		break ;
		case WHITE : 
			return Color::White;
		break;
		default : 
			return Color::Black;
		break ;	
	}
}


int main() {

	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Puyo Puyo");
	bool doStartTourPlayer1 = true;
	bool doStartTourPlayer2 = true; 
	bool gameOver1 = false;
	bool gameOver2 =false;
	bool oup1 = false;
	bool oleft1 = false;
	bool oright1 = false;
	bool odown1 = false; 
	bool mleft1 = false;
	bool mright1 = false;
	bool mdown1 = false; 
	bool oup2 = false;
	bool oleft2 = false;
	bool oright2 = false;
	bool odown2 = false;
	bool mleft2 = false;
	bool mright2 = false;
	bool mdown2 = false; 
	int penaltyReps1;
	int penaltyReps2;
	int nbCombinations; 
	Game game ;
	sf::Clock clock; 

	float delayPlayer1 = 0;
	float delayPlayer2 = 0;


	startGame(game);


	//boucle principale 
	while (window.isOpen()&&!gameOver1&&!gameOver2) {
		//actions du joueur
		sf::Event event ; 
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			if (event.type == sf::Event::KeyPressed) {
				//j1
				if (event.key.code == sf::Keyboard::Left) {
					oleft1 = true;
				}
				if (event.key.code == sf::Keyboard::Right) {
					oright1 = true;
				}
				if (event.key.code == sf::Keyboard::Up) {
					oup1 = true;
				}
				if (event.key.code == sf::Keyboard::Down) {
					odown1 = true;
				}
				if (event.key.code == sf::Keyboard::K) {
					mleft1 = true;
				}
				if (event.key.code == sf::Keyboard::L) {
					mdown1 = true;
				}
				if (event.key.code == sf::Keyboard::M) {
					mright1 = true;
				}
				//j2
				if (event.key.code == sf::Keyboard::Q) {
					oleft2 = true;
				}
				if (event.key.code == sf::Keyboard::D) {
					oright2 = true;
				}
				if (event.key.code == sf::Keyboard::Z) {
					oup2 = true;
				}
				if (event.key.code == sf::Keyboard::S) {
					odown2 = true;
				}
				if (event.key.code == sf::Keyboard::C) {
					mleft2 = true;
				}
				if (event.key.code == sf::Keyboard::V) {
					mdown2 = true;
				}
				if (event.key.code == sf::Keyboard::B) {
					mright2 = true;
				}
			}
			if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::L) {
					mdown1 = false;
				}
				if (event.key.code == sf::Keyboard::V) {
					mdown2 = false;
				}
			}
		}
		
		//mise jour de l'état du jeu 
		float dt = clock.restart().asSeconds();
		delayPlayer1 += dt;
		delayPlayer2 += dt; 
        
		printf ("    nouveau tour    \n");




		//1er joueur

		printf ("  player 1 \n");
		if (doStartTourPlayer1) {
			startTour(game.p1);
			doStartTourPlayer1 = false;
		}
		

		//actions du joueur 1
		
		actionsJoueur(game.p1, oleft1, oright1, oup1, odown1);

		//on souhaite déplacer le bf1 à gauche
		if (mleft1) { 
			left(game.p1); 
			mleft1 = false;
		}

		//on souhaite déplacer le bf1 à droite
		if (mright1) { 
			right(game.p1);
			mright1 = false;
		}

		//on souhaite accelerer le bf du j1
		if (mdown1) {
			game.p1.bf1.speed = FALLSPEED*2;
		}



		if (delayPlayer1 > DELAY) {
			if (continueFall(game.p1)){
				doGravityOnBlockFall(game.p1);
			}
			delayPlayer1 -= DELAY;
		}
		
		if (!continueFall(game.p1)) {
			printf ("bf descendu 1 \n");
			blockDown(game.p1);
			doGravityOnAll(game.p1);
			do {
				checkAllChains(game.p1.blocks);
				nbCombinations = destroyBlock(game.p1.blocks);
				doGravityOnAll(game.p1); 
				if (nbCombinations > 0) {
					penaltyReps2 += nbCombinations;
				}
				printf ("nbComb %d \n", nbCombinations);
			} while (nbCombinations > 0) ;
			if (penaltyReps1 != 0) {
				setMalusOnPlayer(game.p1, penaltyReps1);
				penaltyReps1 = 0; 
			}
			if (!blockAtStart(game.p1)) { 
				doStartTourPlayer1 = true; 
				printf ("dostarttour1 = true \n");
			} else {
				gameOver1 = true;
				printf ("gameOver j1 = true \n");
			}
			printf ("end tour p1 \n");
		}

		mat2S(game.p1);





		//2e joueur
		printf ("  player 2 \n");
		if (doStartTourPlayer2) {
			startTour(game.p2);
			doStartTourPlayer2 = false;
		}
		

		//actions du joueur 2
		
		actionsJoueur(game.p2, oleft2, oright2, oup2, odown2);
		
		//on souhaite déplacer le bf1 à gauche
		if (mleft2) { 
			left(game.p2); 
			mleft2 = false;
		}

		//on souhaite déplacer le bf1 à droite
		if (mright2) { 
			right(game.p2);
			mright2 = false;
		}

		//on souhaite accelerer le bf du j2
		if (mdown2) {
			game.p2.bf1.speed = FALLSPEED*2;
		}



		if (delayPlayer2 > DELAY) {
			if (continueFall(game.p2)){
				doGravityOnBlockFall(game.p2);
			}
			delayPlayer2 -= DELAY;
		}
		
		if (!continueFall(game.p2)) {
			printf("bf descendu 2 \n");
			blockDown(game.p2);
			doGravityOnAll(game.p2);
			do {
				checkAllChains(game.p2.blocks);
				nbCombinations = destroyBlock(game.p2.blocks);
				doGravityOnAll(game.p2); 
				if (nbCombinations > 0) {
					penaltyReps1 += nbCombinations;
				}
			} while (nbCombinations > 0) ;
			if (penaltyReps2 > 0) {
				setMalusOnPlayer(game.p2, penaltyReps2);
				penaltyReps2 = 0; 
			}
			if (!blockAtStart(game.p2)) {
				doStartTourPlayer2 = true; 
				printf ("dostarttour2 = true \n");
			} else {
				gameOver2 = true;
				printf ("gameOver j2 = true \n");
			}
			printf ("end tour p2 \n");
		}
		printf("SizePuyo = %d \n",SIZEPUYO);
		mat2S(game.p2);
		window.clear(Color::White);
		
		RectangleShape rectangle;
		rectangle.setPosition(100, 100);
		rectangle.setFillColor(Color::Red);
		rectangle.setSize(Vector2f(SIZEPUYO, SIZEPUYO));
		
		for (int i = 0 ; i < WIDTHMAT ; i++) {
			for (int j = 0; j < HEIGHTMAT ; j++){
				RectangleShape cases ;
				cases.setPosition(i*SIZEPUYO,j*SIZEPUYO);
				cases.setFillColor(getColor(game.p1.blocks[i][j].color));   
				cases.setSize(Vector2f(SIZEPUYO,SIZEPUYO));
				window.draw(cases) ;
				cases.setPosition(450+i*SIZEPUYO,j*SIZEPUYO);
				cases.setFillColor(getColor(game.p2.blocks[i][j].color));   
				cases.setSize(Vector2f(SIZEPUYO,SIZEPUYO));
				window.draw(cases) ;
			}
		}
		
		window.display();

	}

	if (gameOver2) {
		printf ("player 1 gagne\n");
	} else {
		printf ("player 2 gagne\n");
	}
	usleep(3000000);
	return 0 ;
}
