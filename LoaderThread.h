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

#ifndef LOADERTHREAD_H
#define LOADERTHREAD_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

#include "ImageSource.h"

class LoaderThread : public QThread
{
	Q_OBJECT
public:
	explicit	LoaderThread (QObject *parent = nullptr);

	void		SetSource (ImageSource *source)	{ m_ImageSource = source; }
	void		Continue()	{ m_WaitCondition.wakeAll(); }

signals:
	void		sig_FileCount (unsigned nfiles);
	void		sig_NewThumbnail (unsigned index, const void *data, unsigned size, int orientation);
	void		sig_ThumbnailsDone();
	void		sig_SavedOne (unsigned index, bool success);
	void		sig_SavedAll();

public slots:
	void		Save (const char *tag, const char *path, unsigned first);

private:
	void		run() override;

	QWaitCondition		m_WaitCondition;
	QMutex					m_Mutex;
	ImageSource		  *m_ImageSource {nullptr};
	std::string				m_Tag;
	std::string				m_SavePath;
	size_t					m_First;
};

#endif // LOADERTHREAD_H
