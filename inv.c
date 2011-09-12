/*This file is part of Eyecube.
*
* Eyecube is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Eyecube is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Eyecube. If not, see <http://www.gnu.org/licenses/>.
*/

#include "header.h"

short cur[]={1, 0, 0, 0, 0};

extern struct item {
	short what,
	      num;
} inv[][3],
  *craft,
  cloth[];

struct something {
	struct something *next;
	short  *arr;
} ;

extern WINDOW *world,
              *pocketwin;
extern char   view;

void map(),
     pocketshow(),
     focus();

struct something *open_chest() {
	struct something *findanimal();
	short x, y, z;
	focus(&x, &y, &z);
        return(findanimal(x, y, z));
}

void mark(x, y, wind, c)
short  x, y;
WINDOW *wind;
char   c; {
	wattrset(wind, COLOR_PAIR(8));
	if (c=='e') { //empty
		(void)mvwprintw(wind, y, x-1, ">");
		(void)mvwprintw(wind, y, x+2, "<");
	} else { //full
		(void)mvwprintw(wind, y, x-1, "!");
		(void)mvwprintw(wind, y, x+2, "!");
	}
}

void keytoinv(key)
int key; {
	short pocketflag=0;
	switch (key) {
		case 'u': case 'f': case 'h': case 'k': case 'r': //views;
			free(craft);
			view=key;
			cur[1]=cur[2]=0;
			pocketflag=1;
		break;
		case KEY_LEFT:
			switch (cur[0]) {
				 //chest
				case 1: cur[1]=(cur[1]==0) ? 9 : cur[1]-1; break;
				case 3: //workbench
					if (cur[1]==0) {
						cur[1]=3;
						cur[2]=0;
					} else if (cur[1]==3) {
						cur[1]=(view=='w') ? 2 : 1;
						cur[2]=1;
					} else --cur[1];
				break;
			}
		break;
		case KEY_RIGHT:
			switch (cur[0]) {
				//chest
				case 1: cur[1]=(cur[1]==9) ? 0 : cur[1]+1; break;
				case 3: //workbench
					if (cur[1]==((view=='w') ? 2 : 1)) {
						cur[1]=3;
						cur[2]=0;
					} else if (cur[1]==3) {
						cur[1]=0;
						cur[2]=1;
					} else ++cur[1];
				break;
			}
		break;
		case KEY_UP:
			switch (cur[0]) {
				//chest
				case 1: cur[2]=(cur[2]==0) ? ((view=='c') ? 5 : 2) :
					cur[2]-1;
				break;
				//player
				case 2: cur[2]=(cur[2]==0) ? 3 : cur[2]-1; break;
				case 3: //workbench
					if (cur[1]!=3) cur[2]=(cur[2]==0) ?
						((view=='w') ? 2 : 1) : cur[2]-1;
				break;
			}
		break;
		case KEY_DOWN:
			switch (cur[0]) {
				//chest
				case 1: cur[2]=(cur[2]==((view=='c') ? 5 : 2)) ?
					0 : cur[2]+1;
				break;
				//player
				case 2: cur[2]=(cur[2]==3) ? 0 : cur[2]+1; break;
				case 3: //workbench
					if (cur[1]!=3)
						cur[2]=(cur[2]==((view=='w') ? 2 : 1)) ?
							0 : cur[2]+1;
				break;
			}
		break;
		case '\t':
			cur[1]=cur[2]=0;
			cur[0]=(cur[0]==3) ? 0 : cur[0]+1;
			//all function inventory types should be here
			if (!(view=='n') && cur[0]==0) cur[0]=1;
		break;
		case '\n': {
			short *markedwhat,
			      *markednum;
			switch (cur[0]) {
				case 1: //backpack
					if (cur[2]>2) {
						struct something *chest=open_chest();
						markedwhat=&(chest->arr
							[cur[1]+(cur[2]-3)*10+3]);
						markednum =&(chest->arr
							[cur[1]+(cur[2]-3)*10+33]);
					} else {
						markedwhat=&inv[cur[1]][cur[2]].what;
						markednum =&inv[cur[1]][cur[2]].num;
					}
				break;
				case 2: //player
					markedwhat=&cloth[cur[2]].what;
					markednum =&cloth[cur[2]].num;
				break;
				case 3: {//workbench
					short i=(cur[1]!=3) ? 1+cur[2]+cur[1]*2 : 0;
					markedwhat=&craft[i].what;
					markednum= &craft[i].num;
				} break;
			}
//			fprintf(stderr, "marked is %d\n", *markedwhat);
//			fprintf(stderr, "cur[2] is %d\n", cur[2]);
			if (cur[3]==0) { //get
				cur[3]=*markedwhat;
				*markedwhat=0;
				cur[4]=*markednum;
				*markednum=0;
			} else if (cur[3]==*markedwhat && cur[0]!=2 &&
					 property(*markedwhat, 's') &&
					!property(*markedwhat, 'a')) { //add
				for ( ; *markednum!=9 && cur[4]!=0; --cur[4])
					++*markednum;
				if (cur[4]==0) cur[3]=0;
			} else if (cur[0]!=2 || (cur[0]==2 && cur[2]==0 &&
					property(cur[3], 'a')=='h'))
/*				(cur[2]==0 && property(*markedwhat, 'a')=='h') ||
				(cur[2]==1 && property(*markedwhat, 'a')=='a') ||
				(cur[2]==2 && property(*markedwhat, 'a')=='l') ||
				(cur[2]==3 && property(*markedwhat, 'a')=='b'))*/ {
				//change (put)
				short save=*markedwhat;
				*markedwhat=cur[3];
				cur[3]=save;
				save=*markednum;
				*markednum=cur[4];
				cur[4]=save;
			}
		} break;
	}
	map();
	if (pocketflag) pocketshow();
}

void invview() {
	short i, j;
	//left arm
	wattrset(world, COLOR_PAIR(3));
	(void)mvwprintw(world, 4, 5, "U");
	//right arm
	if (inv[cloth[4].num][2].what) (void)mvwprintw(world, 4, 1, "%c%d",
			getname(inv[cloth[4].num][2].what, world),
			        inv[cloth[4].num][2].num);
	else (void)mvwprintw(world, 4, 2, "U");
	//shoulders
	if (cloth[1].what) (void)getname(cloth[1].what, world);
	else wattrset(world, COLOR_PAIR(1));
	(void)mvwprintw(world, 3, 2, "    ");
	for (i=0; i<=3; ++i)
		if (cloth[i].what) {
			char name=getname(cloth[i].what, world);
			(void)mvwprintw(world, 2+i, 3, "%c%c", name, name);
		} else switch (i) {
			case 0: //head
				wattrset(world, COLOR_PAIR(3));
				(void)mvwprintw(world, 2, 3, "''");
			break;
			case 1:	case 2: //body & legs
				wattrset(world, COLOR_PAIR(1));
				(void)mvwprintw(world, 2+i, 3, "  ");
			break;
			case 3: //feet
				wattrset(world, COLOR_PAIR(3));
				(void)mvwprintw(world, 5, 3, "db");
			break;
		}
	//backpack
	for (j=0; j<=9; ++j)
	for (i=0; i<=2; ++i)
		(void)mvwprintw(world, i+19+((i==2) ? 1 : 0), j*3+7, "%c%d",
			getname(inv[j][i].what, world), inv[j][i].num);
	//chest
	if ('c'==view) {
		(void)mvwprintw(world, 17, 1, "Chest");
		(void)mvwprintw(world, 16, 6, "|");
		(void)mvwprintw(world, 17, 6, "|");
		(void)mvwprintw(world, 18, 6, "|");
		struct something *chest=open_chest();
		for (j=0; j<=9; ++j)
		for (i=0; i<=2; ++i)
			(void)mvwprintw(world, i+16, j*3+7, "%c%d",
				getname(chest->arr[3+j+i*10], world),
				        chest->arr[33+j+i*10]);
	}
	//workbench
	(void)mvwprintw(world, 11, 9, "Workbench");
	if (view!='w')
		for (i=0; i<=1; ++i)
		for (j=0; j<=1; ++j)
			(void)mvwprintw(world, 12+i, 10+j*3, "%c%d",
				getname(craft[i+2*j+1].what, world),
				        craft[i+2*j+1].num);
	else
		for (i=0; i<=2; ++i)
		for (j=0; j<=2; ++j)
			(void)mvwprintw(world, 12+i, 7+j*3, "%c%d",
				getname(craft[i+2*j+1].what, world),
				        craft[i+2*j+1].num);
	(void)mvwprintw(world, 13, 17, "%c%d", getname(craft[0].what, world),
	                                               craft[0].num);
	//cursor (i, j are now coordinates)
	switch (cur[0]) {
		case 1: //chest
			i=cur[2]+((cur[2]>2) ? 13 : 19+((cur[2]==2) ? 1 : 0));
			j=cur[1]*3+7;
		break;
		case 2: //player
			i=cur[2]+2;
			j=3;
		break;
		case 3: //workbench
			i=(cur[1]!=3) ? cur[2]+12 : 13;
			j=(cur[1]!=3) ? (cur[1]*3+((view!='w') ? 10 : 7)) : 17;
		break;
	}
	if (cur[3]!=0) {
		(void)mvwprintw(world, i, j, "%c%d",
			getname(cur[3], world), cur[4]);
		mark(j, i, world, 'f');
	} else  mark(j, i, world, 'e');
	(void)wclear(pocketwin);
	(void)wrefresh(pocketwin);
}
