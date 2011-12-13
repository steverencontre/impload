/*
	impload - simple gphoto2-based camera file importer

	Copyright (c) 2011 Steve Rencontre	q.impload@rsn-tech.co.uk

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

#ifndef PREVIEWGRID_H
#define PREVIEWGRID_H

#include <QTableWidget>

class PreviewGrid : public QTableWidget
  {
    Q_OBJECT

  public:
	explicit PreviewGrid (QWidget *parent = 0);

	void Add (const char *name, const void *data, unsigned size);
	void Saved (unsigned index, bool ok);

  private:
	virtual void  resizeEvent (QResizeEvent *);

	int		  m_Count;
};

#endif // PREVIEWGRID_H
