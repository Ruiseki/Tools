#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
using namespace std;


// IL MANQUE PARFOIS DES CHAMPS.
// VERIFIER ET METTRE AUCUNE VALEUR SI JAMAIS

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

void skipArray(ifstream &fileRead)
{
    char a;
    do
    {
        fileRead.get(a);
        if(a == '[') skipArray(fileRead);
    }while(a != ']');
}

string deleteSpaceAndBackspace(string &element)
{
    for(int i(element.size()); i >= 0; i--)
    {
        if(element[i] == ' ' || element[i] == '\n')
        {
            element.pop_back();
        }
    }

    return element;
}

string getFieldName(char &letter, string &sectionName, ifstream &json)
{
    string fieldName;
    if(sectionName != "") fieldName += sectionName+"/";
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

char newFindFields(string sectionName, ifstream &json, vector<string> &fields)
{
    char letter;
    string fieldName("");
    int currentPos = json.tellg();
    json.seekg(0, ios::end);
    int fileSize(json.tellg());
    json.seekg(currentPos, ios::beg);

    do
    {
        letter = rEmptSpc(json);

        if(letter == '"')
        {
            fieldName = getFieldName(letter, sectionName, json);

            if(letter == '{') // in this case, this is an object in an object.
            {
                vector<string> temp;
                letter = newFindFields(fieldName, json, temp);
                for(string element : temp) fields.push_back(element);
            }
            else if(letter == '[') // array. Need to be skiped.
            {
                skipArray(json);
                fields.push_back(fieldName);
            }
            else // we must go the end of the value.
            {
                do // moving to the next ',' or '}'
                {
                    json.get(letter);
                }while(letter != ',' && letter != '}');

                fields.push_back(fieldName);
                fieldName = "";
            }
        }
        else if(letter == ']') return letter;

        if(json.tellg() % 10000 == 0)
        {
            system("cls");
            cout << "Progression : " << 100 * json.tellg() / fileSize << "%" << endl;
            cout << json.tellg() << " / " << fileSize << endl;
        }
    }while(letter != '}');
    system("cls");
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

    do
    {
        letter = rEmptSpc(json);

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
            }
            else if(letter == '[') // array. Need to be skiped.
            {
                skipArray(json);
                values[fieldNumber] = "[]";
            }
            else // we must go the end of the value.
            {
                do
                {
                    value += letter;
                    json.get(letter);
                }while(letter != ',' && letter != '}');

                deleteSpaceAndBackspace(value);
                values[fieldNumber] = value;
                value = "";
            }
            fieldNumber++;
        }
        else if(letter == ']') return letter;

        if(json.tellg() % 10000 == 0)
        {
            system("cls");
            cout << "Progression : " << 100 * json.tellg() / fileSize << "%" << endl;
            cout << json.tellg() << " / " << fileSize << endl;
        }

    }while(letter != '}');
    system("cls");
    return letter;
}

int main(int agrc, char *argv[])
{
    try
    {
        argv[1];
    }
    catch(exception err)
    {
        return -1;
    }

    ifstream input(argv[1]);
    ofstream output;
    vector<vector<string>> fields;
    
    input.seekg(ios::beg);

    char endingChar;
    do // get all fields of the file for detecting the columns
    {
        vector<string> tempFields;
        endingChar = newFindFields("", input, tempFields);
        fields.push_back(tempFields);
        if(endingChar == '}') input.seekg(1, ios::cur);
    }while(endingChar != ']');

    // determinate the columns
    vector<string> columns;
    
    for(vector<string> field : fields)
    {
        for(string element : field)
        {
            if(!isElementPresentInArray(element, columns))
            {
                columns.push_back(element);
            }
        }
    }

    const int columnsSize(columns.size());
    string values[columnsSize];

    string outputpath;
    try // check the argument for output file
    {
        outputpath = argv[2];
    }
    catch(exception err)
    {
        outputpath = "./file.csv";
    }

    output.open(outputpath.c_str());

    for(int i(0); i < columns.size(); i++) // first row
    {
        output << columns[i];
        if(i == columns.size() - 1) output << '\n';
        else output << ',';
    }

    input.seekg(0, ios::beg);

    do // get all fields of the file for detecting the one that had the more option
    {
        for(string element : values) element = ""; // clear the array
        endingChar = writedata(columns, "", input, output, values); // fill the array with values
        for(int i(0); i < columnsSize; i++) // write the values
        {
            if(values[i] != "") output << values[i];
            else output << "null";
            if(i == columnsSize - 1) output << '\n';
            else output << ',';
        }
        if(endingChar == '}') input.seekg(1, ios::cur);
    }while(endingChar != ']');

    output.close();
    return 0;
}