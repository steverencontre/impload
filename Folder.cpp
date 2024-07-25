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
#include <stdexcept>
#include <filesystem>
#include <iostream>

#include <QFileDialog>


#include <fcntl.h>

#include "Folder.h"
#include "Metadata.h"



Folder::Folder (const std::string& start)
{
	m_BaseDir = QFileDialog::getExistingDirectory (nullptr, "Import images from", QString::fromStdString (start)).toStdString();

	if (m_BaseDir.empty())
		throw std::runtime_error ("no folder selected");
}


Folder::~Folder()
{

}


/*
	AddFiles
*/

void	Folder::AddFiles (const std::string& base)
{
	std::filesystem::directory_iterator dirit {base};

	for (const auto& file : dirit)
	{
		if (file.is_directory())
			AddFiles (file.path().string());
		else
		{
			auto name {file.path().filename().string()};
			auto ext {file.path().extension().string()};
			if
			(
				name [0] != '.' &&
				(ext == ".JPG" || ext == ".jpg" || ext == ".JPEG" || ext == ".jpeg")
			)
				m_Files.emplace_back (FileListItem {base, file.path().filename().string()});
		}
	}
}


/*
	LoadData
*/

ImageSource::ImageData Folder::LoadData (const std::string& folder, const std::string& name, DataType type)
{
	m_CurrentFile = folder + "/" + name;

	int fd = open (m_CurrentFile.c_str(), O_RDONLY);
	size_t size = lseek (fd, 0, SEEK_END);
	lseek (fd, 0, SEEK_SET);

	m_SharedBuffer.resize (size);
	read (fd, m_SharedBuffer.data(), size);		// inefficient if we just want thumbnail, but leave for future enhancement
	close (fd);

	Metadata m {m_SharedBuffer.data(), size, type == VIDEO};


	QDateTime dt {m.Timestamp()};
//	if (type == THUMB)

	return {m_SharedBuffer.data(), size, dt};
}


/*
	WriteImageFile
*/

bool Folder::WriteImageFile(const std::string& destname)
{
	return std::filesystem::copy_file (m_CurrentFile, destname);
}



