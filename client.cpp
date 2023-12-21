#include <iostream>
#include <dbus/dbus.h>
#include <pthread.h>
#include <signal.h>
#include "client.hpp"


int main() {

    server_info *servr = new server_info;

    dbus_error_init(&servr->err);

    if (dbus_error_is_set(&servr->err)) {
        fprintf(stderr, "Error: %s \n", servr->err.message);
        dbus_error_free(&servr->err);
    }

    servr->conn = dbus_connection_open_private("unix:path=/run/user/1001/bus", &servr->err);
    if (!dbus_bus_register(servr->conn, &servr->err)) {
        fprintf(stderr, "Error: %s\n", servr->err.message);
        exit(1);
    }

    if (servr->conn == NULL) {
        EXIT_FAILURE;
    }


    int ret = dbus_bus_request_name(servr->conn, 
        "com.fix.test_app",
        DBUS_NAME_FLAG_ALLOW_REPLACEMENT,
        &servr->err);



    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
        fprintf(stderr, "Name: \nError: %s \n", servr->err.message);
        exit(1);
    }

    fprintf(stdout, "[+]connection established\n");

    signal(SIGINT, signal_handler);

    break_flag = false;

    while (!break_flag) {
        char client_message[100];
        pthread_t thread;

        int thrd = pthread_create(&thread, NULL, (void* (*) (void*))&thread_handler, servr);

        fprintf(stdout, "[+]client: ");

        if (fgets(client_message, 100, stdin) == NULL) {
            fprintf(stderr, "[-]connection terminated");
            break;
        }
    
    }
    
    on_exit(&clean_up, servr->conn);

    return 0;
}

void clean_up(int status, void *conn) {
    dbus_connection_close((DBusConnection*)conn);
    fprintf(stderr, "Status: %d \n", status);
}

void signal_handler(int data) {
    break_flag = true;
}

void thread_handler(server_info *usr_data) {
    // const char *unique_name = dbus_bus_get_unique_name(usr_data->conn);

    // fprintf(stdout, "Unique Name: %s\n", unique_name);
}

