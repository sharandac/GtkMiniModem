/* gtkminimodem-window.c
 *
 * Copyright 2019 Dirk Broßwick
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "gtkminimodem-config.h"
#include "gtkminimodem-window.h"

GThread *minimodem_readthread;
GThread *minimodem_writethread;

static int baudrate=50;
int byte=-1;

struct _GtkminimodemWindow
{
  GtkApplicationWindow  parent_instance;

  /* Template widgets */
  GtkHeaderBar        *header_bar;
  GtkToggleButton     *speed_50bps;
  GtkToggleButton     *speed_100bps;
  GtkToggleButton     *speed_300bps;
  GtkTextBuffer       *recievetextbuffer;
  GtkTextBuffer       *sendtextbuffer;
  GtkTextView         *sendtext;
  GtkTextView         *recievetext;
};

G_DEFINE_TYPE (GtkminimodemWindow, gtkminimodem_window, GTK_TYPE_APPLICATION_WINDOW)

/*
 *
 */
void *
mainread( void * self ) {
  FILE *minimodem;
  char minimodem_exec[128];

  printf("minimodem readthread started ...\r\n");

  snprintf( minimodem_exec, sizeof(minimodem_exec),"minimodem --rx %d -q", baudrate );
  minimodem = popen( minimodem_exec,"r");

  while( !feof( minimodem ) ) {
    if ( byte < 0 ) {
      byte = (char)fgetc( minimodem );
    }
  }
  byte=-1;
  return(0);
}

/*
 *
 */
static void
on_sendbutton_trash_button_press_event( GtkTextBuffer *widget )
{
  GtkTextIter start, end;

  gtk_text_buffer_get_start_iter( widget, &start );
  gtk_text_buffer_get_end_iter( widget, &end );
  gtk_text_buffer_delete( widget, &start , &end );
}

/*
 *
 */
static void
on_recieve_trash_button_press_event( GtkTextBuffer *widget )
{
  GtkTextIter start, end;

  gtk_text_buffer_get_start_iter( widget, &start );
  gtk_text_buffer_get_end_iter( widget, &end );
  gtk_text_buffer_delete( widget, &start , &end );
}

/*
 *
 */
static void
on_speed_50bps_button_press_event( GtkminimodemWindow* window )
{
  baudrate=50;
  system("killall minimodem");
  minimodem_readthread = g_thread_new( "minimodemread", mainread, NULL);
}


/*
 *
 */
static void
on_speed_100bps_button_press_event( GtkminimodemWindow* window)
{
  baudrate=100;
  system("killall minimodem");
  minimodem_readthread = g_thread_new( "minimodemread", mainread, NULL);
}

/*
 *
 */
static void
on_speed_300bps_button_press_event( GtkminimodemWindow* window)
{
  baudrate=300;
  system("killall minimodem");
  minimodem_readthread = g_thread_new( "minimodemread", mainread, NULL);
}

/*
 *
 */
void*
sendtext( void * sendtextbuffer )
{
  FILE* minimodem;
  char minimodem_exec[128];

  if ( snprintf( minimodem_exec, sizeof(minimodem_exec),"minimodem --tx %d -q", baudrate ) >= sizeof(minimodem_exec) ) {
    printf("Error while creating exec-string ...\r\n");
  }
  else {
    minimodem = popen( minimodem_exec,"w");
    if ( minimodem == NULL ) {
      printf("Error wihle open a pipe to minimodem ... \r\n");
    }
    else {
      fwrite( sendtextbuffer, strlen(sendtextbuffer), 1, minimodem );
      fflush( minimodem );
      pclose( minimodem );
    }
  }
  g_thread_exit(0);
}

/*
 *
 */
static void
on_sendbuffer_button_press_event( GtkTextBuffer* widget )
{
  GtkTextIter start, end;

  gtk_text_buffer_get_start_iter( widget, &start );
  gtk_text_buffer_get_end_iter( widget, &end );

  minimodem_writethread = g_thread_new( "minimodem_writethread", sendtext, (void*)gtk_text_buffer_get_text(widget, &start, &end, FALSE ) );
}

/*
 *
 */
void
save_file(GtkTextBuffer * widget) {
  GtkWidget *dialog;
  GtkFileChooser *chooser;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
  gint res;

  dialog = gtk_file_chooser_dialog_new ("Save File",
                                        NULL,
                                        action,
                                        "_Cancel",
                                        GTK_RESPONSE_CANCEL,
                                        "_Save",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
  chooser = GTK_FILE_CHOOSER (dialog);

  gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

  gtk_file_chooser_set_current_name (chooser, "Untitled document.txt" );

  res = gtk_dialog_run (GTK_DIALOG (dialog));
  if (res == GTK_RESPONSE_ACCEPT)
    {
      char *filename;
      GtkTextIter start, end;
      FILE *fd;

      filename = gtk_file_chooser_get_filename (chooser);
      gtk_text_buffer_get_start_iter( widget, &start );
      gtk_text_buffer_get_end_iter( widget, &end );

      fd = fopen( filename, "w");
      fwrite( (void*)gtk_text_buffer_get_text(widget, &start, &end, FALSE ), strlen( (void*)gtk_text_buffer_get_text(widget, &start, &end, FALSE ) ), 1, fd );
      fclose( fd );
      g_free (filename);
    }

  gtk_widget_destroy (dialog);
}

/*
 *
 */
void
convert_file(GtkTextBuffer *widget) {
  GtkWidget *dialog;
  GtkFileChooser *chooser;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
  gint res;

  dialog = gtk_file_chooser_dialog_new ("Save File",
                                        NULL,
                                        action,
                                        "_Cancel",
                                        GTK_RESPONSE_CANCEL,
                                        "_Save",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
  chooser = GTK_FILE_CHOOSER (dialog);

  gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

  gtk_file_chooser_set_current_name (chooser, "Untitled document.ogg" );

  res = gtk_dialog_run (GTK_DIALOG (dialog));
  if (res == GTK_RESPONSE_ACCEPT)
    {
      char *filename;
      char convert_string[512]="";
      GtkTextIter start, end;
      FILE *fd;

      filename = gtk_file_chooser_get_filename (chooser);
      printf("Filename= %s\r\n", filename );
      gtk_text_buffer_get_start_iter( widget, &start );
      gtk_text_buffer_get_end_iter( widget, &end );

      snprintf (convert_string, sizeof( convert_string ), "minimodem --tx %d -f /tmp/test.wav ; oggenc -q 3 -o '%s' /tmp/test.wav ; rm /tmp/test.wav", baudrate, filename );

      fd = popen( convert_string, "w" );

      fwrite( (void*)gtk_text_buffer_get_text(widget, &start, &end, FALSE ), strlen( (void*)gtk_text_buffer_get_text(widget, &start, &end, FALSE ) ), 1, fd );
      fflush( fd );
      pclose( fd );

      g_free (filename);
    }

  gtk_widget_destroy (dialog);
}

/*
 *
 */
static void
on_GtkminimodemWindow_destroy ( GtkWidget* widget )
{
  system("killall minimodem");
}

/*
 *
 */
gboolean
mainloop_exec( GtkminimodemWindow* window )
{
  char buffer[2]="";
  buffer[1]='\0';

  if ( byte >= 0 ) {
    buffer[0]=(char)byte;
    gtk_text_buffer_insert_at_cursor ( (GtkTextBuffer*)window->recievetextbuffer, buffer, 1 );
    byte=-1;
  }

  gtk_toggle_button_set_active( (GtkToggleButton*)window->speed_50bps, FALSE );
  gtk_toggle_button_set_active( (GtkToggleButton*)window->speed_100bps, FALSE );
  gtk_toggle_button_set_active( (GtkToggleButton*)window->speed_300bps, FALSE );

  switch ( baudrate ) {
    case 50:    gtk_toggle_button_set_active( (GtkToggleButton*)window->speed_50bps, TRUE );
                break;
    case 100:   gtk_toggle_button_set_active( (GtkToggleButton*)window->speed_100bps, TRUE );
                break;
    case 300:   gtk_toggle_button_set_active( (GtkToggleButton*)window->speed_300bps, TRUE );
                break;
  }

  return TRUE;
}

/*
 *
 */
static void
gtkminimodem_window_class_init (GtkminimodemWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/sharan/gtkminimodem/gtkminimodem-window.ui");
  gtk_widget_class_bind_template_child (widget_class, GtkminimodemWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, GtkminimodemWindow, speed_50bps);
  gtk_widget_class_bind_template_child (widget_class, GtkminimodemWindow, speed_100bps);
  gtk_widget_class_bind_template_child (widget_class, GtkminimodemWindow, speed_300bps);
  gtk_widget_class_bind_template_child (widget_class, GtkminimodemWindow, recievetextbuffer);
  gtk_widget_class_bind_template_child (widget_class, GtkminimodemWindow, sendtextbuffer);
  gtk_widget_class_bind_template_child (widget_class, GtkminimodemWindow, sendtext);
  gtk_widget_class_bind_template_child (widget_class, GtkminimodemWindow, recievetext);
  gtk_widget_class_bind_template_callback (widget_class, on_sendbutton_trash_button_press_event);
  gtk_widget_class_bind_template_callback (widget_class, on_sendbuffer_button_press_event);
  gtk_widget_class_bind_template_callback (widget_class, on_recieve_trash_button_press_event);
  gtk_widget_class_bind_template_callback (widget_class, on_speed_50bps_button_press_event);
  gtk_widget_class_bind_template_callback (widget_class, on_speed_100bps_button_press_event);
  gtk_widget_class_bind_template_callback (widget_class, on_speed_300bps_button_press_event);
  gtk_widget_class_bind_template_callback (widget_class, convert_file);
  gtk_widget_class_bind_template_callback (widget_class, save_file);
  gtk_widget_class_bind_template_callback (widget_class, on_GtkminimodemWindow_destroy );
}

/*
 *
 */
static void
gtkminimodem_window_init (GtkminimodemWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  minimodem_readthread = g_thread_new( "minimodemread", mainread, NULL);
  gtk_text_view_set_monospace ((GtkTextView*)self->sendtext, TRUE);
  gtk_text_view_set_monospace ((GtkTextView*)self->recievetext, TRUE);
  (void)g_timeout_add (10, (GSourceFunc)mainloop_exec, self );
}
