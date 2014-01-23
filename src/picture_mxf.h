/*
    Copyright (C) 2012 Carl Hetherington <cth@carlh.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef LIBDCP_PICTURE_MXF_H
#define LIBDCP_PICTURE_MXF_H

/** @file  src/picture_asset.h
 *  @brief An asset made up of JPEG2000 data
 */

#include <openjpeg.h>
#include "mxf.h"
#include "util.h"
#include "metadata.h"

namespace ASDCP {
	namespace JP2K {
		class PictureDescriptor;
	}
}

namespace dcp
{

class MonoPictureFrame;	
class StereoPictureFrame;
class PictureMXFWriter;

/** @brief An asset made up of JPEG2000 data */
class PictureMXF : public MXF
{
public:
	PictureMXF (boost::filesystem::path file);
	PictureMXF (int edit_rate);

	virtual boost::shared_ptr<PictureMXFWriter> start_write (boost::filesystem::path file, bool overwrite) = 0;

	Size size () const {
		return _size;
	}

	void set_size (Size s) {
		_size = s;
	}

	void write_to_cpl (xmlpp::Element *) const;

protected:	

	bool frame_buffer_equals (
		int frame, EqualityOptions opt, boost::function<void (NoteType, std::string)> note,
		uint8_t const * data_A, unsigned int size_A, uint8_t const * data_B, unsigned int size_B
		) const;

	bool descriptor_equals (
		ASDCP::JP2K::PictureDescriptor const & a, ASDCP::JP2K::PictureDescriptor const & b, boost::function<void (NoteType, std::string)>
		) const;

	/** picture size in pixels */
	Size _size;

private:
	std::string key_type () const;
	virtual int edit_rate_factor () const = 0;
};
	

}

#endif