#ifndef GW_QUERYLINE_OBJECT_INCLUDED
#define GW_QUERYLINE_OBJECT_INCLUDED

#define GW_QUERYLINE_MAX_ATOMS 20

struct _GwQueryLine {
    //Storage for the original query string
    char *string;

    //General search regexes
    GRegex*** re_kanji;
    GRegex*** re_furi;
    GRegex*** re_roma;
    GRegex*** re_mix;

    //Specific regexes for the kanji dictionary
    GRegex*** re_strokes;
    GRegex*** re_frequency;
    GRegex*** re_grade;
    GRegex*** re_jlpt;
};
typedef struct _GwQueryLine GwQueryLine;


GwQueryLine* gw_queryline_new (void );
void gw_queryline_free (GwQueryLine*);

int gw_queryline_parse_edict_string (GwQueryLine*l, const char*);
int gw_queryline_parse_kanjidict_string (GwQueryLine*, const char*);
int gw_queryline_parse_exampledict_string (GwQueryLine*, const char*);
int gw_queryline_parse_edict_string (GwQueryLine*, const char*);

#endif