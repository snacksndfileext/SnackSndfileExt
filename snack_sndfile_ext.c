/***********
 *
 * SnackSndfileExt snack2.2 extension that adds libsndfile support
 * Copyright (C) 2011 Giulio Paci <giuliopaci@interfree.it>
 *  
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *  
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *  
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 ****/

#include <tcl.h>
#include <snack.h>
#include <sndfile.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__WIN32__)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  undef WIN32_LEAN_AND_MEAN
#  define EXPORT(a,b) __declspec(dllexport) a b
BOOL APIENTRY
DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
	return TRUE;
}
#else
#  define EXPORT(a,b) a b
#endif

struct dummy_file
{
	char *buffer;
	sf_count_t length;
	sf_count_t curpos;
};

static sf_count_t dummy_vio_get_filelen (void *user_data)
{
	struct dummy_file *file = (struct dummy_file*) user_data;
	return file->length;
}

static sf_count_t dummy_vio_read (void *ptr, sf_count_t count, void *user_data)
{
	struct dummy_file *file = (struct dummy_file*) user_data;
	fprintf(stderr, "read %d\n", count);
	if( count + file->curpos > file->length )
	{
		count = file->length - file->curpos;
	}
	if(count > 0)
	{
		memcpy(ptr, file->buffer + file->curpos, count);
	}
	file->curpos += count;
	return count;
}
static sf_count_t dummy_vio_write (const void *ptr, sf_count_t count, void *user_data)
{
	/* Unimplemented */
	return 0;
}

static sf_count_t dummy_vio_seek (sf_count_t offset, int whence, void *user_data)
{
	struct dummy_file *file = (struct dummy_file*) user_data;
	fprintf(stderr, "seek %d %d\n", whence, offset);
	sf_count_t newpos = 0;
	switch(whence)
	{
	case SEEK_SET:
		newpos = offset;
		break;
	case SEEK_CUR:
		newpos = file->curpos + offset;
		break;
	case SEEK_END:
		newpos = file->length + offset;
		break;
	}
	if( ( newpos >= 0 ) && ( newpos < file->length ) )
	{
		file->curpos = newpos;
	}
	else
	{
		return 1;
	}
	return 0;
}

static sf_count_t dummy_vio_tell (void *user_data)
{
	struct dummy_file *file = (struct dummy_file*) user_data;
	return file->curpos;
}


static const char* ExtSndFile(const char* s)
{
	int l2 = strlen(s);

        int k, count ;
        SF_FORMAT_INFO format_info ;

        sf_command (NULL, SFC_GET_FORMAT_MAJOR_COUNT, &count, sizeof (int));

        for (k = 0 ; k < count ; k++)
        {
		format_info.format = k ;
		sf_command (NULL, SFC_GET_FORMAT_MAJOR, &format_info, sizeof (SF_FORMAT_INFO));
		fprintf (stderr, "%08x  %s %s\n", format_info.format, format_info.name, format_info.extension) ;

		int l1 = strlen(format_info.extension);
		if( l2 > l1 )
		{
			fprintf( stderr, "\"%s\" Vs \"%s\"\n", format_info.extension, &s[l2 - l1]);
			if ( (s[l2 - l1 - 1] == '.') && (strncasecmp(format_info.extension, &s[l2 - l1], l1) == 0)) {
				return format_info.name;
			}
		}
	}
        for (k = 0 ; k < count ; k++)
        {
		format_info.format = k ;
		sf_command (NULL, SFC_GET_FORMAT_MAJOR, &format_info, sizeof (SF_FORMAT_INFO));
		fprintf (stderr, "%08x  %s %s\n", format_info.format, format_info.name, format_info.extension) ;
		char* tmp = strchr(format_info.name, ' ');
		int l1;
		if( tmp == NULL)
		{
			l1 = strlen(format_info.name);
		}
		else
		{
			l1 = ( tmp - format_info.name );
		}
		if( l2 > l1 )
		{
			fprintf( stderr, "\"%s\" Vs \"%s\"\n", format_info.name, &s[l2 - l1]);
			if ( (s[l2 - l1 - 1] == '.') && (strncasecmp(format_info.name, &s[l2 - l1], l1) == 0)) {
				return format_info.name;
			}
		}
		tmp = strstr(format_info.name, "Sphere");
		if( tmp == NULL)
		{
			tmp = strstr(format_info.name, "NIST");
		}
		if( tmp != NULL)
		{
			tmp = "sph";
			l1 = 3;
			if( l2 > l1 )
			{
				fprintf( stderr, "\"%s\" Vs \"%s\"\n", tmp, &s[l2 - l1]);
				if ( (s[l2 - l1 - 1] == '.') && (strncasecmp(tmp, &s[l2 - l1], l1) == 0)) {
					return format_info.name;
				}
			}
		}
	}
	return NULL;
}

static const char *GuessSndFile (char *buf, int len)
{
	SF_VIRTUAL_IO io;
	io.get_filelen = dummy_vio_get_filelen;
	io.seek = dummy_vio_seek;
	io.read = dummy_vio_read;
	io.write = dummy_vio_write;
	io.tell = dummy_vio_tell;

	struct dummy_file file;
	file.buffer = buf;
	file.length = len;
	file.curpos = 0;

	SF_INFO info;
	memset(&info, 0, sizeof(SF_INFO));

	SNDFILE* sndfile = sf_open_virtual (&io, SFM_READ, &info, &file);
	if( sndfile == NULL )
	{
		int err = sf_error(NULL);
		fprintf(stderr, "Error occurred: %s\n", sf_error_number(err));
		switch(err)
		{
		case SF_ERR_UNRECOGNISED_FORMAT:
		case SF_ERR_UNSUPPORTED_ENCODING:
			sf_close(sndfile);
			return NULL;
		case SF_ERR_SYSTEM:
		case SF_ERR_MALFORMED_FILE:
		default:
			sf_close(sndfile);
			return QUE_STRING;
			break;
		}
	}
	else
	{
		sf_close(sndfile);
		SF_FORMAT_INFO format_info;
		format_info.format = info.format;
		int ret = sf_command (NULL /* sndfile */, SFC_GET_FORMAT_INFO, &format_info, sizeof(SF_FORMAT_INFO)) ;
		fprintf(stderr, "%s [%s]\n", format_info.name, format_info.extension);
		return "SNDFILE_FORMAT";//format_info.name;
	}
	
	return NULL;
}

static int SndOpenModeFromString(char* mode)
{
	int length;
	int i;
	int ret = SFM_READ;
	length = strlen(mode);
	if(length > 0)
	{
		if(mode[0] == 'r')
		{
			ret = SFM_READ;
			if(strchr(mode, 'w') != NULL)
			{
				ret = SFM_RDWR;
			}
		}
		else if(mode[0] == 'w')
		{
			ret = SFM_WRITE;
			if(strchr(mode, 'r') != NULL)
			{
				ret = SFM_RDWR;
			}
		}
	}
	return ret;
}

static int OpenSndFile(Sound *s, Tcl_Interp *interp, Tcl_Channel *ch, char *mode)
{
	fprintf(stderr, "OpenSndFile\n");
	SF_INFO sfinfo;
	*ch = (Tcl_Channel) sf_open ( Snack_GetSoundFilename(s), SndOpenModeFromString(mode), &sfinfo );
	if (*ch == NULL)
	{
		Tcl_AppendResult(interp, "SNDFILE: unable to open file: ", Snack_GetSoundFilename(s), "\n", sf_strerror(NULL), NULL);
		return TCL_ERROR;
	}

	return TCL_OK;
}

static int SeekSndFile(Sound *s, Tcl_Interp *interp, Tcl_Channel ch, int pos)
{
	fprintf(stderr, "SeekSndFile\n");
	return sf_seek ((SNDFILE*) ch, pos, SEEK_SET);
}

static int CloseSndFile(Sound *s, Tcl_Interp *interp, Tcl_Channel *ch)
{
	fprintf(stderr, "CloseSndFile\n");
	int err = sf_close((SNDFILE*) *ch);
	if (err != 0)
	{
		Tcl_AppendResult(interp, "SNDFILE: error closing file: ", Snack_GetSoundFilename(s), "\n", sf_error_number(err), NULL);
	}
	*ch = NULL;

	return TCL_OK;
}

static int GetSndHeader(Sound *s, Tcl_Interp *interp, Tcl_Channel ch, Tcl_Obj *obj, char *buf)
{
	fprintf(stderr, "GetSndHeader\n");
	if (obj != NULL)
	{
		Tcl_AppendResult(interp, "'data' subcommand forbidden for SNDFILE format", NULL);
		if (ch)
		{
			sf_close((SNDFILE *)ch);
		}
		return TCL_ERROR;
	}

	if (Snack_GetDebugFlag(s) > 2)
	{
		Snack_WriteLog("    Reading SNDFILE header\n");
	}

	SF_INFO file_info;
	int ret = sf_command ((SNDFILE*) ch, SFC_GET_CURRENT_SF_INFO, &file_info, sizeof(SF_INFO));
	Snack_SetSampleRate(s, file_info.samplerate);
	if (Snack_GetDebugFlag(s) > 3) {
		Snack_WriteLogInt("      Setting rate", Snack_GetSampleRate(s));
	}
	Snack_SetNumChannels(s, file_info.channels);
	if (Snack_GetDebugFlag(s) > 3) {
		Snack_WriteLogInt("      Setting channels", Snack_GetNumChannels(s));
	}

	ret = sf_command ((SNDFILE*) ch, SFC_GET_FORMAT_SUBTYPE, &file_info, sizeof(SF_INFO));
	switch(file_info.format)
	{
	case SF_FORMAT_PCM_S8:
	case SF_FORMAT_PCM_U8:
		Snack_SetBytesPerSample(s, 1);
		Snack_SetSampleEncoding(s, LIN8);
		break;
	case SF_FORMAT_PCM_16:
		Snack_SetBytesPerSample(s, 2);
		Snack_SetSampleEncoding(s, LIN16);
		break;
	case SF_FORMAT_PCM_24:
		Snack_SetBytesPerSample(s, 3);
		Snack_SetSampleEncoding(s, LIN24);
		break;
	case SF_FORMAT_PCM_32:
		Snack_SetBytesPerSample(s, 4);
		Snack_SetSampleEncoding(s, LIN32);
		break;
	case SF_FORMAT_FLOAT:
		Snack_SetBytesPerSample(s, 4);
		Snack_SetSampleEncoding(s, SNACK_FLOAT);
		break;
	case SF_FORMAT_DOUBLE:
		Snack_SetBytesPerSample(s, 8);
		Snack_SetSampleEncoding(s, SNACK_DOUBLE);
		break;
	case SF_FORMAT_ULAW:
		Snack_SetBytesPerSample(s, 1);
		Snack_SetSampleEncoding(s, MULAW);
		break;
	case SF_FORMAT_ALAW:
		Snack_SetBytesPerSample(s, 1);
		Snack_SetSampleEncoding(s, ALAW);
		break;
	case SF_FORMAT_IMA_ADPCM:
	case SF_FORMAT_MS_ADPCM:
	case SF_FORMAT_GSM610:
	case SF_FORMAT_VOX_ADPCM:
	case SF_FORMAT_G721_32:
	case SF_FORMAT_G723_24:
	case SF_FORMAT_G723_40:
	case SF_FORMAT_DWVW_12:
	case SF_FORMAT_DWVW_16:
	case SF_FORMAT_DWVW_24:
	case SF_FORMAT_DWVW_N:
	case SF_FORMAT_DPCM_8:
	case SF_FORMAT_DPCM_16:
	case SF_FORMAT_VORBIS:
	default:
		Snack_SetBytesPerSample(s, 4);
		Snack_SetSampleEncoding(s, SNACK_FLOAT);
		/* LIN16 */
		/* ALAW */
		/* MULAW */
		/* LIN8OFFSET */
		/* LIN8 */
		/* LIN24 */
		/* LIN32 */
		/* SNACK_FLOAT */
		/* SNACK_DOUBLE */
		/* LIN24PACKED */
		break;
	}
	Snack_SetHeaderSize(s, 0 /* ?? HEADERSIZE */);
	Snack_SetLength(s, 0 /* ?? FRAMES COUNT */);

	return TCL_OK;
}

static int ReadSndSamples(Sound *s, Tcl_Interp *interp, Tcl_Channel ch, char *ibuf, float *obuf, int len)
{
	fprintf(stderr, "ReadSndSample(..., %d)\n", len);
	if( ( ch == NULL ) || ( sf_error((SNDFILE*) ch) != 0 ) )
	{
		fprintf(stderr, "ReadSndSample ERROR %d\n", sf_error((SNDFILE*) ch));
		return -1;
	}
	/* int nframes = len / Snack_GetNumChannels(s); */
	/* nframes = sf_readf_float ((SNDFILE*) ch, obuf, nframes); */
	/* return nframes * Snack_GetNumChannels(s); */
	return sf_read_float ((SNDFILE*) ch, obuf, len);
}


static int WriteSndSamples(Sound *s, Tcl_Channel ch, Tcl_Obj *obj, int start, int length)
{
	fprintf(stderr, "WriteSndSample(..., %d, %d)\n", start, length);
	fprintf(stderr, "WriteSndSample UNIMPLEMENTED");
	if( ( ch == NULL ) || ( sf_error((SNDFILE*) ch) != 0 ) )
	{
		fprintf(stderr, "WriteSndSample ERROR %d\n", sf_error((SNDFILE*) ch));
		return -1;
	}
	return -1;
	/* int nframes = len / Snack_GetNumChannels(s); */
	/* nframes = sf_readf_float ((SNDFILE*) ch, obuf, nframes); */
	/* return nframes * Snack_GetNumChannels(s); */
	/* return sf_write_float ((SNDFILE*) ch, obuf, len); */
}



Snack_FileFormat SndFileFormat =
{
	/* char* name; */ "SNDFILE_FORMAT",
	/* guessFileTypeProc* guessProc; */ (guessFileTypeProc*) GuessSndFile,
	/* getHeaderProc* getHeaderProc; */ GetSndHeader,
	/* extensionFileTypeProc* extProc; */ (extensionFileTypeProc*) ExtSndFile,
	/* putHeaderProc* putHeaderProc; */ NULL,
	/* openProc* openProc; */ OpenSndFile,
	/* closeProc* closeProc; */ CloseSndFile,
	/* readSamplesProc* readProc; */ ReadSndSamples,
	/* writeSamplesProc* writeProc; */ NULL,
	/* seekProc* seekProc; */ SeekSndFile,
	/* freeHeaderProc* freeHeaderProc; */ NULL,
	/* configureProc* configureProc; */ NULL,
	/* struct Snack_FileFormat* nextPtr; */ (Snack_FileFormat*) NULL
};

/* int main (int argc, char** args) */
/* { */
/* 	int count = 200; */
/* 	const char* format; */
/* 	char* buffer = malloc(count); */
/* 	FILE* f = fopen( args[1], "r" ); */
/* 	count = fread(buffer, 1, count, f); */
/* 	fclose(f); */
/* 	printf("%d\n", count); */
/* 	format = GuessSndFile (buffer, count); */
/* 	free(buffer); */
/* 	if( format != NULL ) */
/* 	{ */
/* 		fprintf(stderr, "Recognition result: %s\n", format); */
/* 	} */
/* 	char* ret; */
/* 	char* ext; */
/* 	ext = ".sph"; */
/* 	fprintf(stderr, "Ext (%s) result: %s\n", ext, ( (( ret = ExtSndFile(ext)) == NULL) ? "none" : ret)); */
/* 	ext = ".ogg"; */
/* 	fprintf(stderr, "Ext (%s) result: %s\n", ext, ( (( ret = ExtSndFile(ext)) == NULL) ? "none" : ret)); */
/* 	ext = ".oga"; */
/* 	fprintf(stderr, "Ext (%s) result: %s\n", ext, ( (( ret = ExtSndFile(ext)) == NULL) ? "none" : ret)); */
/* 	ext = ".wav"; */
/* 	fprintf(stderr, "Ext (%s) result: %s\n", ext, ( (( ret = ExtSndFile(ext)) == NULL) ? "none" : ret)); */
/* 	return 0; */
/* } */

/* Called by "load libsnacksndfile" */
EXPORT(int, Snack_sndfile_ext_Init) _ANSI_ARGS_((Tcl_Interp *interp))
{
	int res;
  
#ifdef USE_TCL_STUBS
	if (Tcl_InitStubs(interp, "8", 0) == NULL) {
		return TCL_ERROR;
	}
#endif
  
#ifdef USE_SNACK_STUBS
	if (Snack_InitStubs(interp, "2", 0) == NULL) {
		return TCL_ERROR;
	}
#endif
  
  
	res = Tcl_PkgProvide(interp, "snacksndfile", sf_version_string());
  
	if (res != TCL_OK) return res;

	Tcl_SetVar(interp, "snack::snacksndfile",  sf_version_string(),TCL_GLOBAL_ONLY);

	Snack_CreateFileFormat(&SndFileFormat);

	return TCL_OK;
}

EXPORT(int, Snacksndfile_SafeInit)(Tcl_Interp *interp)
{
	return Snack_sndfile_ext_Init(interp);
}

#ifdef __cplusplus
}
#endif
