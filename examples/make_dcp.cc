/*
    Copyright (C) 2012 Carl Hetherington <cth@carlh.net>

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

/** @file examples/make_dcp.cc
 *  @brief Shows how to make a DCP from some JPEG2000 and WAV files.
 */

#include <vector>
#include <string>
#include <sigc++/sigc++.h>

/* If you are using an installed libdcp, these #includes would need to be changed to
#include <libdcp/dcp.h>
#include <libdcp/picture_asset.h>
... etc. ...
*/

#include "dcp.h"
#include "picture_asset.h"
#include "sound_asset.h"
#include "reel.h"

/* This method returns the filename of the JPEG2000 file to use for a given frame.
   In this example, we are using the same file for each frame, so we don't bother
   looking at the frame parameter, but it will called with frame=0, frame=1, ...
*/
std::string
video_frame (int /* frame */)
{
	return "examples/help.j2c";
}

int
main ()
{
	/* Make a DCP object.  "My Film DCP" is the directory name for the DCP, "My Film" is the title
	   that will be shown on the projector / TMS when the DCP is ingested.

	   FEATURE is the type that the projector will list the DCP as.

	   24 is the frame rate, and the DCP will be 48 frames long (ie 2 seconds at 24 fps).
	*/
	libdcp::DCP dcp ("My Film DCP", "My Film", libdcp::FEATURE, 24, 48);

	/* Now make a `picture asset'.  This is a collection of the JPEG2000 files that make up the picture, one per frame.
	   Here we're using a function (video_frame) to obtain the name of the JPEG2000 file for each frame.

	   The result will be an MXF file written to the directory "My Film DCP" (which should be the same as the DCP's
	   directory above) called "video.mxf".

	   The other parameters specify the entry_point (the frame at which the projector should start showing the picture),
	   the frame rate, the number of frames and the resolution of the frames; 1998x1080 is the DCI Flat specification
	   for 2K projectors.
	*/
	boost::shared_ptr<libdcp::MonoPictureAsset> picture_asset (
		new libdcp::MonoPictureAsset (sigc::ptr_fun (&video_frame), "My Film DCP", "video.mxf", 0, 24, 48, 1998, 1080)
		);

	/* Now we will create a `sound asset', which is made up of a WAV file for each channel of audio.  Here we're using
	   stereo, so we add two WAV files to a vector.

	   We could add more files here to use more channels; the file order is:
	       Left
	       Right
	       Centre
	       LFE (sub)
	       Left surround
	       Right surround
	*/
	std::vector<std::string> sound_files;
	sound_files.push_back ("examples/sine_440_-12dB.wav");
	sound_files.push_back ("examples/sine_880_-12dB.wav");

	/* Now we can create the sound asset using these files */
	boost::shared_ptr<libdcp::SoundAsset> sound_asset (
		new libdcp::SoundAsset (sound_files, "My Film DCP", "audio.mxf", 0, 24, 48)
		);

	/* Now that we have the assets, we can create a Reel to put them in and add it to the DCP */
	dcp.add_reel (
		boost::shared_ptr<libdcp::Reel> (
			new libdcp::Reel (picture_asset, sound_asset, boost::shared_ptr<libdcp::SubtitleAsset> ())
			)
		);

	/* Finally, we call this to write the XML description files to the DCP.  After this, the DCP
	   is ready to ingest and play.
	*/
	dcp.write_xml ();

	return 0;
}