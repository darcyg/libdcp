/*
    Copyright (C) 2013-2014 Carl Hetherington <cth@carlh.net>

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

#include <boost/shared_ptr.hpp>
#include <stdint.h>

namespace dcp {

class ARGBFrame;	
class XYZFrame;
class GammaLUT;
class Image;
	
extern boost::shared_ptr<ARGBFrame> xyz_to_rgba (
	boost::shared_ptr<const XYZFrame>, boost::shared_ptr<const GammaLUT>, boost::shared_ptr<const GammaLUT>
	);

extern void xyz_to_rgb (
	boost::shared_ptr<const XYZFrame>, boost::shared_ptr<const GammaLUT>, boost::shared_ptr<const GammaLUT>, uint8_t* buffer
	);
	
extern boost::shared_ptr<XYZFrame> rgb_to_xyz (
	boost::shared_ptr<const Image>, boost::shared_ptr<const GammaLUT>, boost::shared_ptr<const GammaLUT>, double const colour_matrix[3][3]
	);
	
}
