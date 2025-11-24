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
#include <QStandardPaths>
#include <QDebug>

#include <fstream>

#include "Camera.h"
#include "Folder.h"

#include "MainWindow.h"
#include "ui_MainWindow.h"

/*
	ctor
*/

MainWindow::MainWindow (bool folder_mode, const std::string& start_folder, double timeshift)
:
	ui {new Ui::MainWindow},
	m_Loader {this, m_AbsNum}
{
	ui->setupUi (this);

	m_ConfigName = QStandardPaths::writableLocation (QStandardPaths::AppConfigLocation).toStdString() + ".config.yaml";
    m_Config = YAML::LoadFile (m_ConfigName);
	m_AbsNum = m_Config ["General"] ["AbsNum"].as<int>();

	bool ok = folder_mode ? GetFolderSource (start_folder) : GetCameraSource();
	if (!ok)
		return;

	if (timeshift)		// force override of setting deduced from camera current time
	{
		m_Source->TimeOffset (timeshift * 3600);
		m_CameraInfo.timerr = m_Source->TimeOffset();
	}

	// populate import details table

	m_DestinationBase = m_Config ["General"] ["BaseFolder"].as<std::string>();

	ui->wImportDetails->setItem (0, 0, new QTableWidgetItem (QString::fromStdString (m_CameraInfo.type)));
	ui->wImportDetails->setItem (0, 1, new QTableWidgetItem (QString::fromStdString (m_CameraInfo.tag)));
	ui->wImportDetails->setItem (0, 2, new QTableWidgetItem (QString::fromStdString (m_DestinationBase)));
	ui->wImportDetails->resizeColumnsToContents();

	// start the loader thread

	m_Loader.Prepare (m_Source);
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
	m_Config ["General"] ["AbsNum"] = (int) m_AbsNum;

	std::ofstream outf {m_ConfigName};
	outf << "%YAML 1.2\n"	"# Impload configuration\n" "---\n" << m_Config;

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
		m_CameraInfo = CameraInfo {"", "Folder", "", 0};
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

	// in principle we could have more than one camera connected, but that's not implemented for now
	pcam->Select (0);

	auto node = m_Config ["Cameras"] [pcam->SerialNo()];

	m_CameraInfo.serial = pcam->SerialNo();
	m_CameraInfo.type = pcam->Type();
	try
	{
		m_CameraInfo.tag = node ["tag"].as<std::string>();
	}
	catch (...)
	{
		m_CameraInfo.tag = "???";
	}
	m_Source = pcam;

	return true;
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
	{
		m_CameraInfo.tag = text.toStdString();
		m_Config ["Cameras"] [m_CameraInfo.serial] ["tag"] = m_CameraInfo.tag;
	}
	else if (column == 2)
	{
		m_DestinationBase = text.toStdString();
		m_Config ["General"] ["BaseFolder"] = m_DestinationBase;
	}
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

	emit sig_Save (m_CameraInfo.tag.c_str(), m_DestinationBase.c_str(), m_First);
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
	QString text {"Not currently used"};//= QString ("time error: %1").arg (m_CameraInfo->timerr);

	QMessageBox (QMessageBox::Information, "impload",
		text,
		QMessageBox::Ok
		).exec();

  }
