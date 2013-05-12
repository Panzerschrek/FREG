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
#define SCREEN_SIZE 30

#include <curses.h>
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
	TAKEOFF
}; //enum actions
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

class IThread;
class Inventory;
class QTimer;

class Screen : public VirtScreen {
	Q_OBJECT

	WINDOW * leftWin,
	       * rightWin,
	       * notifyWin,
	       * hudWin; //head-up display
	IThread * input;
	volatile bool updated;
	bool cleaned;
	QTimer * timer;
	FILE * notifyLog;
	int actionMode;
	short shiftFocus;

	QString command; //save previous command for further execution

	char CharName(int, int) const;
	char CharNumber(ushort x, ushort y, ushort z) const;
	char CharNumberFront(ushort x, ushort y) const;
	void Arrows(
			WINDOW * const & window,
			ushort x, ushort y) const;
	void HorizontalArrows(
			WINDOW * const & window,
			ushort y,
			short color=WHITE_RED) const;
	void ActionXyz(ushort & x, ushort & y, ushort & z) const;

	void PrintNormal(WINDOW *) const;
	void PrintFront(WINDOW *) const;
	void PrintInv(WINDOW *, Inventory *) const;
	void RePrint();

	color_pairs Color(int kind, int sub) const;
	void PrintBlock(ushort x, ushort y, ushort z, WINDOW *) const;

	private slots:
	void Print();

	public slots:
	void Notify(const QString &);
	void CleanAll();
	QString & PassString(QString &) const;
	void Update(ushort, ushort, ushort);
	void UpdateAll();
	void UpdatePlayer();
	void UpdateAround(ushort, ushort, ushort, ushort);
	void Move(int);
	void DeathScreen();

	public:
	void ControlPlayer(int);
	Screen(World *, Player *);
	~Screen();
}; //class screen

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
}; //class IThread

#endif
