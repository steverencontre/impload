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

#ifndef IMAGESOURCE_H
#define IMAGESOURCE_H

#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <tuple>

#include <QDateTime>

class ImageSource
{
public:
	ImageSource();
	virtual ~ImageSource() {}

	enum DataType { FULL, THUMB, EXIF, VIDEO, DATA_TYPES };

	struct FileListItem
	  {
		std::string		folder;
		std::string		name;

		bool operator< (const FileListItem& x) const { return name < x.name; }
	  };

	using ImageData = std::tuple<const uint8_t *, size_t, QDateTime>;

	const std::vector <FileListItem>& Files() const		  { return m_Files; }

	double	TimeOffset() const		{ return m_TimeOffset; }	// note that API allows for fractions of seconds
	void		TimeOffset (double t)	{ m_TimeOffset = t; }		// allow manual override
	ImageData		LoadData (unsigned index, DataType type);
	bool				SaveFile (unsigned index, const std::string& tag, const std::string& path, unsigned absnum);

	void ScanFiles()
	{
		AddFiles (m_BaseDir);
/*
		for (const auto& item : m_Files)
			std::cout << item.folder << " : " << item.name << std::endl;
*/
	}

protected:

	virtual void			AddFiles (const std::string& base) = 0;
	virtual ImageData		LoadData (const std::string& folder, const std::string& name, DataType type) = 0;

	std::string					m_BaseDir;
	std::vector <FileListItem>		m_Files;
	time_t						m_TimeOffset {0};
};
#endif // IMAGESOURCE_H
