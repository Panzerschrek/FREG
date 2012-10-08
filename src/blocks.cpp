	/*
	*This file is part of FREG.
	*
	*FREG is free software: you can redistribute it and/or modify
	*it under the terms of the GNU General Public License as published by
	*the Free Software Foundation, either version 3 of the License, or
	*(at your option) any later version.
	*
	*FREG is distributed in the hope that it will be useful,
	*but WITHOUT ANY WARRANTY; without even the implied warranty of
	*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	*GNU General Public License for more details.
	*
	*You should have received a copy of the GNU General Public License
	*along with FREG. If not, see <http://www.gnu.org/licenses/>.
	*/

#include "blocks.h"
#include "world.h"

void BlockFromFile(FILE * const in, Block * & block, World * const world,
		const unsigned short i, const unsigned short j, const unsigned short k) {
	char str[300];
	fgets(str, 300, in);
	int kind;
	sscanf(str, "%d", &kind);
	//if some kind will not be listed here, blocks of this kind just will not load.
	//unless kind is inherited from Inventory class or one of its derivatives - in this case this may cause something bad.
	switch(kind) {
		case BLOCK:     block=new Block(str);     break;
		case TELEGRAPH: block=new Telegraph(str); break;
		case PICK:      block=new Pick(str);      break;
		case CHEST:     block=new Chest(str, in); break;
		case RABBIT:    block=new Rabbit(world, i, j, k, str); break;
		case ACTIVE:    block=new Active(world, i, j, k, str); break;
		case DWARF:     block=new Dwarf (world, i, j, k, str, in); break;
		case PILE:      block=new Pile  (world, i, j, k, str, in); break;
		case LIQUID:    block=new Liquid(world, i, j, k, str, in); break;
		case GRASS:     block=new Grass (world, i, j, k, str, in); break;
		case BUSH:      block=new Bush  (world, i, j, k, str, in); break;
		case -1:        block=NULL; break;
		default:
			fprintf(stderr, "BlockFromFile(FILE *, Block * &, World *, unsigned short, unsigned short, unsigned short): unlisted kind\n");
			block=NULL;
	}
}

void Active::SafeMove() {
	//when player is on the border of shred, moving him causes World::ReloadShreds(dir). so, cheks are neede for liquids not to push player.
	unsigned short x_focus, y_focus, z_focus;
	if ( !(whereWorld->Focus(x_self, y_self, z_self, x_focus, y_focus, z_focus, direction)) &&
			((Block *)(whereWorld->playerP)!=whereWorld->blocks[x_focus][y_focus][z_focus] ||
			 UP==direction || DOWN==direction ) ) {
		whereWorld->Move(x_self, y_self, z_self, direction);
	}
}
void Active::SafeJump() {
	SetWeight(0);
	if ( whereWorld->Move(x_self, y_self, z_self, UP) ) {
		SetWeight();
		SafeMove();
	} else
		SetWeight();
}

void Active::Register(World * const w, const int x, const int y, const int z) {
	whereWorld=w;
	if (NULL!=whereWorld) {
		x_self=x;
		y_self=y;
		z_self=z;
		prev=NULL;
		if (NULL==whereWorld->activeList)
			next=NULL;
		else {
			next=whereWorld->activeList;
			whereWorld->activeList->prev=this;
		}
		whereWorld->activeList=this;
	} else {
		next=NULL;
		prev=NULL;
	}
}

void Active::Unregister() {
	if (NULL!=whereWorld) {
		if (NULL!=next)
			next->prev=prev;
		if (NULL!=prev)
			prev->next=next;
		else {
			whereWorld->activeList=next;
			if (NULL!=whereWorld->activeList)
				whereWorld->activeList->prev=NULL;
		}
	}
}

void Animal::Act() {
	if ( LIQUID==whereWorld->Kind(x_self, y_self, z_self+1) ) {
		if (0>=breath)
			whereWorld->Damage(x_self, y_self, z_self, 10, BREATH, false);
		else
			--breath;
		return;
	} else if ( breath<max_breath )
		++breath;

	if (0<satiation)
		--satiation;
	else
		whereWorld->Damage(x_self, y_self, z_self, 1, HUNGER, false);
}

int Dwarf::Move(const dirs dir) {
	Active::Move(dir);
	if ( this==whereWorld->playerP && (
			(x_self==shred_width-1 || x_self==shred_width*2 ||
			 y_self==shred_width-1 || y_self==shred_width*2)) ) {
		whereWorld->ReloadShreds(dir);
		return 1;
	}
	return 0;
}

void Dwarf::Act() {
	Animal::Act();
}

before_move_return Dwarf::BeforeMove(const dirs dir) {
	if (dir==direction)
		whereWorld->GetAll(x_self, y_self, z_self);
	return NOTHING;
}

before_move_return Pile::BeforeMove(const dirs dir) {
	direction=dir;
	whereWorld->DropAll(x_self, y_self, z_self);
	return NOTHING;
}

bool Liquid::CheckWater(const dirs dir) const {
	unsigned short i_check, j_check, k_check;
	if ( (whereWorld->Focus(x_self, y_self, z_self, i_check, j_check, k_check, dir)) )
		return false;
	if ( WATER==whereWorld->Sub(i_check, j_check, k_check) )
		return true;
	return false;
}

void Liquid::Act() {
	switch (random()%4) {
		case 0: SetDir(NORTH); break;
		case 1:	SetDir(EAST);  break;
		case 2: SetDir(SOUTH); break;
		case 3:	SetDir(WEST);  break;
	}
	SafeMove();
}

void Grass::Act() {
	if ( random()%seconds_in_hour )
		return;
	
	short i=x_self, j=y_self;
	switch ( random()%4 /* increase this if grass grows too fast */) {
		case 0: ++i; break;
		case 1: --i; break;
		case 2: ++j; break;
		case 3: --j; break;
		default: return;
	}

	if ( whereWorld->InBounds(i, j, z_self) ) {
		if ( AIR==whereWorld->Sub(i, j, z_self) &&
				whereWorld->InBounds(i, j, z_self-1) && SOIL==whereWorld->Sub(i, j, z_self-1) )
			whereWorld->Build(new Grass(), i, j, z_self);
		else if ( SOIL==whereWorld->Sub(i, j, z_self) &&
				whereWorld->InBounds(i, j, z_self+1) && AIR==whereWorld->Sub(i, j, z_self+1) )
			whereWorld->Build(new Grass(), i, j, z_self+1);
	}
}

void Rabbit::Act() {
	float attractive;
	float for_north=0, for_west=0;
	for (short x=x_self-7; x<=x_self+7; ++x)
	for (short y=y_self-7; y<=y_self+7; ++y)
	for (short z=z_self-7; z<=z_self+7; ++z)
		if ( whereWorld->InBounds(x, y, z) ) {
			switch ( whereWorld->Kind(x, y, z) ) {
				case DWARF:  if (whereWorld->DirectlyVisible(x_self, y_self, z_self, x, y, z)) attractive=-9; break;
				case RABBIT: if (whereWorld->DirectlyVisible(x_self, y_self, z_self, x, y, z)) attractive=0.8; break;
				case GRASS:  if (whereWorld->DirectlyVisible(x_self, y_self, z_self, x, y, z)) attractive=0.1; break;
				default: attractive=0;
			}
			if (attractive) {
				if (y!=y_self)
					for_north+=attractive/(y_self-y);
				if (x!=x_self)
					for_west +=attractive/(x_self-x);
			}
		}

	if (abs(for_north)>1 || abs(for_west)>1) {
		if ( abs(for_north)>abs(for_west) ) {
			if (for_north>0) SetDir(NORTH);
			else SetDir(SOUTH);
		} else {
			if (for_west>0) SetDir(WEST);
			else SetDir(EAST);
		}
		(random()%2) ? SafeMove() : SafeJump();
	} else if (0==random()%60) {
		switch (random()%4) {
			case 0: SetDir(NORTH); break;
			case 1: SetDir(SOUTH); break;
			case 2: SetDir(EAST);  break;
			case 3: SetDir(WEST);  break;
		}
		(random()%2) ? SafeMove() : SafeJump();
	}

	if ( seconds_in_day*time_steps_in_sec/2>satiation ) {
		for (short i=x_self-1; i<=x_self+1; ++i)
		for (short j=y_self-1; j<=y_self+1; ++j)
		for (short k=z_self-1; k<=z_self+1; ++k)
			if ( GREENERY==whereWorld->Sub(i, j, k) ) {
				whereWorld->Eat(x_self, y_self, z_self, i, j, k);
				fprintf(stderr, "Rabbit wants to eat\n");
			}
	}

	Animal::Act();
}