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

/** @file  src/sound_asset.cc
 *  @brief An asset made up of WAV files
 */

#include <iostream>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "KM_fileio.h"
#include "AS_DCP.h"
#include "sound_asset.h"
#include "util.h"
#include "exceptions.h"
#include "sound_frame.h"

using namespace std;
using namespace boost;
using namespace libdcp;

SoundAsset::SoundAsset (
	vector<string> const & files, string directory, string mxf_name, sigc::signal1<void, float>* progress, int fps, int length
	)
	: MXFAsset (directory, mxf_name, progress, fps, 0, length)
	, _channels (files.size ())
	, _sampling_rate (0)
{
	construct (sigc::bind (sigc::mem_fun (*this, &SoundAsset::path_from_channel), files));
}

SoundAsset::SoundAsset (
	sigc::slot<string, Channel> get_path, string directory, string mxf_name, sigc::signal1<void, float>* progress, int fps, int length, int channels
	)
	: MXFAsset (directory, mxf_name, progress, fps, 0, length)
	, _channels (channels)
	, _sampling_rate (0)
{
	construct (get_path);
}

SoundAsset::SoundAsset (string directory, string mxf_name, int fps, int entry_point, int length)
	: MXFAsset (directory, mxf_name, 0, fps, entry_point, length)
	, _channels (0)
{
	ASDCP::PCM::MXFReader reader;
	if (ASDCP_FAILURE (reader.OpenRead (path().string().c_str()))) {
		throw MXFFileError ("could not open MXF file for reading", path().string());
	}

	
	ASDCP::PCM::AudioDescriptor desc;
	if (ASDCP_FAILURE (reader.FillAudioDescriptor (desc))) {
		throw DCPReadError ("could not read audio MXF information");
	}

	_sampling_rate = desc.AudioSamplingRate.Numerator / desc.AudioSamplingRate.Denominator;
	_channels = desc.ChannelCount;
}

string
SoundAsset::path_from_channel (Channel channel, vector<string> const & files)
{
	unsigned int const c = int (channel);
	assert (c < files.size ());
	return files[c];
}

void
SoundAsset::construct (sigc::slot<string, Channel> get_path)
{
	ASDCP::Rational asdcp_fps (_fps, 1);
	
 	ASDCP::PCM::WAVParser pcm_parser_channel[_channels];
	if (pcm_parser_channel[0].OpenRead (get_path(LEFT).c_str(), asdcp_fps)) {
		throw FileError ("could not open WAV file for reading", get_path(LEFT));
	}
	
	ASDCP::PCM::AudioDescriptor audio_desc;
	pcm_parser_channel[0].FillAudioDescriptor (audio_desc);
	audio_desc.ChannelCount = 0;
	audio_desc.BlockAlign = 0;
	audio_desc.EditRate = asdcp_fps;
	audio_desc.AvgBps = audio_desc.AvgBps * _channels;

	Channel channels[] = {
		LEFT,
		RIGHT,
		CENTRE,
		LFE,
		LS,
		RS
	};

	ASDCP::PCM::FrameBuffer frame_buffer_channel[_channels];
	ASDCP::PCM::AudioDescriptor audio_desc_channel[_channels];

	for (int i = 0; i < _channels; ++i) {

		string const path = get_path (channels[i]);
		
		if (ASDCP_FAILURE (pcm_parser_channel[i].OpenRead (path.c_str(), asdcp_fps))) {
			throw FileError ("could not open WAV file for reading", path);
		}

		pcm_parser_channel[i].FillAudioDescriptor (audio_desc_channel[i]);
		frame_buffer_channel[i].Capacity (ASDCP::PCM::CalcFrameBufferSize (audio_desc_channel[i]));

		audio_desc.ChannelCount += audio_desc_channel[i].ChannelCount;
		audio_desc.BlockAlign += audio_desc_channel[i].BlockAlign;
	}

	ASDCP::PCM::FrameBuffer frame_buffer;
	frame_buffer.Capacity (ASDCP::PCM::CalcFrameBufferSize (audio_desc));
	frame_buffer.Size (ASDCP::PCM::CalcFrameBufferSize (audio_desc));

	ASDCP::WriterInfo writer_info;
	fill_writer_info (&writer_info);

	ASDCP::PCM::MXFWriter mxf_writer;
	if (ASDCP_FAILURE (mxf_writer.OpenWrite (path().string().c_str(), writer_info, audio_desc))) {
		throw FileError ("could not open audio MXF for writing", path().string());
	}

	for (int i = 0; i < _length; ++i) {

		byte_t *data_s = frame_buffer.Data();
		byte_t *data_e = data_s + frame_buffer.Capacity();
		byte_t sample_size = ASDCP::PCM::CalcSampleSize (audio_desc_channel[0]);
		int offset = 0;

		for (int j = 0; j < _channels; ++j) {
			memset (frame_buffer_channel[j].Data(), 0, frame_buffer_channel[j].Capacity());
			if (ASDCP_FAILURE (pcm_parser_channel[j].ReadFrame (frame_buffer_channel[j]))) {
				throw MiscError ("could not read audio frame");
			}
			
			if (frame_buffer_channel[j].Size() != frame_buffer_channel[j].Capacity()) {
				stringstream s;
				s << "short audio frame; " << _channels << " channels, "
				  << frame_buffer_channel[j].Size() << " vs " << frame_buffer_channel[j].Capacity();
				throw MiscError (s.str ());
			}
		}

		while (data_s < data_e) {
			for (int j = 0; j < _channels; ++j) {
				byte_t* frame = frame_buffer_channel[j].Data() + offset;
				memcpy (data_s, frame, sample_size);
				data_s += sample_size;
			}
			offset += sample_size;
		}

		if (ASDCP_FAILURE (mxf_writer.WriteFrame (frame_buffer, 0, 0))) {
			throw MiscError ("could not write audio MXF frame");
		}

		if (_progress) {
			(*_progress) (0.5 * float (i) / _length);
		}
	}

	if (ASDCP_FAILURE (mxf_writer.Finalize())) {
		throw MiscError ("could not finalise audio MXF");
	}
}

void
SoundAsset::write_to_cpl (ostream& s) const
{
	s << "        <MainSound>\n"
	  << "          <Id>urn:uuid:" << _uuid << "</Id>\n"
	  << "          <AnnotationText>" << _file_name << "</AnnotationText>\n"
	  << "          <EditRate>" << _fps << " 1</EditRate>\n"
	  << "          <IntrinsicDuration>" << _length << "</IntrinsicDuration>\n"
	  << "          <EntryPoint>0</EntryPoint>\n"
	  << "          <Duration>" << _length << "</Duration>\n"
	  << "        </MainSound>\n";
}

bool
SoundAsset::equals (shared_ptr<const Asset> other, EqualityOptions opt, list<string>& notes) const
{
	if (!MXFAsset::equals (other, opt, notes)) {
		return false;
	}
		     
	ASDCP::PCM::MXFReader reader_A;
	if (ASDCP_FAILURE (reader_A.OpenRead (path().string().c_str()))) {
		throw MXFFileError ("could not open MXF file for reading", path().string());
	}

	ASDCP::PCM::MXFReader reader_B;
	if (ASDCP_FAILURE (reader_B.OpenRead (other->path().string().c_str()))) {
		throw MXFFileError ("could not open MXF file for reading", path().string());
	}

	ASDCP::PCM::AudioDescriptor desc_A;
	if (ASDCP_FAILURE (reader_A.FillAudioDescriptor (desc_A))) {
		throw DCPReadError ("could not read audio MXF information");
	}
	ASDCP::PCM::AudioDescriptor desc_B;
	if (ASDCP_FAILURE (reader_B.FillAudioDescriptor (desc_B))) {
		throw DCPReadError ("could not read audio MXF information");
	}
	
	if (
		desc_A.EditRate != desc_B.EditRate ||
		desc_A.AudioSamplingRate != desc_B.AudioSamplingRate ||
		desc_A.Locked != desc_B.Locked ||
		desc_A.ChannelCount != desc_B.ChannelCount ||
		desc_A.QuantizationBits != desc_B.QuantizationBits ||
		desc_A.BlockAlign != desc_B.BlockAlign ||
		desc_A.AvgBps != desc_B.AvgBps ||
		desc_A.LinkedTrackID != desc_B.LinkedTrackID ||
		desc_A.ContainerDuration != desc_B.ContainerDuration
//		desc_A.ChannelFormat != desc_B.ChannelFormat ||
		) {
		
		notes.push_back ("audio MXF picture descriptors differ");
		return false;
	}
	
	ASDCP::PCM::FrameBuffer buffer_A (1 * Kumu::Megabyte);
	ASDCP::PCM::FrameBuffer buffer_B (1 * Kumu::Megabyte);
	
	for (int i = 0; i < _length; ++i) {
		if (ASDCP_FAILURE (reader_A.ReadFrame (i, buffer_A))) {
			throw DCPReadError ("could not read audio frame");
		}
		
		if (ASDCP_FAILURE (reader_B.ReadFrame (i, buffer_B))) {
			throw DCPReadError ("could not read audio frame");
		}
		
		if (buffer_A.Size() != buffer_B.Size()) {
			notes.push_back ("sizes of audio data for frame " + lexical_cast<string>(i) + " differ");
			return false;
		}
		
		if (memcmp (buffer_A.RoData(), buffer_B.RoData(), buffer_A.Size()) != 0) {
			for (uint32_t i = 0; i < buffer_A.Size(); ++i) {
				int const d = abs (buffer_A.RoData()[i] - buffer_B.RoData()[i]);
				if (d > opt.max_audio_sample_error) {
					notes.push_back ("PCM data difference of " + lexical_cast<string> (d));
					return false;
				}
			}
		}
	}

	return true;
}

shared_ptr<const SoundFrame>
SoundAsset::get_frame (int n) const
{
	return shared_ptr<const SoundFrame> (new SoundFrame (path().string(), n + _entry_point));
}
