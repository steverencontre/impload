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

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cstdio>
#include <ctime>
#include <errno.h>

#include "Camera.h"
#include "CameraWidget.h"
#include "Metadata.h"

class GPResult
{
public:
	GPResult()										{}
	GPResult (int r)								{ Set (r); }
	const GPResult& operator= (int r)		{ Set (r); return *this; }
	operator int() const							{ return m_Value; }

private:
	void Set (int r)
	{
		if ((m_Value = r) < 0)
		{
			const char *p = r < -99 ? gp::gp_result_as_string (r) : gp::gp_port_result_as_string (r);
			fprintf (stderr, "%s\n", p);
		}
	}

	int	  m_Value;
};


/*
	class statics
*/
CameraList		Camera::s_CamerasSupported;
GenericList			Camera::s_CamerasDetected;


/*
	ctor
*/

Camera::Camera()
{
#define LOGGING 0
#if LOGGING
	static bool log_added = false;
	if (!log_added)
	  {
		gp::gp_log_add_func (gp::GP_LOG_DEBUG,
			[] (gp::GPLogLevel, const char *dom, const char *msg, void *)
			{
				fprintf (stderr, "%s: %s\n", dom, msg);
			},
			nullptr);
		log_added = true;
	  }
#endif

	gp::gp_camera_new (&m_gpCamera);

#if 0
	int index = gp::gp_port_info_list_lookup_path (m_PortInfoList, "ptpip:");

	//	int infos = gp::gp_port_info_list_count (m_PortInfoList);
	//	for (int index = 0; index < infos; ++index)
	if (index >= 0)
	{
		gp::GPPortInfo info;
		gp::gp_port_info_list_get_info (m_PortInfoList, index, &info);

		gp::gp_port_info_set_path (info, "ptpip:10.99.0.243");

		char *name;
		gp::gp_port_info_get_name (info, &name);

		char *path;
		gp::gp_port_info_get_path (info, &path);

		std::cout << index << ": name=" << name << " path=" << path << std::endl;
	}
#endif

	m_gpContext = gp::gp_context_new();
	s_CamerasSupported.Load (m_gpContext);

	gp::gp_abilities_list_detect (s_CamerasSupported, m_PortInfoList, s_CamerasDetected, m_gpContext);

	unsigned n = s_CamerasDetected.Count();

	if (n == 0)
		throw std::runtime_error ("no cameras found");

	for (unsigned i = 0; i < n; ++i)
		std::cout << s_CamerasDetected[i].Name() << ": " << s_CamerasDetected[i].Value() << std::endl;

	m_BaseDir = "/";
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
	assert (index < s_CamerasDetected.size());

	m_Type = s_CamerasDetected [index].Name();

	int ai = gp::gp_abilities_list_lookup_model (s_CamerasSupported, s_CamerasDetected [index].Name());
	assert (ai >= 0);

	gp::gp_abilities_list_get_abilities (s_CamerasSupported, ai, &m_gpAbilities);
	gp::gp_camera_set_abilities (m_gpCamera, m_gpAbilities);

	int pi = gp::gp_port_info_list_lookup_path (m_PortInfoList, s_CamerasDetected [index].Value());
	assert (pi >= 0);

	gp::gp_port_info_list_get_info (m_PortInfoList, pi, &m_gpPortInfo);
	gp::gp_camera_set_port_info (m_gpCamera, m_gpPortInfo);

	gp::gp_camera_init (m_gpCamera, m_gpContext);

	// get date/time offset correction

	gp::CameraWidget *cwp;
	gp::gp_camera_get_config (m_gpCamera, &cwp, m_gpContext);

	CameraWidget cw (cwp);
	CameraWidget cw2;

	cw2 = cw.Find ("main/status/serialnumber");
	if (cw2)
		cw2.RetrieveValue (m_SerialNo);

	time_t camera_time;

	if ((cw2 = cw.Find ("main/settings/datetimeutc")))
	{
		int value;

		cw2.RetrieveValue (value);
		camera_time = value;

		m_TimeOffset = time (nullptr) - camera_time;
	}

	else if ((cw2 = cw.Find ("main/settings/datetime")))  // beware of localtime assumption :-(
	{
		int value;

		cw2.RetrieveValue (value);
		camera_time = value;

		// copy the local time hack from libgphoto/camlibs/ptp/config.c
		time_t ltime = time (nullptr);
		struct tm *ptm = gmtime (&ltime);
		ptm->tm_isdst = -1;
		ltime = mktime (ptm);

		m_TimeOffset = ltime - camera_time;
	}

	else if ((cw2 = cw.Find ("main/other/d034")))
	{
		char *value;

		cw2.RetrieveValue (value);
		camera_time = atoi (value);

		m_TimeOffset = time (nullptr) - camera_time;
	}

	else
		m_TimeOffset = 0;

	if
	(
		(cw2 = cw.Find ("main/actions/syncdatetimeutc")) ||
		(cw2 = cw.Find ("main/actions/syncdatetime"))
	)
	{
		cw2.SetValue (1);
		gp::gp_camera_set_config (m_gpCamera, cwp, m_gpContext);
	}

	if (m_TimeOffset)
		std::cerr << "Time offset = " << m_TimeOffset << std::endl;
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

	unsigned n = list.Count();
	for (unsigned i = 0; i < n; ++i)
		m_Files.emplace_back (FileListItem {base, list[i].Name()});

	// recursively add files in subfolders

	res = gp::gp_camera_folder_list_folders (m_gpCamera, base.c_str (), list, m_gpContext);

	n =  list.Count();
	for (unsigned i = 0; i < n; ++i)
	{
		if (list[i].Name()[0] != '.')
			AddFiles (basex + list[i].Name());
	}
}


/*
	LoadData
*/

ImageSource::ImageData Camera::LoadData (const std::string& folder, const std::string& name, DataType type)
{
	constexpr gp::CameraFileType typemap [DATA_TYPES] =
	{
		gp::GP_FILE_TYPE_NORMAL,
		gp::GP_FILE_TYPE_PREVIEW,
		gp::GP_FILE_TYPE_EXIF,
		gp::GP_FILE_TYPE_NORMAL
	};

	GPResult res;
	unsigned long size;
	const char *ptr;

	res = gp::gp_camera_file_get (m_gpCamera, folder.c_str(), name.c_str(), typemap [type], m_CameraFile, m_gpContext);
	res = gp::gp_file_get_data_and_size (m_CameraFile, &ptr, &size);

	QDateTime dt;
	if ((type == FULL || type == VIDEO) && res >= 0)
		dt = Metadata {ptr, size, type == VIDEO}.Timestamp();

	return {(const uint8_t *) ptr, size, dt};

}


/*
	WriteImageFile
*/

bool Camera::WriteImageFile(const std::string& destname)
{
	GPResult res =  gp::gp_file_save (m_CameraFile, destname.c_str());
	return res >= 0;
}
