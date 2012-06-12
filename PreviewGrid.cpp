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

#include "PreviewGrid.h"

#include <QIcon>
#include <QPixmap>
#include <QLabel>

#include <iostream>


const int IMAGESIZE = 96;		  // number of pixels for image previews


/*
	ctor
*/

PreviewGrid::PreviewGrid (QWidget *parent)
  :
	QTableWidget (parent),
	m_Count (0)
  {
  }


/*
	Add
*/

void PreviewGrid::Add (const char *name, const void *data, unsigned size)
  {
	int nx = columnCount();
	int ny = rowCount();

	int ix = m_Count % nx;
	int iy = m_Count / nx;
	++m_Count;

	if (iy >= ny)
	  {
		setRowCount (++ny);
		setRowHeight (iy, IMAGESIZE);
	  }

	QPixmap pm;
	bool ok = pm.loadFromData ((const uchar *) data, size);//, "JPEG");

#if 0 // ### don't bother because QPixmap/QImage has no lossless quick rotation mode
	// check for portrait-mode rotation
	Exiv2::Image::AutoPtr img = Exiv2::ImageFactory::open ((const Exiv2::byte *) ptr, size);

	img->readMetadata();

	Exiv2::Exifdatum& ??? = img->exifData() ["Exif.Image.???"];
#endif

	QLabel *label = new QLabel;
	label->setAlignment (Qt::AlignCenter);
	label->setPixmap (pm);

	std::cerr << name << " load " << size << " bytes " << (ok ? "ok" : "failed") << std::endl;

	setCellWidget (iy, ix, label);
  }


/*
	Saved
*/

void PreviewGrid::Saved (unsigned /*index*/, bool /*ok*/)
  {
	// presently does nothing
  }


/*
	resizeEvent
*/

void PreviewGrid::resizeEvent (QResizeEvent *ev)
  {
	QTableWidget::resizeEvent (ev);

	int nx = columnCount();

	if (nx == 0)
	  {
		int w = width();

		nx = w / IMAGESIZE;

		setColumnCount (nx);

		for (int i = 0; i < nx; ++i)
			setColumnWidth (i, IMAGESIZE);
	  }
   }

