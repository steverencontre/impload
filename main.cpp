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
#include <QTimeZone>

#include "MainWindow.h"

#include <iostream>

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
	({
		{{"t", "timeshift"}, "Add hours to EXIF timestamp (may be fractional or negative)", "timeshift"},
		{{"s", "since"}, "Only add photos from this date/time (ISO format, UTC or 0=today, -N since N days ago) onwards", "since"}
	});

	parser.process(a);

	auto list {parser.positionalArguments()};
	std::string folder = list.empty() ? std::string {""} : list[0].toStdString();

	double timeshift = parser.isSet ("timeshift") ? parser.value ("timeshift").toDouble() : 0;
	time_t since {0};

	if (parser.isSet ("since"))
	{
		QString t {parser.value ("since")};

		QDateTime s;

		int n = t.toInt();
		if (n <= 0)
			s = QDateTime {QDate::currentDate().addDays (n), {}};
		else
			s = QDateTime::fromString (t, Qt::ISODate);

		s.setTimeZone (QTimeZone::utc());
		since = s.toSecsSinceEpoch();

		std::cout << "since " << s.toString(Qt::ISODate).toStdString() << std::endl;
	}

	MainWindow w {folder, timeshift, since};
	if (!w.GotValidSource())
		return -1;

	w.show();

	return a.exec();
}
