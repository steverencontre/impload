/*
	impload - simple gphoto2-based camera file importer

	Copyright (c) 2011-12 Steve Rencontre	q.impload@rsn-tech.co.uk

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
#include <iomanip>
#include <stdexcept>
#include <cstdio>

#include <errno.h>

#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>

#include "Camera.h"


class GPResult
  {
  public:
	GPResult()											{}
	GPResult (int r)									{ Set (r); }
	const GPResult& operator= (int r)		{ Set (r); return *this; }
	operator int() const							{ return m_Value; }

  private:
	void Set (int r)
	  {
		if ((m_Value = r) < 0)
		  {
			const char *p = r < -99 ? gp::gp_result_as_string (r) :  gp::gp_port_result_as_string (r);
			fprintf (stderr, "%s\n", p);
		  }
	  }

	int	  m_Value;
  };


namespace {

void logf (gp::GPLogLevel, const char *dom, const char *fmt, va_list args, void *)
  {
	char buf [2048];
	const char *p = buf;
	vsprintf (buf, fmt, args);
	fprintf (stderr, "%s: %s\n", dom, p);
  }

} // unnamed namespace


/*
	ctor
*/

Camera::Camera()
  {
//#ifdef _DEBUG
	static bool log_added = false;
	if (!log_added)
	  {
		gp::gp_log_add_func (gp::GP_LOG_DEBUG, logf, 0);
		log_added = true;
	  }
//#endif

	gp::gp_camera_new (&m_gpCamera);

	m_gpContext = gp::gp_context_new();
	m_AbilitiesList.Load (m_gpContext);

	gp::gp_abilities_list_detect (m_AbilitiesList, m_PortInfoList, m_Cameras, m_gpContext);

	int n = m_Cameras.Count();

	if (n == 0)
	  throw std::runtime_error ("no cameras found");

	for (int i = 0; i < n; ++i)
		m_Detected.push_back (m_Cameras[i].Name());
  }


/*
	dtor
*/

Camera::~Camera()
  {
	gp::gp_camera_exit (m_gpCamera, m_gpContext);
  }


/*
	Select
*/

void	Camera::Select (unsigned index)
  {
	assert (index < m_Detected.size());

	int ai = gp::gp_abilities_list_lookup_model (m_AbilitiesList, m_Cameras [index].Name());
	assert (ai >= 0);

	gp::gp_abilities_list_get_abilities (m_AbilitiesList, ai, &m_gpAbilities);
	gp::gp_camera_set_abilities (m_gpCamera, m_gpAbilities);

	int pi = gp::gp_port_info_list_lookup_path (m_PortInfoList, m_Cameras [index].Value());
	assert (pi >= 0);

	gp::gp_port_info_list_get_info (m_PortInfoList, pi, &m_gpPortInfo);
	gp::gp_camera_set_port_info (m_gpCamera, m_gpPortInfo);

	gp::gp_camera_init (m_gpCamera, m_gpContext);
  }


/*
	ScanFiles
*/

void	Camera::ScanFiles()
  {
	AddFiles ("/");
  }


/*
	AddFiles
*/

void	Camera::AddFiles (const std::string& base)
  {
	GenericList list;
	GPResult res;

	std::string basex (base);
	if (*basex.rbegin() != '/')
		basex.append ("/");

	// add files in base folder

	res = gp::gp_camera_folder_list_files (m_gpCamera, base.c_str (), list, m_gpContext);

	unsigned n = gp::gp_list_count (list);
	for (unsigned i = 0; i < n; ++i)
	  {
		FileListItem fli = { base, list[i].Name() };
		m_Files.push_back (fli);
	  }

	// recursively add files in subfolders

	res = gp::gp_camera_folder_list_folders (m_gpCamera, base.c_str (), list, m_gpContext);

	n = gp::gp_list_count (list);
	for (unsigned i = 0; i < n; ++i)
		AddFiles (basex + list[i].Name());
  }


/*
	ReadFile
*/

size_t Camera::ReadFile (unsigned index, const void ** ptr, bool full)
  {
	if (index >= m_Files.size())
		return 0;

	GPResult res;

	const char *folder = m_Files [index].folder.c_str();
	const char *name = m_Files [index].name.c_str();

//	### CameraFileInfo doesn't contain anything presently useful to us
//	gp::CameraFileInfo cfi;
//	res = gp::gp_camera_file_get_info (m_gpCamera, folder, name, &cfi, m_gpContext);

	res = gp::gp_camera_file_get (m_gpCamera, folder, name, full ? gp::GP_FILE_TYPE_NORMAL : gp::GP_FILE_TYPE_PREVIEW, m_CameraFile, m_gpContext);

	unsigned long size;
	res = gp::gp_file_get_data_and_size (m_CameraFile, (const char **) ptr, &size);

	return size;
  }


/*
	SaveFile
*/

bool Camera::SaveFile (unsigned index, const std::string& prefix, const std::string& path, unsigned absnum)
  {
	std::cerr << "save " << index;

	if (index >= m_Files.size())
		return false;

	GPResult res;

	const char *folder = m_Files [index].folder.c_str();
	const char *name = m_Files [index].name.c_str();

	std::cerr << " " << folder << " " << name << std::endl;

	const char *extension = strrchr (name, '.');
	if (!extension)	  // shouldn't happen!
		extension = ".xxx";

	// get file info and data

	res = gp::gp_camera_file_get (m_gpCamera, folder, name, gp::GP_FILE_TYPE_NORMAL, m_CameraFile, m_gpContext);

	unsigned long size;
	const void *ptr;

	res = gp::gp_file_get_data_and_size (m_CameraFile, (const char **) &ptr, &size);

	if (ptr == 0 || size == 0)
	  {
		std::cerr << "ptr=" << ptr << ", size=" << size << std::endl;
		return false;
	  }

	Exiv2::Image::AutoPtr img = Exiv2::ImageFactory::open ((const Exiv2::byte *) ptr, size);

	img->readMetadata();

	Exiv2::Exifdatum& datetime = img->exifData() ["Exif.Image.DateTime"];
//	Exiv2::Exifdatum& datetime = img->exifData() ["Exif.Photo.DateTimeDigitized"];

	// make new name and create save folder tree if necessary

	std::ostringstream name_oss;
	name_oss << std::setfill ('0');

	name_oss << path;
	if (*path.rbegin() != '/')
		name_oss << "/";

	int made_year_dir = 0;
	int made_day_dir = 0;

	try
	  {
		int year, month, day, hour, minute, second;
		char c;

		std::istringstream (datetime.value ().toString()) >>
			year >> c >> month >> c >> day >> hour >> c >> minute >> c >> second;

		name_oss << year;

		if (year != made_year_dir)
		  {
			std::string dir = name_oss.str();

			if (mkdir (dir.c_str(), ALLPERMS) == 0)
				made_year_dir = year;
			else if (errno != EEXIST)
			  {
				std::cerr << "Unable to create directory " << dir << std::endl;
				throw 1;
			  }
		  }

		 name_oss << '/' << year << '-' << std::setw(2) << month << '-' << std::setw(2) << day;

		 int xday = year * 366 * month * 31 + day;		// simple pseudo day number guaranteed to be unique
		 if (xday != made_day_dir)
		  {
			std::string dir = name_oss.str ();

			if (mkdir (dir.c_str(), ALLPERMS) == 0)
				made_day_dir = xday;
			else if (errno != EEXIST)
			  {
				std::cerr << "Unable to create directory " << dir << std::endl;
				throw 2;
			  }
		  }

		name_oss << '/';

		if (!prefix.empty())
			name_oss << prefix << '_';

		name_oss << year << std::setw(2) << month << std::setw(2) << day
			<< '_' << std::setw(2) << hour << std::setw(2) << minute << std::setw(2) << second
			<< '_' << std::setw(6) << absnum
			<< extension;
	  }
	catch (...)
	  {
		std::cerr << "Unable to create directory and/or new name (maybe can't convert EXIF datum to date or time value)\n";

		name_oss << name;
	  }

	// finally do the actual save

  	res = gp::gp_file_save (m_CameraFile, name_oss.str ().c_str());

	return res >= 0;
  }

