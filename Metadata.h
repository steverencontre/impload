#ifndef EXIFWRAPPER_H
#define EXIFWRAPPER_H

/*!	\brief		Historical wrapper to use Exiv2 if possible, or ExifTool C++ binding if not.
 *					This is no longer relevant now that Exiv2 supports Canon .CR3 format and
 *					should probably be removed for clarity.
 */

#include <QDateTime>

#include <memory>

#define EXIV2API
#include <exiv2/exiv2.hpp>


class ExifOps
{
public:
  virtual ~ExifOps() = default;

  virtual QDateTime Timestamp() const = 0;
  virtual void Timestamp (QDateTime) = 0;

  virtual int Orientation() const = 0;
  virtual void Orientation (int); 		  // not used at present
};


class Metadata : public ExifOps
{
public:
	Metadata (const void *data, size_t size);

	QDateTime Timestamp() const override		{ return m_Delegate->Timestamp(); }
	void Timestamp (QDateTime dt) override	{ m_Delegate->Timestamp (dt); }

	int Orientation() const override				{ return m_Delegate->Orientation(); }
	void Orientation (int o) override			{ m_Delegate->Orientation (o); }

	template <typename T>
	static QDateTime Timestamp (const T& ed);

private:
  std::unique_ptr <ExifOps>		m_Delegate;
};

#endif // EXIFWRAPPER_H
