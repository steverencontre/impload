
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


	auto dtiter = ed.findKey (s_DTOKey);
	auto ssiter = ed.findKey (s_SSOKey);
	if (dtiter == ed.end())
	{
		dtiter = ed.findKey (s_DTDKey);
		ssiter = ed.findKey (s_SSDKey);
	}
	if (dtiter == ed.end())
	{
		std::cout << ed.count() << std::endl;
		for (const auto& key : ed)
			std::cout << key.key() << std::endl;
		throw std::runtime_error ("Can't find date/time in EXIF");
	}
	auto dts = dtiter->getValue()->toString();

	auto ostiter = ed.findKey (s_OSTKey);
	if (ostiter != ed.end())
		dts += ostiter->getValue()->toString();
	else
		dts += "Z";

	auto dt {QDateTime::fromString (QString::fromStdString (dts), Qt::ISODate)};

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
	static enum { NONE, CR2, BMFF} mode {NONE};

	auto memio {new Exiv2::MemIo {(const Exiv2::byte *) data, size}};
	Exiv2::Image *image;

	auto px = (const uint8_t *) data;

	if (px[0] == 0xFF && px[1] == 0xD8)		// file is jpeg
		image = new Exiv2::JpegImage (Exiv2::MemIo::UniquePtr {memio}, false);
	else switch (mode)
	{
	case NONE:
		image = new Exiv2::Cr2Image (Exiv2::MemIo::UniquePtr {memio}, false);
		if (image->good())
			mode = CR2;
		else
		{
			if (!Exiv2::enableBMFF())
				std::cerr << "Recompile with BMFF support!!!\n";
			image = new Exiv2::BmffImage (Exiv2::MemIo::UniquePtr {memio}, false);
			if (image->good())
				mode = BMFF;
			else
			{
				std::cerr << "Unknown image type\n";
				throw std::runtime_error ("Unknown image type");
			}
		}
		break;

	case CR2:
		image = new Exiv2::Cr2Image (Exiv2::MemIo::UniquePtr {memio}, false);
		break;

	case BMFF:
		image = new Exiv2::BmffImage (Exiv2::MemIo::UniquePtr {memio}, false);
		break;
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
