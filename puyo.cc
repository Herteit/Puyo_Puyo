#include <SFML/Graphics.hpp>
#include <cassert>
#include <random>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <SFML/Audio.hpp>
using namespace std;
using namespace sf;

//on définit les constantes hors du main pour ne pas avoir à les appeler à chaque fois qu'elles sont utilisées dans un programme
#pragma region Constante
const int NBCOLORS = 5;
const int HEIGHT = 600;
const int WIDTH = 900;
const int FALLSPEED = 1;
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

const float DELAY = 0.5f;
#pragma endregion Constante

struct Pos {
	int x ;
	int y ;
};

struct ChangeOrient {
	bool left;
	bool right;
	bool up;
	bool down; 
};

struct Motion {
	bool right;
	bool left;
	bool down; 
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
	Motion motion;
	ChangeOrient orient;
	int penalty;
	bool gameOver;
	float delay;
	bool doStartTour;
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
	randBlock.color1 = switchColor(random0toNb(NBCOLORS - 1));
	randBlock.color2 = switchColor(random0toNb(NBCOLORS - 1));
	randBlock.speed = FALLSPEED;
    return randBlock;
}



void startTour(Player& p1) {
	initBlockFall(p1.bf1, p1.bf2);
	p1.bf2 = randBlockFall();
	p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y].color = p1.bf1.color1;
	p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y].exist = true;
	p1.doStartTour = false;
}



void doGravityOnBlockFall (Player& player){
	assert(0 <= player.bf1.posMat.y);
	assert(player.bf1.posMat.y < HEIGHTMAT - 1);
	player.bf1.posMat.y ++;
	player.blocks[player.bf1.posMat.x][player.bf1.posMat.y].exist = true ;
	player.blocks[player.bf1.posMat.x][player.bf1.posMat.y].color = player.bf1.color1 ;
	player.blocks[player.bf1.posMat.x][player.bf1.posMat.y-1].exist = false ;
	player.blocks[player.bf1.posMat.x][player.bf1.posMat.y-1].color = VOID;
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
				test = (!player.blocks[player.bf1.posMat.x][player.bf1.posMat.y + 1].exist && !player.blocks[player.bf1.posMat.x + 1][player.bf1.posMat.y + 1].exist);
			}
		break;
		case UP : 
			assert(0 <= player.bf1.posMat.y + 1);
			if ((player.bf1.posMat.y < HEIGHTMAT - 1)&&(player.bf1.posMat.y - 1 >= 0)) {
				test = (!player.blocks[player.bf1.posMat.x][player.bf1.posMat.y + 1].exist);
			}
		break;
		case DOWN : 
			assert(0 <= player.bf1.posMat.y + 1);
			if (player.bf1.posMat.y + 2 < HEIGHTMAT){
				test = (!player.blocks[player.bf1.posMat.x][player.bf1.posMat.y + 2].exist);
			}
		break;
	}
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
			doGravityOnAll(p1);
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
	sf::SoundBuffer casser;
	if (!casser.loadFromFile("block_casser.wav"))
		return -1;
	for (int k = 1 ; k < WIDTHMAT*HEIGHTMAT ; k++ ) {
		if (countNbBlocksEqualID(mat, k)>3) {
			resetBlocksForID(mat, k) ; 
			nbrChain ++ ; 
			sf::SoundBuffer casser;


			sf::Sound sound;
			sound.setBuffer(casser);
			sound.play();
		}
	} 
	for (int i = 0 ; i < WIDTHMAT ; i++) {
		for (int j = 0 ; j < HEIGHTMAT ; j++) {
			mat[i][j].groupID = 0 ; 
		}
	}
	return nbrChain ; 
}



Pos getPosSecondBlock (const Player& player){
	Pos pos;
	pos.x = player.bf1.posMat.x;
	pos.y = player.bf1.posMat.y;
	switch (player.bf1.orient) {
		case RIGHT : {
			pos.x += 1;
		}break;
		case LEFT : {
			pos.x -= 1;
		}break;
		case UP : {
			pos.y -= 1;
		}break;
		case DOWN : {
			pos.y += 1;
		}break;
	}
	return pos;
}



void mat2S(Player p) {
	//determination emplacement 2nd partie du bf
	Pos pos = getPosSecondBlock(p);

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
		p1.bf1.posMat.x -- ;
		p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y].color = p1.bf1.color1;
		p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y].exist = true; 
		p1.blocks[p1.bf1.posMat.x + 1][p1.bf1.posMat.y].color = VOID;
		p1.blocks[p1.bf1.posMat.x + 1][p1.bf1.posMat.y].exist = false;
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
		p1.bf1.posMat.x ++ ;
		p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y].color = p1.bf1.color1;
		p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y].exist = true; 
		p1.blocks[p1.bf1.posMat.x - 1][p1.bf1.posMat.y].color = VOID;
		p1.blocks[p1.bf1.posMat.x - 1][p1.bf1.posMat.y].exist = false;
	}
}



void actionsJoueur(Player& p1) {
	if (p1.orient.left) {
		if (!p1.blocks[p1.bf1.posMat.x - 1][p1.bf1.posMat.y].exist){
			p1.bf1.orient = LEFT;
			p1.orient.left = false;
		}
	}
	
	if (p1.orient.right) {
		if (!p1.blocks[p1.bf1.posMat.x + 1][p1.bf1.posMat.y].exist){
			p1.bf1.orient = RIGHT;
			p1.orient.right = false;
		}
	}
	
	if (p1.orient.up) {
		if (!p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y - 1].exist){
			p1.bf1.orient = UP;
			p1.orient.up = false;
		}
	}
	
	if (p1.orient.down) {
		if (!p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y + 1].exist){
			p1.bf1.orient = DOWN;
			p1.orient.down = false;
		}
	}

	if (p1.motion.left) { 
		left(p1); 
		p1.motion.left = false;
	}

	if (p1.motion.right) { 
		right(p1);
		p1.motion.right = false;
	}

	if (p1.motion.down) {
		p1.bf1.speed = FALLSPEED*2;
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
	
	//initialisation constantes 
	
	game.p1.motion.right = false;
	game.p1.motion.left = false;
	game.p1.motion.down = false;
	game.p2.motion.right = false;
	game.p2.motion.left = false;
	game.p2.motion.down = false;
		
	game.p1.orient.right = false;
	game.p1.orient.left = false;
	game.p1.orient.up = false;
	game.p1.orient.down = false; 
	game.p2.orient.right = false;
	game.p2.orient.left = false;
	game.p2.orient.up = false;
	game.p2.orient.down = false; 
	
	game.p1.penalty = 0;
	game.p2.penalty = 0; 
	
	game.p1.gameOver = false;
	game.p2.gameOver = false;

	game.p1.delay = 0;
	game.p2.delay = 0;
	
	game.p1.doStartTour = true;
	game.p2.doStartTour = true;
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

void boucleJeu(Player& p1, Player& p2){
	int nbCombinations;
	if (continueFall(p1)) {
		if (p1.delay * p1.bf1.speed > DELAY) {
			doGravityOnBlockFall(p1);
			p1.delay -= DELAY / p1.bf1.speed;
		}
	} else {
		blockDown(p1);
		doGravityOnAll(p1);
		do {
			checkAllChains(p1.blocks);
			nbCombinations = destroyBlock(p1.blocks);
			doGravityOnAll(p1); 
			if (nbCombinations > 0) {
				p2.penalty += nbCombinations;
			}
		} while (nbCombinations > 0) ;
		if (p1.penalty != 0) {
			setMalusOnPlayer(p1, p1.penalty);
			p1.penalty = 0; 
		}
		if (!blockAtStart(p1)) { 
			p1.doStartTour = true; 
		} else {
			p1.gameOver = true;
		}
	}
}


void drawGame (sf::RenderWindow& window, const Player& player, int displacement) {
	for (int i = 0 ; i < WIDTHMAT ; i++) {
		for (int j = 0; j < HEIGHTMAT ; j++){
			RectangleShape cases ;
			cases.setPosition(displacement + i*SIZEPUYO,j*SIZEPUYO);
			Pos pos = getPosSecondBlock(player);
			if (i == pos.x && j == pos.y){
				cases.setFillColor(getColor(player.bf1.color2));
			} else {
				cases.setFillColor(getColor(player.blocks[i][j].color));   
			}
			cases.setSize(Vector2f(SIZEPUYO,SIZEPUYO));
			window.draw(cases);
		}
	}
	
	Font font;
	
	font.loadFromFile("OpenSans-Regular.ttf");
	
	sf::Text text;
	
	text.setFont(font);
	text.setString("texte");
	text.setCharacterSize(20);
	text.setFillColor(Color::Red);
	text.setPosition(Vector2f(450, 450));
	window.draw(text);

}


int main() {
	
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Puyo Puyo");
	Game game ;
	sf::Clock clock; 

	startGame(game);


	//boucle principale 
	while (window.isOpen()&&!game.p1.gameOver&&!game.p2.gameOver) {
		//actions du joueur
		sf::Event event ; 
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			if (event.type == sf::Event::KeyPressed) {
				//j1
				if (event.key.code == sf::Keyboard::Left) {
					game.p1.orient.left = true;
				}
				if (event.key.code == sf::Keyboard::Right) {
					game.p1.orient.right = true;
				}
				if (event.key.code == sf::Keyboard::Up) {
					game.p1.orient.up = true;
				}
				if (event.key.code == sf::Keyboard::Down) {
					game.p1.orient.down = true;
				}
				if (event.key.code == sf::Keyboard::K) {
					game.p1.motion.left = true;
				}
				if (event.key.code == sf::Keyboard::L) {
					game.p1.motion.down = true;
				}
				if (event.key.code == sf::Keyboard::M) {
					game.p1.motion.right = true;
				}
				//j2
				if (event.key.code == sf::Keyboard::Q) {
					game.p2.orient.left = true;
				}
				if (event.key.code == sf::Keyboard::D) {
					game.p2.orient.right = true;
				}
				if (event.key.code == sf::Keyboard::Z) {
					game.p2.orient.up = true;
				}
				if (event.key.code == sf::Keyboard::S) {
					game.p2.orient.down = true;
				}
				if (event.key.code == sf::Keyboard::C) {
					game.p2.motion.left = true;
				}
				if (event.key.code == sf::Keyboard::V) {
					game.p2.motion.down = true;
				}
				if (event.key.code == sf::Keyboard::B) {
					game.p2.motion.right = true;
				}
			}
			if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::L) {
					game.p1.motion.down = false;
				}
				if (event.key.code == sf::Keyboard::V) {
					game.p2.motion.down = false;
				}
			}
		}
		
		//mise jour de l'état du jeu 
		float dt = clock.restart().asSeconds();
		game.p1.delay += dt;
		game.p2.delay += dt; 
        


		//1er joueur
		if (game.p1.doStartTour) {
			startTour(game.p1);
		}
		
		actionsJoueur(game.p1);

		boucleJeu(game.p1, game.p2);
		


		//2e joueur
		if (game.p2.doStartTour) {
			startTour(game.p2);
		}
		
		actionsJoueur(game.p2);

		boucleJeu(game.p2, game.p1);
		
		

		
		window.clear(Color::White);
		
		drawGame(window, game.p2, 0);
		drawGame(window, game.p1, 450);

		window.display();

	}

	if (game.p2.gameOver) {
		printf ("player 1 gagne\n");
	} else {
		printf ("player 2 gagne\n");
	}
	usleep(3000000);
	return 0 ;
}
