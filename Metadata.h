#ifndef METADATA_H
#define METADATA_H

/*!	\brief		Originally a wrapper to use Exiv2 if possible, or ExifTool C++ binding if not.
 *					The latter was less efficient but Exiv2 didn't support the Canon .CR3 format
 *					at the time. This is no longer the case, but now we've added MediaInfo to
 *					get video metadata.
 */

#include <QDateTime>

#include <memory>


class MetadataOps
{
public:
  virtual ~MetadataOps() = default;

  virtual QDateTime Timestamp() const = 0;
  virtual void Timestamp (QDateTime) = 0;

  virtual int Orientation() const = 0;
  virtual void Orientation (int); 		  // not used at present
};


class Metadata : public MetadataOps
{
public:
	Metadata (const void *data, size_t size, bool video=false);

	QDateTime Timestamp() const override		{ return m_Delegate->Timestamp(); }
	void Timestamp (QDateTime dt) override	{ m_Delegate->Timestamp (dt); }

	int Orientation() const override				{ return m_Delegate->Orientation(); }
	void Orientation (int o) override			{ m_Delegate->Orientation (o); }

	template <typename T>
	static QDateTime Timestamp (const T& ed);

private:
  std::unique_ptr <MetadataOps>		m_Delegate;
};

#endif // METADATA_H
