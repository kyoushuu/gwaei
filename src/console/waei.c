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
//! @file src/console/waei.c
//!
//! @brief Main entrance into the program.
//!
//! Main entrance into the program.
//!

#include <stdlib.h>

#include <glib.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>

static gboolean _ncurses_switch = FALSE;

int main (int argc, char *argv[])
{    
    //Declarations
    int resolution;
    GError *error;

#ifdef WITH_NCURSES
    GOptionEntry entries[] = 
    {
      { "ncurses", 'n', 0, G_OPTION_ARG_NONE, &_ncurses_switch, "description", NULL },
      { NULL }
    };

    GOptionContext *context;
    context = g_option_context_new ("ncurses");
    g_option_context_add_main_entries (context, entries, PACKAGE);
    g_option_context_set_help_enabled (context, FALSE);
    g_option_context_parse (context, &argc, &argv, &error);
    g_option_context_free (context);
    context = NULL;
#endif

    //Initializations
    error = NULL;
    gw_backend_initialize (&argc, argv);
    gw_frontend_initialize (&argc, argv);

#ifdef WITH_NCURSES
    //Start the program
    if (_ncurses_switch)
      resolution = gw_frontend_start_ncurses (argc, argv);
    else
#endif
      resolution = gw_frontend_start_console (argc, argv);

    //Cleanup
    gw_frontend_free ();
    gw_backend_free();

    return resolution;
}

