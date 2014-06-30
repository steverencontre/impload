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

#include "CameraWidget.h"

#include <stdexcept>

CameraWidget CameraWidget::operator[] (const std::string& p) const
  {
	size_t px1 = 0, px2;
	gp::CameraWidget *cwp = m_pWidget;

	px1 = 0;
	do
	  {
		px2 = p.find ('/', px1);

		if (gp::gp_widget_get_child_by_name (cwp, p.substr (px1, px2 - px1).c_str (), &cwp) != GP_OK)
			throw std::range_error ("No such camera widget");

		px1 = px2 + 1;

	  }
		while (px2 != std::string::npos);

	return CameraWidget (cwp);
  }
