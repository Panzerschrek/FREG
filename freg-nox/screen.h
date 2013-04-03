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

#include "VirtScreen.h"
#include <curses.h>

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

	char CharName(
			const ushort,
			const ushort,
			const ushort) const;
	char CharName(const int, const int) const;
	char CharNumber(
			const ushort,
			const ushort,
			const ushort) const;
	char CharNumberFront(
			const ushort,
			const ushort) const;
	void Arrows(
			WINDOW * const & window,
			const ushort x,
			const ushort y) const
	{
		wcolor_set(window, WHITE_RED, NULL);
		mvwaddstr(window, 0, x, "vv");
		mvwaddstr(window, SCREEN_SIZE+1, x, "^^");
		HorizontalArrows(window, y);
	}
	void HorizontalArrows(
			WINDOW * const & window,
			const ushort y,
			const short color=WHITE_RED) const
	{
		wcolor_set(window, color, NULL);
		mvwaddch(window, y, 0, '>');
		mvwaddch(window, y, SCREEN_SIZE*2+1, '<');
	}
	void ActionXyz(
			ushort & x,
			ushort & y,
			ushort & z) const;

	void PrintNormal(WINDOW * const) const;
	void PrintFront(WINDOW * const) const;
	void PrintInv(WINDOW * const, Inventory * const) const;
	void RePrint() {
		clear();
		updated=false;
	}

	color_pairs Color(
			const int kind,
			const int sub) const; //пара цветов текст_фон в зависимоти от типа (kind) и вещества (sub) блока.
	color_pairs Color(
			const ushort,
			const ushort,
			const ushort) const;

	private slots:
	void Print();

	public slots:
	void Notify(const QString &);
	void CleanAll();
	QString & PassString(QString &) const;
	void Update(
			const ushort,
			const ushort,
			const ushort)
	{
		updated=false;
	}
	void UpdateAll() { updated=false; }
	void UpdatePlayer() { updated=false; }
	void UpdateAround(
			const ushort,
			const ushort,
			const ushort,
			const ushort)
	{
		updated=false;
	}
	void Move(const int) { updated=false; }

	public:
	void ControlPlayer(const int);
	Screen(World * const, Player * const);
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
