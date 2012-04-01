#ifndef LW_RESULT_INCLUDED
#define LW_RESULT_INCLUDED

#include <libwaei/io.h>

G_BEGIN_DECLS

#define LW_RESULT(object) (LwResult*) object

typedef enum {
  LW_RESULT_RELEVANCE_UNSET,
  LW_RESULT_RELEVANCE_LOW,
  LW_RESULT_RELEVANCE_MEDIUM,
  LW_RESULT_RELEVANCE_HIGH,
  TOTAL_RESULT_RELEVANCE
} LwResultRelevance;

//!
//! @brief Primitive for storing lists of dictionaries
//!

struct _LwResult {
    char text[LW_IO_MAX_FGETS_LINE];     //!< Character array holding the result line for the pointers to reference

    //General result things
    LwResultRelevance relevance;
    char *def_start[50];        //!< Pointer to the definitions
    int def_total;              //!< Total definitions found for a result
    char *number[50];           //!< Pointer to the numbers of the definitions
    char *kanji_start;          //!< The kanji portion of the definition
    char *furigana_start;       //!< The furigana portion of a definition
    char *classification_start; //!< The classification of the word type of the Japanese word.

    //Kanji things
    char *strokes;      //!< Pointer to the number of strokes of a kanji
    char *frequency;    //!< Pointer to the frequency number of a kanji
    char *readings[3];  //!< Pointer to the readings of a kanji
    char *meanings;     //!< Pointer to the meanings of a kanji
    char *grade;        //!< Pointer to the grade of a kanji
    char *jlpt;         //!< Pointer to the JLPT level of a kanji
    char *kanji;        //!< Pointer to the kanji itself
    char *radicals;     //!< Pointer to a kanji's radicals

    gboolean important; //!< Weather a word/phrase has a high frequency of usage.

};
typedef struct _LwResult LwResult;


LwResult* lw_result_new (void);
void lw_result_free (LwResult*);
void lw_result_init (LwResult*);
void lw_result_deinit (LwResult*);

gboolean lw_result_is_similar (LwResult*, LwResult*);
void lw_result_clear (LwResult*);

G_END_DECLS

#endif
