/*
	impload - simple gphoto2-based camera file importer

	Copyright (c) 2011-24 Steve Rencontre	q.impload@rsn-tech.co.uk

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

#ifndef CAMERA_H
#define CAMERA_H


#include <vector>
#include <string>
#include <cassert>

#include "gphoto2.h"

#include <iostream>

#include <ImageSource.h>

// *** helpers *** //

template <typename T>
class Wrapper
  {
  public:
	operator T *()	  { return m_Gp; }
	unsigned		Count() const;

  protected:
	Wrapper()		{ New(); }
	~Wrapper()	{ Del(); }

	T	  *m_Gp;

  private:
	bool  New();
	bool  Del();
  };


class PortInfoList : public Wrapper <gp::GPPortInfoList>
  {
  public:
	PortInfoList()				{ gp::gp_port_info_list_load (m_Gp); }

	void Append (gp::GPPortInfo inf) { gp::gp_port_info_list_append (m_Gp, inf); }
  };


class GLItemHelper
  {
  public:
	GLItemHelper (gp::CameraList *clp, size_t i) : m_Clp (clp), m_Index (i) {}

	const char *Name()	{ const char *p; gp_list_get_name (m_Clp, m_Index, &p); return p; }
	const char *Value()	{ const char *p; gp_list_get_value (m_Clp, m_Index, &p); return p; }

  private:
	gp::CameraList *m_Clp;
	size_t				  m_Index;
  };

// note, 'CameraList' is a bit of a misnomer; it's not a list of cameras, so we rename it for our own use

class GenericList : public Wrapper <gp::CameraList>
  {
  public:
	GLItemHelper operator[] (size_t i)	{ return GLItemHelper (m_Gp, i); }
  };


class AbilitiesList : public Wrapper <gp::CameraAbilitiesList>
  {
  public:
	void Load (gp::GPContext *context)	{ gp_abilities_list_load (m_Gp, context);  }
  };

class CameraFile : public Wrapper <gp::CameraFile>
  {
  };

namespace gp { inline int gp_file_count(gp::CameraFile *) { return 1; }	}	// ### dummy


#define FUNCS(T, F)	  \
	template<>	inline \
	bool Wrapper<T>::New() { return gp::gp_##F##_new (&m_Gp); } \
	template<>	inline \
	bool Wrapper<T>::Del() { return gp::gp_##F##_free (m_Gp); } \
	template<> inline \
	unsigned Wrapper<T>::Count() const { auto n = gp::gp_##F##_count (m_Gp); assert (n >= 0); return unsigned (n); }

FUNCS (gp::GPPortInfoList, port_info_list)
FUNCS (gp::CameraList, list)
FUNCS (gp::CameraAbilitiesList, abilities_list)
FUNCS (gp::CameraFile, file)


//////////////////////////////////////////////


class Camera : public ImageSource
  {
  public:
	Camera();
	~Camera() override;


	const std::vector <std::string>& Detected() const	  { return m_Detected; }

	void		Select (unsigned i);

  private:
	void				AddFiles (const std::string& base) override;
	ImageData		LoadData (const std::string& folder, const std::string& name, DataType type) override;
	bool				WriteImageFile(const std::string& destname) override;

	gp::Camera				  *m_gpCamera;
	gp::GPContext			  *m_gpContext;
	gp::CameraAbilities		m_gpAbilities;
	gp::GPPortInfo				m_gpPortInfo;

	PortInfoList					m_PortInfoList;
	GenericList						m_Cameras;
	AbilitiesList					m_AbilitiesList;
	GenericList						m_CameraFiles;

	CameraFile						m_CameraFile;


	std::vector <std::string>		m_Detected;
  };

#endif // CAMERA_H
