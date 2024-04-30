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
#include <QTransform>

class PreviewGrid : public QTableWidget
  {
    Q_OBJECT

  public:
	explicit PreviewGrid (QWidget *parent = nullptr);
	~PreviewGrid();

	void Add (const void *data, unsigned size, int orientation);
	void Saved (unsigned index, bool ok);

  signals:
	void  sig_SetFirst (unsigned index);

  private slots:
	void CellClicked (int r, int c);

  private:
	virtual void  resizeEvent (QResizeEvent *);

	int					m_Count;
	QTransform 	m_RotateCW;
	QTransform 	m_RotateCCW;
	QTransform	m_Scale;
  };

#endif // PREVIEWGRID_H
