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

    servr->conn = dbus_connection_open_private("unix:path=/run/user/1001/bus", &servr->err);
    // servr->conn = dbus_bus_get(DBUS_BUS_SESSION, &servr->err);
    if (!dbus_bus_register(servr->conn, &servr->err)) {
        fprintf(stderr, "Error: %s\n", servr->err.message);
        exit(1);
    }

    if (servr->conn == NULL) {
        EXIT_FAILURE;
    }


    signal(SIGINT, signal_handler);

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
        
        pthread_t thread;

        int thrd = pthread_create(&thread, NULL, (void* (*) (void*))&thread_handler, servr);

        DBusMessage *ping_msg = dbus_message_new_method_call(
            servr->server_bus_name,
            "/",
            "org.freedesktop.DBus.Peer",
            "Ping"
        );

        DBusMessage *ping_reply = dbus_connection_send_with_reply_and_block(servr->conn, ping_msg, -1, &servr->err);


        if (ping_reply == nullptr && dbus_error_is_set(&servr->err)) {
            if (servr->conn_flag) {
                servr->conn_flag = !servr->conn_flag;
            }
            fprintf(stderr, "Error: %s\n", servr->err.message);
            sleep(1);
            continue;
        } else {
            if (!servr->conn_flag) {
                fprintf(stdout, "[+]connection established\n");
                servr->conn_flag = !servr->conn_flag;
            }
        }

    }
    
    on_exit(&clean_up, servr->conn);

    return 0;
}

void clean_up(int status, void *conn) {
    
    fprintf(stderr, "Status: %d \n", status);
}

void signal_handler(int data) {
    fprintf(stdout, "Press Enter to exit!\n");
    break_flag = true;
}

void thread_handler(server_info *servr) {

    if (!servr->conn_flag) {
        pthread_exit(nullptr);
    }

    fprintf(stdout, "[+]client: ");

    char* message_buffer = nullptr;
    size_t msg_buffer_size = 0;

    if (getline(&message_buffer, &msg_buffer_size, stdin) == -1) {
        fprintf(stderr, "Error: getline() function failed! \n");
        free(message_buffer);
        pthread_exit(nullptr);
    }

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

    dbus_pending_call_block(pending_return);

    DBusMessage *reply;

    if ((reply = dbus_pending_call_steal_reply(pending_return)) == nullptr) {
        fprintf(stderr, "Error: dbus_pending_call_steal_reply \n");
        exit(EXIT_FAILURE);
    }

    dbus_pending_call_unref(pending_return);

    char *response;

    if (!dbus_message_get_args (reply, &servr->err, DBUS_TYPE_STRING, &response, DBUS_TYPE_INVALID)) {
        fprintf(stderr, "Error %s\n", servr->err.message);
        exit(EXIT_FAILURE);
    }

    dbus_message_unref(reply);
    
    dbus_bus_release_name(servr->conn, servr->client_bus_name, &servr->err);
}

