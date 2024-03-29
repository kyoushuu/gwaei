/*****************************************************************************
    AUTHOR:
    File written and Copyrighted by Zachary Dovel. All Rights Reserved.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//! @file io.c
//!


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <curl/curl.h>
#include <zlib.h>

#include <libwaei/gettext.h>
#include <libwaei/libwaei.h>


static gchar *_savepath = NULL;

struct _LwIoProcessFdData {
  const char* uri;         //!< The file path being passed
  int fd;                  //!< The file descriptor to be used with the path
  LwIoProgressCallback cb; //!< Callback to update progress
  gpointer data;           //!< Data to be passed to the LwIoProgressCallback
  GCancellable *cancellable;
  GError *error;           //!< A GError
};
typedef struct _LwIoProcessFdData LwIoProcessFdData; //!< Used for passing data to LwIo functions



//!
//! @brief Creates a savepath that is used with the save/save as functions
//! @param PATH a path to save to
//!
void 
lw_io_set_savepath (const gchar *PATH)
{
    if (_savepath != NULL)
    {
      g_free (_savepath);
      _savepath = NULL;
    }

    if (PATH != NULL)
      _savepath = g_strdup (PATH);
}


//!
//! @brief Gets the savepath used with the save/save as functions
//! @returns A constant path string that is not to be freed
//!
const gchar* 
lw_io_get_savepath ()
{
  return _savepath;
}


//!
//! @brief Writes a file using the given text and write mode
//! @param PATH The Path to write the file to
//! @param mode A constant char representing the write mode to be used (w,a)
//! @param text A char pointer to some text to write to the file.
//! @param cb A LwIoProgressCallback function to give progress feedback or NULL
//! @param data A generic pointer to data to pass to the callback
//! @param error A pointer to a GError object to write errors to or NULL
//!
void 
lw_io_write_file (const char* PATH, const char* mode, gchar *text, LwIoProgressCallback cb, gpointer data, GError **error)
{
    //Sanity checks
    g_assert (PATH != NULL && mode != NULL && text != NULL);
    if (*error != NULL) return;

    //Declarations
    gchar *ptr;
    FILE* file;

    //Initializations
    ptr = &text[0];
    file = fopen(_savepath, mode);

    while (*ptr != '\0' && feof(file) == 0 && ferror(file) == 0)
    {
      fputc(*ptr, file);
      ptr++;
    }

    if (feof(file) == 0 && ferror(file) == 0)
    {
      fputc('\n', file);
    }

    //Cleanup
    fclose(file);
    file = NULL;
    ptr = NULL;
}


//!
//! @brief Copies a file and creates a new one using the new encoding
//! @param SOURCE_PATH The source file to change the encoding on.
//! @param TARGET_PATH The place to save the new file with the new encoding.
//! @param SOURCE_ENCODING The encoding of the source file.
//! @param TARGET_ENCODING THe wanted encoding in the new file to be created.
//! @param cb A LwIoProgressCallback to use to give progress feedback or NULL
//! @param data A gpointer to data to pass to the LwIoProgressCallback
//! @param error pointer to a GError to write errors to
//! @return The status of the conversion opertaion
//!
gboolean 
lw_io_copy_with_encoding (const gchar           *SOURCE_PATH, 
                          const gchar           *TARGET_PATH,
                          const gchar           *SOURCE_ENCODING, 
                          const gchar           *TARGET_ENCODING,
                          LwIoProgressCallback   cb, 
                          gpointer               data, 
                          GCancellable          *cancellable,
                          GError               **error)
{
    if (*error != NULL) return FALSE;

    //Declarations
    FILE* readfd = NULL;
    FILE* writefd = NULL;
    const gint MAX = 1024 * 2;
    gchar source_buffer[MAX];
    gchar target_buffer[MAX];
    gchar *sptr, *tptr;
    size_t read, source_bytes_left, target_bytes_left;
    gdouble fraction;
    size_t position, filesize;
    GIConv conv;

    filesize = lw_io_get_filesize (SOURCE_PATH);
    position = 0;

    //Initializations
    readfd = fopen (SOURCE_PATH, "rb");
    writefd = fopen (TARGET_PATH, "wb");
    conv = g_iconv_open (TARGET_ENCODING, SOURCE_ENCODING);

    //Read a chunk
    while (ferror(readfd) == 0 && feof(readfd) == 0)
    {
      read = fread(source_buffer, sizeof(gchar), MAX, readfd);
      source_bytes_left = read;
      sptr = source_buffer;

      //Try to convert and write the chunk
      while (source_bytes_left > 0 && ferror(writefd) == 0 && feof(writefd) == 0)
      {
        target_bytes_left = MAX;
        tptr = target_buffer;

        g_iconv (conv, &sptr, &source_bytes_left, &tptr, &target_bytes_left);
        if (MAX != target_bytes_left) //Bytes were converted!
        {
          fwrite(target_buffer, sizeof(gchar), MAX - target_bytes_left, writefd); 
        }
        else if (source_bytes_left == MAX && target_bytes_left == MAX)
        {
          fprintf(stderr, "The file you are converting may be corrupt! Trying to skip a character...\n");
          fseek(readfd, 1L - source_bytes_left, SEEK_CUR);
        }
        else if (source_bytes_left > 0) //Bytes failed to convert!
        {
          fseek(readfd, -source_bytes_left, SEEK_CUR);
          source_bytes_left = 0;
        }
      }
      position = ftell(readfd);
      fraction = (gdouble) position / (gdouble) filesize;
      if (cb != NULL) cb (fraction, data);
    }

    //Cleanup
    g_iconv_close (conv);
    fclose(readfd);
    fclose(writefd);

    return TRUE;
}


//!
//! @brief Private function made to be used with lw_io_download
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
static size_t _libcurl_write_func (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}
 

//!
//! @brief Private function made to be used with lw_io_download
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
static size_t _libcurl_read_func (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fread (ptr, size, nmemb, stream);
}

//!
//! @brief Private struct made to be used with lw_io_download
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
static int _libcurl_update_progress (void   *custom,
                                     double  dltotal,
                                     double  dlnow,
                                     double  ultotal,
                                     double  ulnow   )
{
    //Declarations
    LwIoProgressCallbackWithData *cbwdata;
    LwIoProgressCallback cb;
    gpointer data;
    double fraction;
    GCancellable *cancellable;
    gboolean is_cancelled;
    
    //Initializations
    cbwdata = (LwIoProgressCallbackWithData*) custom;
    cb = cbwdata->cb;
    data = cbwdata->data;
    cancellable = cbwdata->cancellable;
    fraction = 0;

    if (dltotal > 0.0)
      fraction = dlnow / dltotal;

    //Update the interface
    is_cancelled = (cancellable != NULL && g_cancellable_is_cancelled (cancellable));
    if (is_cancelled) return 1;

    else return cb (fraction, data);
}




//!
//! @brief Downloads a file using libcurl
//! @param source_path String of the source url
//! @param target_path String of the path to save the file locally
//! @param cb A LwIoProgressCallback to use to give progress feedback or NULL
//! @param data gpointer to data to pass to the function pointer
//! @param error Error handling
//!
gboolean 
lw_io_download (const gchar           *SOURCE_PATH, 
                const gchar           *TARGET_PATH, 
                LwIoProgressCallback   cb,
                gpointer               data, 
                GCancellable          *cancellable,
                GError               **error)
{
    if (error != NULL && *error != NULL) return FALSE;

  curl_global_init (CURL_GLOBAL_ALL);

    //Declarations
    GQuark quark;
    CURL *curl;
    CURLcode res;
    FILE *outfile;
    const gchar *message;
    LwIoProgressCallbackWithData cbwdata;
    gboolean is_cancelled;

    //Initializations
    curl = curl_easy_init ();
    outfile = fopen(TARGET_PATH, "wb");
    cbwdata.cb = cb;
    cbwdata.data = data;
    cbwdata.cancellable = cancellable;
    res = 0;

    if (curl != NULL || outfile != NULL)
    {
      curl_easy_setopt(curl, CURLOPT_URL, SOURCE_PATH);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _libcurl_write_func);
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, _libcurl_read_func);

      if (cb != NULL)
      {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, _libcurl_update_progress);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &cbwdata);
      }

      res = curl_easy_perform(curl);
    }
     
    fclose(outfile);
    curl_easy_cleanup(curl);

    is_cancelled = (cancellable != NULL && g_cancellable_is_cancelled (cancellable));
    if (is_cancelled) return 1;

    if (res != 0 && is_cancelled == FALSE)
    {
      g_remove (TARGET_PATH);

      if (error != NULL) {
        message = gettext(curl_easy_strerror(res));
        quark = g_quark_from_string (LW_IO_ERROR);
        *error = g_error_new_literal (quark, LW_IO_DOWNLOAD_ERROR, message);
      }
    }

  curl_global_cleanup ();

    return (res == 0);
}


//!
//! @brief Copies a local file to another local location
//! @param SOURCE_PATH String of the source url
//! @param TARGET_PATH String of the path to save the file locally
//! @param cb A LwIoProgressCallback to use to give progress feedback or NULL
//! @param data A gpointer to data to pass to the LwIoProgressCallback
//! @param error Error handling
//!
gboolean 
lw_io_copy (const gchar           *SOURCE_PATH, 
            const gchar           *TARGET_PATH, 
            LwIoProgressCallback   cb, 
            gpointer               data, 
            GCancellable          *cancellable,
            GError               **error)
{
    if (*error != NULL) return FALSE;

    //Declarations
    FILE *infd;
    FILE *outfd;
    size_t chunk;
    size_t end;
    size_t curpos;
    const int MAX = 1024;
    char buffer[MAX];
    double fraction;
    gboolean is_cancelled;

    //Initalizations
    infd = fopen(SOURCE_PATH, "rb");
    outfd = fopen(TARGET_PATH, "wb");
    chunk = 1;
    end = lw_io_get_filesize (SOURCE_PATH);
    curpos = 0;
    fraction = 0.0;

    while (chunk)
    {
      is_cancelled = (cancellable != NULL && g_cancellable_is_cancelled (cancellable));
      if (is_cancelled) break; 

      fraction = ((double) curpos) / ((double) end);
      if (cb != NULL) cb (fraction, data);
      chunk = fread(buffer, sizeof(char), MAX, infd);
      chunk = fwrite(buffer, sizeof(char), chunk, outfd);
      curpos += chunk;
    }
    fraction = 1.0;
    if (cb != NULL) cb (fraction, data);

    //Cleanup
    fclose(infd);
    fclose(outfd);

    return (error == NULL && *error == NULL);;
}


//!
//! @brief Creates a single dictionary containing both the radical dict and kanji dict
//! @param output_path Mix dictionary path to write to
//! @param kanji_dictionary_path Kanjidic dictionary path
//! @param radicals_dictionary_path raddic dictionary path
//! @param cb A LwIoProgressCallback to use to give progress feedback or NULL
//! @param data A gpointer to data to pass to the LwIoProgressCallback
//! @param error pointer to a GError to write errors to
//!
gboolean 
lw_io_create_mix_dictionary (const gchar           *OUTPUT_PATH, 
                             const gchar           *KANJI_DICTIONARY_PATH, 
                             const gchar           *RADICALS_DICTIONARY_PATH, 
                             LwIoProgressCallback   cb,
                             gpointer               data,
                             GCancellable          *cancellable,
                             GError               **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;

    //Declarations
    FILE *output_file, *kanji_file, *radicals_file;
    char radicals_input[LW_IO_MAX_FGETS_LINE];
    char kanji_input[LW_IO_MAX_FGETS_LINE];
    char output[LW_IO_MAX_FGETS_LINE * 2];
    char *radicals_ptr, *kanji_ptr, *output_ptr, *temp_ptr;

    size_t curpos;
    size_t end;
    double fraction;
    gboolean is_cancelled;

    //Initializations
    kanji_file =  fopen(KANJI_DICTIONARY_PATH, "r");
    radicals_file = fopen(RADICALS_DICTIONARY_PATH, "r");
    output_file = fopen(OUTPUT_PATH, "w");
    radicals_ptr = NULL;
    kanji_ptr = NULL;
    output_ptr = NULL;

    curpos = 0;
    end = lw_io_get_filesize (KANJI_DICTIONARY_PATH);
    fraction = 0.0;

    //Loop through the kanji file
    while (fgets(kanji_input, LW_IO_MAX_FGETS_LINE, kanji_file) != NULL)
    {
      is_cancelled = (cancellable != NULL && g_cancellable_is_cancelled (cancellable));
      if (is_cancelled) break;

      fraction = ((double) curpos)/((double) end);
      if (cb != NULL) cb (fraction, data);

      curpos += strlen (kanji_input);

      if (kanji_input[0] == '#') continue;

      kanji_ptr = kanji_input;
      output_ptr = output;

      //1. Copy the kanji character from the kanji line
      while (*kanji_ptr != ' ')
      {
        *output_ptr = *kanji_ptr;
        output_ptr++;
        kanji_ptr++;
      }

      //2. Find the relevent radical line and insert it if available
      rewind (radicals_file);
      while (fgets(radicals_input, LW_IO_MAX_FGETS_LINE, radicals_file) != NULL)
      {
        //Check for a match
        temp_ptr = kanji_input;
        radicals_ptr = radicals_input;
        while (*radicals_ptr != ' ' && *radicals_ptr == *temp_ptr)
        {
          temp_ptr++;
          radicals_ptr++;
        }

        //If a match is found...
        if (*radicals_ptr == ' ')
        {
          //Skip over the colon
          radicals_ptr++;
          radicals_ptr++;
   
          //Copy the data
          while (*(radicals_ptr + 1) != '\0')
          {
            *output_ptr = *radicals_ptr;
            output_ptr++;
            radicals_ptr++;
          }

          break;
        }
      }

      //3. Copy the rest of the kanji line to output
      while (*kanji_ptr != '\0')
      {
        *output_ptr = *kanji_ptr;
        output_ptr++;
        kanji_ptr++;
      }

      //4. Close off the string and write it to the file
      *output_ptr = '\0';
      fputs(output, output_file);
      output[0] = '\0';
    }
    fraction = 1.0;
    if (cb != NULL) cb (fraction, data);

    //Cleanup
    fclose(kanji_file);
    fclose(output_file);
    fclose(radicals_file);

    return TRUE;
}


//!
//! @brief Splits the Names 
//! @param OUTPUT_NAMES_PATH The path to write the new Names dictionary to
//! @param OUTPUT_PLACES_PATH The path to write the new Places dictionary to
//! @param INPUT_NAMES_PLACES_PATH The file to use to generate the split dictionaries
//! @param cb A LwIoProgressCallback to use to give progress feedback or NULL
//! @param data A gpointer to data to pass to the LwIoProgressCallback
//! @param error pointer to a GError to write errors to
//!
gboolean 
lw_io_split_places_from_names_dictionary (const gchar           *OUTPUT_NAMES_PATH, 
                                          const gchar           *OUTPUT_PLACES_PATH,
                                          const gchar           *INPUT_NAMES_PLACES_PATH,
                                          LwIoProgressCallback   cb,
                                          gpointer               data,
                                          GCancellable          *cancellable,
                                          GError               **error)
{
    if (error != NULL && *error != NULL) return FALSE;

    /*
      Current composition of the Enamdic dictionary
      ----------------------------------------------
      s - surname (138,500)
      p - place-name (99,500)
      u - person name, either given or surname, as-yet unclassified (139,000) 
      g - given name, as-yet not classified by sex (64,600)
      f - female given name (106,300)
      m - male given name (14,500)
      h - full (family plus given) name of a particular person (30,500)
      pr - product name (55)
      co - company name (34)
      ---------------------------------------------
    */

    //Declarations
    char buffer[LW_IO_MAX_FGETS_LINE];
    FILE *inputf;
    size_t curpos;
    size_t end;
    double fraction;
    gboolean is_cancelled;

    FILE *placesf;
    GRegex *re_place;
    const gchar *place_pattern = "([\\(,])((p)|(st))([\\),])";
    int  place_write_error;

    FILE *namesf;
    GRegex *re_name;
    const gchar *name_pattern = "([\\(,])((s)|(u)|(g)|(f)|(m)|(h)|(pr)|(co))([\\),])";
    int  name_write_error;

    //Initializations
    inputf = fopen(INPUT_NAMES_PLACES_PATH, "r");
    curpos = 0;
    end = lw_io_get_filesize (INPUT_NAMES_PLACES_PATH);
    fraction = 0.0;

    re_place = g_regex_new (place_pattern,  LW_RE_COMPILE_FLAGS, LW_RE_LOCATE_FLAGS, error);
    placesf = fopen(OUTPUT_PLACES_PATH, "w");
    place_write_error = 0;

    re_name = g_regex_new (name_pattern,  LW_RE_COMPILE_FLAGS, LW_RE_LOCATE_FLAGS, error);
    namesf = fopen(OUTPUT_NAMES_PATH, "w");
    name_write_error  = 0;


    //Start writing the child files
    while (fgets(buffer, LW_IO_MAX_FGETS_LINE, inputf) != NULL &&
           place_write_error != EOF &&
           name_write_error  != EOF &&
           *error == NULL)
    {
      is_cancelled = (cancellable != NULL && g_cancellable_is_cancelled (cancellable));
      if (is_cancelled) break;

      fraction = ((double) curpos) / ((double) end);
      if (cb != NULL) cb (fraction, data);

      if (placesf != NULL && g_regex_match (re_place, buffer, 0, NULL))
        place_write_error = fputs(buffer, placesf);
      if (namesf != NULL && g_regex_match(re_name, buffer, 0, NULL))
        name_write_error =  fputs(buffer, namesf);
      curpos += strlen(buffer);
    }
    fraction = 1.0;
    if (cb != NULL) cb (fraction, data);

    //Cleanup
    fclose(inputf);
    fclose(placesf);
    fclose(namesf);
    g_regex_unref (re_place);
    g_regex_unref (re_name);

    return (place_write_error != EOF && name_write_error != EOF);
}


//!
//! @brief Decompresses a gzip file
//! @param SOURCE_PATH The path to the file that is gzipped
//! @param TARGET_PATH The path to write the uncompressed file to
//! @param cb A LwIoProgressCallback function to give progress feedback or NULL
//! @param data A generic pointer to data to pass to the LwIoProgressCallback
//! @param error A pointer to a GError object to write an error to or NULL
//!
gboolean 
lw_io_gunzip_file (const gchar           *SOURCE_PATH, 
                   const gchar           *TARGET_PATH,
                   LwIoProgressCallback   cb, 
                   gpointer               data, 
                   GCancellable          *cancellable,
                   GError               **error)
{
    if (error != NULL && *error != NULL) return FALSE;

    //Declarations
    gzFile source;
    FILE *target;
    int read;
    const int MAX = 1024;
    char buffer[MAX];
    gdouble fraction;
    size_t filesize, position;
    position = 0;

    source = gzopen (SOURCE_PATH, "rb");
    if (source != NULL)
    {
      filesize = lw_io_get_filesize (SOURCE_PATH);

      target = fopen(TARGET_PATH, "wb");
      if (target != NULL)
      {
        do {
          read = gzread (source, buffer, MAX);
          if (read > 0) 
          {
            position += MAX;
            if (position > filesize) position = filesize;
            fraction = (gdouble) position / (gdouble) filesize;
            if (cb != NULL) cb (fraction, data);
            fwrite(buffer, sizeof(char), read, target);
          }
        } while (read > 0);

        fclose(target); target = NULL;
      }
      gzclose(source); source = NULL;
    }

    return (error == NULL || *error == NULL);
} 


//!
//! @brief Decompresses a zip file
//! @param SOURCE_PATH The path to the file that is gzipped
//! @param cb A LwIoProgressCallback function to give progress feedback or NULL
//! @param data A generic pointer to data to pass to the LwIoProgressCallback
//! @param error A pointer to a GError object to write an error to or NULL
//!
gboolean 
lw_io_unzip_file (gchar                 *SOURCE_PATH, 
                  LwIoProgressCallback   cb, 
                  gpointer               data, 
                  GCancellable          *cancellable,
                  GError               **error)
{
    return TRUE;
}


//!
//! @brief Gets the size of a file in bytes
//! @param URI The path to the file to calculate the size of
//! @returns The size of the file in bytes
//!
size_t 
lw_io_get_filesize (const gchar *URI)
{
    //Sanity check
    g_assert (g_file_test (URI, G_FILE_TEST_IS_REGULAR));

    //Declarations
    const int MAX_CHUNK = 128;
    char buffer[MAX_CHUNK];
    FILE *file;
    size_t size;

    //Initializations
    file = fopen(URI, "rb");
    size = 0;

    while (file != NULL && ferror(file) == 0 && feof(file) == 0)
        size += fread(buffer, sizeof(char), MAX_CHUNK, file);

    //Cleanup
    fclose(file);

    return size;
}


//!
//! @brief Callback function to read data in from a file and send it to a stream
//! @param data A pointer to a LwIoProcessFdData object
//! @returns Returns if there was an error
//!
gpointer _stdin_func (gpointer data)
{
    //Sanity checks
    g_return_val_if_fail (data != NULL, NULL);

    //Declarations
    const int MAX_CHUNK = 128;
    char buffer[MAX_CHUNK];
    size_t chunk;
    size_t curpos;
    size_t end;
    double fraction;
    FILE *file;
    FILE *stream;
    LwIoProcessFdData* in;
    const char *message;
    GQuark domain;
    GCancellable *cancellable;
    gboolean is_cancelled;

    //Initalizations
    in = data;
    chunk = 0;
    curpos = 0;
    end = lw_io_get_filesize (in->uri);
    fraction = 0.0;
    file = fopen(in->uri, "rb");
    stream = fdopen(in->fd, "ab");
    cancellable = in->cancellable;

    while (file != NULL && ferror(file) == 0 && feof(file) == 0)
    {
      is_cancelled = (cancellable != NULL && g_cancellable_is_cancelled (cancellable));
      if (is_cancelled) break;

      fraction = ((double) curpos / (double) end);
      if (in->cb != NULL) in->cb (fraction, in->data);

      chunk = fread(buffer, sizeof(char), MAX_CHUNK, file);
      curpos += chunk;
      chunk = fwrite(buffer, sizeof(char), chunk, stream);
      fflush(stream);
    }
    fraction = 1.0;
    if (in->cb != NULL) in->cb (fraction, in->data);

    if (ferror(file) != 0)
    {
      domain = g_quark_from_string (LW_IO_ERROR);
      message = gettext("Unable to read data from the input file.");
      in->error = g_error_new (domain, LW_IO_READ_ERROR, "%s", message);
    }
    else if(ferror(stream) != 0)
    {
      domain = g_quark_from_string (LW_IO_ERROR);
      message = gettext("Unable to write to the external program's input stream.");
      in->error = g_error_new (domain, LW_IO_WRITE_ERROR, "%s", message);
    }

    //Cleanup
    fclose(file);
    fclose(stream);

    return (in->error);
}


//!
//! @brief Callback function to read data from a stream and write it to a file
//! @param data A pointer to a LwIoProcessFdData object
//! @returns Returns if there was an error
//!
gpointer _stdout_func (gpointer data)
{
    //Declarations
    const int MAX_CHUNK = 128;
    char buffer[MAX_CHUNK];
    size_t chunk;
    size_t curpos;
    FILE *file;
    FILE *stream;
    LwIoProcessFdData* out;
    const char *message;
    GQuark domain;
    GCancellable *cancellable;
    gboolean is_cancelled;

    //Initalizations
    out = data;
    chunk = 1;
    curpos = 0;
    file = fopen(out->uri, "wb");
    stream = fdopen(out->fd, "rb");
    cancellable = out->cancellable;

    while (file != NULL && ferror(file) == 0 && feof(file) == 0 && chunk != 0)
    {
      is_cancelled = (cancellable != NULL && g_cancellable_is_cancelled (cancellable));
      if (is_cancelled) break;

      chunk = fread(buffer, sizeof(char), MAX_CHUNK, stream);
      curpos += chunk;
      chunk = fwrite(buffer, sizeof(char), chunk, file);
    }

    if (ferror(stream) != 0)
    {
      domain = g_quark_from_string (LW_IO_ERROR);
      message = gettext("Unable to read data from the external program's pipe.");
      out->error = g_error_new (domain, LW_IO_READ_ERROR, "%s", message);
    }
    else if(ferror(stream) != 0)
    {
      domain = g_quark_from_string (LW_IO_ERROR);
      message = gettext("Unable to write the stream's output to a file.");
      out->error = g_error_new (domain, LW_IO_WRITE_ERROR, "%s", message);
    }

    //Cleanup
    fclose(stream);
    fclose(file);

    return (out->error);
}



//!
//! @brief Deletes a file from the filesystem
//! @param URI The path to the file to delet
//! @param error A pointer to a GError object to write errors to or NULL
//!
gboolean 
lw_io_remove (const gchar   *URI, 
              GCancellable  *cancellable,
              GError       **error)
{
  if (error != NULL && *error != NULL) return FALSE;

  g_remove (URI);

  return (error == NULL && *error == NULL);
}


//!
//! @brief A quick way to get the number of lines in a file for use in progress functions
//! @param FILENAME The path to the file to see how many lines it has
//!
long 
lw_io_get_size_for_uri (const char *URI)
{
    //Declarations
    FILE *file;
    long length;

    //Initializations
    file = fopen (URI, "r");
    length = 0L;

    if (file != NULL)
    {
      fseek (file, 0L, SEEK_END);
      length = ftell (file);
      fclose(file);
    }
   
    return length;
}


