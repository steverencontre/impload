/*
	FiXer - FX-Sport file manager
	
	Copyright © 2013 Steve Rencontre		q.fixer@rsn-tech.co.uk
	All rights reserved.
	
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

#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <string>

#include "gphoto2.h"

class CameraWidget
  {
  public:
	CameraWidget () : m_pWidget {nullptr} {}
	CameraWidget (gp::CameraWidget *p) : m_pWidget (p) {}
	CameraWidget (const CameraWidget& w) : m_pWidget (w.m_pWidget) {}

	bool  Find (CameraWidget cw, const std::string& path)  { m_pWidget = cw.FindChild (path); return m_pWidget != nullptr; }

	CameraWidget operator[] (int i) const  { gp::CameraWidget *p; gp::gp_widget_get_child (m_pWidget, i, &p); return p; }
	CameraWidget operator[] (const char *p) const { return operator[] (std::string (p)); }
	CameraWidget operator[] (const std::string& p) const;	// throws exception if not found

	operator bool() const { return m_pWidget != nullptr; }
	std::string Name() const { const char *p; gp::gp_widget_get_name (m_pWidget, &p); return std::string (p); }

	template <typename T>
	int RetrieveValue (T& pt) { return gp::gp_widget_get_value (m_pWidget, &pt); }

	template <typename T>
	int SetValue (const T& pt) { return gp::gp_widget_set_value (m_pWidget, &pt); }

  private:
	gp::CameraWidget *FindChild (const std::string& p) const;	  // returns null widget if not found

	gp::CameraWidget *m_pWidget;
  };

#endif // CAMERAWIDGET_H
