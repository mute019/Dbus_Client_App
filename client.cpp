#include <iostream>
#include <dbus/dbus.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>
#include <stdlib.h>
#include "client.hpp"


int main() {

    server_info *servr = new server_info;

    if (dbus_error_is_set(&servr->err)) {
        fprintf(stderr, "Error: %s \n", servr->err.message);
        dbus_error_free(&servr->err);
    }

    servr->conn = dbus_connection_open("unix:path=/run/user/1001/bus", &servr->err);
    // servr->conn = dbus_bus_get(DBUS_BUS_SESSION, &servr->err);
    if (!dbus_bus_register(servr->conn, &servr->err)) {
        fprintf(stderr, "Error: %s\n", servr->err.message);
        exit(1);
    }

    if (servr->conn == NULL) {
        EXIT_FAILURE;
    }


    // signal(SIGINT, signal_handler);

    break_flag = false;

    while (!break_flag) {

        while(1) {
            if (dbus_error_is_set(&servr->err)) {
                dbus_error_free(&servr->err);
            }
            dbus_error_init(&servr->err);
            
            if (dbus_bus_name_has_owner(servr->conn, servr->client_bus_name, &servr->err)) {
                if (dbus_bus_release_name(servr->conn, servr->client_bus_name, &servr->err) == -1) {
                    fprintf(stderr, "Error: %s\n", servr->err.message);
                    exit(EXIT_FAILURE);
                }
            }
            int ret = dbus_bus_request_name(servr->conn, 
            servr->client_bus_name,
            DBUS_NAME_FLAG_REPLACE_EXISTING,
            &servr->err);



            if (ret == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
                break;
            }

            if (ret == DBUS_REQUEST_NAME_REPLY_IN_QUEUE) {
                fprintf(stderr, "Waiting for the bus connection \n");
                sleep(1);
                continue;
            }

            if (dbus_error_is_set(&servr->err)) {
                fprintf(stderr, "Error: %s\n", servr->err.message);
            }

        }
        fprintf(stdout, "[+]client: ");
    
    char* message_buffer = nullptr;
    size_t msg_buffer_size = 0;

    if (getline(&message_buffer, &msg_buffer_size, stdin) == -1) {
        fprintf(stderr, "Error: getline() function failed! \n");
        free(message_buffer);
        pthread_exit(nullptr);
    }

    // if (!servr->conn_flag) {
    //     // exit(EXIT_FAILURE);
    //     return;
    // }

    DBusMessage *request;

    if ((request = dbus_message_new_method_call(
        servr->server_bus_name,
        servr->object_path_name,
        servr->interface_name,
        servr->method_name
    )) == nullptr) {
        fprintf(stderr, "Error while processing requesting!\n");
        exit(EXIT_FAILURE);
    }

    DBusMessageIter iter;

    dbus_message_iter_init_append (request, &iter);

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &message_buffer)) {
        fprintf(stderr, "Error: dbus_message_iter_append_basic failed \n");
        exit(EXIT_FAILURE);
    }

    DBusPendingCall *pending_return;

    if (!dbus_connection_send_with_reply(servr->conn, request, &pending_return, -1)) {
        fprintf(stderr,"Error: dbus_connection_send_with_reply \n");
        exit(EXIT_FAILURE);
    }

    if (pending_return == nullptr) {
        fprintf(stderr, "Pending return is NULL\n");
        exit(EXIT_FAILURE);
    }

    dbus_connection_flush(servr->conn);

    dbus_message_unref(request);

    DBusMessage *reply;

    dbus_pending_call_block (pending_return);

    if ((reply = dbus_pending_call_steal_reply (pending_return)) == NULL) {
        fprintf (stderr, "Error in dbus_pending_call_steal_reply");
        exit (1);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(reply, &args)) {
        fprintf(stdout, "Message has no argument \n");
        break;
    }

    // auto value = dbus_message_iter_get_arg_type(&args);
    if (dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_ARRAY) {
        fprintf(stdout, "Here I am\n");
        break;
    }


    
    

    
    
    dbus_pending_call_unref(pending_return);

    dbus_message_unref(reply);

    dbus_bus_release_name(servr->conn, servr->client_bus_name, &servr->err);


    }
    
    // on_exit(&clean_up, servr->conn);

    return 0;
}

// void clean_up(int status, void *conn) {
    
//     fprintf(stderr, "status: %d \n", status);
// }

// void signal_handler(int data) {
//     fprintf(stdout, "\nPress Enter to exit!\n");
//     break_flag = true;
// }


