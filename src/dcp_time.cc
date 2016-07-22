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

/** @file  src/dcp_time.cc
 *  @brief Time class.
 */

#include "raw_convert.h"
#include "dcp_time.h"
#include "exceptions.h"
#include "compose.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;
using namespace boost;
using namespace dcp;

Time::Time (int frame, double frames_per_second, int tcr_)
{
	set (double (frame) / frames_per_second, tcr_);
}

/** Construct a Time from a number of seconds and a timecode rate.
 *
 *  @param seconds A number of seconds.
 *  @param tcr_ Timecode rate.
 */
Time::Time (double seconds, int tcr_)
{
	set (seconds, tcr_);
}

/** Construct a Time with specified timecode rate and using the supplied
 *  number of seconds.
 *
 *  @param seconds A number of seconds.
 *  @param tcr_ Timecode rate to use.
 */
void
Time::set (double seconds, int tcr_)
{
	s = floor (seconds);
	tcr = tcr_;

	e = int (round ((seconds - s) * tcr));

	if (s >= 60) {
		m = s / 60;
		s -= m * 60;
	} else {
		m = 0;
	}

	if (m >= 60) {
		h = m / 60;
		m -= h * 60;
	} else {
		h = 0;
	}
}

/** @param time String of the form
 *     HH:MM:SS:EE                          for SMPTE
 *     HH:MM:SS:E[E[E]] or HH:MM:SS.s[s[s]] for Interop
 *  where HH are hours, MM minutes, SS seconds, EE editable units and
 *  sss millseconds.
 *
 *  @param tcr_ Timecode rate if this is a SMPTE time, otherwise empty for an Interop time.
 */
Time::Time (string time, optional<int> tcr_)
{
	vector<string> b;
	split (b, time, is_any_of (":"));

	if (b.size() < 3 || b[0].empty() || b[1].empty() || b[0].length() > 2 || b[1].length() > 2) {
		boost::throw_exception (DCPReadError (String::compose ("unrecognised time specification %1", time)));
	}

	if (!tcr_) {
		/* Interop */
		if (b.size() == 3) {
			/* HH:MM:SS.s[s[s]] */
			vector<string> bs;
			split (bs, b[2], is_any_of ("."));
			if (bs.size() != 2) {
				boost::throw_exception (DCPReadError (String::compose ("unrecognised time specification %1", time)));
			}

			h = raw_convert<int> (b[0]);
			m = raw_convert<int> (b[1]);
			if (bs[0].empty() || bs[0].length() > 2) {
				boost::throw_exception (DCPReadError (String::compose ("unrecognised time specification %1; %2 has bad length", time, bs[0])));
			}
			s = raw_convert<int> (bs[0]);
			if (bs[1].empty() || bs[1].length() > 3) {
				boost::throw_exception (DCPReadError (String::compose ("unrecognised time specification %1; %2 has bad length", time, bs[1])));
			}
			e = raw_convert<int> (bs[1]);
			tcr = 1000;
		} else if (b.size() == 4) {
			/* HH:MM:SS:EE[E] */
			h = raw_convert<int> (b[0]);
			m = raw_convert<int> (b[1]);
			if (b[2].empty() || b[2].length() > 2) {
				boost::throw_exception (DCPReadError (String::compose ("unrecognised time specification %1; %2 has bad length", time, b[2])));
			}
			s = raw_convert<int> (b[2]);
			if (b[3].empty() || b[3].length() > 3) {
				boost::throw_exception (DCPReadError (String::compose ("unrecognised time specification %1; %2 has bad length", time, b[3])));
			}
			e = raw_convert<int> (b[3]);
			tcr = 250;
		} else {
			boost::throw_exception (DCPReadError (String::compose ("unrecognised time specification %1", time)));
		}

	} else {
		/* SMPTE: HH:MM:SS:EE */
		split (b, time, is_any_of (":"));
		if (b.size() != 4) {
			boost::throw_exception (DCPReadError (String::compose ("unrecognised time specification %1; does not have 4 parts", time)));
		}

		h = raw_convert<int> (b[0]);
		m = raw_convert<int> (b[1]);
		if (b[2].empty() || b[2].length() > 2) {
			boost::throw_exception (DCPReadError (String::compose ("unrecognised time specification %1; %2 has bad length", time, b[2])));
		}
		s = raw_convert<int> (b[2]);
		if (b[3].empty() || b[3].length() > 2) {
			boost::throw_exception (DCPReadError (String::compose ("unrecognised time specification %1; %2 has bad length", time, b[3])));
		}
		e = raw_convert<int> (b[3]);
		tcr = tcr_.get();
	}
}

bool
dcp::operator== (Time const & a, Time const & b)
{
	return (a.h == b.h && a.m == b.m && a.s == b.s && (a.e * b.tcr) == (b.e * a.tcr));
}

bool
dcp::operator!= (Time const & a, Time const & b)
{
	return !(a == b);
}

bool
dcp::operator<= (Time const & a, Time const & b)
{
	return a < b || a == b;
}

bool
dcp::operator>= (Time const & a, Time const & b)
{
	return a > b || a == b;
}

bool
dcp::operator< (Time const & a, Time const & b)
{
	if (a.h != b.h) {
		return a.h < b.h;
	}

	if (a.m != b.m) {
		return a.m < b.m;
	}

	if (a.s != b.s) {
		return a.s < b.s;
	}

	if ((a.e * b.tcr) != (b.e * a.tcr)) {
		return (a.e * b.tcr) < (b.e * a.tcr);
	}

	return true;
}

bool
dcp::operator> (Time const & a, Time const & b)
{
	if (a.h != b.h) {
		return a.h > b.h;
	}

	if (a.m != b.m) {
		return a.m > b.m;
	}

	if (a.s != b.s) {
		return a.s > b.s;
	}

	if ((a.e * b.tcr) != (b.e * a.tcr)) {
		return (a.e * b.tcr) > (b.e * a.tcr);
	}

	return true;
}

ostream &
dcp::operator<< (ostream& s, Time const & t)
{
	s << t.h << ":" << t.m << ":" << t.s << "." << t.e;
	return s;
}

dcp::Time
dcp::operator+ (Time a, Time b)
{
	Time r;

	/* Make sure we have a common tcr */
	if (a.tcr != b.tcr) {
		a.e *= b.tcr;
		b.e *= a.tcr;
		r.tcr = a.tcr * b.tcr;
	} else {
		r.tcr = a.tcr;
	}

	r.e = a.e + b.e;
	if (r.e >= r.tcr) {
		r.e -= r.tcr;
		r.s++;
	}

	r.s += a.s + b.s;
	if (r.s >= 60) {
		r.s -= 60;
		r.m++;
	}

	r.m += a.m + b.m;
	if (r.m >= 60) {
		r.m -= 60;
		r.h++;
	}

	r.h += a.h + b.h;

	return r;
}

dcp::Time
dcp::operator- (Time a, Time b)
{
	Time r;

	/* Make sure we have a common tcr */
	if (a.tcr != b.tcr) {
		a.e *= b.tcr;
		b.e *= a.tcr;
		r.tcr = a.tcr * b.tcr;
	} else {
		r.tcr = a.tcr;
	}

	r.e = a.e - b.e;
	if (r.e < 0) {
		r.e += r.tcr;
		r.s--;
	}

	r.s += (a.s - b.s);
	if (r.s < 0) {
		r.s += 60;
		r.m--;
	}

	r.m += (a.m - b.m);
	if (r.m < 0) {
		r.m += 60;
		r.h--;
	}

	r.h += (a.h - b.h);

	return r;
}

float
dcp::operator/ (Time a, Time const & b)
{
	int64_t const at = a.h * 3600 + a.m * 60 + a.s * float (a.e) / a.tcr;
	int64_t const bt = b.h * 3600 + b.m * 60 + b.s * float (b.e) / b.tcr;
	return float (at) / bt;
}

/** @return A string of the form h:m:s:e padded as in 00:00:00:000 (for Interop) or 00:00:00:00 (for SMPTE) */
string
Time::as_string (Standard standard) const
{
	locked_stringstream str;
	str << setw(2) << setfill('0') << h << ":"
	    << setw(2) << setfill('0') << m << ":"
	    << setw(2) << setfill('0') << s << ":";

	if (standard == SMPTE) {
		str << setw(2) << setfill('0') << e;
	} else {
		str << setw(3) << setfill('0') << e;
	}

	return str.str ();
}

/** @param tcr_ Timecode rate with which the return value should be counted.
 *  @return the total number of editable units that this time consists of at the specified timecode rate, rounded up
 *  to the nearest editable unit. For example, as_editable_units (24) returns the total time in frames at 24fps.
 */
int64_t
Time::as_editable_units (int tcr_) const
{
	return ceil (int64_t(e) * double (tcr_) / tcr) + int64_t(s) * tcr_ + int64_t(m) * 60 * tcr_ + int64_t(h) * 60 * 60 * tcr_;
}

/** @return the total number of seconds that this time consists of */
double
Time::as_seconds () const
{
	return h * 3600 + m * 60 + s + double(e) / tcr;
}

/** @param tcr_ New timecode rate.
 *  @return A new Time which is this time at the spcified new timecode rate.
 */
Time
Time::rebase (int tcr_) const
{
	return Time (h, m, s, floor (float (e) * tcr_ / tcr), tcr_);
}
