#include <iostream>
#include <dbus/dbus.h>
#include <signal.h>

#ifndef CLIENT_H
#define CLIENT_H

typedef struct comm_info {
    const char *interface_name = "org.new.Methods";
    const char *server_bus_name = "com.test.app_bus";
    const char *client_bus_name = "com.fix.test";
    const char *object_path_name = "org/new/pack";
    const char *method_name = "SayHello";
    bool conn_flag = false;
    DBusConnection *conn = nullptr;
    DBusError err;
} server_info;

volatile sig_atomic_t break_flag;

void clean_up(int , void* );
void signal_handler(int );
void thread_handler(server_info *);

#endif