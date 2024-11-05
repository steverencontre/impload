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

#include "PreviewGrid.h"

#include <QIcon>
#include <QImage>
#include <QPixmap>
#include <QLabel>

#include <iostream>



using std::max;

const int IMAGESIZE = 96;		  // number of pixels for image previews


/*
	ctor
*/

PreviewGrid::PreviewGrid (QWidget *parent)
  :
	QTableWidget (parent),
	m_Count (0)
  {
	m_RotateCW.rotate (-90);
	m_RotateCCW.rotate (90);

	connect (this, SIGNAL (cellClicked (int, int)), this, SLOT (CellClicked (int, int)));
  }



/*
	dtor
*/

PreviewGrid::~PreviewGrid()
  {
	disconnect (SIGNAL (cellClicked (int, int)));
  }


/*
	Add
*/

void PreviewGrid::Add (const void *data, unsigned size, int orientation)
{
	int nx = columnCount();
	int ny = rowCount();

	int ix = m_Count % nx;
	int iy = m_Count / nx;
	++m_Count;

	if (size == 0)		// missing thumb for some reason
	{
		QLabel *label = new QLabel;
		label->setText("(missing)");
		label->setAlignment (Qt::AlignCenter);
		setCellWidget (iy, ix, label);
		return;
	}

	if (iy >= ny)
	{
		setRowCount (++ny);
		setRowHeight (iy, IMAGESIZE);
	}

	QImage thumb;
	thumb.loadFromData ((const uchar *) data, size);

	static int dx, dy, xy;

	if (m_Count == 1)	// first time through, calculate scale factor
	{
		xy = 107;
		dy = (thumb.height() - xy) / 2;
		dx = (thumb.width() - xy) / 2;

		double s = (double) IMAGESIZE / (double) xy;

		m_Scale.scale (s, s);
		m_RotateCCW.scale (s, s);
		m_RotateCW.scale (s, s);
	}

	QImage crop = thumb.copy (dx, dy, xy, xy);
	QImage square = crop;//: crop.transformed (m_Scale);

	switch (orientation)
	{
	case 1:	// no rotation
		thumb = thumb.transformed (m_Scale);
		break;

	case 6:	// +90
		thumb = thumb.transformed (m_RotateCCW);
		break;

	case 8:	// -90
		thumb = thumb.transformed (m_RotateCW);
		break;
	}


	QLabel *label = new QLabel;
	label->setPixmap (QPixmap::fromImage (square));

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


/*
	Cell clicked
*/

void PreviewGrid::CellClicked (int r, int c)
  {
	int nx = columnCount();

	int index = r * nx + c;

	for (int i = 0; i  < m_Count; ++i)
	{
		r = i / nx;
		c = i % nx;
		auto pcw = cellWidget (r, c);
		if (pcw)
			pcw->setEnabled (i >= index);
		else
			std::cout << "index " << i << " ("  << r << "," << c << ") has no widget\n";
	}
	emit sig_SetFirst (index);
  }

