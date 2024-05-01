#ifndef IMAGESOURCE_H
#define IMAGESOURCE_H

#include <cstdint>
#include <vector>
#include <string>
#include <tuple>

#include <QDateTime>

class ImageSource
{
public:
	ImageSource();
	virtual ~ImageSource() {}

	enum DataType { FULL, THUMB, EXIF, VIDEO, DATA_TYPES };

	struct FileListItem
	  {
		std::string		folder;
		std::string		name;

		bool operator< (const FileListItem& x) const { return name < x.name; }
	  };

	using ImageData = std::tuple<const uint8_t *, size_t, QDateTime>;

	const std::vector <FileListItem>& Files() const		  { return m_Files; }

	double	TimeOffset() const		{ return m_TimeOffset; }	// note that API allows for fractions of seconds
	void		TimeOffset (double t)	{ m_TimeOffset = t; }		// allow manual override
	ImageData		LoadData (unsigned index, DataType type);
	bool				SaveFile (unsigned index, const std::string& tag, const std::string& path, unsigned absnum);

	void ScanFiles()	{ AddFiles (m_BaseDir); }

protected:

	virtual void			AddFiles (const std::string& base) = 0;
	virtual ImageData	LoadData (const std::string& folder, const std::string& name, DataType type) = 0;
	virtual bool			WriteImageFile (const std::string& destname) = 0;

	std::string								m_BaseDir {"/"};		// ### need to fix this for file source
	std::vector <FileListItem>		m_Files;
	time_t									m_TimeOffset {0};
};
#endif // IMAGESOURCE_H
