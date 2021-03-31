# Puyo_Puyo
Amusez vous bien Kappa

struct Pos {
	int x ;
	int y ;
}

struct Block {
	bool exist;
	char color;
	int groupID;
}

struct BlockFall {
	entier orient ;
	char color1 ;
	char color2 ; 
	Pos posMat ; 
	int speed ; 
}

struct Player {
	BlockFall bf1;
	BlockFall bf2 ;
	Block[][] blocks //pas sure de ca ;
	int score ;
}

struct Game {
	Player p1;
	Player p2; 
}
