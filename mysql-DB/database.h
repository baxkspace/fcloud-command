#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include "database.h"

void finish_error(MYSQL *conn);
void mysqlConenct(char* id, char* pw);
void mysqlMake();