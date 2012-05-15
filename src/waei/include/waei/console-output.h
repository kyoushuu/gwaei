#ifndef W_CONSOLE_OUTPUT_INCLUDED
#define W_CONSOLE_OUTPUT_INCLUDED

void w_console_append_result (WApplication*, LwSearch*);
void w_console_no_result (WApplication*, LwSearch*);

int w_console_install_progress_cb (gdouble, gpointer);
int w_console_uninstall_progress_cb (gdouble, gpointer);


#endif
