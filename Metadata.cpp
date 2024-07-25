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

#include "Metadata.h"

#define EXIV2API
#include <exiv2/exiv2.hpp>

#include <MediaInfo/MediaInfo.h>

#include <iostream>
#include <fstream>
#include <exception>

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>





template<>
QDateTime Metadata::Timestamp (const Exiv2::ExifData& ed)
{
	const Exiv2::ExifKey s_DTOKey {"Exif.Photo.DateTimeOriginal"};
	const Exiv2::ExifKey s_SSOKey {"Exif.Photo.SubSecTimeOriginal"};

	const Exiv2::ExifKey s_DTDKey {"Exif.Photo.DateTimeDigitized"};
	const Exiv2::ExifKey s_SSDKey {"Exif.Photo.SubSecTimeDigitized"};

	const Exiv2::ExifKey s_OSTKey {"Exif.Photo.OffsetTime"};
	const Exiv2::ExifKey s_GTSKey ("Exif.GPSInfo.GPSTimeStamp");

	// get DateTimeOriginal if possible, else DateTimeDigitized

	auto iter = ed.findKey (s_DTOKey);
	auto ssiter = ed.findKey (s_SSOKey);
	if (iter == ed.end())
	{
		iter = ed.findKey (s_DTDKey);
		ssiter = ed.findKey (s_SSDKey);
	}
	if (iter == ed.end())
	{
		std::cout << ed.count() << std::endl;
		for (const auto& key : ed)
			std::cout << key.key() << std::endl;
		throw std::runtime_error ("Can't find date/time in EXIF");
	}

	std::string dts = iter->getValue()->toString();

	// get timezone offset if possible

	iter = ed.findKey (s_OSTKey);
	if (iter != ed.end())						// got an explicit offset value
		dts += iter->getValue()->toString();
	else
		dts += "Z";								// assume UTC unless we can deduce from GPS

	auto dt {QDateTime::fromString (QString::fromStdString (dts), Qt::ISODate)};

	if (iter == ed.end())						// not explicit offset, GPS?
	{
		iter = ed.findKey (s_GTSKey);
		if (iter != ed.end())
		{
			auto h = iter->toFloat (0);
			auto m = iter->toFloat (1);

			auto dh = dt.time().hour() - (int) h;
			auto dm = dt.time().minute() - (int) m;

			dm = ((dm + 7) / 15) * 15;		// round to nearest 15 min to get timezone offset (usually whole hours, sometimes half, _very_ occasionally quater)

			dt = dt.addSecs (-(dh * 3600 + dm * 60));
		}
	}

	if (ssiter != ed.end())
		dt = dt.addMSecs (ssiter->getValue()->toInt64());

	return dt;
}

void MetadataOps::Orientation (int) {}	  // not presently used

/*
 * Exiv2 wrapper
 */

class MetadataExiv2 : public MetadataOps
{
public:
  MetadataExiv2 (const void *data, size_t size);

  QDateTime Timestamp() const override { return Metadata::Timestamp (m_ExifData); }
  void Timestamp (QDateTime) override;

  int Orientation() const override;
//  void Orientation (int) override;

private:
  std::unique_ptr <Exiv2::Image> m_ImagePtr;
  Exiv2::ExifData			  m_ExifData;

};



MetadataExiv2::MetadataExiv2 (const void *data, size_t size)
{
	assert (Exiv2::enableBMFF());

	enum ImageType { NONE, JPEG, CR2, BMFF, ORF };

	static ImageType last_type {NONE};
	ImageType type {NONE};

	auto px = (const uint8_t *) data;
	if (px[0] == 0xFF && px[1] == 0xD8)		// file is jpeg
		type = JPEG;
	else
		type = last_type;

	Exiv2::MemIo *io {nullptr};
	Exiv2::Image *image {nullptr};

	switch (type)
	{
	case JPEG:
		io = new Exiv2::MemIo {(const Exiv2::byte *) data, size};
		image = new Exiv2::JpegImage (Exiv2::MemIo::UniquePtr {io}, false);
		break;

	case NONE:		// we'll just drop through until we find something that works
	case CR2:
		io = new Exiv2::MemIo {(const Exiv2::byte *) data, size};
		image = new Exiv2::Cr2Image (Exiv2::MemIo::UniquePtr {io}, false);
		if (image->good())		// once raw type has been set it should never change, so failure is only possible when type is unknown
		{
			last_type = CR2;
			break;
		}
		[[fallthrough]];

	case BMFF:
		io = new Exiv2::MemIo {(const Exiv2::byte *) data, size};
		image = new Exiv2::BmffImage (Exiv2::MemIo::UniquePtr {io}, false);
		if (image->good())
		{
			last_type = BMFF;
			break;
		}
		[[fallthrough]];

	case ORF:
		io = new Exiv2::MemIo {(const Exiv2::byte *) data, size};
		image = new Exiv2::OrfImage (Exiv2::MemIo::UniquePtr {io}, false);
		if (image->good())
		{
			last_type = ORF;
			break;
		}
	}

	if (!image || !image->good())
	{
		std::cerr << "Unknown type or apparent change of raw type!" << std::endl;
		throw std::runtime_error ("Image type error");
	}

	m_ImagePtr.reset (image);
	m_ImagePtr->readMetadata();
	m_ExifData = m_ImagePtr->exifData();
}


int MetadataExiv2::Orientation() const
{
	const Exiv2::ExifKey s_OrientationKey {"Exif.Image.Orientation"};

	int orientation = 0;

	auto o = m_ExifData.findKey (s_OrientationKey);
	if (o != m_ExifData.end())
		orientation = o->value().toInt64();

	return orientation;
}



void MetadataExiv2::Timestamp (QDateTime dt)
{
	auto dtstr = dt.toString (Qt::ISODate);
	m_ExifData ["Exif.Photo.DateTimeDigitized"] = dtstr.toStdString();

	try
	{
		m_ImagePtr->writeMetadata();
	}
	catch (const std::exception& x)
	{
		std::cerr << x.what() << std::endl;
	}

  // #### write exif update
}


/*
	MediaInfo wrapper
*/

using namespace MediaInfoLib;

class MetadataMediaInfo	: public MetadataOps, private MediaInfo
{
public:
  MetadataMediaInfo (const void *data, size_t size);

  QDateTime Timestamp() const override;
  void Timestamp (QDateTime) override {}

  int Orientation() const override { return 0; }
//  void Orientation (int) override;

private:


};




MetadataMediaInfo::MetadataMediaInfo (const void *data, size_t size)
{
	auto x = Open ((ZenLib::int8u *) data, size);
	std::cout << x << " 0x" << std::hex <<  x << std::dec << std::endl;
	std::cout << Inform() << std::endl;
}

QDateTime MetadataMediaInfo::Timestamp() const
{
	auto p = const_cast <MetadataMediaInfo *> (this);

	String param {"Encoded_Date"};
	QString sdate {QString::fromStdString (p->Get (Stream_General, 0, param))};

	if (sdate.endsWith (" UTC"))
	{
		sdate.chop (4);
		sdate += "Z";
	}

	auto qdate {QDateTime::fromString (sdate, Qt::ISODate)};

	if (qdate.isNull() || !qdate.isValid())
	{
		std::cerr << "Couldn't translate '" << sdate.toStdString() << "' as date" << std::endl;
		throw std::runtime_error ("MediaInfo date problem");
	}

	return qdate;
}


/*
 * Metadata Wrapper
 */

Metadata::Metadata (const void *data, size_t size, bool video)
{
	if (video)
		m_Delegate = std::make_unique <MetadataMediaInfo> (data, size);
	else
		m_Delegate = std::make_unique <MetadataExiv2> (data, size);
}
