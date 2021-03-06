/*
    Copyright (C) 2013-2015 Carl Hetherington <cth@carlh.net>

    This file is part of libdcp.

    libdcp is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    libdcp is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libdcp.  If not, see <http://www.gnu.org/licenses/>.

    In addition, as a special exception, the copyright holders give
    permission to link the code of portions of this program with the
    OpenSSL library under certain conditions as described in each
    individual source file, and distribute linked combinations
    including the two.

    You must obey the GNU General Public License in all respects
    for all of the code used other than OpenSSL.  If you modify
    file(s) with this exception, you may extend this exception to your
    version of the file(s), but you are not obligated to do so.  If you
    do not wish to do so, delete this exception statement from your
    version.  If you delete this exception statement from all source
    files in the program, then also delete it here.
*/

#include "types.h"
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <stdint.h>

namespace dcp {

class OpenJPEGImage;
class Image;
class ColourConversion;

extern void xyz_to_rgba (
	boost::shared_ptr<const OpenJPEGImage>,
	ColourConversion const & conversion,
	uint8_t* rgba,
	int stride
	);

extern void xyz_to_rgb (
	boost::shared_ptr<const OpenJPEGImage>,
	ColourConversion const & conversion,
	uint8_t* rgb,
	int stride,
	boost::optional<NoteHandler> note = boost::optional<NoteHandler> ()
	);

extern boost::shared_ptr<OpenJPEGImage> rgb_to_xyz (
	uint8_t const * rgb,
	dcp::Size size,
	int stride,
	ColourConversion const & conversion,
	boost::optional<NoteHandler> note = boost::optional<NoteHandler> ()
	);

extern void combined_rgb_to_xyz (ColourConversion const & conversion, double* matrix);

}
