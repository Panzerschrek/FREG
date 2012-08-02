#ifndef WORLD_H
#define WORLD_H

#include <pthread.h>
#include <cmath>

class Screen;

void *PhysThread(void *vptr_args);
class World {
	unsigned long time;
	Block *blocks[shred_width*3][shred_width*3][height];
	Dwarf * playerP;
	unsigned short playerX, playerY, playerZ;
	long longitude, latitude;
	pthread_t eventsThread;
	pthread_mutex_t mutex;
	Active * activeList;
	void LoadShred(long, long, unsigned short, unsigned short);
	void SaveShred(long, long, unsigned short, unsigned short);
	void ReloadShreds(dirs);
	void MakeSky() {
		FILE * sky=fopen("sky.txt", "r");
		if (NULL==sky) {
			for (unsigned short i=0; i<shred_width*3; ++i)
			for (unsigned short j=0; j<shred_width*3; ++j)
				blocks[i][j][height-1]=new Block( random()%5 ? STAR : SKY );
		} else {
			char c=fgetc(sky)-'0';
			for (unsigned short i=0; i<shred_width*3; ++i)
			for (unsigned short j=0; j<shred_width*3; ++j)
				if (c) {
					blocks[i][j][height-1]=new Block(SKY);
					--c;
				} else {
					blocks[i][j][height-1]=new Block(STAR);
					c=fgetc(sky)-'0';
				}
			fclose(sky);
		}
	}
	dirs MakeDir(unsigned short x_center, unsigned short y_center, unsigned short x_target, unsigned short y_target) {
		//if (x_center==x_target && y_center==y_target) return HERE;
		if (abs(x_center-x_target)<=1 && abs(y_center-y_target)<=1) return HERE;
		float x=x_target-x_center,
		      y=y_target-y_center;
		if (y<=3*x && y<=-3*x) return NORTH;
		else if (y>-3*x && y<-x/3) return NORTH_EAST;
		else if (y>=-x/3 && y<=x/3) return EAST;
		else if (y>x/3 && y<3*x) return SOUTH_EAST;
		else if (y>=3*x && y>=-3*x) return SOUTH;
		else if (y<-3*x && y>-x/3) return SOUTH_WEST;
		else if (y<=-x/3 && y>=x/3) return WEST;
		else return NORTH_WEST;
	}
	double Distance(unsigned short x_from, unsigned short y_from, unsigned short z_from,
	                unsigned short x_to,   unsigned short y_to,   unsigned short z_to) {
		return sqrt( (x_from-x_to)*(x_from-x_to)+
		             (y_from-y_to)*(y_from-y_to)+
		             (z_from-z_to)*(z_from-z_to) );
	}
	public:
	Screen * scr;
	struct {
		char ch;
		unsigned short lev;
		color_pairs col;
	} soundMap[9];
	void PhysEvents();
	char CharNumber(int, int, int);
	char CharNumberFront(int, int);
	bool DirectlyVisible(int, int, int, int, int, int);
	bool Visible(int, int, int, int, int, int);
	bool Visible(int x_to, int y_to, int z_to) { return Visible(playerX, playerY, playerZ, x_to, y_to, z_to); }
	int  Move(int, int, int, dirs);
	void Jump(int, int, int);
	void Focus(int i, int j, int k, int & i_target, int & j_target, int & k_target) {
		i_target=i;
		j_target=j;
		k_target=k;
		switch ( blocks[i][j][k]->GetDir() ) {
			case NORTH: --j_target; break;
			case SOUTH: ++j_target; break;
			case EAST:  ++i_target; break;
			case WEST:  --i_target; break;
			case DOWN:  --k_target; break;
			case UP:    ++k_target; break;
		}
	}
	void PlayerFocus(int & i_target, int & j_target, int & k_target) { Focus(playerX, playerY, playerZ, i_target, j_target, k_target); }
	void SetPlayerDir(dirs dir) { playerP->SetDir(dir); }
	dirs GetPlayerDir() { return playerP->GetDir(); }
	void GetPlayerCoords(short & x, short & y, short & z) { x=playerX; y=playerY; z=playerZ; }
	Dwarf * GetPlayerP() { return playerP; }
	unsigned long GetTime() { return time; }
	times_of_day PartOfDay() {
		unsigned short time_day=TimeOfDay();
		if (time_day<end_of_night)   return NIGHT;
		if (time_day<end_of_morning) return MORNING;
		if (time_day<end_of_noon)    return NOON;
		return EVENING;
	}
	int TimeOfDay() { return time%seconds_in_day; }
	unsigned long Time() { return time; }
	void FullName(char * str, int i, int j, int k) { (NULL==blocks[i][j][k]) ? WriteName(str, "Air") : blocks[i][j][k]->FullName(str); }
	subs Sub(int i, int j, int k)         { return (NULL==blocks[i][j][k]) ? AIR : blocks[i][j][k]->Sub(); }
	kinds Kind(int i, int j, int k)       { return (NULL==blocks[i][j][k]) ? BLOCK : blocks[i][j][k]->Kind(); }
	int  Transparent(int i, int j, int k) { return (NULL==blocks[i][j][k]) ? 2 : blocks[i][j][k]->Transparent(); }
	int  Movable(Block * block)           { return (NULL==block) ? GAS : block->Movable(); }
	double Weight(Block * block)          { return (NULL==block) ? 0 : block->Weight(); }
	int  PlayerMove(dirs dir)             { return Move( playerX, playerY, playerZ, dir ); }
	int  PlayerMove()                     { return Move( playerX, playerY, playerZ, playerP->GetDir() ); }
	void PlayerJump() {
		playerP->SetWeight(0);
		if ( PlayerMove(UP) ) PlayerMove();
		playerP->SetWeight();
		PlayerMove(DOWN);
	}
	char MakeSound(int i, int j, int k) { return (NULL==blocks[i][j][k]) ? ' ' : blocks[i][j][k]->MakeSound(); }
	friend void Screen::Print();
	friend class Active;
	World();
	~World();
};

#endif
