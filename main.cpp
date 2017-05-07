#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

sqlite3 *db = NULL;
char *err = NULL;

void stdclear() {
    while (getchar() != '\n');
}

void getstring(char* buf) {
    char c;
    int i = 0;
    scanf("%c", &c);
    while (c != '\n') {
        buf[i++] = c;
        scanf("%c", &c);
    }
    buf[i] = 0;
}

int printTable(void *arg, int argc, char **argv, char **azColName) {
    int i;
    int ID = -1;
    int Photo = -1;
    for (i = 0; i < argc; i++) {
        if (!strcmp(azColName[i], "ID")) {
            printf("\n\n___________________Employee with ID: %-3s___________________\n", argv[i]);
            ID = i;
        }
        if (!strcmp(azColName[i], "photo"))
            Photo = i;
        if (ID != -1 && Photo != -1)
            break;
    }
    for (i = 0; i<argc; i++)
        if (i != ID && i != Photo)
            printf("%-18s:%40s\n", azColName[i], argv[i]);

    return 0;
}

char Menu() {
    printf("1 - SELECT query\n");
    printf("2 - INSERT query\n");
    printf("3 - DELETE query\n");
    printf("4 - Show photo\n");
    printf("0 - Quit\n");
    char command = getchar();
    while (command<'0' || command>'4') {
        printf("No such command, choose(1, 2, 3, 4 or 0 to quit)!");
        stdclear();
    }
    stdclear();
    return command - 48;
}

void Insert() {
    char* sql = (char*)malloc(4098);
    char* buf = (char*)malloc(256);
    printf("\nWould you like to insert a photo?(y/n)");
    char command = getchar();

    while (command != 'y' && command != 'n') {
        printf("Answer yes or no(y/n)");
        stdclear();
        command = getchar();
    }
    stdclear();
    if (command == 'n')
        strcpy(sql, "insert into Employees(name, patronymic, surname, birthDate, locality, birthCountry, address, department, post, dateBegin) values(\'");
    else
        strcpy(sql, "insert into Employees(name, patronymic, surname, birthDate, photo, locality, birthCountry, address, department, post, dateBegin) values(\'");

    printf("\n\nName: ");
    scanf("%s", buf);
    strcat(sql, buf);
    strcat(sql, "\', \'");

    printf("Patronymic: ");
    scanf("%s", buf);
    strcat(sql, buf);
    strcat(sql, "\', \'");

    printf("Surname: ");
    scanf("%s", buf);
    strcat(sql, buf);
    strcat(sql, "\', \'");

    printf("Date of birth: ");
    scanf("%s", buf);
    strcat(sql, buf);


    if (command == 'y')
        strcat(sql, "\', ?, \'");
    else
        strcat(sql, "\', \'");


    printf("Locality: ");
    scanf("%s", buf);
    strcat(sql, buf);
    strcat(sql, "\', \'");

    printf("Homeland: ");
    scanf("%s", buf);
    strcat(sql, buf);
    strcat(sql, "\', \'");
    getchar();

    printf("Address: ");
    getstring(buf);
    strcat(sql, buf);
    strcat(sql, "\', \'");

    printf("Department: ");
    scanf("%s", buf);
    strcat(sql, buf);
    strcat(sql, "\', \'");

    printf("Current position: ");
    scanf("%s", buf);
    strcat(sql, buf);
    strcat(sql, "\', \'");


    printf("Date of employment: ");
    scanf("%s", buf);
    strcat(sql, buf);

    if (command == 'n') {
        strcat(sql, "\');");
        if (sqlite3_exec(db, sql, 0, 0, &err))
        {
            fprintf(stderr, "Error SQL: %s", err);
            sqlite3_free(err);
        }
    }
    if (command == 'y')
    {
        strcat(sql, "\')");
        sqlite3_stmt *res;
        if (sqlite3_prepare_v2(db, sql, -1, &res, 0))
            fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
        char* filename = (char*)malloc(256);
        printf("Enter name of file(or path): ");
        scanf("%s", filename);
        FILE* f = fopen(filename, "rb");
        while (f == NULL) {
            printf("Incorrect name of file!\nEnter name of file(or path): ");
            scanf("%s", filename);
            f = fopen(filename, "rb");
        }

        fseek(f, 0, SEEK_END);
        int nFileLen = ftell(f);
        fseek(f, 0, SEEK_SET);
        char* binPhoto = (char*)malloc(nFileLen);
        fread(binPhoto, 1, nFileLen, f);
        sqlite3_bind_blob(res, 1, binPhoto, nFileLen, NULL);
        int k = sqlite3_step(res);
        sqlite3_finalize(res);
        free(binPhoto);
    }
    free(sql);
}

char* paramName[] = { "ID", "surname", "birthCountry" };
void Select() {
    printf("__Select menu__\n");
    printf("0 - all\n");
    int i;
    for (i = 0; i < 3; ++i)
        printf("%d - by %s\n", i + 1, paramName[i]);

    char command = getchar();
    while (command<'0' || command>'3') {
        printf("No such command, choose(0 - 3)!\n");
        stdclear();
        command = getchar();
    }
    stdclear();
    command -= 48;
    if (command == 0)
        if (!sqlite3_exec(db, "select * from Employees", printTable, 0, &err))
            return;
        else
        {
            fprintf(stderr, "Error SQL: %s", err);
            sqlite3_free(err);
            return;
        }

    char* sql = (char*)malloc(64);
    sqlite3_stmt *res;

    strcpy(sql, "select * from Employees where ");
    strcat(sql, paramName[command - 1]);
    strcat(sql, " = ?");
    int rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    free(sql);
    if (rc == SQLITE_OK)
    {
        printf("Enter parameter: ");
        if (command == 1)
        {
            int id;
            scanf("%d", &id);
            rc = sqlite3_bind_int(res, 1, id);

        }
        else {
            char* param = (char*)malloc(128);
            scanf("%s", param);
            rc = sqlite3_bind_text(res, 1, param, -1, SQLITE_TRANSIENT);
            free(param);
        }
        if (rc != SQLITE_OK) {
            fprintf(stderr, "Failed to bind param: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(res);
            return;
        }
    }
    else
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));

    char anyData = 0;
    while (sqlite3_step(res) == SQLITE_ROW) {
        anyData = 1;
        int ID = -1;
        int Photo = -1;
        int columnCnt = sqlite3_column_count(res);
        for (i = 0; i < columnCnt; i++) {
            if (!strcmp(sqlite3_column_name(res, i), "ID")) {
                printf("\n\n___________________Employee with ID: %-3s___________________\n", sqlite3_column_text(res, i));
                ID = i;
            }
            if (!strcmp(sqlite3_column_name(res, i), "photo"))
                Photo = i;
            if (ID != -1 && Photo != -1)
                break;
        }
        for (i = 0; i < columnCnt; i++)
            if (i != ID && i != Photo)
                printf("%-18s:%40s\n", sqlite3_column_name(res, i), sqlite3_column_text(res, i)); // text
    }
    if (anyData)
        printf("No results found!\n");
    printf("\n\n");
    sqlite3_finalize(res);
}

void Delete(char tr) {
    if (tr) {
        int i;
        char* sql = (char*)malloc(4098);
        strcpy(sql, "begin");
        printf("How many employees you want to exclude: ");
        int cnt;
        scanf("%d", &cnt);
        for (i = 0; i < cnt; ++i) {
            printf("\n%d :", i + 1);
            int id;
            scanf("%d", &id);
            strcat(sql, "; delete from main where id = ");
            sprintf(sql + strlen(sql), "%d", id);
        }
        strcat(sql, "; commit;");
        sqlite3_exec(db, sql, 0, 0, &err);
        sqlite3_free(err);
        printf("\n\n");
        free(sql);
    }
    else {
        char* sql = (char*)malloc(64);
        strcpy(sql, "delete from Employees where ID = ");
        printf("Enter the employee ID you want to exclude : ");
        scanf("%s", sql + strlen(sql));
        strcat(sql, ";");		
        if (sqlite3_exec(db, sql, 0, 0, &err))
        {
            fprintf(stderr, "Error SQL: %s", err);
            sqlite3_free(err);
        }
        free(sql);
    }
    stdclear();
}

void Photo() {
    sqlite3_stmt *res;
    if (!sqlite3_prepare_v2(db, "select ID, photo from Employees where ID = ?", -1, &res, 0))
    {
        printf("Enter ID of employee: ");
        int id;
        scanf("%d", &id);
        if (sqlite3_bind_int(res, 1, id)) {
            fprintf(stderr, "Failed to bind param: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(res);
            return;
        }
    }
    else
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));


    if (sqlite3_step(res) == SQLITE_ROW) {
        char* filename = (char*)malloc(256);
        printf("Enter name of file(or path): ");
        scanf("%s", filename);
        FILE* f = fopen(filename, "wb");
        while (f == NULL) {
            printf("Incorrect name of file!\nEnter name of file(or path): ");
            scanf("%s", filename);
            f = fopen(filename, "wb");
        }
        
        fwrite(sqlite3_column_blob(res, 1), 1, sqlite3_column_bytes(res, 1), f);
        fclose(f);
        printf("Photo of employee with ID = %d successfully writen in file:\n%s\n\n", sqlite3_column_int(res, 0), filename);
        free(filename);
    }
    else
        printf("No results found!\n");
    printf("\n\n");
    sqlite3_finalize(res);
}

int main() {
    if (sqlite3_open("My_SQLite3_Database.db", &db)) {
        fprintf(stderr, "Error opening: %s", sqlite3_errmsg(db));
        return 1;
    }
    char lb = 1;
    while (lb)
    {
        switch (Menu()) {
            case 1: Select(); break;
            case 2: Insert(); break;
            case 3: {
                printf("Delete one or many employees?(o/m):");
                char command = getchar();
                while (command != 'o' && command != 'm') {
                    printf("Answer one or many(o/m)");
                    stdclear();
                    command = getchar();
                }
                stdclear();
                Delete(command - 'o');
            }break;
            case 4: Photo(); break;
            case 0: lb = 0;
        }
        getchar();
        printf("\n");
    }
    sqlite3_close(db);
}
