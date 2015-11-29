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

#include "j2k.h"
#include "exceptions.h"
#include "openjpeg_image.h"
#include "data.h"
#include "dcp_assert.h"
#include "compose.hpp"
#include <openjpeg.h>
#include <cmath>

using std::min;
using std::pow;
using boost::shared_ptr;
using boost::shared_array;
using namespace dcp;

shared_ptr<dcp::OpenJPEGImage>
dcp::decompress_j2k (Data data, int reduce)
{
	return dcp::decompress_j2k (data.data().get(), data.size(), reduce);
}

class ReadBuffer
{
public:
	ReadBuffer (uint8_t* data, int64_t size)
		: _data (data)
		, _size (size)
		, _offset (0)
	{}

	OPJ_SIZE_T read (void* buffer, OPJ_SIZE_T nb_bytes)
	{
		int64_t N = min (nb_bytes, _size - _offset);
		memcpy (buffer, _data + _offset, N);
		_offset += N;
		return N;
	}

private:
	uint8_t* _data;
	OPJ_SIZE_T _size;
	OPJ_SIZE_T _offset;
};

static OPJ_SIZE_T
read_function (void* buffer, OPJ_SIZE_T nb_bytes, void* data)
{
	return reinterpret_cast<ReadBuffer*>(data)->read (buffer, nb_bytes);
}

static void
read_free_function (void* data)
{
	delete reinterpret_cast<ReadBuffer*>(data);
}

/** Decompress a JPEG2000 image to a bitmap.
 *  @param data JPEG2000 data.
 *  @param size Size of data in bytes.
 *  @param reduce A power of 2 by which to reduce the size of the decoded image;
 *  e.g. 0 reduces by (2^0 == 1), ie keeping the same size.
 *       1 reduces by (2^1 == 2), ie halving the size of the image.
 *  This is useful for scaling 4K DCP images down to 2K.
 *  @return OpenJPEGImage.
 */
shared_ptr<dcp::OpenJPEGImage>
dcp::decompress_j2k (uint8_t* data, int64_t size, int reduce)
{
	uint8_t const jp2_magic[] = {
		0x00,
		0x00,
		0x00,
		0x0c,
		'j',
		'P',
		0x20,
		0x20
	};

	OPJ_CODEC_FORMAT format = OPJ_CODEC_J2K;
	if (size >= int (sizeof (jp2_magic)) && memcmp (data, jp2_magic, sizeof (jp2_magic)) == 0) {
		format = OPJ_CODEC_JP2;
	}

	opj_codec_t* decoder = opj_create_decompress (format);
	if (!decoder) {
		boost::throw_exception (DCPReadError ("could not create JPEG2000 decompresser"));
	}
	opj_dparameters_t parameters;
	opj_set_default_decoder_parameters (&parameters);
	parameters.cp_reduce = reduce;
	opj_setup_decoder (decoder, &parameters);

	opj_stream_t* stream = opj_stream_default_create (OPJ_TRUE);
	if (!stream) {
		throw MiscError ("could not create JPEG2000 stream");
	}

	opj_stream_set_read_function (stream, read_function);
	ReadBuffer* buffer = new ReadBuffer (data, size);
	opj_stream_set_user_data (stream, buffer, read_free_function);
	opj_stream_set_user_data_length (stream, size);

	opj_image_t* image = 0;
	opj_read_header (stream, decoder, &image);
	if (opj_decode (decoder, stream, image) == OPJ_FALSE) {
		opj_destroy_codec (decoder);
		opj_stream_destroy (stream);
		if (format == OPJ_CODEC_J2K) {
			boost::throw_exception (DCPReadError (String::compose ("could not decode JPEG2000 codestream of %1 bytes.", size)));
		} else {
			boost::throw_exception (DCPReadError (String::compose ("could not decode JP2 file of %1 bytes.", size)));
		}
	}

	opj_destroy_codec (decoder);
	opj_stream_destroy (stream);

	image->x1 = rint (float(image->x1) / pow (2, reduce));
	image->y1 = rint (float(image->y1) / pow (2, reduce));
	return shared_ptr<OpenJPEGImage> (new OpenJPEGImage (image));
}

class WriteBuffer
{
public:
/* XXX: is there a better strategy for this? */
#define MAX_J2K_SIZE (1024 * 1024 * 2)
	WriteBuffer ()
		: _data (shared_array<uint8_t> (new uint8_t[MAX_J2K_SIZE]), MAX_J2K_SIZE)
		, _offset (0)
	{}

	OPJ_SIZE_T write (void* buffer, OPJ_SIZE_T nb_bytes)
	{
		DCP_ASSERT ((_offset + nb_bytes) < MAX_J2K_SIZE);
		memcpy (_data.data().get() + _offset, buffer, nb_bytes);
		_offset += nb_bytes;
		return nb_bytes;
	}

	Data data () const {
		return _data;
	}

private:
	Data _data;
	OPJ_SIZE_T _offset;
};

static OPJ_SIZE_T
write_function (void* buffer, OPJ_SIZE_T nb_bytes, void* data)
{
	return reinterpret_cast<WriteBuffer*>(data)->write (buffer, nb_bytes);
}

static void
write_free_function (void* data)
{
	delete reinterpret_cast<WriteBuffer*>(data);
}

Data
dcp::compress_j2k (shared_ptr<const OpenJPEGImage> xyz, int bandwidth, int frames_per_second, bool threed, bool fourk)
{
	/* XXX: should probably use opj_set_*_handler */

	/* Set the max image and component sizes based on frame_rate */
	int max_cs_len = ((float) bandwidth) / 8 / frames_per_second;
	if (threed) {
		/* In 3D we have only half the normal bandwidth per eye */
		max_cs_len /= 2;
	}
	int const max_comp_size = max_cs_len / 1.25;

	/* get a J2K compressor handle */
	opj_codec_t* encoder = opj_create_compress (OPJ_CODEC_J2K);
	if (encoder == 0) {
		throw MiscError ("could not create JPEG2000 encoder");
	}

	/* Set encoding parameters to default values */
	opj_cparameters_t parameters;
	opj_set_default_encoder_parameters (&parameters);

	/* Set default cinema parameters */
	parameters.tile_size_on = OPJ_FALSE;
	parameters.cp_tdx = 1;
	parameters.cp_tdy = 1;

	/* Tile part */
	parameters.tp_flag = 'C';
	parameters.tp_on = 1;

	/* Tile and Image shall be at (0,0) */
	parameters.cp_tx0 = 0;
	parameters.cp_ty0 = 0;
	parameters.image_offset_x0 = 0;
	parameters.image_offset_y0 = 0;

	/* Codeblock size = 32x32 */
	parameters.cblockw_init = 32;
	parameters.cblockh_init = 32;
	parameters.csty |= 0x01;

	/* The progression order shall be CPRL */
	parameters.prog_order = OPJ_CPRL;

	/* No ROI */
	parameters.roi_compno = -1;

	parameters.subsampling_dx = 1;
	parameters.subsampling_dy = 1;

	/* 9-7 transform */
	parameters.irreversible = 1;

	parameters.tcp_rates[0] = 0;
	parameters.tcp_numlayers++;
	parameters.cp_disto_alloc = 1;
	parameters.cp_rsiz = fourk ? OPJ_CINEMA4K : OPJ_CINEMA2K;
	if (fourk) {
		parameters.numpocs = 2;
		parameters.POC[0].tile = 1;
		parameters.POC[0].resno0 = 0;
		parameters.POC[0].compno0 = 0;
		parameters.POC[0].layno1 = 1;
		parameters.POC[0].resno1 = parameters.numresolution - 1;
		parameters.POC[0].compno1 = 3;
		parameters.POC[0].prg1 = OPJ_CPRL;
		parameters.POC[1].tile = 1;
		parameters.POC[1].resno0 = parameters.numresolution - 1;
		parameters.POC[1].compno0 = 0;
		parameters.POC[1].layno1 = 1;
		parameters.POC[1].resno1 = parameters.numresolution;
		parameters.POC[1].compno1 = 3;
		parameters.POC[1].prg1 = OPJ_CPRL;
	}

	parameters.cp_comment = strdup ("libdcp");
	parameters.cp_cinema = fourk ? OPJ_CINEMA4K_24 : OPJ_CINEMA2K_24;

	/* 3 components, so use MCT */
	parameters.tcp_mct = 1;

	/* set max image */
	parameters.max_comp_size = max_comp_size;
	parameters.tcp_rates[0] = ((float) (3 * xyz->size().width * xyz->size().height * 12)) / (max_cs_len * 8);

	/* Setup the encoder parameters using the current image and user parameters */
	opj_setup_encoder (encoder, &parameters, xyz->opj_image());

	opj_stream_t* stream = opj_stream_default_create (OPJ_FALSE);
	if (!stream) {
		throw MiscError ("could not create JPEG2000 stream");
	}

	opj_stream_set_write_function (stream, write_function);
	WriteBuffer* buffer = new WriteBuffer ();
	opj_stream_set_user_data (stream, buffer, write_free_function);

	if (!opj_start_compress (encoder, xyz->opj_image(), stream)) {
		throw MiscError ("could not start JPEG2000 encoding");
	}

	if (!opj_encode (encoder, stream)) {
		opj_destroy_codec (encoder);
		opj_stream_destroy (stream);
		throw MiscError ("JPEG2000 encoding failed");
	}

	if (!opj_end_compress (encoder, stream)) {
		throw MiscError ("could not end JPEG2000 encoding");
	}

	Data enc (buffer->data ());

	free (parameters.cp_comment);
	opj_destroy_codec (encoder);
	opj_stream_destroy (stream);

	return enc;
}
