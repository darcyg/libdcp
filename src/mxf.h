/*
    Copyright (C) 2012-2014 Carl Hetherington <cth@carlh.net>

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

#ifndef LIBDCP_MXF_H
#define LIBDCP_MXF_H

#include "asset.h"
#include "key.h"
#include "metadata.h"

#include <boost/signals2.hpp>

namespace ASDCP {
	class AESDecContext;
	struct WriterInfo;
}

/* Undefine some stuff that the OS X 10.5 SDK defines */
#undef Key
#undef set_key

namespace dcp
{

class MXFMetadata;
class PictureAssetWriter;

/** @class MXF
 *  @brief Parent for classes which represent MXF files.
 */
class MXF
{
public:
	virtual ~MXF () {}

	/** @return true if the data is encrypted */
	bool encrypted () const {
		return static_cast<bool>(_key_id);
	}

	/** Set the ID of the key that is used for encryption/decryption.
	 *  @param i key ID.
	 */
	void set_key_id (std::string i) {
		_key_id = i;
	}

	/** @return the ID of the key used for encryption/decryption, if there is one */
	boost::optional<std::string> key_id () const {
		return _key_id;
	}

	virtual void set_key (Key);

	/** @return encryption/decryption key, if one has been set */
	boost::optional<Key> key () const {
		return _key;
	}

	/** Set the metadata that is written to the MXF file.
	 *  @param m Metadata.
	 */
	void set_metadata (MXFMetadata m) {
		_metadata = m;
	}

	/** @return metadata from the MXF file */
	MXFMetadata metadata () const {
		return _metadata;
	}

protected:
	template <class P, class Q>
	friend void start (PictureAssetWriter* writer, boost::shared_ptr<P> state, Standard standard, Q* mxf, uint8_t* data, int size);

	std::string read_writer_info (ASDCP::WriterInfo const &);
	/** Fill in a ADSCP::WriteInfo struct.
	 *  @param w struct to fill in.
	 *  @param standard INTEROP or SMPTE.
	 */
	void fill_writer_info (ASDCP::WriterInfo* w, std::string id, Standard standard) const;

	/** ID of the key used for encryption/decryption, if there is one */
	boost::optional<std::string> _key_id;
	/** Key used for encryption/decryption, if there is one */
	boost::optional<Key> _key;
	MXFMetadata _metadata;
};

}

#endif
