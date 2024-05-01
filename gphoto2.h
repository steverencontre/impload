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

#ifndef GPHOTO2_H
#define GPHOTO2_H

// Not everything in gphoto2 has the gp_ prefix, so use a namespace. Beware that it refers to some standard
// headers, which if not already included will end up in the namespace too, causing strange compilation errors.
// Include any referenced standard headers here

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ltdl.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>

namespace gp {
#include <gphoto2/gphoto2.h>
}

#endif // GPHOTO2_H
