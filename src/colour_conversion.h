/*
    Copyright (C) 2014-2015 Carl Hetherington <cth@carlh.net>

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

/** @file  src/colour_conversion.h
 *  @brief ColourConversion class.
 */

#ifndef DCP_COLOUR_CONVERSION_H
#define DCP_COLOUR_CONVERSION_H

#include "chromaticity.h"
#include <boost/shared_ptr.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/optional.hpp>

namespace dcp {

class TransferFunction;

enum YUVToRGB {
	YUV_TO_RGB_REC601,
	YUV_TO_RGB_REC709,
	YUV_TO_RGB_COUNT
};

/** @class ColourConversion
 *  @brief A representation of all the parameters involved the colourspace conversion
 *  of a YUV image to XYZ (via RGB).
 */
class ColourConversion
{
public:
	ColourConversion ()
		: _yuv_to_rgb (YUV_TO_RGB_REC601)
	{}

	ColourConversion (
		boost::shared_ptr<const TransferFunction> in,
		YUVToRGB yuv_to_rgb,
		Chromaticity red,
		Chromaticity green,
		Chromaticity blue,
		Chromaticity white,
		boost::optional<Chromaticity> adjusted_white,
		boost::shared_ptr<const TransferFunction> out
		);

	boost::shared_ptr<const TransferFunction> in () const {
		return _in;
	}

	YUVToRGB yuv_to_rgb () const {
		return _yuv_to_rgb;
	}

	Chromaticity red () const {
		return _red;
	}

	Chromaticity green () const {
		return _green;
	}

	Chromaticity blue () const {
		return _blue;
	}

	Chromaticity white () const {
		return _white;
	}

	boost::optional<Chromaticity> adjusted_white () const {
		return _adjusted_white;
	}

	boost::shared_ptr<const TransferFunction> out () const {
		return _out;
	}

	void set_in (boost::shared_ptr<const TransferFunction> f) {
		_in = f;
	}

	void set_yuv_to_rgb (YUVToRGB y) {
		_yuv_to_rgb = y;
	}

	void set_red (Chromaticity red) {
		_red = red;
	}

	void set_green (Chromaticity green) {
		_green = green;
	}

	void set_blue (Chromaticity blue) {
		_blue = blue;
	}

	void set_white (Chromaticity white) {
		_white = white;
	}

	void set_adjusted_white (Chromaticity adjusted_white) {
		_adjusted_white = adjusted_white;
	}

	void unset_adjusted_white () {
		_adjusted_white = boost::optional<Chromaticity> ();
	}

	void set_out (boost::shared_ptr<const TransferFunction> f) {
		_out = f;
	}

	bool about_equal (ColourConversion const & other, float epsilon) const;

	boost::numeric::ublas::matrix<double> rgb_to_xyz () const;
	boost::numeric::ublas::matrix<double> xyz_to_rgb () const;
	boost::numeric::ublas::matrix<double> bradford () const;

	static ColourConversion const & srgb_to_xyz ();
	static ColourConversion const & rec601_to_xyz ();
	static ColourConversion const & rec709_to_xyz ();
	static ColourConversion const & p3_to_xyz ();
	static ColourConversion const & rec1886_to_xyz ();
	static ColourConversion const & rec2020_to_xyz ();
	static ColourConversion const & s_gamut3_to_xyz ();

protected:
	/** Input transfer function (probably a gamma function, or something similar) */
	boost::shared_ptr<const TransferFunction> _in;
	/** Conversion to use from YUV to RGB */
	YUVToRGB _yuv_to_rgb;
	Chromaticity _red;
	Chromaticity _green;
	Chromaticity _blue;
	Chromaticity _white;
	/** White point that we are adjusting to using a Bradford matrix */
	boost::optional<Chromaticity> _adjusted_white;
	/** Output transfer function (probably an inverse gamma function, or something similar) */
	boost::shared_ptr<const TransferFunction> _out;
};

}

#endif
