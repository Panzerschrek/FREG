/*Copyright (C) 2011 Alexander Kromm
 *
 *This file is part of Eyecube.
 *
 *Eyecube is free software: you can redistribute it and/or modify
 *it under the terms of the GNU General Public License as published by
 *the Free Software Foundation, either version 3 of the License, or
 *(at your option) any later version.
 *
 *Eyecube is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with Eyecube.  If not, see <http://www.gnu.org/licenses/>.
 */

//chunk sizes
//height
#define HEAVEN 128
#define WIDTH 64

//for sounds
#define NEAR 3
#define FAR 30

//sky blocks. color for stars and birds means background color
#define NOTHING 0
#define CYAN_SKY 1
#define BLUE_SKY 2
#define BLACK_SKY 3
#define BLUE_STAR 4
#define BLACK_STAR 5
//sun/moon
#define SUN 6
#define MOON 7
#define CLOUD 8
//birds
#define BLUE_RAVEN 9
#define CYAN_RAVEN 10
#define BLACK_RAVEN 11
#define CLOUD_RAVEN 12
#define SUN_RAVEN 13
#define BLUE_BIRD 14
#define CYAN_BIRD 15
#define BLACK_BIRD 16
#define CLOUD_BIRD 17
#define SUN_BIRD 18

//color pairs (foreground_background)
#define WHITE_BLUE 1
#define BLACK_GREEN 2
#define BLACK_WHITE 3
#define RED_YELLOW 4
#define RED_WHITE 5
#define WHITE_BLACK 6
#define YELLOW_RED 7
#define BLACK_RED 8
#define BLACK_YELLOW 9
#define BLUE_YELLOW 10
#define WHITE_CYAN 11
#define BLACK_BLUE 12
#define BLACK_CYAN 13
#define RED_BLUE 14
#define RED_CYAN 15
#define RED_BLACK 16

//clock, compass
#define NUMBER_OF_USEFUL 2

#define COMPONENTS_NUMBER 4

//blocks (components for crafting should stay in the beginning for check_craft())
#define AIR 0
#define IRON_INGOT 1
#define STONE 2
#define CHEST 3
#define WORKBENCH 4
#define GRASS 5
#define CHICKEN 6
#define FIRE 7
#define IRON_HELMET 8
#define PILE 9
#define CLOCK 10
#define COMPASS 11
#define IRON_GREAVES 12
#define IRON_BOOTS 13
#define IRON_CHESTPLATE 14
#define SIGN 15

//views
#define VIEW_SURFACE 0
#define VIEW_FLOOR 1
#define VIEW_HEAD 2
#define VIEW_SKY 3
#define VIEW_FRONT 4
#define VIEW_MENU 5
#define VIEW_INVENTORY 6
#define VIEW_CHEST 7
#define VIEW_WORKBENCH 8
#define VIEW_FURNACE 9

//sicnals
#define STOP 0
#define RUN 1

//for groups
#define ANIMALS CHICKEN
#define LIGHTS FIRE
#define CHESTLIKE CHEST: case PILE

struct something {
	struct something *next;
	short  *arr;
};
struct item {
	short what,
	      num;
};
struct for_sky {
	unsigned sky    : 5;
	unsigned sun    : 5;
	unsigned clouds : 5;
	unsigned birds  : 5;
};
struct list_to_clear {
	struct something *address;
	struct list_to_clear *next;
};
