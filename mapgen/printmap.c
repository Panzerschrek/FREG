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

#include <stdio.h>
#include <curses.h>

enum color_pairs { //do not change colors order! //foreground_background
        BLACK_BLACK=1,
        BLACK_RED,
        BLACK_GREEN,
        BLACK_YELLOW,
        BLACK_BLUE,
        BLACK_MAGENTA,
        BLACK_CYAN,
        BLACK_WHITE,
        //
        RED_BLACK,
        RED_RED,
        RED_GREEN,
        RED_YELLOW,
        RED_BLUE,
        RED_MAGENTA,
        RED_CYAN,
        RED_WHITE,
        //
        GREEN_BLACK,
        GREEN_RED,
        GREEN_GREEN,
        GREEN_YELLOW,
        GREEN_BLUE,
        GREEN_MAGENTA,
        GREEN_CYAN,
        GREEN_WHITE,
        //
        YELLOW_BLACK,
        YELLOW_RED,
        YELLOW_GREEN,
        YELLOW_YELLOW,
        YELLOW_BLUE,
        YELLOW_MAGENTA,
        YELLOW_CYAN,
        YELLOW_WHITE,
        //
        BLUE_BLACK,
        BLUE_RED,
        BLUE_GREEN,
        BLUE_YELLOW,
        BLUE_BLUE,
        BLUE_MAGENTA,
        BLUE_CYAN,
        BLUE_WHITE,
        //
	MAGENTA_BLACK,
        MAGENTA_RED,
        MAGENTA_GREEN,
        MAGENTA_YELLOW,
        MAGENTA_BLUE,
        MAGENTA_MAGENTA,
        MAGENTA_CYAN,
        MAGENTA_WHITE,
        //
        CYAN_BLACK,
        CYAN_RED,
        CYAN_GREEN,
        CYAN_YELLOW,
        CYAN_BLUE,
        CYAN_MAGENTA,
        CYAN_CYAN,
        CYAN_WHITE,
        //
        WHITE_BLACK,
        WHITE_RED,
        WHITE_GREEN,
        WHITE_YELLOW,
        WHITE_BLUE,
        WHITE_MAGENTA,
        WHITE_CYAN,
        WHITE_WHITE
}; //enum color_pairs

int main() {
	(void)initscr();
	(void)start_color();
	const short colors[]={ //do not change colors order!
		COLOR_BLACK,
		COLOR_RED,
		COLOR_GREEN,
		COLOR_YELLOW,
		COLOR_BLUE,
		COLOR_MAGENTA,
		COLOR_CYAN,
		COLOR_WHITE
	};
	short i;
	for (i=BLACK_BLACK; i<=WHITE_WHITE; ++i) {
		init_pair(i, colors[(i-1)/8], colors[(i-1)%8]);
	}
	
	FILE * map=fopen("map.txt", "r");

	char c=fgetc(map);
	while( c!=EOF ) {
		switch ( c ) {
			case '~':
				color_set(CYAN_BLUE, NULL);
			break;
			case '%':
			case '.':
				color_set(BLACK_GREEN, NULL);
			break;
			case '+':
				color_set(WHITE_GREEN, NULL);
			break;
			case '^':
				color_set(BLACK_WHITE, NULL);
			break;
		}
		addch(' ');
		addch(c);
		c=fgetc(map);
	}
	fclose(map);
	refresh();
	getch();

	endwin();
}
