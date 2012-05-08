#ifndef LW_VOCABULARY_INCLUDED
#define LW_VOCABULARY_INCLUDED

G_BEGIN_DECLS

typedef enum {
  LW_VOCABULARY_FIELD_KANJI,
  LW_VOCABULARY_FIELD_FURIGANA,
  LW_VOCABULARY_FIELD_DEFINITIONS,
  LW_VOCABULARY_FIELD_CORRECT_GUESSES,
  LW_VOCABULARY_FIELD_INCORRECT_GUESSES,
  LW_VOCABULARY_FIELD_TIMESTAMP,
  TOTAL_LW_VOCABULARY_FIELDS
} LwVocabularyField;


struct _LwVocabulary {
  gchar *fields[TOTAL_LW_VOCABULARY_FIELDS];
  gchar *score;
  gchar *days;
  gint correct_guesses;
  gint incorrect_guesses;
  gint32 timestamp;
};

typedef struct _LwVocabulary LwVocabulary;

#define LW_VOCABULARY(obj) (LwVocabulary*)obj

LwVocabulary* lw_vocabulary_new ();
LwVocabulary* lw_vocabulary_new_from_string (const gchar*);
void lw_vocabulary_free (LwVocabulary*);

void lw_vocabulary_set_kanji (LwVocabulary*, const gchar*);
const gchar* lw_vocabulary_get_kanji (LwVocabulary*);

void lw_vocabulary_set_furigana (LwVocabulary*, const gchar*);
const gchar* lw_vocabulary_get_furigana (LwVocabulary*);

void lw_vocabulary_set_definitions (LwVocabulary*, const gchar*);
const gchar* lw_vocabulary_get_definitions (LwVocabulary*);

gint lw_vocabulary_get_correct_guesses (LwVocabulary*);
void lw_vocabulary_set_correct_guesses (LwVocabulary*, gint);

gint lw_vocabulary_get_incorrect_guesses (LwVocabulary*);
void lw_vocabulary_set_incorrect_guesses (LwVocabulary*, gint);

gint lw_vocabulary_get_score (LwVocabulary*);
const gchar* lw_vocabulary_get_score_as_string (LwVocabulary*);

guint32 lw_vocabulary_timestamp_to_hours (gint64);
void lw_vocabulary_set_timestamp (LwVocabulary*, gint64);
void lw_vocabulary_update_timestamp (LwVocabulary*);
void lw_vocabulary_set_hours (LwVocabulary*, guint32);
guint32 lw_vocabulary_get_hours (LwVocabulary*);
const gchar* lw_vocabulary_get_timestamp_as_string (LwVocabulary*);

gchar* lw_vocabulary_to_string (LwVocabulary*);
void lw_vocabulary_update_timestamp (LwVocabulary*);
gint64 lw_vocabulary_get_timestamp (LwVocabulary *vocabulary);
const gchar* lw_vocabulary_get_timestamp_as_string (LwVocabulary *vocabulary);

G_END_DECLS

#endif
