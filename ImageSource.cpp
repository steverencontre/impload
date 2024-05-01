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

#include <iostream>
#include <cstring>
#include <cstdlib>

#include <sys/stat.h>

#include "Metadata.h"
#include "ImageSource.h"

ImageSource::ImageSource()
{

}



ImageSource::ImageData ImageSource::LoadData (unsigned index, DataType type)
  {
	if (index >= m_Files.size())
		return {nullptr, 0, {}};

	return LoadData (m_Files [index].folder,  m_Files [index].name, type);
  }




/*
	SaveFile
*/

bool ImageSource::SaveFile (unsigned index, const std::string& tag, const std::string& path, unsigned absnum)
  {
	if (index >= m_Files.size())
		return false;

	const char *folder = m_Files [index].folder.c_str();
	const char *name = m_Files [index].name.c_str();

	const char *extension = strrchr (name, '.');
	if (!extension)	  // shouldn't happen!
		extension = ".xxx";

	// get file info and data

	auto [ptr, size, dt] = LoadData (folder, name, strcmp (extension, ".MP4") == 0 ? VIDEO : FULL);

	if (ptr == nullptr)
		return false;

	dt = dt.addSecs (m_TimeOffset);

	if (size > 0)		// this means we've got real data and can modify the timestamp if needed
	{
		Metadata {ptr, size}.Timestamp (dt);
	}

	// make new name and create save folder tree if necessary

	std::ostringstream name_oss;
	name_oss << std::setfill ('0');

	name_oss << path;
	if (*path.rbegin() != '/')
		name_oss << "/";

	static int made_year_dir = 0;
	static int made_day_dir = 0;

	try
	  {
		auto year = dt.date().year();
		auto month = dt.date().month();
		auto day = dt.date().day();

		constexpr unsigned STDPERMS {S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH};

		name_oss << year;

		if (year != made_year_dir)
		  {
			std::string dir = name_oss.str();

			if (mkdir (dir.c_str(), STDPERMS) == 0)
				made_year_dir = year;
			else if (errno != EEXIST)
			  {
				std::cerr << "Unable to create directory " << dir << std::endl;
				throw 1;
			  }
		  }

		 name_oss << '/' << year << '-' << std::setw(2) << month << '-' << std::setw(2) << day;

		 int xday = year * 366 + month * 31 + day;		// use simple pseudo day number instead of full date comparison
		 if (xday != made_day_dir)
		  {
			std::string dir = name_oss.str ();

			if (mkdir (dir.c_str(), STDPERMS) == 0)
				made_day_dir = xday;
			else if (errno != EEXIST)
			  {
				std::cerr << "Unable to create directory " << dir << std::endl;
				throw 2;
			  }
		  }

		name_oss << '/';

		name_oss << dt.toString ("yyyyMMdd_hhmmss.zzz_").toStdString();
		if (!tag.empty())
			name_oss << tag << '_';
		name_oss << std::setw(6) << absnum << extension;
	  }
	catch (...)
	  {
		std::cerr << "Unable to create directory and/or new name (maybe can't convert EXIF datum to date or time value)\n";

		name_oss << name;
	  }

	// finally do the actual save

	return WriteImageFile (name_oss.str ());
  }

