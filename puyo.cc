#include <SFML/Graphics.hpp>
#include <cassert>
#include <random>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <SFML/Audio.hpp>
#include <SFML/Graphics/Font.hpp>
using namespace std;
using namespace sf;

//on définit les constantes hors du main pour ne pas avoir à les appeler à chaque fois qu'elles sont utilisées dans un programme

const int NBCOLORS = 5;
const int HEIGHT = 720;
const int WIDTH = 1000;
const int FALLSPEED = 1;
const int SIZEPUYO = 60;
const int WIDTHMAT = 6;
const int HEIGHTMAT = 12; 
const int WAITINGPOSX = WIDTH /2 - 90;
const int WAITINGPOSY = 100;
const int LEFT = 0;
const int UP = 1;
const int RIGHT = 2;
const int DOWN = 3;
const int NOT_STARTED = 0;
const int RUNNING = 1;
const int END = 2; 
const int RESTART = 3;
const int PAUSE = 4; 

const char VOID = 'v';
const char WHITE = 'w';
const char RED = 'r';
const char YELLOW = 'y';
const char BLUE = 'b';
const char GREEN = 'g';
const char PURPLE = 'p';

const float DELAY = 0.7f;

struct Pos {
	int x ;
	int y ;
};

struct ChangeOrient {
	bool clockwise;
	bool anticlockwise;
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
	int state;
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
	switch (nb) {
		case 0 :
			return YELLOW;
		break;
		case 1 : 
			return RED ; 
		break ; 
		case 2 : 
			return BLUE ; 
		break ; 
		case 3 : 
			return GREEN ;
		break ;
		case 4 :
			return PURPLE ;
		break ;
		default : 
			return VOID ;
		break ;	
	}
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
	for (int i = 0; i < WIDTHMAT ; i++) {
		for (int j = 0; j<HEIGHTMAT-1;j++){
			if (!player.blocks[i][j+1].exist){
				for (int k = j; k>=0; k--) {
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
			p1.blocks[p1.bf1.posMat.x-1][p1.bf1.posMat.y].color = p1.bf1.color2;
			p1.blocks[p1.bf1.posMat.x-1][p1.bf1.posMat.y].exist = true;
		} break;
		case RIGHT : {
			p1.blocks[p1.bf1.posMat.x+1][p1.bf1.posMat.y].color = p1.bf1.color2;
			p1.blocks[p1.bf1.posMat.x+1][p1.bf1.posMat.y].exist = true;
		} break;
		case UP : {
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y-1].color = p1.bf1.color2;
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y-1].exist = true;
		} break;
		case DOWN : {
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y+1].color = p1.bf1.color2;
			p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y+1].exist = true;
		} break;
	}
}



int AttribuerGroupe (Player& player,int x1, int y1,  int groupID){
	int x;
	int y;
	int chainLength=0;
	for (int i = 0; i < 4; i++){
		if (i%2 == 0){
			x = x1 +i -1;
			y = y1;
		} else {
			x = x1;
			y = y1 +i -2;
		}
		if (x >= 0 && x < WIDTHMAT && y >= 0 && y < HEIGHTMAT)  {
			if ( player.blocks[x][y].exist ) {
				if ( player.blocks[x1][y1].color == player.blocks[x][y].color && player.blocks[x][y].groupID == 0) {
					player.blocks[x][y].groupID = groupID;
					chainLength += (1 + AttribuerGroupe (player, x, y, groupID));
				}
			}
		}
	}
	return chainLength ;
}



void checkAllChains (Player& player) {
	int baseGroupId = 1;
	for (int i = 0; i < WIDTHMAT; i++) {
		for (int j = 0; j < HEIGHTMAT; j++) {
			if (player.blocks[i][j].exist && player.blocks[i][j].groupID == 0) {
				int length = AttribuerGroupe (player, i, j, baseGroupId) ;
				if (length > 0) {
				baseGroupId += 1 ;
				}
			}
		}
	}
}



bool blockAtStart (const Player& player) {
	return (player.blocks[WIDTHMAT/2 - 1][0].exist || player.blocks[WIDTHMAT/2][0].exist);
}



int countNbBlocksEqualID (Player& player, int ID) {
	int nb = 0; 
	for (int i = 0 ; i < WIDTHMAT ; i++) {
		for (int j = 0 ; j < HEIGHTMAT ; j++) {
			if (player.blocks[i][j].groupID == ID && player.blocks[i][j].color != VOID && player.blocks[i][j].color != WHITE) {
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


void resetBlocksForID (Player& player, int ID) {
	for (int i = 0 ; i < WIDTHMAT ; i++) {
		for (int j = 0 ; j < HEIGHTMAT ; j++) {
			if (player.blocks[i][j].groupID == ID && player.blocks[i][j].color != WHITE) {
				player.blocks[i][j].exist = false ;
				player.blocks[i][j].color = VOID ; 
				if (j+1 < HEIGHTMAT){
					if (player.blocks[i][j+1].color == WHITE) {
						player.blocks[i][j+1].exist = false ; 
						player.blocks[i][j+1].color = VOID ; 
					}
				}
				if (j-1 >= 0){
					if (player.blocks[i][j-1].color == WHITE) {
						player.blocks[i][j-1].exist = false ; 
						player.blocks[i][j-1].color = VOID ; 
					}
				}
				if (i-1 >= 0){
					if (player.blocks[i-1][j].color == WHITE) {
						player.blocks[i-1][j].exist = false ; 
						player.blocks[i-1][j].color = VOID ; 
					}
				}
				if (i+1 < WIDTHMAT){
					if (player.blocks[i+1][j].color == WHITE) {
						player.blocks[i+1][j].exist = false ; 
						player.blocks[i+1][j].color = VOID ; 
					}
				}
			}
		}
	}
}



int destroyBlock (Player& player) {
	int nbrChain = 0 ; 
	sf::SoundBuffer casser;
	if (!casser.loadFromFile("block_casser.wav"))
		return -1;
	for (int k = 1 ; k < WIDTHMAT*HEIGHTMAT ; k++ ) {
		if (countNbBlocksEqualID(player, k)>3) {
			resetBlocksForID(player, k) ; 
			nbrChain ++ ; 
			sf::SoundBuffer casser;


			sf::Sound sound;
			sound.setBuffer(casser);
			sound.play();
		}
	} 
	for (int i = 0 ; i < WIDTHMAT ; i++) {
		for (int j = 0 ; j < HEIGHTMAT ; j++) {
			player.blocks[i][j].groupID = 0 ; 
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
	bool doChange = false;
	if (p1.orient.clockwise) {
		if (p1.bf1.orient == LEFT){
			doChange = (p1.bf1.posMat.y - 1 >=0 && !p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y - 1].exist);
		}
		if (p1.bf1.orient == RIGHT){
			doChange = (p1.bf1.posMat.y + 1 < HEIGHTMAT && !p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y + 1].exist);
		}
		if (p1.bf1.orient == UP){
			doChange = (p1.bf1.posMat.x + 1 < WIDTHMAT && !p1.blocks[p1.bf1.posMat.x + 1][p1.bf1.posMat.y].exist);
		}
		if (p1.bf1.orient == DOWN){
			doChange = (p1.bf1.posMat.x - 1 >=0 && !p1.blocks[p1.bf1.posMat.x - 1][p1.bf1.posMat.y].exist);
		}
		
		if (doChange){
			p1.bf1.orient = (p1.bf1.orient + 1) % 4;
		}
		p1.orient.clockwise = false;
	}
	
	if (p1.orient.anticlockwise) {
		if (p1.bf1.orient == RIGHT){
			doChange = (p1.bf1.posMat.y - 1 >=0 && !p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y - 1].exist);
		}
		if (p1.bf1.orient == LEFT){
			doChange = (p1.bf1.posMat.y + 1 < HEIGHTMAT && !p1.blocks[p1.bf1.posMat.x][p1.bf1.posMat.y + 1].exist);
		}
		if (p1.bf1.orient == DOWN){
			doChange = (p1.bf1.posMat.x + 1 < WIDTHMAT && !p1.blocks[p1.bf1.posMat.x + 1][p1.bf1.posMat.y].exist);
		}
		if (p1.bf1.orient == UP){
			doChange = (p1.bf1.posMat.x - 1 >=0 && !p1.blocks[p1.bf1.posMat.x - 1][p1.bf1.posMat.y].exist);
		}
		if (doChange){
			p1.bf1.orient = (p1.bf1.orient + 3) % 4;
		}
		p1.orient.anticlockwise = false;
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
		p1.bf1.speed = FALLSPEED*3;
	}
	
	if (!p1.motion.down) {
		p1.bf1.speed = FALLSPEED;
	}
}


void startPlayer(Player& player){
	player.motion.right = false;
	player.motion.left = false;
	player.motion.down = false;
		
	player.orient.clockwise = false;
	player.orient.anticlockwise = false;
	
	player.penalty = 0;
	player.gameOver = false;
	player.delay = 0;
	player.doStartTour = true;
	player.score = 0;
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
	game.state = NOT_STARTED;
	
	//initialisation constantes 
	
	startPlayer(game.p1);
	startPlayer(game.p2);
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
			checkAllChains(p1);
			nbCombinations = destroyBlock(p1);
			doGravityOnAll(p1); 
			if (nbCombinations > 0) {
				p2.penalty += nbCombinations;
				p1.score += nbCombinations * 100;
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
			Pos pos = getPosSecondBlock(player);
			cases.setPosition(displacement + i*SIZEPUYO,j*SIZEPUYO);
			if ((i == pos.x && j == pos.y)||(i == player.bf1.posMat.x && j == player.bf1.posMat.y)){
				cases.setFillColor(getColor(VOID));
			} else {
				cases.setFillColor(getColor(player.blocks[i][j].color));   
			}
			cases.setSize(Vector2f(SIZEPUYO,SIZEPUYO));
			window.draw(cases);
		}
	}

	RectangleShape cases ;
	cases.setPosition(displacement + player.bf1.posMat.x*SIZEPUYO,player.bf1.posMat.y*SIZEPUYO+SIZEPUYO/DELAY*player.delay*player.bf1.speed);
	cases.setFillColor(getColor(player.bf1.color1));
	cases.setSize(Vector2f(SIZEPUYO,SIZEPUYO));
	window.draw(cases);

	Pos pos = getPosSecondBlock(player);
	cases.setPosition(displacement + pos.x*SIZEPUYO,pos.y*SIZEPUYO+SIZEPUYO/DELAY*player.delay*player.bf1.speed);
	cases.setFillColor(getColor(player.bf1.color2));
	cases.setSize(Vector2f(SIZEPUYO,SIZEPUYO));
	window.draw(cases);
		

	RectangleShape waitingBF ;
	waitingBF.setPosition(WAITINGPOSX + (displacement /12), WAITINGPOSY + (displacement/5));
	waitingBF.setFillColor(getColor(player.bf2.color1));
	waitingBF.setSize(Vector2f(SIZEPUYO,SIZEPUYO));
	window.draw(waitingBF);
	waitingBF.setPosition(WAITINGPOSX + SIZEPUYO + (displacement /12), WAITINGPOSY + (displacement/5));
	waitingBF.setFillColor(getColor(player.bf2.color2));
	window.draw(waitingBF);

	Font font;	
	font.loadFromFile("OpenSans-Regular.ttf");
	sf::Text text;
	text.setFont(font);
	text.setString(to_string(player.score));
	text.setCharacterSize((int)22*WIDTH/1000);
	text.setFillColor(Color::Black);
	if (displacement ==0){
		text.setPosition(Vector2f(410*WIDTH/1000, 550*HEIGHT/720));
	} else {
		text.setPosition(Vector2f(displacement - 100*WIDTH/1000, 550*HEIGHT/720));
	}
	window.draw(text);
	
	text.setString("Player 1 : ");
	text.setPosition(Vector2f(390*WIDTH/1000, 500*HEIGHT/720));
	window.draw(text);
	
	text.setString("Player 2 : ");
	text.setPosition(Vector2f(displacement - 120*WIDTH/1000, 500*HEIGHT/720));
	window.draw(text);
	
	text.setString("Press P to pause");
	text.setPosition(Vector2f(0.435*WIDTH, 0.95*HEIGHT));
	text.setCharacterSize((int)16*WIDTH/1000);
	window.draw(text);
	

}


void drawEndOfGame (sf::RenderWindow& window, const Game& game){
	window.clear(Color::Black);
	
	Font font;
	font.loadFromFile("OpenSans-Regular.ttf");
	sf::Text text;
	
	text.setFont(font);
	text.setString("Game Over");
	text.setCharacterSize((int) WIDTH*65/1000);
	text.setFillColor(Color::White);
	text.setPosition(Vector2f(WIDTH/3, 50*HEIGHT/720));
	window.draw(text);
	
	String player1, player2;
	
	if (game.p1.gameOver){
		player1 = "Player 1 lost";
		player2 = "Player 2 won";
	} else {
		player2 = "Player 2 lost";
		player1 = "Player 1 won";	
	}
	
	text.setString(player1);
	text.setCharacterSize((int)30*WIDTH/1000);
	text.setPosition(Vector2f(WIDTH/5, 200*HEIGHT/720));
	window.draw(text);
	text.setString(player2);
	text.setPosition(Vector2f(WIDTH-2*WIDTH/5, 200*HEIGHT/720));

	window.draw(text);
	
	player1 = "Score : " + to_string(game.p1.score);
	player2 = "Score : " + to_string(game.p2.score);
	text.setString(player1);
	text.setPosition(Vector2f(WIDTH/5, 250*HEIGHT/720));
	window.draw(text);
	text.setString(player2);
	text.setPosition(Vector2f(3*WIDTH/5, 250*HEIGHT/720));
	window.draw(text);
	

	text.setCharacterSize((int)30*WIDTH/1000);
	text.setString("-- Press Space and RShift to restart --");
	text.setPosition(Vector2f(230, 640));
	window.draw(text);
	
	window.display();
}

void drawStart (sf::RenderWindow& window){
	window.clear(Color::Black);
	
	Font font;
	font.loadFromFile("OpenSans-Regular.ttf");
	sf::Text text;
	text.setFont(font);
	
	text.setCharacterSize(110);
	text.setString("PUYO PUYO");
	text.setPosition(Vector2f(0.2*WIDTH, 80));
	text.setFillColor(Color::White);
	window.draw(text);
	
	text.setCharacterSize(40);
	text.setString("Player 1");
	text.setPosition(Vector2f(0.16*WIDTH, 280));
	window.draw(text);
	
	text.setString("Player 2");
	text.setPosition(Vector2f(0.68*WIDTH, 280));
	window.draw(text);
	
	RectangleShape touche;
	touche.setPosition(0.2*WIDTH, 400*HEIGHT/720);
	touche.setSize(Vector2f(SIZEPUYO,SIZEPUYO));
	window.draw(touche);
	touche.setPosition(0.2*WIDTH, 480*HEIGHT/720);
	window.draw(touche);
	touche.setPosition(0.12*WIDTH, 480*HEIGHT/720);
	window.draw(touche);
	touche.setPosition(0.28*WIDTH, 480*HEIGHT/720);
	window.draw(touche);
	touche.setPosition(0.12*WIDTH, 600*HEIGHT/720);
	touche.setSize(Vector2f(0.16*WIDTH + SIZEPUYO,SIZEPUYO));
	window.draw(touche);
	
	touche.setPosition(0.720*WIDTH, 400*HEIGHT/720);
	touche.setSize(Vector2f(SIZEPUYO,SIZEPUYO));
	window.draw(touche);
	touche.setPosition(0.720*WIDTH, 480*HEIGHT/720);
	window.draw(touche);
	touche.setPosition(0.640*WIDTH, 480*HEIGHT/720);
	window.draw(touche);
	touche.setPosition(0.800*WIDTH, 480*HEIGHT/720);
	window.draw(touche);
	touche.setPosition(0.640*WIDTH, 600*HEIGHT/720);
	touche.setSize(Vector2f(0.160*WIDTH + SIZEPUYO,SIZEPUYO));
	window.draw(touche);
	
	text.setString("R");
	text.setFillColor(Color::Black);
	text.setPosition(Vector2f(0.215*WIDTH, 405*HEIGHT/720));
	window.draw(text);
	text.setString("F");
	text.setPosition(Vector2f(0.220*WIDTH, 485*HEIGHT/720));
	window.draw(text);
	text.setString("D");
	text.setPosition(Vector2f(0.135*WIDTH, 485*HEIGHT/720));
	window.draw(text);
	text.setString("G");
	text.setPosition(Vector2f(0.295*WIDTH, 485*HEIGHT/720));
	window.draw(text);
	text.setString("Space");
	text.setPosition(Vector2f(0.165*WIDTH, 605*HEIGHT/720));
	window.draw(text);
	text.setString("RShift");
	text.setPosition(Vector2f(0.695*WIDTH, 605*HEIGHT/720));
	window.draw(text);
	text.setString("^");
	text.setPosition(Vector2f(0.740*WIDTH, 405*HEIGHT/720));
	window.draw(text);
	text.setString("<");
	text.setPosition(Vector2f(0.655*WIDTH, 485*HEIGHT/720));
	window.draw(text);
	text.setString(">");
	text.setPosition(Vector2f(0.815*WIDTH, 485*HEIGHT/720));
	window.draw(text);
	text.setString("v");
	text.setPosition(Vector2f(0.740*WIDTH, 485*HEIGHT/720));
	window.draw(text);
	
	text.setFillColor(Color::White);
	text.setCharacterSize(20);
	text.setString("-- Press Space and RShift to start --");
	text.setPosition(Vector2f(340, 40));
	window.draw(text);
	
	window.display();

}

void drawPause(sf::RenderWindow& window){
	
	RectangleShape pause ;
	pause.setPosition(WIDTH / 2 - 0.7 * SIZEPUYO, HEIGHT/2.1);
	pause.setFillColor(Color(128,128,128,128));
	pause.setSize(Vector2f(0.5*SIZEPUYO,2*SIZEPUYO));
	window.draw(pause);
	pause.setPosition(WIDTH/2+ 0.2 *SIZEPUYO, HEIGHT/2.1);
	window.draw(pause);
	window.display();
}

int main() {
	
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Puyo Puyo");
	Game game ;
	sf::Clock clock; 
	bool space = false;
	bool shift = false;
	bool pause = false;
	float dt;

	startGame(game);
	
	//boucle principale 
	while (window.isOpen()) {
		//actions du joueur
		sf::Event event ; 
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::P) {
					pause = true;
				}
				//j1
				if (event.key.code == sf::Keyboard::R) {
					game.p1.orient.clockwise = true;
				}
				if (event.key.code == sf::Keyboard::F) {
					game.p1.orient.anticlockwise = true;
				}
				if (event.key.code == sf::Keyboard::D) {
					game.p1.motion.left = true;
				}
				if (event.key.code == sf::Keyboard::G) {
					game.p1.motion.right = true;
				}
				if (event.key.code == sf::Keyboard::Space) {
					if (game.state == RUNNING){
						game.p1.motion.down = true;
					} else {
						space = true;
					}
				}
				//j2
				if (event.key.code == sf::Keyboard::Left) {
					game.p2.motion.left = true;
				}
				if (event.key.code == sf::Keyboard::Right) {
					game.p2.motion.right = true;
				}
				if (event.key.code == sf::Keyboard::Up) {
					game.p2.orient.clockwise = true;
				}
				if (event.key.code == sf::Keyboard::Down) {
					game.p2.orient.anticlockwise = true;
				}
				if (event.key.code == sf::Keyboard::RShift) {
					if (game.state == RUNNING){
						game.p2.motion.down = true;
					} else {
						shift = true;
					}
				}
			}
			if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::Space) {
					game.p1.motion.down = false;
				}
				if (event.key.code == sf::Keyboard::RShift) {
					game.p2.motion.down = false;
				}
				if (event.key.code == sf::Keyboard::R) {
					game.p1.orient.clockwise = false;
				}
				if (event.key.code == sf::Keyboard::F) {
					game.p1.orient.anticlockwise = false;
				}
				if (event.key.code == sf::Keyboard::D) {
					game.p1.motion.left = false;
				}
				if (event.key.code == sf::Keyboard::G) {
					game.p1.motion.right = false;
				}
				if (event.key.code == sf::Keyboard::Left) {
					game.p2.motion.left = false;
				}
				if (event.key.code == sf::Keyboard::Right) {
					game.p2.motion.right = false;
				}
				if (event.key.code == sf::Keyboard::Up) {
					game.p2.orient.clockwise = false;
				}
				if (event.key.code == sf::Keyboard::Down) {
					game.p2.orient.anticlockwise = false;
				}
			}
		}
		
		dt = clock.restart().asSeconds();
		
		if (game.state == NOT_STARTED){
			drawStart(window);
			if (space && shift){
				game.state = RUNNING;
				space = false;
				shift = false;
			}
		}
		
		
        

		if (game.state == RUNNING){
		
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
			
			drawGame(window, game.p2, 640*WIDTH/1000);
			drawGame(window, game.p1, 0);

			window.display();
			
			if (pause){
				game.state = PAUSE;
				pause = false;
			}
			
			if (game.p1.gameOver || game.p2.gameOver){
				game.state = END ;
			}
		}
		
		if (game.state == END){
			drawEndOfGame(window, game);
			if (space && shift){
				game.state = RESTART;
				space = false;
				shift = false;
			}
		}
		
		if (game.state == RESTART){
			startGame(game);
			game.state = RUNNING;
		}
		
		if (game.state == PAUSE){
			drawPause(window);
			if (pause){
				game.state = RUNNING;
				pause = false;
			}
		}

	}
	
	return 0 ;
}
