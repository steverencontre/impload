/*
	impload - simple gphoto2-based camera file importer

	Copyright (c) 2011-24 Steve Rencontre	q.impload@rsn-tech.co.uk
	All rights reserved

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtWidgets/QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "MainWindow.h"


int main (int argc, char *argv[])
  {
	QApplication a (argc, argv);
	a.setOrganizationName ("RSN Technology");
	a.setApplicationName ("impload");

	QCommandLineParser parser;
	parser.setApplicationDescription ("Import images from camera or file folder");
	parser.addHelpOption();
	parser.addVersionOption();

	parser.addOptions
	(
		{
			{{"f", "folder"}, "Use folder source, not camera"},
			{{"t", "timeshift"}, "Add hours to EXIF timestamp (may be fractional or negative)", "timeshift"}
		}
	);

	parser.process(a);

	auto list {parser.positionalArguments()};
	std::string start = list.empty() ? std::string {""} : list[0].toStdString();

	double timeshift = parser.isSet ("timeshift") ? parser.value("timeshift").toDouble() : -999;

	MainWindow w {parser.isSet ("f"), start, timeshift};

	if (!w.GotValidSource())
		return -1;

	w.show();

	return a.exec();
  }
