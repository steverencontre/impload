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

#include <QMessageBox>
#include <QSettings>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QFileDialog>

#include <QDebug>

#include <algorithm>
#include <functional>

#include "Camera.h"
#include "Folder.h"

#include "MainWindow.h"
#include "ui_MainWindow.h"

/*
	ctor
*/

MainWindow::MainWindow (bool folder_mode, const std::string& start_folder, double timeshift)
:
	ui (new Ui::MainWindow),
	m_Loader (this)
{
	ui->setupUi (this);

	bool ok = folder_mode ? GetFolderSource (start_folder) : GetCameraSource();
	if (!ok)
		return;

	if (timeshift >= -24)
		m_Source->TimeOffset (timeshift * 3600);
	m_CameraInfo->timerr = m_Source->TimeOffset();

	// populate import details table

	m_DestinationBase = QSettings{}.value ("BaseFolder").toString();

	ui->wImportDetails->setItem (0, 0, new QTableWidgetItem (m_CameraInfo->type));
	ui->wImportDetails->setItem (0, 1, new QTableWidgetItem (m_CameraInfo->tag));
	ui->wImportDetails->setItem (0, 2, new QTableWidgetItem (m_DestinationBase));
	ui->wImportDetails->resizeColumnsToContents();

	// start the loader thread

	m_Loader.SetSource (m_Source);
	m_Loader.start();


	connect (ui->wImagePreview, SIGNAL (sig_SetFirst(uint)), this, SLOT (SetFirst(uint)));

	// connect save button

	connect (ui->buttonBox, SIGNAL (accepted()), SLOT (Save()));

	// disable buttons for the moment

	ui->buttonBox->setEnabled (false);
}


/*
	dtor
*/

MainWindow::~MainWindow()
{
	delete m_Source;
	delete ui;
}


/*
	GetFolderSource
*/

bool MainWindow::GetFolderSource (const std::string& start)
{
	try
	{
		auto pfolder = new Folder (start);
		m_Source = pfolder;
		m_CIList.emplace_back (CameraInfo {"Folder", "", "X", 0, 0});
		m_CameraInfo = &*m_CIList.begin();
	}
	catch (std::exception&)
	{
		return false;
	}


	return true;
}


/*
	GetCameraSource
*/

bool MainWindow::GetCameraSource()
{
	// create gphoto2 camera object

	Camera *pcam {nullptr};

	while (!pcam)
	{
		try
		{
			pcam = new Camera;
		}
		catch (const std::exception&)
		{
			int ret = QMessageBox (QMessageBox::Warning, "impload",
								   "No cameras were detected. Please check connections and power",
								   QMessageBox::Retry | QMessageBox::Cancel
								   ).exec();

			if (ret == QMessageBox::Cancel)
				return false;
		}
	}

	// get stored camera settings and match/add detected camera(s)

	ReadCameraSettings();

	auto n = pcam->Detected().size();

	if (n > 1)
		std::cerr << "Note: " << n << " cameras detected, but only first will be used for now\n";

	int i = 0;

	pcam->Select (i);

	QString cam (pcam->Detected() [i].c_str());

	auto it = std::find_if (m_CIList.begin(), m_CIList.end(),
		[&cam] (const CameraInfo& ci) { return cam == ci.type; }
	);

	if (it == m_CIList.end())
	{
		CameraInfo ci;

		ci.type		= cam;
		ci.serial		= "";
		ci.tag		    = "";
		ci.conidx	= (int) i;
		ci.timerr	= 0;

		it = m_CIList.insert (it, ci);
	}
	else
		it->conidx = (int) i;

	if (!m_CameraInfo)
		m_CameraInfo = &*it;

	m_Source = pcam;

	return true;
}


/*
	ReadCameraSettings
*/

void MainWindow::ReadCameraSettings()
  {
	QSettings settings;

	unsigned n = settings.beginReadArray ("Camera");
	m_CIList.resize (n);

	for (unsigned i = 0; i < n; ++i)
	  {
		settings.setArrayIndex (i);

		CameraInfo& ci = m_CIList [i];

		ci.type		= settings.value ("Type").toString();
		ci.serial		= settings.value ("Serial").toString();
		ci.tag		    = settings.value ("Tag").toString();
		ci.conidx	= -1;
		ci.timerr	= 0;  // note that we don't read the time error as we only persist that for others to use
	  }

	settings.endArray();
  }


/*
	WriteCameraSettings
*/

void MainWindow::WriteCameraSettings()
  {
	QSettings settings;

	settings.beginWriteArray ("Camera");

	unsigned n = m_CIList.size();

	for (unsigned i = 0; i < n; ++i)
	  {
		settings.setArrayIndex (i);

		CameraInfo& ci = m_CIList [i];

		settings.setValue ("Type", ci.type);
		settings.setValue ("Serial", ci.serial);
		settings.setValue ("Tag", ci.tag);
		settings.setValue ("Timerr", ci.timerr);
	  }

	settings.endArray();

  }


/*
	on_wImportDetails_cellDoubleClicked
*/

void MainWindow::on_wImportDetails_cellDoubleClicked (int row, int column)
  {
	if (row != 0 || column == 0)
		return;

	ui->wImportDetails->editItem (ui->wImportDetails->currentItem());
  }


/*
	on_wImportDetails_cellChanged
*/

void MainWindow::on_wImportDetails_cellChanged (int row, int column)
  {
	if (row != 0)
		return;

	QString text = ui->wImportDetails->item (row, column)->text();

	if (column == 1)
		m_CameraInfo->tag = text;
	else if (column == 2)
		m_DestinationBase = text;

	QSettings{}.setValue ("BaseFolder", m_DestinationBase);

	WriteCameraSettings();
  }


/*
	FileCount
*/

void MainWindow::FileCount (unsigned nfiles)
  {
	m_Total = nfiles;

	ui->wProgressBar->setMinimum (0);
	ui->wProgressBar->setMaximum (m_Total);
	ui->wProgressBar->setValue (0);
  }



/*
	NewThumbnail
*/

void MainWindow::NewThumbnail (unsigned index, const void *data, unsigned size, int orientation)
  {
	ui->wImagePreview->Add (data, size, orientation);
	ui->wProgressBar->setValue ((int) index + 1);
	m_Loader.Continue();
  }


/*
	ThumbnailsDone
*/

void MainWindow::ThumbnailsDone()
  {
	ui->buttonBox->setEnabled (true);
  }


/*
	Save
*/

void MainWindow::Save()
  {
	ui->wProgressBar->setMinimum (m_First);
	ui->wProgressBar->setValue (0);

	emit sig_Save (m_CameraInfo->tag.toUtf8().data(), m_DestinationBase.toUtf8().data(), m_First);
  }


/*
	SavedOne
*/

void MainWindow::SavedOne (unsigned index, bool ok)
  {
	ui->wImagePreview->Saved (index, ok);
	ui->wProgressBar->setValue (int (index + 1));
  }


/*
	SavedAll
*/

void MainWindow::SavedAll()
  {
	QCoreApplication::quit();
  }


/*
	on_AddFiles_triggered
*/

void MainWindow::on_AddFiles_triggered()
  {
	QMessageBox (QMessageBox::Warning, "impload",
				"Function is not yet implemented",
				QMessageBox::Ok
				).exec();

//	QString addtree = QFileDialog::getExistingDirectory (this, "Choose directory containing files to add");

  }

void MainWindow::on_actionInfo_triggered()
  {
	QString text = QString ("time error: %1").arg (m_CameraInfo->timerr);

	QMessageBox (QMessageBox::Information, "impload",
		text,
		QMessageBox::Ok
		).exec();

  }
