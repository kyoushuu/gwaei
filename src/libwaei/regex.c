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
//!  @file regex.c
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <libwaei/libwaei.h>


static int _regex_expressions_reference_count = 0; //!< Internal reference count for the regexes
GRegex *lw_re[LW_RE_TOTAL + 1]; //!< Globally accessable pre-compiled regexes

/*


  { LW_RE_FILENAME_GZ, "\\.gz$", LW_REGEX_EFLAGS_EXIST },

  { LW_RE_WORD_I_ADJ_PASTFORM,    "\\B((かった))$", LW_REGEX_EFLAGS_LOCATE },
  { LW_RE_WORD_I_ADJ_NEGATIVE,    "\\B((くない))$", LW_REGEX_EFLAGS_LOCATE },
  { LW_RE_WORD_I_ADJ_TE_FORM,     "\\B((くて))$", LW_REGEX_EFLAGS_LOCATE },
  { LW_RE_WORD_I_ADJ_CAUSATIVE,   "\\B((くさせる))$", LW_REGEX_EFLAGS_LOCATE },
  { LW_RE_WORD_I_ADJ_CONDITIONAL, "\\B((ければ))$", LW_REGEX_EFLAGS_LOCATE },


  { LW_RE_WORD_NA_ADJ_PASTFORM,    "\\B((だった))$", LW_REGEX_EFLAGS_LOCATE },
  { LW_RE_WORD_NA_ADJ_NEGATIVE,    "\\B((ではない)|(じゃない))$", LW_REGEX_EFLAGS_LOCATE },
  { LW_RE_WORD_NA_ADJ_TE_FORM,     "\\B((で))$", LW_REGEX_EFLAGS_LOCATE },
  { LW_RE_WORD_NA_ADJ_CAUSATIVE,   "\\B((にさせる))$", LW_REGEX_EFLAGS_LOCATE },
  { LW_RE_WORD_NA_ADJ_CONDITIONAL, "\\B((であれば))$", LW_REGEX_EFLAGS_LOCATE },

    //Verb forms
    regcomp (re_verb_presentform, "\\B((ます))$", LW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_politepast, "\\B((ました))$", LW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_pastform_negative, "\\B((なかった))$", LW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_pastform, "\\B((った)|(いた)|(いだ)|(した)|(んだ)|(えた))$", LW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_negative, "\\B((わない)|(かない)|(がない)|(さない)|(たない)|(なない)|(まない)|(いない))$", LW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_te_form, "\\B((って)|(いて)|(いで)|(して)|(んで))$", LW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_potention, "\\B((える)|(ける)|(げる)|(せる)|(てる)|(ねる)|(べる)|(める)|(れる)|(いられる)|(えられる)|(いれる))$", LW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_causative, "\\B((させる)|(わせる)|(かせる)|(がせる)|(なせる)|(たせる)|(ばせる)|ませる(らせる)|(いさせる)|())$", LW_REGEX_EFLAGS_EXIST);
    regcomp (re_conditional, "\\B((すれば)|(くれば)|(であれば)|(えば)|(けば)|(げば)|(せば)|(てば)|(ねば)|(べば)|(めば)|(れば)|(いれば)|(れば))$", LW_REGEX_EFLAGS_EXIST);
    regcomp (re_negative_conditional, "\\B((なければ))$", LW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_imperative, "\\B((しろ)|(せよ)|(こい)|(くれ)|(ませ)|(であれ)|(え)|(け)|(せ)|(て)|(ね)|(べ)|(め)|(れ)|(いろ)|(えろ))$", LW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_passive, "\\B((される)|(こられる)|(われる)|(かれる)|(がれる)|(される)|(たれる)|(なれる)|(ばれる)|(まれる)|(られる)|(いられる)|(えられる))$", LW_REGEX_EFLAGS_EXIST);
    regcomp (re_verb_volitional, "\\B((しよう)|(せよう)|(こよう)|(だろう)|(ましょう)|(おう)|(こう)|(ごう)|(そう)|(とう)|(のう)|(ぼう)|(もう)|(ろう)|(いよう)|(よう))$", LW_REGEX_EFLAGS_EXIST);
*/



//!
//! @brief Initializes often used prebuilt regex expressions
//!
void 
lw_regex_initialize ()
{
    if (_regex_expressions_reference_count > 0) return;
    _regex_expressions_reference_count++;

    //Declarations
    GError *error;
    int i;
    
    //Initializations
    error = NULL;

    //Setup the built in regexes
    for (i = 0; i < LW_RE_TOTAL; i++)
    {
      switch (i)
      {
        case LW_RE_NUMBER:
          lw_re[i] = g_regex_new ("[a-zA-Z][0-9]{1,4}",  LW_RE_COMPILE_FLAGS, LW_RE_LOCATE_FLAGS, &error);
          break;
        case LW_RE_STROKES:
          lw_re[i] = g_regex_new ("S[0-9]{1,2}",  LW_RE_COMPILE_FLAGS, LW_RE_LOCATE_FLAGS, &error);
          break;
        case LW_RE_GRADE:
          lw_re[i] = g_regex_new ("G[0-9]{1,2}",  LW_RE_COMPILE_FLAGS, LW_RE_LOCATE_FLAGS, &error);
          break;
        case LW_RE_FREQUENCY:
          lw_re[i] = g_regex_new ("F[0-9]{1,4}",  LW_RE_COMPILE_FLAGS, LW_RE_LOCATE_FLAGS, &error);
          break;
        case LW_RE_JLPT:
          lw_re[i] = g_regex_new ("J[0-4]{1,1}",  LW_RE_COMPILE_FLAGS, LW_RE_LOCATE_FLAGS, &error);
          break;
        default:
          g_assert_not_reached();
      }
    }
    lw_re[i] = NULL;

    if (error != NULL)
    {
       fprintf (stderr, "Unable to read file: %s\n", error->message);
       g_error_free (error);
    }
}


//!
//! @brief Frees often used prebuilt regex expressions
//!
void 
lw_regex_free ()
{
    if (_regex_expressions_reference_count < 1) return;
    _regex_expressions_reference_count--;

    //Declarations
    int i;

    //Setup the built in regexes
    for (i = 0; i < LW_RE_TOTAL; i++)
    {
      g_regex_unref (lw_re[i]);
      lw_re[i] = NULL;
    }
}

