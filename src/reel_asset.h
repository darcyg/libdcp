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

/** @file  src/reel_asset.h
 *  @brief ReelAsset class.
 */

#ifndef LIBDCP_REEL_ASSET_H
#define LIBDCP_REEL_ASSET_H

#include "object.h"
#include "util.h"
#include "ref.h"
#include <boost/shared_ptr.hpp>

namespace cxml {
	class Node;
}

namespace xmlpp {
	class Node;
}

namespace dcp {

class Asset;

/** @class ReelAsset
 *  @brief An entry in a &lt;Reel&gt; which refers to a use of a piece of content.
 *
 *  This class encapsulates the XML that exists in a &lt;Reel&gt; to say
 *  that a piece of content is used in this reel.  It does not
 *  describe the content itself (but links to an Asset object which does).
 */
class ReelAsset : public Object
{
public:
	ReelAsset ();
	ReelAsset (boost::shared_ptr<Asset> asset, Fraction edit_rate, int64_t intrinsic_duration, int64_t entry_point);
	explicit ReelAsset (boost::shared_ptr<const cxml::Node>);

	virtual xmlpp::Node* write_to_cpl (xmlpp::Node* node, Standard standard) const;
	virtual bool equals (boost::shared_ptr<const ReelAsset>, EqualityOptions, NoteHandler) const;

	/** @return a Ref to our actual asset */
	Ref const & asset_ref () const {
		return _asset_ref;
	}

	/** @return a Ref to our actual asset */
	Ref & asset_ref () {
		return _asset_ref;
	}

	Fraction edit_rate () const {
		return _edit_rate;
	}

	int64_t intrinsic_duration () const {
		return _intrinsic_duration;
	}

	void set_entry_point (int64_t e) {
		_entry_point = e;
	}

	int64_t entry_point () const {
		return _entry_point;
	}

	void set_duration (int64_t d) {
		_duration = d;
	}

	int64_t duration () const {
		return _duration;
	}

	/** @return the asset's hash, if this ReelAsset has been created from one,
	 *  otherwise the hash written to the CPL for this asset (if present).
	 */
	boost::optional<std::string> hash () const {
		return _hash;
	}

	std::string annotation_text () const {
		return _annotation_text;
	}

	void set_annotation_text (std::string at) {
		_annotation_text = at;
	}

protected:

	template <class T>
	boost::shared_ptr<T> asset_of_type () const {
		return boost::dynamic_pointer_cast<T> (_asset_ref.asset ());
	}

	template <class T>
	boost::shared_ptr<T> asset_of_type () {
		return boost::dynamic_pointer_cast<T> (_asset_ref.asset ());
	}

	/** @return the node name that this asset uses in the CPL's &lt;Reel&gt; node
	 *  e.g. MainPicture, MainSound etc.
	 */
	virtual std::string cpl_node_name (Standard) const = 0;

	/** @return Any attribute that should be used on the asset's node in the CPL */
	virtual std::pair<std::string, std::string> cpl_node_attribute (Standard) const;

	/** @return Any namespace that should be used on the asset's node in the CPL */
	virtual std::pair<std::string, std::string> cpl_node_namespace (Standard) const;

	/** Reference to the asset (MXF or XML file) that this reel entry
	 *  applies to.
	 */
	Ref _asset_ref;

private:
	std::string _annotation_text; ///< The &lt;AnnotationText&gt; from the reel's entry for this asset
	Fraction _edit_rate;          ///< The &lt;EditRate&gt; from the reel's entry for this asset
	int64_t _intrinsic_duration;  ///< The &lt;IntrinsicDuration&gt; from the reel's entry for this asset
	int64_t _entry_point;         ///< The &lt;EntryPoint&gt; from the reel's entry for this asset
	int64_t _duration;            ///< The &lt;Duration&gt; from the reel's entry for this asset
	/** Either our asset's computed hash or the hash read in from the CPL, if it's present */
	boost::optional<std::string> _hash;
};

}

#endif
