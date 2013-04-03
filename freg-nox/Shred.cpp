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

#include <QFile>
#include <QByteArray>
#include <QDataStream>
#include "Shred.h"
#include "world.h"

const int datastream_version=QDataStream::Qt_4_6; //Qt version in Debian stable now.

int Shred::LoadShred(QFile & file) {
		QByteArray read_data=file.readAll();
		QByteArray uncompressed=qUncompress(read_data);
		QDataStream in(uncompressed);
		quint8 version;
		in >> version;
		if ( datastream_version!=version ) {
			fprintf(stderr,
				"Wrong version: %d\nGenerating new shred.\n",
				datastream_version);
			return 1;
		}
		in.setVersion(datastream_version);
		for (ushort i=0; i<SHRED_WIDTH; ++i)
		for (ushort j=0; j<SHRED_WIDTH; ++j) {
			for (ushort k=0; k<HEIGHT; ++k) {
				blocks[i][j][k]=BlockFromFile(in, i, j, k);
				lightMap[i][j][k]=0;
			}
			lightMap[i][j][HEIGHT-1]=MAX_LIGHT_RADIUS;
		}
		return 0;
}

Shred::Shred(
		World * const world_,
		const ushort shred_x,
		const ushort shred_y,
		const long longi,
		const long lati)
		:
		world(world_),
		longitude(longi),
		latitude(lati),
		shredX(shred_x),
		shredY(shred_y)
{
	QFile file(FileName());
	if ( file.open(QIODevice::ReadOnly) && !LoadShred(file) )
		return;

	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j) {
		blocks[i][j][0]=NewNormal(NULLSTONE);
		for (ushort k=1; k<HEIGHT-1; ++k) {
			blocks[i][j][k]=NewNormal(AIR);
			lightMap[i][j][k]=0;
		}
		blocks[i][j][HEIGHT-1]=NewNormal( (rand()%5) ? SKY : STAR );
		lightMap[i][j][HEIGHT-1]=MAX_LIGHT_RADIUS;
	}

	switch ( TypeOfShred(longi, lati) ) {
		case '#': NullMountain(); break;
		case '.': Plain(); break;
		case 't': TestShred(); break;
		case '%': Forest(longi, lati); break;
		case '~': Water( longi, lati); break;
		case '+': Hill(  longi, lati); break;
		case '_': /* empty shred */    break;
		case 'p': Pyramid();           break;
		case '^': Mountain();          break;
		default:
			Plain();
			fprintf(stderr,
				"Shred::Shred: unknown type of shred: %c\n",
				TypeOfShred(longi, lati));
	}
}

Shred::~Shred() {
	ushort i, j, k;
	const long mapSize=world->MapSize();
	if (
			(longitude < mapSize) && (longitude >= 0) &&
			(latitude  < mapSize) && (latitude  >= 0) )
	{
		QFile file(FileName());
		if ( !file.open(QIODevice::WriteOnly) ) {
			fputs("Shred::~Shred: Write Error\n", stderr);
			return;
		}

		QByteArray shred_data;
		shred_data.reserve(200000);
		QDataStream outstr(&shred_data, QIODevice::WriteOnly);
		outstr << (quint8)datastream_version;
		outstr.setVersion(datastream_version);
		for (i=0; i<SHRED_WIDTH; ++i)
		for (j=0; j<SHRED_WIDTH; ++j)
		for (k=0; k<HEIGHT; ++k) {
			blocks[i][j][k]->SaveToFile(outstr);
			if ( !(blocks[i][j][k]->Normal()) )
				delete blocks[i][j][k];
		}
		file.write(qCompress(shred_data));
		return;
	}

	for (i=0; i<SHRED_WIDTH; ++i)
	for (j=0; j<SHRED_WIDTH; ++j)
	for (k=0; k<HEIGHT; ++k)
		if ( !(blocks[i][j][k]->Normal()) )
			delete blocks[i][j][k];
}

void Shred::SetNewBlock(
		const int kind,
		const int sub,
		const ushort x,
		const ushort y,
		const ushort z)
{
	Block * const block=NewBlock(kind, sub);
	blocks[x][y][z]=block;
	Active * const active=block->ActiveBlock();
	if ( active )
		active->Register(this,
			SHRED_WIDTH*shredX+x,
			SHRED_WIDTH*shredY+y, z);
	Inventory * const inventory=block->HasInventory();
	if ( inventory )
		inventory->SetShred(this);
}

Block * Shred::NewBlock(const int kind, int sub) const {
	if ( sub > AIR ) {
		fprintf(stderr,
			"Don't know such substance: %d.\n",
			sub);
		sub=STONE;
	}
	switch ( kind ) {
		case BLOCK:  return NewNormal(sub);
		case GRASS:  return new Grass();
		case PICK:   return new Pick(sub);
		case PLATE:  return new Plate(sub);
		case ACTIVE: return new Active(sub);
		case LADDER: return new Ladder(sub);
		case WEAPON: return new Weapon(sub);
		case BUSH:   return new Bush();
		case CHEST:  return new Chest(0, sub);
		case PILE:   return new Pile  (0, 0, 0, 0);
		case DWARF:  return new Dwarf (0, 0, 0, 0);
		case RABBIT: return new Rabbit(0, 0, 0, 0);
		case DOOR:   return new Door  (0, 0, 0, 0, sub);
		case LIQUID: return new Liquid(0, 0, 0, 0, sub);
		case CLOCK:  return new Clock(GetWorld(), sub);
		case WORKBENCH: return new Workbench(0, sub);
		default:
			fprintf(stderr,
				"Shred::NewBlock: unlisted kind: %d\n",
				kind);
			return NewNormal(sub);
	}
}

Block * Shred::NewNormal(const int sub) const {
	return world->NewNormal(sub);
}

void Shred::PhysEvents() {
	for (int j=0; j<activeList.size(); ++j) {
		Active * const temp=activeList[j];
		const ushort x=temp->X();
		const ushort y=temp->Y();
		const ushort z=temp->Z();
		const float weight=Weight(x%SHRED_WIDTH, y%SHRED_WIDTH, z);
		if ( temp->ShouldFall() && weight ) {
			temp->SetNotFalling();
			if ( z > 0 &&
					weight > Weight(x%SHRED_WIDTH, y%SHRED_WIDTH, z-1) )
				world->Move(x, y, z, DOWN);
			else if ( z < HEIGHT-1 &&
					weight < Weight(x%SHRED_WIDTH, y%SHRED_WIDTH, z+1) )
				world->Move(x, y, z, UP);
		}
		temp->Act();
	}
}

Block * Shred::BlockFromFile(
		QDataStream & str,
		ushort i,
		ushort j,
		const ushort k)
{
	quint16 kind, sub;
	bool normal;
	str >> kind >> sub >> normal;
	if ( normal ) {
		return NewNormal(sub);
	}

	//if block is loaded into inventory, do not register it.
	Shred * const shred_reg=( k==HEIGHT ) ? 0 : this;
	i+=shredX*SHRED_WIDTH;
	j+=shredY*SHRED_WIDTH;

	//if some kind will not be listed here,
	//blocks of this kind just will not load,
	//unless kind is inherited from Inventory class or one
	//of its derivatives - in this case this may cause something bad.
	switch ( kind ) {
		case BLOCK:  return new Block (str, sub);
		case PICK:   return new Pick  (str, sub);
		case PLATE:  return new Plate (str, sub);
		case LADDER: return new Ladder(str, sub);
		case WEAPON: return new Weapon(str, sub);

		case BUSH:   return new Bush (shred_reg, str);
		case CHEST:  return new Chest(shred_reg, str, sub);
		case WORKBENCH: return new Workbench(shred_reg, str, sub);

		case RABBIT: return new Rabbit(shred_reg, i, j, k, str);
		case DWARF:  return new Dwarf (shred_reg, i, j, k, str);
		case PILE:   return new Pile  (shred_reg, i, j, k, str);
		case GRASS:  return new Grass (shred_reg, i, j, k, str);
		case ACTIVE: return new Active(shred_reg, i, j, k, str, sub);
		case LIQUID: return new Liquid(shred_reg, i, j, k, str, sub);
		case DOOR:   return new Door  (shred_reg, i, j, k, str, sub);

		case CLOCK:  return new Clock(str, world, sub);
		default:
			fprintf(stderr,
				"Shred::BlockFromFile: unlisted kind: %d, x: %hu, y: %hu, z: %hu.\n",
				kind, i, j, k);
			return NewNormal(sub);
	}
}

int Shred::Sub(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return blocks[x][y][z]->Sub();
}
int Shred::Kind(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return blocks[x][y][z]->Kind();
}
int Shred::Durability(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return blocks[x][y][z]->Durability();
}
int Shred::Movable(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return blocks[x][y][z]->Movable();
}
int Shred::Transparent(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return blocks[x][y][z]->Transparent();
}
float Shred::Weight(
		const ushort x,
		const ushort y,
		const ushort z) const
{
	return blocks[x][y][z]->Weight();
}

void Shred::AddActive(Active * const active) {
	activeList.append(active);
}

bool Shred::RemActive(Active * const active) {
	return activeList.removeOne(active);
}

void Shred::ReloadToNorth() {
	for (ushort i=0; i<activeList.size(); ++i)
		activeList[i]->ReloadToNorth();
	++shredY;
}
void Shred::ReloadToEast() {
	for (ushort i=0; i<activeList.size(); ++i)
		activeList[i]->ReloadToEast();
	--shredX;
}
void Shred::ReloadToSouth() {
	for (ushort i=0; i<activeList.size(); ++i)
		activeList[i]->ReloadToSouth();
	--shredY;
}
void Shred::ReloadToWest() {
	for (ushort i=0; i<activeList.size(); ++i)
		activeList[i]->ReloadToWest();
	++shredX;
}

Block * Shred::GetBlock(
		const ushort x,
		const ushort y,
		const ushort z) const {
	//fprintf(stderr, "Shred::GetBlock::x: %hu, y: %hu, z:%hu\n", x, y, z);
	return blocks[x][y][z];
}
void Shred::SetBlock(Block * block,
		const ushort x,
		const ushort y,
		const ushort z)
{
	blocks[x][y][z]=block;
}

QString Shred::FileName() const {
	QString str;
	world->WorldName(str);
	return str=str+"_shreds/y"+
		QString::number(longitude)+"x"+
		QString::number(latitude);
}

char Shred::TypeOfShred(
		const long longi,
		const long lati) const
{
	const long mapSize=world->MapSize();
	if (
			longi >= mapSize || longi < 0 ||
			lati  >= mapSize || lati  < 0 )
		return '.';

	QString temp;
	QFile map(world->WorldName(temp));
	if ( !map.open(QIODevice::ReadOnly | QIODevice::Text) )
		return '.';

	map.seek((mapSize+1)*longi+lati); //+1 is for '\n' in file
	char c;
	return ( map.getChar(&c) ) ? c : '.';
}

//shred generators section
//these functions fill space between the lowest nullstone layer and sky. so use k from 1 to heigth-2.
void Shred::NormalUnderground(const ushort depth=0) {
	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j) {
		ushort k;
		for (k=1; k<HEIGHT/2-6 && k<HEIGHT/2-depth-1; ++k)
			blocks[i][j][k]=NewNormal(STONE);
		blocks[i][j][k]=NewNormal((rand()%2) ? STONE : SOIL);
		for (++k; k<HEIGHT/2-depth; ++k)
			blocks[i][j][k]=NewNormal(SOIL);
	}
}

void Shred::PlantGrass() {
	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j) {
		ushort k;
		for (k=HEIGHT-2; Transparent(i, j, k); --k);
		if ( SOIL==Sub(i, j, k++) && AIR==Sub(i, j, k) )
			blocks[i][j][k]=new Grass(this,
				i+shredX*SHRED_WIDTH,
				j+shredY*SHRED_WIDTH, k);
	}
}

void Shred::TestShred() {
	NormalUnderground();

	//row 1
	SetNewBlock(CLOCK, IRON, 1, 1, HEIGHT/2);
	SetNewBlock(CHEST, WOOD, 3, 1, HEIGHT/2);
	SetNewBlock(ACTIVE, SAND, 5, 1, HEIGHT/2);
	SetNewBlock(BLOCK, GLASS, 7, 1, HEIGHT/2);
	SetNewBlock(PILE, DIFFERENT, 9, 1, HEIGHT/2);
	SetNewBlock(PLATE, STONE, 11, 1, HEIGHT/2);
	SetNewBlock(BLOCK, NULLSTONE, 13, 1, HEIGHT/2);

	//row 2
	SetNewBlock(LADDER, NULLSTONE, 1, 3, HEIGHT/2);
	SetNewBlock(LADDER, WOOD, 1, 3, HEIGHT/2+1);
	SetNewBlock(DWARF, H_MEAT, 3, 3, HEIGHT/2);
	SetNewBlock(LIQUID, WATER, 5, 3, HEIGHT/2-3);
	SetNewBlock(LIQUID, WATER, 5, 3, HEIGHT/2-3);
	SetNewBlock(LIQUID, WATER, 5, 3, HEIGHT/2-2);
	SetNewBlock(BLOCK, AIR, 5, 3, HEIGHT/2-1);
	SetNewBlock(BUSH, GREENERY, 7, 3, HEIGHT/2);
	SetNewBlock(RABBIT, A_MEAT, 9, 3, HEIGHT/2-2);
	SetNewBlock(BLOCK, AIR, 9, 3, HEIGHT/2-1);
	SetNewBlock(WORKBENCH, IRON, 11, 3, HEIGHT/2);
	SetNewBlock(DOOR, GLASS, 13, 3, HEIGHT/2);
	blocks[13][3][HEIGHT/2]->SetDir(NORTH);

	//row 3
	SetNewBlock(WEAPON, IRON, 1, 5, HEIGHT/2);

	//suicide booth
	for (ushort i=1; i<4; ++i)
	for (ushort j=7; j<10; ++j)
	for (ushort k=HEIGHT/2; k<HEIGHT/2+5; ++k)
		SetNewBlock(BLOCK, GLASS, i, j, k);
	SetNewBlock(RABBIT, A_MEAT, 2, 8, HEIGHT/2);
}

void Shred::NullMountain() {
	ushort i, j, k;
	for (i=0; i<SHRED_WIDTH; ++i)
	for (j=0; j<SHRED_WIDTH; ++j) {
		for (k=1; k<HEIGHT/2; ++k)
			blocks[i][j][k]=NewNormal( (i==4 ||
			                            i==5 ||
			                            j==4 ||
			                            j==+5) ?
					NULLSTONE : STONE );

		for ( ; k<HEIGHT-1; ++k)
			if (i==4 || i==5 || j==4 || j==5)
				blocks[i][j][k]=NewNormal(NULLSTONE);
	}
}

void Shred::Plain() {
	NormalUnderground();
	ushort i, num, x, y;

	//bush
	num=rand()%4;
	for (i=0; i<=num; ++i) {
		x=rand()%SHRED_WIDTH;
		y=rand()%SHRED_WIDTH;
		if ( AIR==Sub(x, y, HEIGHT/2) )
			blocks[x][y][HEIGHT/2]=new Bush(this);
	}

	//rabbits
	num=rand()%4;
	for (i=0; i<=num; ++i) {
		x=rand()%SHRED_WIDTH;
		y=rand()%SHRED_WIDTH;
		if ( AIR==Sub(x, y, HEIGHT/2) )
			blocks[x][y][HEIGHT/2]=new Rabbit(this,
				shredX*SHRED_WIDTH+x,
				shredY*SHRED_WIDTH+y, HEIGHT/2);
	}

	PlantGrass();
}

void Shred::Forest(const long longi, const long lati) {
	NormalUnderground();

	long i, j;
	ushort number_of_trees=0;
	for (i=longi-1; i<=longi+1; ++i)
	for (j=lati-1;  j<=lati+1;  ++j)
		if ( '%'==TypeOfShred(i, j) )
			++number_of_trees;

	for (i=0; i<number_of_trees; ++i) {
		short x=rand()%(SHRED_WIDTH-2),
		      y=rand()%(SHRED_WIDTH-2);
		Tree(x, y, HEIGHT/2, 4+rand()%5);
	}

	PlantGrass();
}

void Shred::Water(const long longi, const long lati) {
	ushort depth=1;
	char map[3][3];
	for (long i=longi-1; i<=longi+1; ++i)
	for (long j=lati-1;  j<=lati+1;  ++j)
		if ( '~'==(map[i-longi+1][j-lati+1]=TypeOfShred(i, j)) )
			++depth;

	NormalUnderground(depth);
	ushort i, j, k;

	if ('~'!=map[1][0] && '~'!=map[0][1]) { //north-west rounding
		for (i=0; i<SHRED_WIDTH/2; ++i)
		for (j=0; j<SHRED_WIDTH/2; ++j)
			for (k=HEIGHT/2-depth; k<HEIGHT/2; ++k)
				if ( ((7-i)*(7-i) +
				      (7-j)*(7-j) +
				      (HEIGHT/2-k)*(HEIGHT/2-k)*16/depth/depth)
						> 49 )
					blocks[i][j][k]=NewNormal(SOIL);
	}
	if ('~'!=map[1][0] && '~'!=map[2][1]) { //south-west rounding
		for (i=0; i<SHRED_WIDTH/2; ++i)
		for (j=SHRED_WIDTH/2; j<SHRED_WIDTH; ++j)
			for (k=HEIGHT/2-depth; k<HEIGHT/2; ++k)
				if ( ((7-i)*(7-i)+
				      (8-j)*(8-j)+
				      (HEIGHT/2-k)*(HEIGHT/2-k)*16/depth/depth)
						> 49 )
					blocks[i][j][k]=NewNormal(SOIL);
	}
	if ('~'!=map[2][1] && '~'!=map[1][2]) { //south-east rounding
		for (i=SHRED_WIDTH/2; i<SHRED_WIDTH; ++i)
		for (j=SHRED_WIDTH/2; j<SHRED_WIDTH; ++j)
			for (k=HEIGHT/2-depth; k<HEIGHT/2; ++k)
				if ( ((8-i)*(8-i)+
				      (8-j)*(8-j)+
				      (HEIGHT/2-k)*(HEIGHT/2-k)*16/depth/depth)
						> 49 )
					blocks[i][j][k]=NewNormal(SOIL);
	}
	if ('~'!=map[1][2] && '~'!=map[0][1]) { //north-east rounding
		for (i=SHRED_WIDTH/2; i<SHRED_WIDTH; ++i)
		for (j=0; j<SHRED_WIDTH/2; ++j)
			for (k=HEIGHT/2-depth; k<HEIGHT/2; ++k)
				if ( ((8-i)*(8-i)+
				      (7-j)*(7-j)+
				      (HEIGHT/2-k)*(HEIGHT/2-k)*16/depth/depth)
						> 49 )
					blocks[i][j][k]=NewNormal(SOIL);
	}
	for (i=0; i<SHRED_WIDTH; ++i)
	for (j=0; j<SHRED_WIDTH; ++j)
	for (k=HEIGHT/2-depth; k<HEIGHT/2; ++k)
		if ( AIR==Sub(i, j, k) )
			blocks[i][j][k]=new Liquid(this,
			                i+SHRED_WIDTH*shredX,
			                j+SHRED_WIDTH*shredY, k);

	PlantGrass();
}

void Shred::Hill(const long longi, const long lati) {
	ushort hill_height=1;
	for (long i=longi-1; i<=longi+1; ++i)
	for (long j=lati-1;  j<=lati+1;  ++j)
		if ( '+'==TypeOfShred(i, j) )
			++hill_height;

	NormalUnderground();

	for (ushort i=0; i<SHRED_WIDTH; ++i)
	for (ushort j=0; j<SHRED_WIDTH; ++j)
	for (ushort k=HEIGHT/2; k<HEIGHT/2+hill_height; ++k)
		if (((4.5-i)*(4.5-i)+
		     (4.5-j)*(4.5-j)+
		     (HEIGHT/2-0.5-k)*(HEIGHT/2-0.5-k)*16/hill_height/hill_height)<=16)
			blocks[i][j][k]=NewNormal(SOIL);

	PlantGrass();
}

void Shred::Pyramid()
{
	//pyramid by Panzerschrek
	//'p' - pyramid symbol
	NormalUnderground();
	unsigned short z, dz, x, y;

	//пирамида
	for( z= HEIGHT/2, dz= 0; dz< 8; z +=2, dz++ )
	{
		for( x= dz; x< ( 16 - dz ); x++ )
		{
			blocks[x][dz][z]=
			blocks[x][15 - dz][z]=
			blocks[x][dz][z+1]=
			blocks[x][15 - dz][z+1]=NewNormal(STONE);
		}
		for( y= dz; y< ( 16 - dz ); y++ )
		{
			blocks[dz][y][z]=
			blocks[15 - dz][y][z]=
			blocks[dz][y][z + 1]=
			blocks[15 - dz][y][z + 1]=NewNormal(STONE);
		}
	}

	//вход
	blocks[SHRED_WIDTH/2][0][HEIGHT/2]= NewNormal( AIR );

	//камера внутри
	for( z= HEIGHT/2 - 60, dz=0; dz< 8; dz++, z++ )
	for( x= 1; x< SHRED_WIDTH - 1; x++ )
	for( y= 1; y< SHRED_WIDTH - 1; y++ )
		blocks[x][y][z]= NewNormal( AIR );

	//шахта
	for( z= HEIGHT/2 - 52, dz= 0; dz< 52; z++, dz++ )
		blocks[SHRED_WIDTH/2][SHRED_WIDTH/2][z]= NewNormal( AIR );

	//летающая тарелка
	return;
	for( x=0; x< SHRED_WIDTH; x++ )
	for( y=0; y< SHRED_WIDTH; y++ )
	{
		float r= float( ( x - SHRED_WIDTH/2 ) * ( x - SHRED_WIDTH/2 ) )
		 + float( ( y - SHRED_WIDTH/2 ) * ( y - SHRED_WIDTH/2 ) );
		if( r < 64.0f )
			blocks[x][y][ 124 ]= NewNormal( STONE );
		if( r < 36.0f )
			blocks[x][y][ 125 ]= NewNormal( STONE );
	}
}

void Shred::Mountain() {
	//Доступные координаты:
	//x, y - от 0 до SHRED_WIDTH-1 включительно
	//k - от 1 до HEIGHT-2 включительно

	//заполнить нижнюю часть лоскута камнем и землёй
	NormalUnderground();

	//ushort == unsigned short
	//столб из камня, y=3, x=3
	//вид сверху:
	//*---->x
	//|
	//| +
	//|
	//v y
	//SetNewBlock устанавливает новый блок. Первый параметр - тип,
	//второй - вещество, потом координаты x, y, z;
	for(ushort k=HEIGHT/2; k<3*HEIGHT/4; ++k) {
		SetNewBlock(BLOCK, STONE, 2, 2, k);
	}

	//стена из нуль-камня с запада на восток высотой 3 блока
	for(ushort i=4; i<10; ++i)
	for(ushort k=HEIGHT/2; k<HEIGHT/2+3; ++k) {
		SetNewBlock(BLOCK, NULLSTONE, i, 2, k);
	}
	//в стене вырезать дырку (AIR - воздух):
	SetNewBlock(BLOCK, AIR, 6, 2, HEIGHT/2+1);

	//блок из дерева появится с вероятностью 1/2.
	//Если random()%3 - с вероятностью 2/3 и т.д.
	if ( random()%2 ) {
		SetNewBlock(BLOCK, WOOD, 7, 7, HEIGHT/2);
	}
}

bool Shred::Tree(
		const ushort x,
		const ushort y,
		const ushort z,
		const ushort height)
{
	if ( SHRED_WIDTH<=x+2 ||
			SHRED_WIDTH<=y+2 ||
			HEIGHT-1<=z+height ||
			height<2 )
		return false;

	ushort i, j, k;
	for (i=x; i<=x+2; ++i)
	for (j=y; j<=y+2; ++j)
	for (k=z; k<z+height; ++k)
		if ( AIR!=Sub(i, j, k) )
			return false;

	for (k=z; k<z+height-1; ++k) //trunk
		blocks[x+1][y+1][k]=NewNormal(WOOD);

	for (i=x; i<=x+2; ++i) //leaves
	for (j=y; j<=y+2; ++j)
	for (k=z+height/2; k<z+height; ++k)
		if ( AIR==Sub(i, j, k) )
			blocks[i][j][k]=NewNormal(GREENERY);

	return true;
}
