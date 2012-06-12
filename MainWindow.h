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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QResizeEvent>

#include <vector>

#include "PreviewGrid.h"
#include "LoaderThread.h"
#include "Camera.h"



namespace Ui {
    class MainWindow;
}


struct CameraInfo
  {
	QString		type;
	QString		serial;
	QString		prefix;
	int			conidx;
  };

typedef std::vector <CameraInfo> CameraInfoList;

class MainWindow : public QMainWindow
  {
    Q_OBJECT

  public:
	explicit MainWindow (QWidget *parent = 0);
	~MainWindow();

	bool	  DetectedCamera()	{ return m_Camera != 0; }

  signals:
	void sig_SaveAll (const char *prefix, const char *path);

  public slots:
	void FileCount (unsigned nfiles);
	void NewThumbnail (unsigned index, const char *name, const void *data, unsigned size);
	void ThumbnailsDone();
	void SaveAll();
	void Saved (unsigned index, bool ok);
	void SavedAll();

  private slots:
	void on_wImportDetails_cellDoubleClicked(int row, int column);
	void on_wImportDetails_cellChanged(int row, int column);

  private:
	void		ReadCameraSettings();
	void		WriteCameraSettings();

	virtual void closeEvent (QCloseEvent *e)  { m_Loader.Continue(); QMainWindow::closeEvent (e); }

	Ui::MainWindow *ui;

	LoaderThread		m_Loader;
	Camera				  *m_Camera;
	CameraInfoList		m_CIList;
	CameraInfo		  *m_CameraInfo;
	QString					m_BaseFolder;

	int						m_ThumbnailRows;
	int						m_ThumbnailColums;
  };

#endif // MAINWINDOW_H
