#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

// Bitwise flags for constraints
const unsigned int PRIMARY_KEY = 1;   
const unsigned int NOT_NULL = 2;      
const unsigned int UNIQUE = 4;        


class Column {
public:
    string name;
    string type;
    unsigned int constraints;

    Column(string n, string t, unsigned int c) {
        name = n;
        type = t;
        constraints = c;
    }
};


class Row {
public:
    vector<string> values;
};


// Table class
class Table {
public:
    string tableName;
    vector<Column> columns;
    vector<Row*> rows;

    Table(string name) {
        tableName = name;
    }

    //free all allocated memory
    ~Table() {
        for (int i = 0; i < rows.size(); i++) {
            delete rows[i];
        }
    }

   
    void addColumn(string name, string type, unsigned int constraints) {
        columns.push_back(Column(name, type, constraints));
    }

    // Check if value is valid for column constraints
    bool isValid(int colIndex, string value, string& errorMsg) {
        Column& col = columns[colIndex];
        
        // Check NOT NULL using bitwise AND
        if ((col.constraints & NOT_NULL) == NOT_NULL) {
            if (value == "NULL" || value == "") {
                errorMsg = "Column " + col.name + " cannot be NULL";
                return false;
            }
        }

        // Check UNIQUE using bitwise AND
        if ((col.constraints & UNIQUE) == UNIQUE) {
            for (int i = 0; i < rows.size(); i++) {
                if (rows[i]->values[colIndex] == value) {
                    errorMsg = "Duplicate value in unique column " + col.name;
                    return false;
                }
            }
        }

        // Check PRIMARY KEY using bitwise AND
        if ((col.constraints & PRIMARY_KEY) == PRIMARY_KEY) {
           
            if (value == "NULL" || value == "") {
                errorMsg = "Primary key " + col.name + " cannot be NULL";
                return false;
            }
            
            for (int i = 0; i < rows.size(); i++) {
                if (rows[i]->values[colIndex] == value) {
                    errorMsg = "Duplicate primary key " + col.name;
                    return false;
                }
            }
        }

        return true;
    }

    // Insert a row
    bool insertRow(vector<string> values) {
        if (values.size() != columns.size()) {
            cout << "Error: Column count mismatch" << endl;
            return false;
        }

        // Validate all values
        for (int i = 0; i < columns.size(); i++) {
            string errorMsg;
            if (!isValid(i, values[i], errorMsg)) {
                cout << "Error: " << errorMsg << endl;
                return false;
            }
        }

        // Create new row dynamically
        Row* newRow = new Row();
        newRow->values = values;
        rows.push_back(newRow);
        return true;
    }

    // Display all rows
    void selectAll() {
       
        for (int i = 0; i < columns.size(); i++) {
            cout << columns[i].name << "\t";
        }
        cout << endl;

        for (int i = 0; i < columns.size(); i++) {
            cout << "--------";
        }
        cout << endl;

        // Print rows
        for (int i = 0; i < rows.size(); i++) {
            for (int j = 0; j < rows[i]->values.size(); j++) {
                cout << rows[i]->values[j] << "\t";
            }
            cout << endl;
        }
    }

    // Save table to file
    void saveToFile() {
        ofstream file(tableName + ".txt");
        
        file << "TABLE " << tableName << endl;

        for (int i = 0; i < columns.size(); i++) {
            file << columns[i].name << " " 
                 << columns[i].type << " " 
                 << columns[i].constraints << endl;
        }

        // Write data section
        file << "DATA" << endl;

        for (int i = 0; i < rows.size(); i++) {
            for (int j = 0; j < rows[i]->values.size(); j++) {
                file << rows[i]->values[j];
                if (j < rows[i]->values.size() - 1) file << " ";
            }
            file << endl;
        }

        file.close();
        cout << "Table saved to " << tableName << ".txt" << endl;
    }
};

Table* currentTable = NULL;

// constraints
unsigned int parseConstraints(string flags) {
    unsigned int result = 0;
    
    for (int i = 0; i < flags.length(); i++) {
        if (flags[i] == '1') {
            // Primary Key
            result = result | PRIMARY_KEY;  
        }
        else if (flags[i] == '2') {
            // Not Null
            result = result | NOT_NULL;     
        }
        else if (flags[i] == '4') {
            // Unique
            result = result | UNIQUE;      
        }
    }
    return result;
}


void createTable() {
    char buffer[256];
    string tableName;
    int numCols;

    cout << "Enter table name: ";
    cin >> tableName;

    // Delete old table if exists
    if (currentTable != NULL) {
        delete currentTable;
    }

    currentTable = new Table(tableName);

    cout << "Enter number of columns: ";
    cin >> numCols;
    cin.ignore(); 

    for (int i = 0; i < numCols; i++) {
        string colName, colType, constraints;
        
        cout << "\nColumn " << (i + 1) << ":\n";
        cout << "Name: ";
        cin >> colName;
        cout << "Type (int/string): ";
        cin >> colType;
        cout << "Constraints (1=PK, 2=NotNull, 4=Unique, combine with +, 0=None): ";
        cin >> constraints;

        // Parse constraints using bitwise OR
        unsigned int flags = 0;
        for (int j = 0; j < constraints.length(); j++) {
            if (constraints[j] == '1') flags = flags | PRIMARY_KEY;
            else if (constraints[j] == '2') flags = flags | NOT_NULL;
            else if (constraints[j] == '4') flags = flags | UNIQUE;
        }

        currentTable->addColumn(colName, colType, flags);
    }

    cout << "Table '" << tableName << "' created successfully!" << endl;
}


void insertInto() {
    if (currentTable == NULL) {
        cout << "No table created yet!" << endl;
        return;
    }

    vector<string> values;
    string input;

    cout << "Enter values (space separated): ";
    cin.ignore();
    getline(cin, input);

    // Parse values using stringstream
    stringstream ss(input);
    string value;
    while (ss >> value) {
        values.push_back(value);
    }

    if (currentTable->insertRow(values)) {
        cout << "Record inserted successfully!" << endl;
    }
}

// Select command
void selectFrom() {
    if (currentTable == NULL) {
        cout << "No table created yet!" << endl;
        return;
    }

    cout << "\n--- " << currentTable->tableName << " ---\n";
    currentTable->selectAll();
    cout << endl;
}

// Save command
void saveTable() {
    if (currentTable == NULL) {
        cout << "No table to save!" << endl;
        return;
    }
    currentTable->saveToFile();
}

// Load command
void loadTable() {
    string tableName;
    cout << "Enter table name to load: ";
    cin >> tableName;

    ifstream file(tableName + ".txt");
    if (!file.is_open()) {
        cout << "File not found!" << endl;
        return;
    }

    // Delete old table
    if (currentTable != NULL) {
        delete currentTable;
    }

    string line, word;
    
    // Read TABLE header
    file >> word >> tableName;
    currentTable = new Table(tableName);

    // Read columns until DATA
    while (file >> word) {
        if (word == "DATA") break;

        string colName = word;
        string colType;
        unsigned int constraints;

        file >> colType >> constraints;
        currentTable->addColumn(colName, colType, constraints);
    }

    // Read data rows
    getline(file, line); 
    while (getline(file, line)) {
        if (line == "") continue;
        
        vector<string> values;
        stringstream ss(line);
        string val;
        while (ss >> val) {
            values.push_back(val);
        }
        
        Row* newRow = new Row();
        newRow->values = values;
        currentTable->rows.push_back(newRow);
    }

    file.close();
    cout << "Table '" << tableName << "' loaded successfully!" << endl;
}


void Menu() {
    cout << "\n========== MINI DATABASE ENGINE ==========\n";
    cout << "1. CREATE TABLE\n";
    cout << "2. INSERT INTO\n";
    cout << "3. SELECT * FROM\n";
    cout << "4. SAVE TO FILE\n";
    cout << "5. LOAD FROM FILE\n";
    cout << "6. EXIT\n";
    cout << "==========================================\n";
    cout << "Choice: ";
}

int main() {
    int choice;

    do {
        Menu();
        cin >> choice;

        switch (choice) {
            case 1: createTable(); break;
            case 2: insertInto(); break;
            case 3: selectFrom(); break;
            case 4: saveTable(); break;
            case 5: loadTable(); break;
            case 6: 
                cout << "Goodbye!\n";
                break;
            default:
                cout << "Invalid choice!\n";
        }

    } while (choice != 6);

    // Clean up memory
    if (currentTable != NULL) {
        delete currentTable;
    }

    return 0;
}