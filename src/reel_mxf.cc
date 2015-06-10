/*
    Copyright (C) 2012-2015 Carl Hetherington <cth@carlh.net>

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

#include "reel_mxf.h"
#include "mxf.h"
#include "dcp_assert.h"
#include <libcxml/cxml.h>
#include <libxml++/libxml++.h>

using std::string;
using boost::shared_ptr;
using boost::optional;
using namespace dcp;

ReelMXF::ReelMXF (optional<string> key_id)
	: _key_id (key_id)
{

}

ReelMXF::ReelMXF (shared_ptr<const cxml::Node> node)
	: _key_id (node->optional_string_child ("KeyId"))
{
	if (_key_id && _key_id.get().length() > 9) {
		_key_id = _key_id.get().substr (9);
	}
}