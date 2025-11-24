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


#ifndef FOLDER_H
#define FOLDER_H

#include <string>
#include <vector>

#include "ImageSource.h"


class Folder : public ImageSource
{
public:
	Folder (const std::string& start);
	~Folder() override;


  private:
	void									AddFiles (const std::string& base) override;
	ImageSource::ImageData	LoadData (const std::string& folder, const std::string& name, DataType type) override;

	std::vector<unsigned char>	m_SharedBuffer;
	std::string								m_CurrentFile;
};

#endif // FOLDER_H
