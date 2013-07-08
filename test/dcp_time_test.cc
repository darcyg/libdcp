/*
    Copyright (C) 2013 Carl Hetherington <cth@carlh.net>

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

/** Check that libdcp::Time works */
BOOST_AUTO_TEST_CASE (dcp_time)
{
	libdcp::Time t (977143, 24);

	BOOST_CHECK_EQUAL (t.t, 73);
	BOOST_CHECK_EQUAL (t.s, 34);
	BOOST_CHECK_EQUAL (t.m, 18);
	BOOST_CHECK_EQUAL (t.h, 11);
	BOOST_CHECK_EQUAL (t.to_string(), "11:18:34:73");
	BOOST_CHECK_EQUAL (t.to_ticks(), 1017923);

	libdcp::Time a (3, 2, 3, 4);
	libdcp::Time b (2, 3, 4, 5);

	libdcp::Time r = a - b;
	BOOST_CHECK_EQUAL (r.h, 0);
	BOOST_CHECK_EQUAL (r.m, 58);
	BOOST_CHECK_EQUAL (r.s, 58);
	BOOST_CHECK_EQUAL (r.t, 249);
	BOOST_CHECK_EQUAL (r.to_string(), "0:58:58:249");
	BOOST_CHECK_EQUAL (r.to_ticks(), 88699);

	a = libdcp::Time (1, 58, 56, 240);
	b = libdcp::Time (1, 7, 12, 120);
	r = a + b;
	BOOST_CHECK_EQUAL (r.h, 3);
	BOOST_CHECK_EQUAL (r.m, 6);
	BOOST_CHECK_EQUAL (r.s, 9);
	BOOST_CHECK_EQUAL (r.t, 110);
	BOOST_CHECK_EQUAL (r.to_string(), "3:6:9:110");
	BOOST_CHECK_EQUAL (r.to_ticks(), 279335);

	a = libdcp::Time (24, 12, 6, 3);
	b = libdcp::Time (16, 8, 4, 2);
	BOOST_CHECK_CLOSE (a / b, 1.5, 1e-5);
}