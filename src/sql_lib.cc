
#include "sql_lib.h"

//add a query param
void query_params::push (std::string key, std::string value)
{
    std::pair<std::string, std::string> toAdd(key, value);
    selectors.push_back(toAdd);
}

//constructor takes a name for the database to connect to
sql_manager::sql_manager (std::string db_name)
{
    db_name_ = db_name;
}

//open a connection to the internal database
bool sql_manager::connect ()
{
    int rc = sqlite3_open(db_name_.c_str(), &db_);
    if (rc) { return false; }
    else { return true; }
}

//open a connection to the internal database
bool sql_manager::disconnect ()
{
    int rc = sqlite3_close(db_);
    if (rc) { return false; }
    else { return true; }
}

//create a table if it doesn't already exist
bool sql_manager::create_table(std::string table_name, query_params table_params)
{
    std::string q = "CREATE TABLE IF NOT EXISTS ";
    q += (table_name + "(");
    for (unsigned int i=0; i<table_params.selectors.size(); i++)
    {
        q += (table_params.selectors[i].first + " " + table_params.selectors[i].second);
        if (i != table_params.selectors.size() - 1) { q += ","; }
        else { q += ");"; }
    }
    
    char* err;
    int rc = sqlite3_exec(db_, q.c_str(), NULL, NULL, &err);
    if (rc != SQLITE_OK) { return false; }
    else { return true; }
}

//drop the given table if it exists
bool sql_manager::drop_table(std::string table_name)
{
    std::string q = "DROP TABLE IF EXISTS " + table_name + ";";
    
    char* err;
    int rc = sqlite3_exec(db_, q.c_str(), NULL, NULL, &err);
    if (rc != SQLITE_OK) { return false; }
    else { return true; }
}

//insert a record into a table
bool sql_manager::insert_record(std::string table_name, query_params table_params){
    // create insert prepared statement
    std::string q = "INSERT INTO " + table_name + "(";
    for (unsigned int i=0; i<table_params.selectors.size(); i++)
    {
        q += table_params.selectors[i].first;
        if (i != table_params.selectors.size() - 1) { q += ","; }
        else { q += ") VALUES("; }
    }
    for (unsigned int i=0; i<table_params.selectors.size(); i++)
    {
        int bind_param = i + 1;
        q += ("?" + std::to_string(bind_param));
        if (i != table_params.selectors.size() - 1) { q += ","; }
        else { q += ");"; }
    }

    sqlite3_stmt *stmt; 
    sqlite3_prepare_v2(db_, q.c_str(), -1, &stmt, NULL);

    // bind the ? with values
    for (unsigned int i=0; i<table_params.selectors.size(); i++)
    {
        int bind_param = i + 1;
        sqlite3_bind_text(stmt, bind_param, table_params.selectors[i].second.c_str(), -1, SQLITE_STATIC); 
    }
    
    // check that inserted successfully
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR << "ERROR inserting data: " << sqlite3_errmsg(db_);
        return false;
    }
    sqlite3_finalize(stmt); 
    return true;
}

//update records in a table under certain conditions
bool sql_manager::update_record(std::string table_name, query_params set_params, query_params where_params) {
    // create update prepared statement
    std::string q = "UPDATE " + table_name + " SET ";
    int bind_param = 1;
    for (unsigned int i=0; i<set_params.selectors.size(); i++)
    {
        q += (set_params.selectors[i].first + "= ?" + std::to_string(bind_param));
        if (i != set_params.selectors.size() - 1) { q += ","; }
        bind_param++;
    }
    for (unsigned int i=0; i<where_params.selectors.size(); i++)
    {
        if (i == 0) { q+= " WHERE "; }
        q += (where_params.selectors[i].first + "= ?" + std::to_string(bind_param));
        if (i != where_params.selectors.size() - 1) { q += " AND "; }
        else { q += ";"; }
        bind_param++;
    }

    sqlite3_stmt *stmt; 
    sqlite3_prepare_v2(db_, q.c_str(), -1, &stmt, NULL);

    // bind the ? with values
    bind_param = 1;
    for (unsigned int i=0; i<set_params.selectors.size(); i++)
    {
        sqlite3_bind_text(stmt, bind_param, set_params.selectors[i].second.c_str(), -1, SQLITE_STATIC); 
        bind_param++;
    }
    for (unsigned int i=0; i<where_params.selectors.size(); i++)
    {
        sqlite3_bind_text(stmt, bind_param, where_params.selectors[i].second.c_str(), -1, SQLITE_STATIC); 
        bind_param++;
    }
    
    // check that updated successfully
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR << "ERROR updating data: " << sqlite3_errmsg(db_);
        return false;
    }
    sqlite3_finalize(stmt); 
    return true;
}

//delete records in a table under certain conditions
bool sql_manager::delete_record(std::string table_name, query_params table_params) {
    // create delete prepared statement
    std::string q = "DELETE FROM " + table_name + " ";
    for (unsigned int i=0; i<table_params.selectors.size(); i++)
    {
        int bind_param = i + 1;
        if (i == 0) { q+= "WHERE "; }
        q += (table_params.selectors[i].first + "= ?" + std::to_string(bind_param));
        if (i != table_params.selectors.size() - 1) { q += " AND "; }
        else { q += ";"; }
        bind_param++;
    }

    sqlite3_stmt *stmt; 
    sqlite3_prepare_v2(db_, q.c_str(), -1, &stmt, NULL);

    // bind the ? with values
    for (unsigned int i=0; i<table_params.selectors.size(); i++)
    {
        int bind_param = i + 1;
        sqlite3_bind_text(stmt, bind_param, table_params.selectors[i].second.c_str(), -1, SQLITE_STATIC); 
    }
    
    // check that deleted successfully
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        LOG_ERROR << "ERROR deleting data: " << sqlite3_errmsg(db_);
        return false;
    }
    sqlite3_finalize(stmt); 
    return true;
} 

//select records in a table under certain conditions
std::vector<std::vector<std::string>> sql_manager::select_record(std::string table_name, query_params table_params) {
    // create select prepared statement
    std::string q = "SELECT * FROM " + table_name + " ";
    for (unsigned int i=0; i<table_params.selectors.size(); i++)
    {
        int bind_param = i + 1;
        if (i == 0) { q+= "WHERE "; }
        q += (table_params.selectors[i].first + "= ?" + std::to_string(bind_param));
        if (i != table_params.selectors.size() - 1) { q += " AND "; }
        else { q += ";"; }
    }

    sqlite3_stmt *stmt; 
    sqlite3_prepare_v2(db_, q.c_str(), -1, &stmt, NULL);

    // bind the ? with values
    for (unsigned int i=0; i<table_params.selectors.size(); i++)
    {
        int bind_param = i + 1;
        sqlite3_bind_text(stmt, bind_param, table_params.selectors[i].second.c_str(), -1, SQLITE_STATIC); 
    }
   
    // loop storing the result 
    std::vector<std::vector<std::string>> result;
    bool done = false;
    while (!done) {
        switch (sqlite3_step (stmt)) {
            case SQLITE_ROW: {
                // loops through each column of the row, storing the value
                std::vector<std::string> temp;
                for (int i = 0; i < sqlite3_data_count(stmt); i++) {
                    std::string text = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
                    temp.push_back(text);
                }
                result.push_back(temp);
                break;
            }
            case SQLITE_DONE:
                done = true;
                break;
            default:
                LOG_ERROR << "ERROR selecting data: " << sqlite3_errmsg(db_);
                return result;
        }
    }
    sqlite3_finalize(stmt); 
    return result;
}

//search records in a table under certain conditions
std::vector<std::vector<std::string>> sql_manager::search_record(std::string table_name, query_params table_params) {
    // create select prepared statement
    std::string q = "SELECT * FROM " + table_name + " ";
    for (unsigned int i=0; i<table_params.selectors.size(); i++)
    {
        int bind_param = i + 1;
        if (i == 0) { q+= "WHERE "; }
        q += (table_params.selectors[i].first + " LIKE '%' || ?" + std::to_string(bind_param) + "|| '%'");
        if (i != table_params.selectors.size() - 1) { q += " OR "; }
        else { q += ";"; }
    }

    sqlite3_stmt *stmt; 
    sqlite3_prepare_v2(db_, q.c_str(), -1, &stmt, NULL);

    // bind the ? with values
    for (unsigned int i=0; i<table_params.selectors.size(); i++)
    {
        int bind_param = i + 1;
        // std::string search = "%" + table_params.selectors[i].second + "%";
        sqlite3_bind_text(stmt, bind_param, table_params.selectors[i].second.c_str(), -1, SQLITE_STATIC); 
    }
   
    // loop storing the result 
    std::vector<std::vector<std::string>> result;
    bool done = false;
    while (!done) {
        switch (sqlite3_step (stmt)) {
            case SQLITE_ROW: {
                // loops through each column of the row, storing the value
                std::vector<std::string> temp;
                for (int i = 0; i < sqlite3_data_count(stmt); i++) {
                    std::string text = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
                    temp.push_back(text);
                }
                result.push_back(temp);
                break;
            }
            case SQLITE_DONE:
                done = true;
                break;
            default:
                LOG_ERROR << "ERROR searching data: " << sqlite3_errmsg(db_);
                return result;
        }
    }
    sqlite3_finalize(stmt); 
    return result;
}