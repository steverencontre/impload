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

#include <QMessageBox>
#include <QSettings>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <algorithm>
#include <functional>

#include "MainWindow.h"
#include "ui_MainWindow.h"

/*
	ctor
*/

MainWindow::MainWindow (QWidget *parent)
  :
	QMainWindow (parent),
	ui (new Ui::MainWindow),
	m_Loader (this),
	m_Camera (0),
	m_CameraInfo (0)
  {
	ui->setupUi (this);

	// create gphoto2 camera object

	while (!m_Camera) try
	  {
		m_Camera = new Camera;
	  }
	catch (const std::exception& x)
	  {
		if
		  (
			QMessageBox (QMessageBox::Warning, "impload",
				"No cameras were detected. Please check connections and power",
				QMessageBox::Retry | QMessageBox::Cancel
				).exec()
			== QMessageBox::Cancel
		  )
			return;
	  }

	// get stored camera settings and match/add detected camera(s)

	ReadCameraSettings();

	const std::vector <std::string>& detected (m_Camera->Detected());

	unsigned n = 1; // detected.size();			### get user to choose if more than one camera detected

	for (unsigned i = 0; i < n; ++i)
	  {
		m_Camera->Select (i);

		QString cam (detected [i].c_str());

		auto it = std::find_if (m_CIList.begin(), m_CIList.end(),
			[&cam] (const CameraInfo& ci) { return cam == ci.type; }
			);

		if (it == m_CIList.end())
		  {
			CameraInfo ci;

			ci.type		= cam;
			ci.serial		= "";
			ci.prefix		= "";
			ci.conidx	= (int) i;

			it = m_CIList.insert (it, ci);
		  }
		else
			it->conidx = (int) i;

		if (!m_CameraInfo)
			m_CameraInfo = &*it;
	  }

	// populate import details table

	ui->wImportDetails->setColumnWidth (0, 120);
	ui->wImportDetails->setColumnWidth (1, 60);

	ui->wImportDetails->setItem (0, 0, new QTableWidgetItem (m_CameraInfo->type));
	ui->wImportDetails->setItem (0, 1, new QTableWidgetItem (m_CameraInfo->prefix));
	ui->wImportDetails->setItem (0, 2, new QTableWidgetItem (m_BaseFolder));

	// start the loader thread

	m_Loader.SetCamera (m_Camera);
	m_Loader.start();

	// connect save all button

	connect (ui->buttonBox, SIGNAL (accepted()), SLOT (SaveAll()));

	// disable buttons for the moment

	ui->buttonBox->setEnabled (false);
  }


/*
	dtor
*/

MainWindow::~MainWindow()
  {
	delete m_Camera;
	delete ui;
  }


/*
	ReadCameraSettings
*/

void MainWindow::ReadCameraSettings()
  {
	QSettings settings;

	m_BaseFolder = settings.value ("BaseFolder").toString();

	unsigned n = settings.beginReadArray ("Camera");
	m_CIList.resize (n);

	for (unsigned i = 0; i < n; ++i)
	  {
		settings.setArrayIndex (i);

		CameraInfo& ci = m_CIList [i];

		ci.type		= settings.value ("Type").toString();
		ci.serial		= settings.value ("Serial").toString();
		ci.prefix		= settings.value ("Prefix").toString();
		ci.conidx	= -1;
	  }

	settings.endArray();
  }


/*
	WriteCameraSettings
*/

void MainWindow::WriteCameraSettings()
  {
	QSettings settings;

	settings.setValue ("BaseFolder", m_BaseFolder);

	settings.beginWriteArray ("Camera");

	unsigned n = m_CIList.size();

	for (unsigned i = 0; i < n; ++i)
	  {
		settings.setArrayIndex (i);

		CameraInfo& ci = m_CIList [i];

		settings.setValue ("Type", ci.type);
		settings.setValue ("Serial", ci.serial);
		settings.setValue ("Prefix", ci.prefix);
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
		m_CameraInfo->prefix = text;
	else if (column == 2)
		m_BaseFolder = text;

	WriteCameraSettings();
  }


/*
	NewThumbnail
*/

void MainWindow::NewThumbnail (const char *name, const void *data, unsigned size)
  {
	ui->wImagePreview->Add (name, data, size);
	m_Loader.Continue();
  }


/*
	ThumbnailsDone
*/

void MainWindow::ThumbnailsDone (unsigned nfiles)
  {
	ui->wProgressBar->setMinimum (0);
	ui->wProgressBar->setMaximum ((int) nfiles);
	ui->wProgressBar->setValue (0);

	ui->buttonBox->setEnabled (true);
  }


/*
	Saved
*/

void MainWindow::Saved (unsigned index, bool ok)
  {
	ui->wImagePreview->Saved (index, ok);
	ui->wProgressBar->setValue ((int) index);
  }


/*
	SavedAll
*/

void MainWindow::SavedAll()
  {
	QCoreApplication::quit();
  }
