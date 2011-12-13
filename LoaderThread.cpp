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

#include <QSettings>

#include "LoaderThread.h"

#include <algorithm>
#include <functional>

/*
	ctor
*/

LoaderThread::LoaderThread (QObject *parent)
  :
  	QThread(parent),
  	m_Camera (0)
  {
  }


/*
	run
*/

void LoaderThread::run()
  {
	connect (this, SIGNAL (sig_NewThumbnail (const char *, const void *, unsigned)), parent(), SLOT (NewThumbnail (const char *, const void *, unsigned)), Qt::QueuedConnection);
	connect (this, SIGNAL (sig_ThumbnailsDone (unsigned)), parent(), SLOT (ThumbnailsDone (unsigned)), Qt::QueuedConnection);
	connect (this, SIGNAL (sig_Saved (unsigned, bool)), parent(), SLOT (Saved (unsigned, bool)), Qt::QueuedConnection);
	connect (this, SIGNAL (sig_SavedAll()), parent(), SLOT (SavedAll()), Qt::QueuedConnection);

	connect (parent(), SIGNAL (sig_SaveAll (const char *, const char *)), SLOT (SaveAll (const char *, const char *)));

	// get the file list from the camera

	m_Camera->ScanFiles();
	unsigned nfiles = m_Camera->Files ().size();


	// send thumbnails to GUI

	m_Mutex.lock();

	for (unsigned i = 0; i < nfiles; ++i)
	  {
		const void *data;
		size_t size = m_Camera->ReadFile (i, &data);

		emit sig_NewThumbnail (m_Camera->Files() [i].name.c_str(), data, size);

		m_WaitCondition.wait (&m_Mutex);	// wait until GUI has handled thumbnail
	  }

	emit sig_ThumbnailsDone (nfiles);

	// now wait until GUI signals time to save or pack up

	m_WaitCondition.wait (&m_Mutex);

	if (!m_SavePath.empty())
	  {
		QSettings settings;
		unsigned absnum = settings.value ("AbsNum", 0).toUInt();

		for (unsigned i = 0; i < nfiles; ++i)
		  {
			bool ok = m_Camera->SaveFile (i, m_Prefix, m_SavePath, absnum++);

			emit sig_Saved (i, ok);
		  }

		settings.setValue ("AbsNum", absnum);
	  }

	emit sig_SavedAll();
  }


/*
	SaveAll
*/

void  LoaderThread::SaveAll (const char *prefix, const char *path)
  {
	m_Prefix.assign (prefix);
	m_SavePath.assign (path);
	Continue();
  }
