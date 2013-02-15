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

#ifndef WORLD_H
#define WORLD_H

#include <cmath>
#include <QReadWriteLock>
#include <QThread>
#include "header.h"

class Block;
class Dwarf;
class Inventory;
class Active;
class Shred;

typedef struct {
	ushort num;
	int kind;
	int sub;
} craft_item;

class World : public QThread {
	Q_OBJECT

	ulong time;
	Shred ** shreds;
	Block * normal_blocks[AIR+1];
	ulong longitude, latitude; //center of active zone
	ulong spawnLongi, spawnLati;
	const QString worldName;
	ushort numShreds; //size of active zone
	ushort numActiveShreds;
	QReadWriteLock rwLock;

	bool cleaned;
	ushort sun_moon_x;
	bool ifStar;

	ulong mapSize;

	typedef QList<craft_item *> craft_recipe;
	QList<craft_recipe *> recipes;

	void LoadRecipes();
	void CleanRecipes();

	Block * NewNormal(const int sub) const {
		return normal_blocks[sub];
	}
	void ReplaceWithNormal(
			const ushort x,
			const ushort y,
			const ushort z)
	{
		SetBlock(ReplaceWithNormal(GetBlock(x, y, z)),
			x, y, z);
	}
	Block * ReplaceWithNormal(Block * const block);
	void MakeSun();
	void RemSun() {
		SetBlock(NewNormal(ifStar ? STAR : SKY),
			sun_moon_x,
			shred_width*numShreds/2,
			height-1);
	}

	protected:
	void run();

	//block work section
	public:
	Block * GetBlock(
			const ushort,
			const ushort,
			const ushort) const;
	Shred * GetShred(
			const ushort i,
			const ushort j) const
	{
		return shreds[j/shred_width*numShreds+
		              i/shred_width];
	}	
	void SetBlock(Block *,
			const ushort,
			const ushort,
			const ushort);

	//lighting section
	public:
	uchar Enlightened(
			const ushort,
			const ushort,
			const ushort) const;
	uchar Enlightened(
			const ushort,
			const ushort,
			const ushort,
			const int dir) const;
	uchar SunLight(
			const ushort,
			const ushort,
			const ushort) const;
	uchar FireLight(
			const ushort,
			const ushort,
			const ushort) const;

	private:
	uchar LightMap(
			const ushort,
			const ushort,
			const ushort) const;
	bool SetLightMap(
			const ushort level,
			const ushort,
			const ushort,
			const ushort);

	void ReEnlighten(
			const ushort i,
			const ushort j,
			const ushort k);
	void ReEnlightenAll();
	void ReEnlightenTime();
	///Called from World::ReloadShreds(int), enlighens only need shreds.
	void ReEnlightenMove(const int direction);

	void SunShine(
			const ushort i,
			const ushort j);
	void Shine(
			const ushort,
			const ushort,
			const ushort,
			const ushort level,
			const bool init=false);

	//information section
	public:
	QString & WorldName(QString & str) const {
		return str=worldName;
	}
	int Focus(const ushort,
	          const ushort,
	          const ushort,
	          ushort &,
	          ushort &,
	          ushort &,
		  const int dir) const;
	int Focus(const ushort,
	          const ushort,
	          const ushort,
	          ushort &,
	          ushort &,
	          ushort &) const;
	ushort NumShreds() const { return numShreds; }
	int TurnRight(const int dir) const;
	int TurnLeft(const int dir) const;
	int Anti(const int dir) const;
	ulong GetSpawnLongi() const { return spawnLongi; }
	ulong GetSpawnLati()  const { return spawnLati; }
	
	private:
	ulong MapSize() const { return mapSize; }
	ushort SunMoonX() const {
		return ( NIGHT==PartOfDay() ) ?
			TimeOfDay()*shred_width*numShreds/
				seconds_in_night :
			(TimeOfDay()-seconds_in_night)*shred_width*numShreds/
				seconds_in_daylight;
	}
	int MakeDir(
			const ushort,
			const ushort,
			const ushort,
			const ushort) const;
	float Distance(
			const ushort x_from,
			const ushort y_from,
			const ushort z_from,
	                const ushort x_to,
			const ushort y_to,
			const ushort z_to) const
	{
		return sqrt( float((x_from-x_to)*(x_from-x_to)+
		                   (y_from-y_to)*(y_from-y_to)+
		                   (z_from-z_to)*(z_from-z_to)) );
	}

	//visibility section
	public:
	bool DirectlyVisible(
			float,
			float,
			float,
			const ushort,
			const ushort,
			const ushort) const;
	bool Visible(
			const ushort,
			const ushort,
			const ushort,
			const ushort,
			const ushort,
			const ushort) const;

	//movement section
	public:
	int  Move(
			const ushort,
			const ushort,
			const ushort,
			const int dir,
			const ushort=2); //how much block fall/rise at one turn
	void Jump(
			const ushort,
			const ushort,
			ushort);

	//time section
	public:
	times_of_day PartOfDay() const {
		ushort time_day=TimeOfDay();
		if (time_day<end_of_night)   return NIGHT;
		if (time_day<end_of_morning) return MORNING;
		if (time_day<end_of_noon)    return NOON;
		return EVENING;
	}
	int TimeOfDay() const { return time%seconds_in_day; }
	ulong Time() const { return time; }

	//interactions section
	public:
	bool Damage(
			const ushort,
			const ushort,
			const ushort,
			const ushort=1,
			const int=CRUSH);
	int Use(
			const ushort,
			const ushort,
			const ushort);
	int Build(
			Block * const,
			const ushort,
			const ushort,
			const ushort);
	bool Inscribe(
			const ushort,
			const ushort,
			const ushort);
	void Eat(
			const ushort i,
			const ushort j,
			const ushort k,
			const ushort i_food,
			const ushort j_food,
			const ushort k_food);

	//inventory functions section
	private:
	bool Exchange(
			const ushort i_from,
			const ushort j_from,
			const ushort k_from,
			const ushort i_to,
			const ushort j_to,
			const ushort k_to,
			const ushort n);
	bool ExchangeAll(
			const ushort x_from,
			const ushort y_from,
			const ushort z_from,
			const ushort x_to,
			const ushort y_to,
			const ushort z_to);
	public:
	bool Drop(
			const ushort i,
			const ushort j,
			const ushort k,
			const ushort n)
	{
		ushort i_to, j_to, k_to;
		return ( Focus(i, j, k, i_to, j_to, k_to) ) ? false :
			Exchange(i, j, k, i_to, j_to, k_to, n);
	}
	bool Get(
			const ushort i,
			const ushort j,
			const ushort k,
			const ushort n)
	{
		ushort i_from, j_from, k_from;
		return ( Focus(i, j, k, i_from, j_from, k_from) ) ? false :
			Exchange(i_from, j_from, k_from, i, j, k, n);
	}
	bool DropAll(
			const ushort i_from,
			const ushort j_from,
			const ushort k_from)
	{
		ushort i, j, k;
		if ( Focus(i_from, j_from, k_from, i, j, k) )
			return false;
		return ExchangeAll(i_from, j_from, k_from, i, j, k);
	}
	bool GetAll(
			const ushort i_to,
			const ushort j_to,
			const ushort k_to)
	{
		ushort i, j, k;
		if ( Focus(i_to, j_to, k_to, i, j, k) )
			return false;
		return ExchangeAll(i, j, k, i_to, j_to, k_to);
	}

	//block information section
	public:
	bool InBounds(
			const ushort i,
			const ushort j,
			const ushort k=0) const
	{
		const ushort max_x_y=shred_width*numShreds;
		return (i<max_x_y && j<max_x_y && k<height);
	}
	QString & FullName(QString &,
			const ushort,
			const ushort,
			const ushort) const;
	int Transparent(
			const ushort,
			const ushort,
			const ushort) const;
	int Durability(
			const ushort,
			const ushort,
			const ushort) const;
	int Kind(
			const ushort,
			const ushort,
			const ushort) const;
	int Sub(
			const ushort,
			const ushort,
			const ushort) const;
	int Movable(
			const ushort,
			const ushort,
			const ushort) const;
	float Weight(
			const ushort,
			const ushort,
			const ushort) const;
	uchar LightRadius(
			const ushort x,
			const ushort y,
			const ushort z) const;
	Inventory * HasInventory(
			const ushort,
			const ushort,
			const ushort k) const;	

	Active * ActiveBlock(
			const ushort,
			const ushort,
			const ushort) const;

	QString & GetNote(QString &,
			const ushort,
			const ushort,
			const ushort) const;
	int Temperature(
			const ushort,
			const ushort,
			const ushort) const;
	bool Equal(const Block * const, const Block * const) const;
	char MakeSound(
			const ushort,
			const ushort,
		       	const ushort) const;

	//craft section
	bool MiniCraft(craft_item & item, craft_item & result) {
		craft_recipe recipe;
		recipe.append(&item);
		return Craft(recipe, result);
	}
	bool Craft(const craft_recipe & recipe, craft_item & result);

	private:
	friend class Active;
	friend class Shred;

	public:
	void WriteLock() { rwLock.lockForWrite(); }
	void ReadLock() { rwLock.lockForRead(); }
	bool TryReadLock() { return rwLock.tryLockForRead(); }
	void Unlock() { rwLock.unlock(); }

	void EmitNotify(const QString & str) const { emit Notify(str); }

	public:
	World(
			const QString &,
			const ushort,
			const ushort);
	~World();

	public slots:
	void CleanAll();
	void ReloadShreds(const int);	
	void PhysEvents();

	signals:
	void Notify(const QString &) const;
	void GetString(QString &) const;
	void Updated(
			const ushort,
			const ushort,
			const ushort);
	void UpdatedAll();
	void UpdatedAround(
			const ushort,
			const ushort,
			const ushort,
			const ushort level);
	///Emitted when world active zone moved to int direction.
	void Moved(const int);
	void ReConnect();
	///This is emitted when a pack of updates is complete.
	void UpdatesEnded();
};

#endif
