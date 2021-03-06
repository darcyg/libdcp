/*
    Copyright (C) 2017 Carl Hetherington <cth@carlh.net>

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

#include "encrypted_kdm.h"
#include "decrypted_kdm.h"
#include "util.h"
#include "exceptions.h"
#include <boost/foreach.hpp>
#include <getopt.h>

using std::string;
using std::cout;
using std::cerr;
using boost::optional;

static void
help (string n)
{
	cerr << "Syntax: " << n << " [OPTION] <KDM>]\n"
	     << "  -h, --help         show this help\n"
	     << "  -p, --private-key  private key file\n";
}

int
main (int argc, char* argv[])
{
	optional<boost::filesystem::path> private_key_file;

	int option_index = 0;
	while (true) {
		struct option long_options[] = {
			{ "help", no_argument, 0, 'h' },
			{ "private-key", required_argument, 0, 'p'},
			{ 0, 0, 0, 0 }
		};

		int c = getopt_long (argc, argv, "hp:", long_options, &option_index);

		if (c == -1) {
			break;
		}

		switch (c) {
		case 'h':
			help (argv[0]);
			exit (EXIT_SUCCESS);
		case 'p':
			private_key_file = optarg;
			break;
		}
	}

	if (optind >= argc) {
		help (argv[0]);
		exit (EXIT_FAILURE);
	}

	boost::filesystem::path kdm_file = argv[optind];

	dcp::EncryptedKDM enc_kdm (dcp::file_to_string (kdm_file));

	if (enc_kdm.annotation_text()) {
		cout << "Annotation:    " << enc_kdm.annotation_text().get() << "\n";
	}
	cout << "Content title: " << enc_kdm.content_title_text() << "\n";
	cout << "CPL id:        " << enc_kdm.cpl_id() << "\n";
	cout << "Recipient:     " << enc_kdm.recipient_x509_subject_name() << "\n";

	if (private_key_file) {
		try {
			dcp::DecryptedKDM dec_kdm (enc_kdm, dcp::file_to_string (private_key_file.get()));
			cout << "\nKeys";
			BOOST_FOREACH (dcp::DecryptedKDMKey i, dec_kdm.keys ()) {
				cout << "\n";
				cout << "\tID:       " << i.id() << "\n";
				cout << "\tStandard: " << (i.standard() == dcp::SMPTE ? "SMPTE" : "Interop") << "\n";
				cout << "\tCPL ID:   " << i.cpl_id() << "\n";
				if (i.type()) {
					cout << "\tType:     " << i.type().get() << "\n";
				}
				cout << "\tKey:      " << i.key().hex() << "\n";
			}
		} catch (dcp::KDMDecryptionError& e) {
			cerr << e.what() << "\n";
			exit (EXIT_FAILURE);
		}
	}

	return 0;
}
