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
		auto ext {file.path().extension().string()};
		if (ext == ".JPG" || ext == ".jpg" || ext == ".JPEG" || ext == ".jpeg"	)
			m_Files.emplace_back (FileListItem {base, file.path().filename().string()});
	}
	std::sort (m_Files.begin(), m_Files.end());
}


/*
	LoadData
*/

ImageSource::ImageData Folder::LoadData (const std::string& folder, const std::string& name, DataType type)
{
#if 0
	m_CurrentFile = folder + "/" + name;
	auto fio {new Exiv2::FileIo {m_CurrentFile}};

	fio->open();

	Exiv2::JpegImage image {Exiv2::FileIo::UniquePtr {fio}, false};

	image.readMetadata();
	auto ed {image.exifData()};
	auto dt {Metadata::Timestamp (ed)};
	size_t size {0};

	if (type == THUMB)
	{
		Exiv2::ExifThumbC thumb {ed};

		auto databuf {thumb.copy()};

		size = databuf.size();
		m_SharedBuffer.reserve (size);
		memcpy (m_SharedBuffer.data(), databuf.data(), size);
	}
#else
	size_t size {0};
	QDateTime dt;
#endif
	return {m_SharedBuffer.data(), size, dt};		// note, we only return real data for thumbnail, because full file will be copied directly
}


/*
	WriteImageFile
*/

bool Folder::WriteImageFile(const std::string& destname)
{
	return std::filesystem::copy_file (m_CurrentFile, destname);
}



