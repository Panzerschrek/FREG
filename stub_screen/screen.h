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

//this file provides curses (text-based graphics interface) screen for freg.
//screen.cpp provides definitions for methods.

#ifndef SCREEN_H
#define SCREEN_H

#define NOX

#include "VirtScreen.h"

enum actions {
	USE,
	THROW,
	OBTAIN, 
	WIELD,
	INSCRIBE,
	EAT,
	BUILD,
	CRAFT,
	TAKEOFF,
};

class IThread;

class Screen : public VirtScreen {
	Q_OBJECT

	IThread * input;
	bool cleaned;
	FILE * notifyLog; //весь текст уведомлений (notification) дублируется в файл.
	int actionMode;
	
	char CharName(
			const ushort,
			const ushort,
			const ushort) const;
	char CharName(const int, const int) const;

	private slots:
	void Print() {}

	public slots:
	void Notify(const QString &);
	void CleanAll();
	void PassString(QString &) const;
	void Update(
			const ushort,
			const ushort,
			const ushort)
	{}
	void UpdateAll() {}
	void UpdatePlayer() {}
	void UpdateAround(
			const ushort,
			const ushort,
			const ushort,
			const ushort)
	{}
	void Move(const int) {}

	public:
	void ControlPlayer(const int);
	Screen(World * const, Player * const);
};

/** \class IThread screen.h
 * \brief Keyboard input thread for curses screen for freg.
 *
 * This class is thread, with IThread::run containing input loop.
 */

#include <QThread>

class IThread : public QThread {
	Q_OBJECT

	Screen * const screen;

	public:
		IThread(Screen * const);
		void Stop();

	protected:
		void run();
	
	private:
		volatile bool stopped;
};

#endif
