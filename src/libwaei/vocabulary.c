/******************************************************************************
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
//!  @file vocabulary.c
//!

#include <locale.h>

#include <libwaei/gettext.h>
#include <libwaei/libwaei.h>

const gchar* lw_vocabulary_get_kanji (LwVocabulary *vocabulary)
{
  return vocabulary->fields[LW_VOCABULARY_FIELD_KANJI];
}

void lw_vocabulary_set_kanji (LwVocabulary *vocabulary, const gchar *text)
{
  if (vocabulary->fields[LW_VOCABULARY_FIELD_KANJI] != NULL)
    g_free (vocabulary->fields[LW_VOCABULARY_FIELD_KANJI]);
  vocabulary->fields[LW_VOCABULARY_FIELD_KANJI] = g_strdup (text);
}

const gchar* lw_vocabulary_get_furigana (LwVocabulary *vocabulary)
{
  return vocabulary->fields[LW_VOCABULARY_FIELD_FURIGANA];
}

void lw_vocabulary_set_furigana (LwVocabulary *vocabulary, const gchar *text)
{
  if (vocabulary->fields[LW_VOCABULARY_FIELD_FURIGANA] != NULL)
    g_free (vocabulary->fields[LW_VOCABULARY_FIELD_FURIGANA]);
  vocabulary->fields[LW_VOCABULARY_FIELD_FURIGANA] = g_strdup (text);
}

const gchar* lw_vocabulary_get_definitions (LwVocabulary *vocabulary)
{
  return vocabulary->fields[LW_VOCABULARY_FIELD_DEFINITIONS];
}

void lw_vocabulary_set_definitions (LwVocabulary *vocabulary, const gchar *text)
{
  if (vocabulary->fields[LW_VOCABULARY_FIELD_DEFINITIONS] != NULL)
    g_free (vocabulary->fields[LW_VOCABULARY_FIELD_DEFINITIONS]);
  vocabulary->fields[LW_VOCABULARY_FIELD_DEFINITIONS] = g_strdup (text);
}

gint lw_vocabulary_get_correct_guesses (LwVocabulary *vocabulary)
{
  return vocabulary->correct_guesses;
}

void lw_vocabulary_set_correct_guesses (LwVocabulary *vocabulary, gint number)
{
  if (vocabulary->fields[LW_VOCABULARY_FIELD_CORRECT_GUESSES] != NULL)
    g_free (vocabulary->fields[LW_VOCABULARY_FIELD_CORRECT_GUESSES]);
  vocabulary->fields[LW_VOCABULARY_FIELD_CORRECT_GUESSES] = g_strdup_printf ("%d", number);
  vocabulary->correct_guesses = number;
  if (vocabulary->score != NULL) g_free (vocabulary->score); vocabulary->score = NULL;
}

gint lw_vocabulary_get_incorrect_guesses (LwVocabulary *vocabulary)
{
  return vocabulary->incorrect_guesses;
}

void lw_vocabulary_set_incorrect_guesses (LwVocabulary *vocabulary, gint number)
{
  if (vocabulary->fields[LW_VOCABULARY_FIELD_INCORRECT_GUESSES] != NULL)
    g_free (vocabulary->fields[LW_VOCABULARY_FIELD_INCORRECT_GUESSES]);
  vocabulary->fields[LW_VOCABULARY_FIELD_INCORRECT_GUESSES] = g_strdup_printf ("%d", number);
  vocabulary->incorrect_guesses = number;
  if (vocabulary->score != NULL) g_free (vocabulary->score); vocabulary->score = NULL;
}


gint 
lw_vocabulary_get_score (LwVocabulary *vocabulary)
{
    gint total = vocabulary->correct_guesses + vocabulary->incorrect_guesses;
    if (total == 0) return 0.0;
    else return (vocabulary->correct_guesses * 100 / total);
}


const gchar* 
lw_vocabulary_get_score_as_string (LwVocabulary *vocabulary)
{
    gint total;
    
    if (vocabulary->score == NULL)
    {
      total = vocabulary->correct_guesses + vocabulary->incorrect_guesses;
      if (total == 0)
        vocabulary->score = g_strdup (gettext("Untested"));
      else
        vocabulary->score = g_strdup_printf ("%3d%%", lw_vocabulary_get_score (vocabulary));
    }

    return vocabulary->score;
}

guint32
lw_vocabulary_timestamp_to_hours (gint64 timestamp)
{
    const gint MICROSECONDS = 1000000;
    const gint SECONDS = 60;
    const gint MINUTES = 60;
    return (guint32) (timestamp / MICROSECONDS / SECONDS / MINUTES);
}


void
lw_vocabulary_set_timestamp (LwVocabulary *vocabulary, gint64 timestamp)
{
    guint32 hours = lw_vocabulary_timestamp_to_hours (timestamp);
    lw_vocabulary_set_hours (vocabulary, hours);
}


void
lw_vocabulary_update_timestamp (LwVocabulary *vocabulary)
{
    lw_vocabulary_set_timestamp (vocabulary, g_get_real_time ());
}


void
lw_vocabulary_set_hours (LwVocabulary *vocabulary, guint32 hours)
{
    vocabulary->timestamp = hours;

    if (vocabulary->days != NULL) g_free (vocabulary->days); vocabulary->days = NULL;
    if (vocabulary->fields[LW_VOCABULARY_FIELD_TIMESTAMP] != NULL)
      g_free (vocabulary->fields[LW_VOCABULARY_FIELD_TIMESTAMP]);

    vocabulary->fields[LW_VOCABULARY_FIELD_TIMESTAMP] = g_strdup_printf ("%" G_GUINT32_FORMAT, vocabulary->timestamp);
}


guint32
lw_vocabulary_get_hours (LwVocabulary *vocabulary)
{
    return vocabulary->timestamp;
}


const gchar*
lw_vocabulary_get_timestamp_as_string (LwVocabulary *vocabulary)
{
    if (vocabulary->days == NULL)
    {
      guint32 days = lw_vocabulary_get_hours (vocabulary) / 24;
      guint32 today = lw_vocabulary_timestamp_to_hours ( g_get_real_time ()) / 24;
      guint32 difference = today - days;
      if (difference < 0) difference = 0;

      if (days == 0) vocabulary->days = g_strdup (pgettext("noun", "Never"));
      else if (difference == 0) vocabulary->days = g_strdup (gettext("Today"));
      else if (difference == 1) vocabulary->days = g_strdup (gettext("Yesterday"));
      else vocabulary->days = g_strdup_printf (ngettext("%d Day Ago", "%d Days Ago", difference), difference);
    }

    return vocabulary->days;
}


LwVocabulary*
lw_vocabulary_new ()
{
    LwVocabulary *vocabulary;

    vocabulary = g_new0 (LwVocabulary, 1);

    return vocabulary;
}


LwVocabulary*
lw_vocabulary_new_from_string (const gchar *text)
{
    //Declarations
    LwVocabulary *vocabulary;
    gchar *ptr;
    gchar *endptr;
    gchar **atoms;
    gint i;

    vocabulary = g_new0 (LwVocabulary, 1);
    if (vocabulary != NULL)
    {

      atoms = g_strsplit (text, ";", TOTAL_LW_VOCABULARY_FIELDS);
      if (atoms != NULL)
      {
        //Set up the strings
        for (i = 0; atoms[i] != NULL && i < TOTAL_LW_VOCABULARY_FIELDS; i++)
        {
          vocabulary->fields[i] = g_strdup (g_strstrip(atoms[i]));
        }
        for (i = 0; i < TOTAL_LW_VOCABULARY_FIELDS; i++)
        {
          if (vocabulary->fields[i] == NULL) vocabulary->fields[i] = g_strdup ("");
        }

        //Set up the integers
        ptr = vocabulary->fields[LW_VOCABULARY_FIELD_CORRECT_GUESSES];
        vocabulary->correct_guesses = (gint) g_ascii_strtoll (ptr, &endptr, 10);
        ptr = vocabulary->fields[LW_VOCABULARY_FIELD_INCORRECT_GUESSES];
        vocabulary->incorrect_guesses =  (gint) g_ascii_strtoll (ptr, &endptr, 10);
        ptr = vocabulary->fields[LW_VOCABULARY_FIELD_TIMESTAMP];
        vocabulary->timestamp =  (guint32) g_ascii_strtoll (ptr, &endptr, 10);
      }
      g_strfreev (atoms); atoms = NULL;
    }

    return vocabulary;
}


void
lw_vocabulary_free (LwVocabulary *vocabulary)
{
  gint i;
  for (i = 0; i < TOTAL_LW_VOCABULARY_FIELDS; i++)
  {
    if (vocabulary->fields[i] != NULL)
    {
      g_free (vocabulary->fields[i]);
      vocabulary->fields[i] = NULL;
    }
  }

  g_free (vocabulary->score); vocabulary->score = NULL;
  g_free (vocabulary->days); vocabulary->days = NULL;

  g_free (vocabulary);
}


gchar* 
lw_vocabulary_to_string (LwVocabulary *vocabulary)
{
    gchar* text;
    text = lw_strjoinv (';', vocabulary->fields, TOTAL_LW_VOCABULARY_FIELDS);
    return text;
}
