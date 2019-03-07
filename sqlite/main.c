#include <stdio.h>
#include <sqlite3.h>
//sudo apt-get install sqlite3 libsqlite3-dev
//linker -lsqlite3 -std=c99

int callback(void *NotUsed, int argc, char **argv,char **azColName)
{
    NotUsed = 0;
    for (int i = 0; i < argc; i++) {
        printf(" %s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
     }
    printf("\n");
    return 0;
}

int main(void)
{
    sqlite3 *db;
    char *err_msg = 0;
    char *sql;

    int rc = sqlite3_open("../networkSocket/dzData", &db);
    printf("\nRC open Database = %d\n",rc);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }/*

    sql = "DROP TABLE IF EXISTS User;"
                "CREATE TABLE User(Id INT, Name TEXT, Password TEXT);"
                "INSERT INTO User VALUES(1, 'dz', '123');"
                "INSERT INTO User VALUES(2, 'dz', '123');"
                "INSERT INTO User VALUES(3, 'haipen', '123');"
                "INSERT INTO User VALUES(4, 'aaa', '123');"
                "INSERT INTO User VALUES(5, 'bbb', '123');"
                "INSERT INTO User VALUES(6, 'ccc', '123');"
                "INSERT INTO User VALUES(7, 'ddd', '123');"
                "INSERT INTO User VALUES(8, 'eee', '123');";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    printf("\nRC sqltite_exec = %d\n\n",rc);
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    } else {
        fprintf(stdout, "Table Cars created successfully\n");
    }*/
    /*
    int last_id = sqlite3_last_insert_rowid(db);
    printf("The last Id of the inserted row is %d\n", last_id);

    sql = "SELECT * FROM Users group by Price";


    rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    printf("\nRC sqltite_exec = %d -> %s\n",rc,sql);
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
*/
    sqlite3_stmt *res;
    /*
    sql = "SELECT Name, Password FROM User WHERE Name = ? ";
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    printf("\nRC sqltite_exec = %d -> %s\n",rc,sql);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(res, 1, 'dz');
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    int step ;
    while ( (step = sqlite3_step(res) == SQLITE_ROW)) {
        printf("%s: ", sqlite3_column_text(res, 0));
        printf("%s\n", sqlite3_column_text(res, 1));
    }*/

    sql = "SELECT Name,Password FROM User WHERE Name = @id";
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc == SQLITE_OK) {
        int idx = sqlite3_bind_parameter_index(res, "@id");
        //char * cvalue = "Volvo";
        rc=sqlite3_bind_text(res, idx, "dz", -1, SQLITE_STATIC);     // the string is static
        printf("\nRC sqltite_exec = %d -> %s\n",rc,sql);
     } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }
    int step=sqlite3_step(res);
    printf("%d\n",step);
    printf("%s: ", sqlite3_column_text(res, 0));
    printf("%s\n", sqlite3_column_text(res, 1));

    sqlite3_finalize(res);
    sqlite3_close(db);
    return 0;
}
