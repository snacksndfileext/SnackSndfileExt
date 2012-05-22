/***********
 *
 * SnackSndfileExt snack2.2 extension that adds libsndfile support
 * Copyright (C) 2011 Giulio Paci <giuliopaci@gmail.com>
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

#ifndef PACKAGE
#  define PACKAGE "snack_sndfile_ext"
#endif

#ifndef PACKAGE_VERSION
#  define PACKAGE_VERSION "0.0.1"
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
		return format_info.name;
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
	fprintf(stderr, "OpenSndFile(%s)\n", Snack_GetSoundFilename(s));
	SF_INFO sfinfo;
	*ch = (Tcl_Channel) sf_open ( Snack_GetSoundFilename(s), SndOpenModeFromString(mode), &sfinfo );
	if (*ch == NULL)
	{
		Tcl_AppendResult(interp, "SNDFILE: unable to open file: ", Snack_GetSoundFilename(s), "\n", sf_strerror(NULL), NULL);
		return TCL_ERROR;
	}
	sf_command (*ch, SFC_SET_NORM_FLOAT, NULL, SF_FALSE) ;
	//sf_command (*ch, SFC_SET_NORM_FLOAT, NULL, SF_TRUE) ;
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

	switch(file_info.format & SF_FORMAT_SUBMASK)
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
		fprintf(stderr, "GetSndHeader: format defaults\n");
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
	Snack_SetLength(s, file_info.frames);
	Snack_SetHeaderSize(s, 0 /* ?? HEADERSIZE */);

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
	/* Snack_GetSoundData(s, pos, pcmout, READBUFSIZE); */
	/* if (s->readStatus == READ) { */
	/*   buffer[j][i] = FSAMPLE(s, pos) / 32768.0f; */
	/* } else { */
	/*   buffer[j][i] = pcmout[k] / 32768.0f; */
	/* } */
	/* return sf_write_float ((SNDFILE*) ch, obuf, len); */
}


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

/* This function is used instead of the snack_sndfile_ext.tcl script in order
   to generate the tcl variables that are needed by snack. Doing it here allows
   keeping the formats always up to date with the current version of libsndfile
*/
int CreateTclVariablesForSnack(Tcl_Interp *interp)
{
  int k, count ;
  SF_FORMAT_INFO format_info ;
  Tcl_Obj *scriptPtr = Tcl_NewStringObj("", 0);
  Tcl_Obj *scriptPtr1 = Tcl_NewStringObj("", 0);
  Tcl_Obj *scriptPtr2 = Tcl_NewStringObj("", 0);
  Tcl_Obj *formatExtUC = Tcl_NewStringObj("", 0);

  Tcl_AppendStringsToObj(scriptPtr,
			 "namespace eval snack::snack_sndfile_ext {\n",
			 "    variable extTypes\n",
			 "    variable loadTypes\n",
			 "    variable loadKeys\n\n", (char *) NULL);
  Tcl_AppendStringsToObj(scriptPtr1, "    set extTypesMC {\n", (char *) NULL);
  Tcl_AppendStringsToObj(scriptPtr2, "    set loadTypes {\n", (char *) NULL);

  sf_command (NULL, SFC_GET_FORMAT_MAJOR_COUNT, &count, sizeof (int));
  
  for (k = 0 ; k < count ; k++) {
    format_info.format = k ;
    sf_command (NULL, SFC_GET_FORMAT_MAJOR, &format_info, sizeof (SF_FORMAT_INFO));

    /* convert extension to upper case */
    Tcl_SetStringObj(formatExtUC, format_info.extension, strlen(format_info.extension));
    Tcl_UtfToUpper(Tcl_GetString(formatExtUC));

    /* append to variable extTypesMC */
    Tcl_AppendStringsToObj(scriptPtr1, "        {{", format_info.name,
			   "} .", format_info.extension, "}\n", (char *) NULL);

    /* append to variable loadTypes */
    Tcl_AppendStringsToObj(scriptPtr2, "        {{", format_info.name,
			   "} {.", format_info.extension,
			   " .", Tcl_GetString(formatExtUC),
			   "}}\n", (char *) NULL);
  }
  Tcl_AppendStringsToObj(scriptPtr1, "    }\n\n", (char *) NULL);
  Tcl_AppendStringsToObj(scriptPtr2, "    }\n\n", (char *) NULL);

  Tcl_AppendObjToObj(scriptPtr, scriptPtr1);
  Tcl_AppendObjToObj(scriptPtr, scriptPtr2);

  Tcl_AppendStringsToObj(scriptPtr,
			 "    set extTypes [list]\n",
			 "    set loadKeys [list]\n",
			 "    foreach pair $extTypesMC {\n",
			 "	set type [string toupper [lindex $pair 0]]\n",
			 "	set ext [lindex $pair 1]\n",
			 "	lappend extTypes [list $type $ext]\n",
			 "	lappend loadKeys $type\n"
			 "    }\n\n",
			 "    snack::addLoadTypes $loadTypes $loadKeys\n",
			 "    snack::addExtTypes $extTypes\n",
			 "}\n", (char *) NULL);

  fprintf(stderr, "%s\n", Tcl_GetString(scriptPtr));

  return Tcl_EvalObjEx(interp, scriptPtr, TCL_EVAL_DIRECT);
}


/* Called by "load libsnacksndfile" */
EXPORT(int, Snack_sndfile_ext_Init) _ANSI_ARGS_((Tcl_Interp *interp))
{
	int res;
	int k, count ;
        SF_FORMAT_INFO format_info ;
	Snack_FileFormat *SndFileFormatPtr;

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
  
  
	res = Tcl_PkgProvide(interp, PACKAGE, PACKAGE_VERSION);
  
	if (res != TCL_OK) return res;

	Tcl_SetVar(interp, "snack::" PACKAGE,  PACKAGE_VERSION, TCL_GLOBAL_ONLY);

        sf_command (NULL, SFC_GET_FORMAT_MAJOR_COUNT, &count, sizeof (int));

        for (k = 0 ; k < count ; k++)
        {
	  format_info.format = k ;
	  sf_command (NULL, SFC_GET_FORMAT_MAJOR, &format_info, sizeof (SF_FORMAT_INFO));

	  SndFileFormatPtr = (Snack_FileFormat *) malloc(sizeof(Snack_FileFormat));
	  /* only copy pointer to format name */
	  SndFileFormatPtr->name = format_info.name;
	  SndFileFormatPtr->guessProc = (guessFileTypeProc*) GuessSndFile;
	  SndFileFormatPtr->getHeaderProc = GetSndHeader;
	  SndFileFormatPtr->extProc = (extensionFileTypeProc*) ExtSndFile;
	  SndFileFormatPtr->putHeaderProc =  NULL;
	  SndFileFormatPtr->openProc =  OpenSndFile;
	  SndFileFormatPtr->closeProc =  CloseSndFile;
	  SndFileFormatPtr->readProc =  ReadSndSamples;
	  SndFileFormatPtr->writeProc =  NULL;
	  SndFileFormatPtr->seekProc =  SeekSndFile;
	  SndFileFormatPtr->freeHeaderProc =  NULL;
	  SndFileFormatPtr->configureProc = NULL;
	  SndFileFormatPtr->nextPtr = (Snack_FileFormat*) NULL;

	  Snack_CreateFileFormat(SndFileFormatPtr);
	}

	return CreateTclVariablesForSnack(interp);
}

EXPORT(int, Snacksndfile_SafeInit)(Tcl_Interp *interp)
{
	return Snack_sndfile_ext_Init(interp);
}

#ifdef __cplusplus
}
#endif
