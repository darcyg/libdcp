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

/** @file  src/encrypted_kdm.h
 *  @brief EncryptedKDM class.
 */

#ifndef LIBDCP_ENCRYPTED_KDM_H
#define LIBDCP_ENCRYPTED_KDM_H

#include "local_time.h"
#include "types.h"
#include <boost/filesystem.hpp>
#include <boost/date_time/local_time/local_time.hpp>

namespace cxml {
	class Node;
}

namespace dcp {

namespace data {
	class EncryptedKDMData;
}

class CertificateChain;
class Certificate;

/** @class EncryptedKDM
 *  @brief An encrypted KDM.
 *
 *  This is a KDM whose keys are encrypted using the target projector's private key.
 *  An EncryptedKDM object can be initialised from a KDM XML file, or created from
 *  a DecryptedKDM (using DecryptedKDM::encrypt).
 */
class EncryptedKDM
{
public:
	EncryptedKDM (std::string);
	EncryptedKDM (EncryptedKDM const & kdm);
	EncryptedKDM & operator= (EncryptedKDM const &);
	~EncryptedKDM ();

	/** Write this KDM as XML to a file.
	 *  @param file File to write to.
	 */
	void as_xml (boost::filesystem::path file) const;

	/** @return This KDM as XML */
	std::string as_xml () const;

	/** @return The base64-encoded and encrypted keys that this KDM delivers.
	 *  Note that the returned `keys' contain more than just the asset decryption
	 *  keys (also key id, CPL id etc.)
	 */
	std::list<std::string> keys () const;

	std::string annotation_text () const;
	std::string content_title_text () const;
	std::string issue_date () const;

private:

	friend class DecryptedKDM;

	/** Construct an EncryptedKDM from a set of details */
	EncryptedKDM (
		boost::shared_ptr<const CertificateChain> signer,
		Certificate recipient,
		std::string device_list_description,
		std::string cpl_id,
		std::string cpl_content_title_text,
		LocalTime not_valid_before,
		LocalTime not_valid_after,
		Formulation formulation,
		std::list<std::pair<std::string, std::string> > key_ids,
		std::list<std::string> keys
		);

	data::EncryptedKDMData* _data;
};

}

#endif
