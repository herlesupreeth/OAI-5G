#include <stdlib.h>
#include <stdint.h>

#include <gtk/gtk.h>

#include "rc.h"

#include "socket.h"

#include "ui_notif_dlg.h"
#include "ui_main_screen.h"
#include "ui_callbacks.h"
#include "ui_interface.h"
#include "ui_notifications.h"
#include "ui_tree_view.h"
#include "ui_signal_dissect_view.h"

#include "types.h"
#include "locate_root.h"
#include "xml_parse.h"

static gboolean ui_handle_socket_connection_failed(gint fd);

gboolean ui_callback_on_open(GtkWidget *widget,
                             GdkEvent  *event,
                             gpointer   data)
{
    g_debug("Open event occurred");
    CHECK_FCT(ui_file_chooser());
    return TRUE;
}

gboolean ui_callback_on_about(GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data)
{

    return TRUE;
}

gboolean
ui_callback_on_select_signal(GtkTreeSelection *selection,
                             GtkTreeModel     *model,
                             GtkTreePath      *path,
                             gboolean          path_currently_selected,
                             gpointer          user_data)
{
    ui_text_view_t *text_view;
    GtkTreeIter iter;

    text_view = (ui_text_view_t *)user_data;

    g_assert(text_view != NULL);

    if (gtk_tree_model_get_iter(model, &iter, path))
    {
//         gchar *name;
        GValue buffer_store = G_VALUE_INIT;
        gpointer buffer;

//         g_value_init (&buffer_store, G_TYPE_POINTER);

//         gtk_tree_model_get(model, &iter, 0, &name, -1);

        gtk_tree_model_get_value(model, &iter, COL_BUFFER, &buffer_store);

        buffer = g_value_get_pointer(&buffer_store);

        if (!path_currently_selected)
        {
            /* Clear the view */
            CHECK_FCT_DO(ui_signal_dissect_clear_view(text_view), return FALSE);

            /* Dissect the signal */
            CHECK_FCT_DO(dissect_signal((buffer_t*)buffer, ui_signal_set_text, text_view), return FALSE);
        }
//         else
//         {
//             g_debug("%s is going to be unselected", name);
//         }
// 
//         g_free(name);
    }
    return TRUE;
}

void ui_signal_add_to_list(gpointer data, gpointer user_data)
{
    buffer_t *signal_buffer;

    signal_buffer = (buffer_t *)data;

    get_message_id(root, signal_buffer, &signal_buffer->message_id);

    ui_tree_view_new_signal_ind(signal_buffer->message_number,
                                message_id_to_string(signal_buffer->message_id),
                                get_origin_task_id(signal_buffer),
                                get_destination_task_id(signal_buffer),
                                data);
}

static gboolean ui_handle_update_signal_list(gint fd, void *data,
                                             size_t data_length)
{
    pipe_new_signals_list_message_t *signal_list_message;

    signal_list_message = (pipe_new_signals_list_message_t *)data;

    g_assert(signal_list_message != NULL);
    g_assert(signal_list_message->signal_list != NULL);

    g_list_foreach(signal_list_message->signal_list, ui_signal_add_to_list, NULL);

    free(data);

    return TRUE;
}

static gboolean ui_handle_socket_connection_failed(gint fd)
{
    GtkWidget *dialogbox;

    dialogbox = gtk_message_dialog_new(GTK_WINDOW(ui_main_data.window),
                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_ERROR,
                                       GTK_BUTTONS_CLOSE,
                                       "Failed to connect to provided host/ip address");

    gtk_dialog_run(GTK_DIALOG(dialogbox));
    gtk_widget_destroy(dialogbox);

    /* Re-enable connect button */
    ui_enable_connect_button();
    return TRUE;
}

static gboolean ui_handle_socket_connection_lost(gint fd)
{
    GtkWidget *dialogbox;

    dialogbox = gtk_message_dialog_new(GTK_WINDOW(ui_main_data.window),
                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_ERROR,
                                       GTK_BUTTONS_CLOSE,
                                       "Connection with remote host has been lost");

    gtk_dialog_run(GTK_DIALOG(dialogbox));
    gtk_widget_destroy(dialogbox);

    /* Re-enable connect button */
    ui_enable_connect_button();
    return TRUE;
}

static gboolean ui_handle_socket_xml_definition(gint fd, void *data,
                                                size_t data_length)
{
    pipe_xml_definition_message_t *xml_definition_message;

    xml_definition_message = (pipe_xml_definition_message_t *)data;
    g_assert(xml_definition_message != NULL);
    g_assert(data_length == sizeof(pipe_xml_definition_message_t));

    xml_parse_buffer(xml_definition_message->xml_definition,
                     xml_definition_message->xml_definition_length);

    free(data);

    return TRUE;
}

gboolean ui_pipe_callback(gint source, gpointer user_data)
{
    void                *input_data = NULL;
    size_t               input_data_length = 0;
    pipe_input_header_t  input_header;

    /* Read the header */
    if (read(source, &input_header, sizeof(input_header)) < 0) {
        g_warning("Failed to read from pipe %d: %s", source, g_strerror(errno));
        return FALSE;
    }

    input_data_length = input_header.message_size - sizeof(input_header);

    /* Checking for non-header part */
    if (input_data_length > 0) {
        input_data = malloc(input_data_length);

        if (read(source, input_data, input_data_length) < 0) {
            g_warning("Failed to read from pipe %d: %s", source, g_strerror(errno));
            return FALSE;
        }
    }

    switch (input_header.message_type) {
        case UI_PIPE_CONNECTION_FAILED:
            return ui_handle_socket_connection_failed(source);
        case UI_PIPE_XML_DEFINITION:
            return ui_handle_socket_xml_definition(source, input_data, input_data_length);
        case UI_PIPE_CONNECTION_LOST:
            return ui_handle_socket_connection_lost(source);
        case UI_PIPE_UPDATE_SIGNAL_LIST:
            return ui_handle_update_signal_list(source, input_data, input_data_length);
        default:
            g_debug("[gui] Unhandled message type %u", input_header.message_type);
            g_assert_not_reached();
    }
    return FALSE;
}

gboolean ui_callback_on_connect(GtkWidget *widget,
                                GdkEvent  *event,
                                gpointer   data)
{
    /* We have to retrieve the ip address and port of remote host */
    const char *ip;
    uint16_t    port;
    int         pipe_fd[2];

    g_debug("Connect event occurred");

    port = atoi(gtk_entry_get_text(GTK_ENTRY(ui_main_data.portentry)));
    ip = gtk_entry_get_text(GTK_ENTRY(ui_main_data.ipentry));

    if ((ip == NULL) || (port == 0)) {
        g_warning("NULL parameter given for ip address or port = 0");
        /* TODO: add dialog box here */
        return FALSE;
    }

    ui_pipe_new(pipe_fd, ui_pipe_callback, NULL);

    memcpy(ui_main_data.pipe_fd, pipe_fd, sizeof(int) * 2);

    /* Disable the connect button */
    ui_disable_connect_button();

    if (socket_connect_to_remote_host(ip, port, pipe_fd[1]) != 0) {
        ui_enable_connect_button();
        return FALSE;
    }

    return TRUE;
}

gboolean ui_callback_on_disconnect(GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data)
{
    /* We have to retrieve the ip address and port of remote host */

    g_debug("Disconnect event occurred");

    ui_pipe_write_message(ui_main_data.pipe_fd[0], UI_PIPE_DISCONNECT_EVT,
                          NULL, 0);

    ui_enable_connect_button();
    return TRUE;
}

gboolean ui_callback_on_tree_view_select(GtkWidget *widget,
                                         GdkEvent  *event,
                                         gpointer   data)
{
    /* We have to retrieve the ip address and port of remote host */
    g_debug("List selection event occurred");
    return TRUE;
}
