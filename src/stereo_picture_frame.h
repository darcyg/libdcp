/*
    Copyright (C) 2012-2016 Carl Hetherington <cth@carlh.net>

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

#ifndef LIBDCP_STEREO_PICTURE_FRAME_H
#define LIBDCP_STEREO_PICTURE_FRAME_H

#include "types.h"
#include "asset_reader.h"
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <stdint.h>
#include <string>

namespace ASDCP {
	namespace JP2K {
		struct SFrameBuffer;
		class MXFSReader;
	}
	class AESDecContext;
}

namespace dcp {

class OpenJPEGImage;

/** A single frame of a 3D (stereoscopic) picture asset */
class StereoPictureFrame : public boost::noncopyable
{
public:
	StereoPictureFrame ();
	~StereoPictureFrame ();

	boost::shared_ptr<OpenJPEGImage> xyz_image (Eye eye, int reduce = 0) const;

	uint8_t const * left_j2k_data () const;
	uint8_t* left_j2k_data ();
	int left_j2k_size () const;

	uint8_t const * right_j2k_data () const;
	uint8_t* right_j2k_data ();
	int right_j2k_size () const;

private:
	/* XXX: this is a bit of a shame, but I tried friend StereoPictureAssetReader and it's
	   rejected by some (seemingly older) GCCs.
	*/
	friend class AssetReader<ASDCP::JP2K::MXFSReader, StereoPictureFrame>;

	StereoPictureFrame (ASDCP::JP2K::MXFSReader* reader, int n, boost::shared_ptr<DecryptionContext>);

	ASDCP::JP2K::SFrameBuffer* _buffer;
};

}

#endif
