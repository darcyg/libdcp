/*
    Copyright (C) 2012-2014 Carl Hetherington <cth@carlh.net>

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

#ifndef LIBDCP_TRANSFER_FUNCTION_H
#define LIBDCP_TRANSFER_FUNCTION_H

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <map>

namespace dcp {

class TransferFunction : public boost::noncopyable
{
public:
	TransferFunction (bool inverse);
	
	virtual ~TransferFunction ();

	/** @return A look-up table (of size 2^bit_depth) whose values range from 0 to 1 */
	double const * lut (int bit_depth) const;

	virtual bool about_equal (boost::shared_ptr<const TransferFunction> other, double epsilon) const;

protected:
	virtual double * make_lut (int bit_depth) const = 0;

	bool _inverse;

private:	
	mutable std::map<int, double*> _luts;
};

}

#endif
