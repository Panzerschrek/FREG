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

//this file is related to curses screen for freg.

#include <QString>
#include <QTimer>
#include "screen.h"
#include "world.h"
#include "blocks.h"
#include "Player.h"

void Screen::PassString(QString & str) const {
	puts("Enter inscription:");
	char temp_str[note_length+1];
	fgets(temp_str, note_length, stdin);
	str=temp_str;
}

char Screen::CharName(
		const int kind,
	       	const int sub) const
{
	switch (kind)  {
		case CHEST:
		case PILE:   return '&';
		case BUSH:   return ';';
		case PICK:   return '\\';
		case DWARF:  return '@';
		case LIQUID: return '~';
		case GRASS:  return '.';
		case RABBIT: return 'r';
		case CLOCK:  return 'c';  
		case TELEGRAPH: return 't';
		default: switch (sub) {
			case NULLSTONE: case MOSS_STONE: case WOOD:
			case STONE: return '#';
			case GLASS: return 'g';
			case SUN_MOON:
			case SKY:   return ' ';
			case STAR:  return '.';
			case WATER: return '~';
			case SAND:
			case SOIL:  return '.';
			case GREENERY: return '%';
			case ROSE:  return ';';
			case A_MEAT: case H_MEAT:
			case HAZELNUT: return ',';
			case AIR:   return ' ';
			default:
				fprintf(stderr,
					"Screen::CharName(uns short, uns short, uns short): Block has unlisted substance: %d\n",
					int(sub));
				return '?';
		}
	}
}
char Screen::CharName(
		const ushort i,
		const ushort j,
		const ushort k) const
{
	return CharName( w->Kind(i, j, k),
		w->Sub(i, j, k) );
}

void Screen::ControlPlayer(const int ch) {
	if ( 'Q'==ch ) {
		emit ExitReceived();
		return;
	}
	if ( ch>='a' && ch<='z' ) {
		const int num=ch-'a';
		switch ( actionMode ) {
			case USE:   player->Use(num); break;
			case THROW: player->Throw(num); break;
			case OBTAIN: player->Obtain(num); break;
			case WIELD: player->Wield(num); break;
			case INSCRIBE: player->Inscribe(num); break;
			case EAT: player->Eat(num); break;
			case BUILD: player->Build(num); break;
			case CRAFT: player->Craft(num); break;
			case TAKEOFF: player->TakeOff(num); break;
			default:
				fprintf(stderr,
					"Screen::ControlPlayer: unlisted action mode: %d\n",
					actionMode);
		}
		return;
	}
	switch ( ch ) {
		/*case KEY_UP: player->Move(NORTH); break;
		case KEY_DOWN: player->Move(SOUTH); break;
		case KEY_RIGHT: player->Move(EAST); break;
		case KEY_LEFT: player->Move(WEST); break;*/
		case ' ': player->Jump(); break;
		
		case '>': player->Turn(w->TurnRight(player->Dir())); break;
		case '<': player->Turn(w->TurnLeft(player->Dir())); break;
		/*case KEY_NPAGE: player->Turn(DOWN); break;
		case KEY_PPAGE: player->Turn(UP); break;
	
		case KEY_HOME: player->Backpack(); break;
		case KEY_BACKSPACE: player->Damage(); break;*/
		case '\n': player->Use(); break;
		case  '?': player->Examine(); break;
		case  '~': player->Inscribe(); break;
		
		case 'U': actionMode=USE; break;
		case 'T': actionMode=THROW; break;
		case 'O': actionMode=OBTAIN; break;
		case 'W': actionMode=WIELD; break;
		case 'I': actionMode=INSCRIBE; break;
		case 'E': actionMode=EAT; break;
		case 'B': actionMode=BUILD; break;
		case 'C': actionMode=CRAFT; break;
		case 'F': actionMode=TAKEOFF; break;

		default: Notify("Don't know what such key means."); break;
	}
}

void Screen::Notify(const QString & str) {
	puts(str.toLocal8Bit().constData());
	fputs(str.toLocal8Bit().constData(), notifyLog);
}

Screen::Screen(
		World * const wor,
		Player * const pl)
		:
		VirtScreen(wor, pl),
		cleaned(false),
		actionMode(USE)
{
	input=new IThread(this);

	input->start();

	notifyLog=fopen("messages.txt", "a");
	Notify("Game started.");
}

void Screen::CleanAll() {
	//TODO: make own lock
	w->WriteLock();
	if ( cleaned ) {
		w->Unlock();
		return;
	}

	cleaned=true;//prevent double cleaning
	input->Stop();
	input->wait();
	delete input;
	w->Unlock();

	if ( NULL!=notifyLog )
		fclose(notifyLog);
}

IThread::IThread(Screen * const scr)
		:
	screen(scr),
	stopped(false)
{}

void IThread::run() {
	while ( !stopped )
		screen->ControlPlayer(getchar());
}

void IThread::Stop() { stopped=true; }
