#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include <chrono>
using namespace std;

bool isElementPresentInArray(string element,vector<string> array)
{
    for(string x : array)
    {
        if(x == element) return true;
    }
    return false;
}

char rEmptSpc(ifstream &fileRead) // skip the empty space
{
    char a;
    do
    {
        fileRead.get(a);
    }while(a == ' ');
    return a;
}

void arrayMgr(ifstream &fileRead)
{
    char a;
    do
    {
        fileRead.get(a);
        if(a == '[') arrayMgr(fileRead);
    }while(a != ']');
}

string deleteSpaceAndBackspace(string &element)
{
    for(int i(element.size() - 1); i >= 0; i--)
    {
        if(element[i] == ' ' || element[i] == '\n')
        {
            element.pop_back();
        }
        else break;
    }

    return element;
}

string getFieldName(char &letter, string &sectionName, ifstream &json)
{
    string fieldName;
    if(sectionName != "") fieldName += sectionName+"_";
    {
        do // get the field name
        {
            json.get(letter);
            if(letter != '"') fieldName += letter;
        }while(letter != '"');
    }

    letter = rEmptSpc(json); // removing empty space befor ':'
    // here, letter = ':'
    letter = rEmptSpc(json); // removing the empty space after the ':'

    return fieldName;
}

char findFields(string sectionName, ifstream &json, vector<string> &fields)
{
    char letter;
    string fieldName("");
    int currentPos = json.tellg();
    json.seekg(0, ios::end);
    int fileSize(json.tellg());
    json.seekg(currentPos, ios::beg);

    int displayWhen;
    if(fileSize > 1000) displayWhen = fileSize / 1000;
    else displayWhen = 1;

    do
    {
        if(currentPos != fileSize) letter = rEmptSpc(json);
        else return ']';
        int getPos = json.tellg();

        if(letter == '"')
        {
            fieldName = getFieldName(letter, sectionName, json);

            if(letter == '{') // in this case, this is an object in an object.
            {
                vector<string> temp;
                letter = findFields(fieldName, json, temp);
                for(string element : temp) fields.push_back(element);
            }
            else if(letter == '[') // array. Need to be skiped.
            {
                arrayMgr(json);
                fields.push_back(fieldName);
            }
            else // we must go the end of the value.    
            {
                if(letter == '"') // if the value is in double-quote
                {
                    do // moving to the next ',' or '}'
                    {
                        json.get(letter);
                    }while(letter != '"');
                }

                do // moving to the next ',' or '}'
                {
                    json.get(letter);
                }while(letter != ',' && letter != '}');

                fields.push_back(fieldName);
                fieldName = "";
            }
        }
        else if(letter == ']') return letter;

        if(json.tellg() % displayWhen == 0)
        {
            system("cls");
            cout << "Reading : " << 100 * json.tellg() / fileSize << "%" << endl;
            cout << json.tellg() << " / " << fileSize << endl << endl;
        }
    }while(letter != '}');
    return letter;
}

char writedata(vector<string> &ref, string sectionName, ifstream &json, ofstream &csv, string values[])
{
    char letter;
    string fieldName = "";
    string value = "";
    int fieldNumber;
    int currentPos = json.tellg();
    json.seekg(0, ios::end);
    int fileSize(json.tellg());
    json.seekg(currentPos, ios::beg);

    int displayWhen;
    if(fileSize > 1000) displayWhen = fileSize / 1000;
    else displayWhen = 1;

    do
    {
        if(currentPos != fileSize) letter = rEmptSpc(json);
        else return ']';

        if(letter == '"')
        {
            fieldName = getFieldName(letter, sectionName, json);
            
            if(letter != '{')
            {
                for(int i(0); i < ref.size(); i++) // check the field number
                {
                    if(fieldName == ref[i])
                    {
                        fieldNumber = i;
                        break;
                    }
                }
            }

            if(letter == '{') // in this case, this is an object in an object.
            {
                letter = writedata(ref, fieldName, json, csv, values);
                json.get(letter);
            }
            else if(letter == '[') // array. Need to be skiped.
            {
                arrayMgr(json);
                values[fieldNumber] = "[]";
            }
            else // saving the value
            {
                if(letter == '"') // if the value is in double-quote
                {
                    value += letter;
                    do // moving to the next '"'
                    {
                        json.get(letter);
                        value += letter;
                    }while(letter != '"');

                    do // moving the the next ',' or '}' without saving the value
                    {
                        json.get(letter);
                    }while(letter != ',' && letter != '}');
                }
                else
                {
                    do 
                    {
                        value += letter;
                        json.get(letter);
                    }while(letter != ',' && letter != '}');
                }

                deleteSpaceAndBackspace(value);
                values[fieldNumber] = value;
                value = "";
            }
            fieldNumber++;
        }
        else if(letter == ']') return letter;

        if(json.tellg() % displayWhen == 0)
        {
            system("cls");
            cout << "Writing : " << 100 * json.tellg() / fileSize << "%" << endl;
            cout << json.tellg() << " / " << fileSize << endl << endl;
        }

    }while(letter != '}');
    return letter;
}

int main(int argc, char *argv[])
{
    auto start = chrono::system_clock::now();
    ifstream input;

    if(argc > 1) // check the input
    {
        input.open(argv[1]);
    }
    else
    {
        cout << "No input file.";
        return -1;
    }

    if(!input)
    {
        cout << "file doesn't exist (" << argv[1] << ")";
        return -1;
    }

    input.seekg(ios::beg);

    ofstream output;

    string outputpath;
    if(argc > 2) // check the output
    {
        outputpath = argv[2];
    }
    else
    {
        outputpath = "./mainResult.csv";
    }

    output.open(outputpath.c_str());

    if(!output)
    {
        cout << "Impossible d'ecrire dans le fichiers cible. (" << outputpath << ")";
        return -1;
    }

    // COLUMNS

    vector<vector<string>> fields;
    
    char endingChar;
    do // get all fields of the file for detecting the columns
    {
        vector<string> tempFields;
        endingChar = findFields("", input, tempFields);
        fields.push_back(tempFields);
        if(endingChar == '}') input.seekg(1, ios::cur);
    }while(endingChar != ']');

    // determinate the columns
    vector<string> columns;

    int fieldNumber = 0;
    for(int i(0); i < fields.size(); i++)
    {
        for(string element : fields[i])
        {
            if(!isElementPresentInArray(element, columns))
            {
                columns.push_back(element);
            }
        }
        
        if(i % 5000 == 0)
        {
            system("cls");
            cout << "Determinating the columns : " << 100 * i / fields.size() << "%" << endl;
        }
    }

    const int columnsSize(columns.size());
    string values[columnsSize];

    system("cls");
    cout << "Writing columns...";

    for(int i(0); i < columns.size(); i++) // first row
    {
        output << columns[i];
        if(i == columns.size() - 1) output << '\n';
        else output << ',';
    }

    // MAKING THE CSV

    input.seekg(0, ios::beg);

    do
    {
        for(string element : values) element = ""; // clear the array
        endingChar = writedata(columns, "", input, output, values); // fill the array with values
        for(int i(0); i < columnsSize; i++) // write the values
        {
            output << values[i];
            if(i == columnsSize - 1) output << '\n';
            else output << ',';
        }
        if(endingChar == '}')
        { // get the next character that is not a space or a backspace
            do
            {
                input.get(endingChar);
            }while(endingChar == ' ' || endingChar == '\n' || endingChar == '\0');
        }
    }while(endingChar != ']');

    input.close();
    output.close();

    auto end = chrono::system_clock::now();
    chrono::duration<double> elapsed_time = end-start;

    system("cls");
    cout << "Done." << endl << "Executed in " << elapsed_time.count() << "s";
    return 0;
}