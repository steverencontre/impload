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

#ifndef LOADERTHREAD_H
#define LOADERTHREAD_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

#include "Camera.h"

class LoaderThread : public QThread
  {
	Q_OBJECT

  public:
	explicit LoaderThread (QObject *parent = 0);

	void	SetCamera (Camera *pc)	{ m_Camera = pc; }
	void  Continue()	{ m_WaitCondition.wakeAll(); }

  signals:
	void  sig_NewThumbnail (const char *name, const void *data, unsigned size);
	void  sig_ThumbnailsDone (unsigned nfiles);
	void  sig_Saved (unsigned i, bool success);
	void  sig_SavedAll();

  public slots:
	void  SaveAll (const char *prefix, const char *path);

  private:
	virtual void	run();

	QWaitCondition	    m_WaitCondition;
	QMutex					m_Mutex;
	Camera				  *m_Camera;
	std::string				m_Prefix;
	std::string				m_SavePath;
  };

#endif // LOADERTHREAD_H
