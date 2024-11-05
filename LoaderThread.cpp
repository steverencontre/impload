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

#include <QSettings>
#include <QFile>

#include "LoaderThread.h"

#include <algorithm>
#include <functional>
#include <iostream>

/*
	ctor
*/

LoaderThread::LoaderThread (QObject *parent, std::atomic<int>& absnum)
  :
	QThread {parent},
	m_AbsNum {absnum}
  {
  }


/*
	run
*/

void LoaderThread::run()
  {
	connect (this, SIGNAL (sig_FileCount(uint)), parent(), SLOT (FileCount(uint)), Qt::QueuedConnection);
	connect (this, SIGNAL (sig_NewThumbnail (unsigned, const void *, unsigned, int)), parent(), SLOT (NewThumbnail (unsigned, const void *, unsigned, int)), Qt::QueuedConnection);
	connect (this, SIGNAL (sig_ThumbnailsDone()), parent(), SLOT (ThumbnailsDone()), Qt::QueuedConnection);
	connect (this, SIGNAL (sig_SavedOne(uint,bool)), parent(), SLOT (SavedOne(uint,bool)), Qt::QueuedConnection);
	connect (this, SIGNAL (sig_SavedAll()), parent(), SLOT (SavedAll()), Qt::QueuedConnection);

	connect (parent(), SIGNAL (sig_Save (const char *, const char *, unsigned)), SLOT (Save (const char *, const char *, unsigned)));

	// get the file list from the camera

	std::cout << "Getting camera file list..." << std::endl;

	m_ImageSource->ScanFiles();
	unsigned nfiles = m_ImageSource->Files ().size();
	int orientation = 0;	// default if no EXIF override

	std::cout << "Got " << nfiles << " files" << std::endl;

	emit sig_FileCount (nfiles);

	// send thumbnails to GUI

	m_Mutex.lock();

	for (unsigned i = 0; i < nfiles; ++i)
	  {
		auto [data, size, dt] = m_ImageSource->LoadData (i, ImageSource::THUMB);

		// check for portrait-mode rotation -- doesn't work at present!
//		orientation = Metadata {data, size}.Orientation();

		std::cout << m_ImageSource->Files() [i].name << std::endl;

		emit sig_NewThumbnail (i, data, size, orientation);
		m_WaitCondition.wait (&m_Mutex);	// wait until GUI has handled previous thumbnail
	  }

	emit sig_ThumbnailsDone();

	// now wait until GUI signals to save or pack up

	m_WaitCondition.wait (&m_Mutex);

	if (!m_SavePath.empty())
	  {
		for (unsigned i = m_First; i < nfiles; ++i)
		  {
			try
			{
				bool ok = m_ImageSource->SaveFile (i, m_Tag, m_SavePath, m_AbsNum++);
				emit sig_SavedOne (i, ok);
			}
			catch (const std::exception& e)
			{
				std::cout << "Image save error " << e.what() << std::endl;
			}
		  }
	  }

	m_Mutex.unlock();

	emit sig_SavedAll();
  }


/*
	Save
*/

void  LoaderThread::Save (const char *tag, const char *path, unsigned first)
  {
	m_Tag.assign (tag);
	m_SavePath.assign (path);
	m_First = first;

	Continue();
  }
