/*
    Copyright (C) 2012-2017 Carl Hetherington <cth@carlh.net>

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

/** @file  src/reel_subtitle_asset.cc
 *  @brief ReelSubtitleAsset class.
 */

#include "subtitle_asset.h"
#include "reel_subtitle_asset.h"
#include "smpte_subtitle_asset.h"
#include <libxml++/libxml++.h>

using std::string;
using boost::shared_ptr;
using boost::dynamic_pointer_cast;
using boost::optional;
using namespace dcp;

ReelSubtitleAsset::ReelSubtitleAsset (boost::shared_ptr<SubtitleAsset> asset, Fraction edit_rate, int64_t intrinsic_duration, int64_t entry_point)
	: ReelAsset (asset, edit_rate, intrinsic_duration, entry_point)
	, ReelMXF (dynamic_pointer_cast<SMPTESubtitleAsset>(asset) ? dynamic_pointer_cast<SMPTESubtitleAsset>(asset)->key_id() : optional<string>())
{

}

ReelSubtitleAsset::ReelSubtitleAsset (boost::shared_ptr<const cxml::Node> node)
	: ReelAsset (node)
	, ReelMXF (node)
{
	node->ignore_child ("Language");
	node->done ();
}

string
ReelSubtitleAsset::cpl_node_name (Standard) const
{
	return "MainSubtitle";
}

string
ReelSubtitleAsset::key_type () const
{
	return "MDSK";
}

xmlpp::Node *
ReelSubtitleAsset::write_to_cpl (xmlpp::Node* node, Standard standard) const
{
	xmlpp::Node* asset = ReelAsset::write_to_cpl (node, standard);

	if (key_id ()) {
		/* Find <Hash> */
		xmlpp::Node* hash = find_child (asset, "Hash");
		asset->add_child_before(hash, "KeyId")->add_child_text("urn:uuid:" + key_id().get());
	}

	return asset;
}
