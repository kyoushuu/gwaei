#ifndef LW_SEARCH_INCLUDED
#define LW_SEARCH_INCLUDED

#include <stdio.h>

#include <libwaei/query.h>
#include <libwaei/result.h>
#include <libwaei/dictionary.h>

G_BEGIN_DECLS

#define LW_MAX_HIGH_RELEVENT_RESULTS 1000
#define LW_MAX_MEDIUM_IRRELEVENT_RESULTS 1000
#define LW_MAX_LOW_IRRELEVENT_RESULTS    1000

#define LW_SEARCH(object) (LwSearch*) object
#define LW_SEARCH_DATA_FREE_FUNC(object) (LwSearchDataFreeFunc)object
#define LW_HISTORY_TIME_TO_RELEVANCE 20

//!
//! @brief Search status types
//!
typedef enum
{
  LW_SEARCHSTATUS_IDLE,
  LW_SEARCHSTATUS_SEARCHING,
  LW_SEARCHSTATUS_CANCELING,
  LW_SEARCHSTATUS_FINISHING
} LwSearchStatus;


typedef enum
{
  LW_SEARCH_FLAG_EXACT              = (1 << 0)
/*
  LW_SEARCH_FLAG_DELIMIT_WHITESPACE = (1 << 1),
  LW_SEARCH_FLAG_JAPANESE_STEP      = (1 << 2),
  LW_SEARCH_FLAG_ROMAJI_STEP        = (1 << 3)
*/
} LwSearchFlags;

typedef void(*LwSearchDataFreeFunc)(gpointer);

//!
//! @brief Primitive for storing search item information
//!
//Object
struct _LwSearch {
    LwQuery* query;                 //!< Result line to store parsed result
    LwDictionary* dictionary;                 //!< Pointer to the dictionary used

    FILE* fd;                               //!< File descriptor for file search position
    GThread *thread;                        //!< Thread the search is processed in
    GMutex mutex;                          //!< Mutext to help ensure threadsafe operation

    LwSearchStatus status;                  //!< Used to test if a search is in progress.
    LwSearchFlags flags;
    char *scratch_buffer;                   //!< Scratch space
    long current;                           //!< Current line in the dictionary file
    int history_relevance_idle_timer;       //!< Helps determine if something is added to the history or not

    int total_relevant_results;             //!< Total results guessed to be highly relevant to the query
    int total_irrelevant_results;           //!< Total results guessed to be vaguely relevant to the query
    int total_results;                      //!< Total results returned from the search

    GList *results_high;                    //!< Buffer storing mediumly relevant result for later display
    GList *results_medium;                  //!< Buffer storing mediumly relevant result for later display
    GList *results_low;                     //!< Buffer storing lowly relevant result for later display

    LwResult* result;               //!< Result line to store parsed result

    gpointer data;                 //!< Pointer to a buffer that stays constant unlike when the target attribute is used

    gint16 preferences;

    LwSearchDataFreeFunc free_data_func;
};
typedef struct _LwSearch LwSearch;

//Methods
LwSearch* lw_search_new (const gchar*, LwDictionary*, LwSearchFlags, GError**);
void lw_search_free (LwSearch*);

void lw_search_cleanup_search (LwSearch*);
void lw_search_clear_results (LwSearch*);
void lw_search_prepare_search (LwSearch*);

gboolean lw_search_compare (LwSearch *, const LwRelevance);
gboolean lw_search_is_equal (LwSearch*, LwSearch*);
gboolean lw_search_has_history_relevance (LwSearch*, gboolean);
void lw_search_increment_history_relevance_timer (LwSearch*);

void lw_search_set_data (LwSearch*, gpointer, LwSearchDataFreeFunc);
gpointer lw_search_get_data (LwSearch*);
void lw_search_free_data (LwSearch*);
gboolean lw_search_has_data (LwSearch*);

gboolean lw_search_should_check_results (LwSearch*);
LwResult* lw_search_get_result (LwSearch*);
void lw_search_parse_result_string (LwSearch*);
void lw_search_cancel (LwSearch*);

void lw_search_lock (LwSearch*);
void lw_search_unlock (LwSearch*);

void lw_search_set_status (LwSearch*, LwSearchStatus);
LwSearchStatus lw_search_get_status (LwSearch*);

double lw_search_get_progress (LwSearch*);
gboolean lw_search_read_line (LwSearch*);

void lw_search_start (LwSearch*, gboolean);



G_END_DECLS

#endif
