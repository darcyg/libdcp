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

/** @file  src/picture_asset.h
 *  @brief An asset made up of JPEG2000 files
 */

#include "asset.h"

namespace libdcp
{

/** @brief An asset made up of JPEG2000 files */
class PictureAsset : public Asset
{
public:
	/** Construct a PictureAsset, generating the MXF from the JPEG2000 files.
	 *  This may take some time; progress is indicated by emission of the Progress signal.
	 *  @param files Pathnames of JPEG2000 files, in frame order.
	 *  @param mxf_path Pathname of MXF file to create.
	 *  @param progress Signal to inform of progress.
	 *  @param fps Frames per second.
	 *  @param length Length in frames.
	 *  @param width Width of images in pixels.
	 *  @param height Height of images in pixels.
	 */
	PictureAsset (
		std::list<std::string> const & files,
		std::string mxf_path,
		sigc::signal1<void, float>* progress,
		int fps,
		int length,
		int width,
		int height
		);

	/** Write details of this asset to a CPL stream.
	 *  @param s Stream.
	 */
	void write_to_cpl (std::ostream& s) const;

private:
	/** picture width in pixels */
	int _width;
	/** picture height in pixels */
	int _height;
};

}
