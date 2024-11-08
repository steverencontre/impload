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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QResizeEvent>

#include <vector>
#include <atomic>

#include <yaml-cpp/yaml.h>

#include "LoaderThread.h"
#include "ImageSource.h"



namespace Ui {
    class MainWindow;
}


struct CameraInfo
  {
	std::string		serial;
	std::string		type;
	std::string		tag;
	double			timerr;
  };

//typedef std::map <std::string, CameraInfo> CameraInfoMap;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow (bool folder_mode, const std::string& start, double timeshift);
	~MainWindow();

	bool	  GotValidSource()	{ return m_Source != nullptr; }

signals:
	void sig_Save (const char *tag, const char *path, unsigned first = 0);

public slots:
	void FileCount (unsigned nfiles);
	void NewThumbnail (unsigned index, const void *data, unsigned size, int orientation);
	void ThumbnailsDone();
	void SetFirst (unsigned first) { m_First = first; }
	void Save();
	void SavedOne (unsigned index, bool ok);
	void SavedAll();

private slots:
	void on_wImportDetails_cellDoubleClicked(int row, int column);
	void on_wImportDetails_cellChanged(int row, int column);
	void on_AddFiles_triggered();

	void on_actionInfo_triggered();

private:
	bool	GetCameraSource();
	bool	GetFolderSource (const std::string& start);


	virtual void closeEvent (QCloseEvent *e)  { m_Loader.Continue(); QMainWindow::closeEvent (e); }

	Ui::MainWindow	  *ui;

	LoaderThread			m_Loader;
	ImageSource		  *m_Source {nullptr};
	CameraInfo			m_CameraInfo;
	std::string				m_DestinationBase;
	unsigned				m_First {0};
	unsigned				m_Total {0};
	unsigned				m_ThumbnailRows;
	unsigned				m_ThumbnailColums;
	std::string				m_ConfigName;
	YAML::Node			m_Config;
	std::atomic<int>	m_AbsNum;
};

#endif // MAINWINDOW_H
