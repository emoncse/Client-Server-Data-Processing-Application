#ifndef DB_HANDLER_H
#define DB_HANDLER_H

#include <mysql/mysql.h>

#define BUFFER_SIZE 1024

MYSQL *connect_to_database();
int parse_user_data(const char *user_data, int *id, char *first_name, char *last_name, char *email, char *city);
int execute_insert_query(MYSQL *conn, int id, const char *first_name, const char *last_name, const char *safe_email, const char *safe_city, int client_fd);

#endif
