using MySql.Data.MySqlClient;
using System;
/*
    ARGUMENTS :

    -s : IP of the database
    -port : the network port of the server. By default 3306
    -u : user of the database
    -p : password of the user
    -d : the database name
    -key : set the primary key. By default, the field how had the name "id" will be the primary key
    -f : set the file path
    -ft : set the field terminator. By default, '\n'
    -fs : field separator. By default, ','
    -n : set the name of the table. By default, the name of the input file
*/
public class Program
{
    static void Main(string[] args)
    {
        string filePath = "", primaryKey = "id", rowsTerminator = "\\n", fieldSeparator = ",", tableName = ""; // table
        string database = "", serverIP="", password="", user="", port = "3306"; // mysql
        bool help = false;

        if(args.Length == 0) help = true;

        for(int i=0; i < args.Length; i++) // args
        {
            string arg = args[i];
            
            switch(arg)
            {
                case "-s":
                    serverIP = args[i+1];
                    break;

                case "-port":
                    port = args[i+1];
                    break;
                    
                case "-p":
                    password = args[i+1];
                    break;
                    
                case "-u":
                    user = args[i+1];
                    break;

                case "-d":
                    database = args[i+1];
                    break;
                    
                case "-key":
                    primaryKey = args[i+1];
                    break;

                case "-f":
                    filePath = args[i+1];
                    break;

                case "-ft":
                    rowsTerminator = args[i+1];
                    break;

                case "-fs":
                    fieldSeparator = args[i+1];
                    break;

                case "-n":
                    tableName = args[i+1];
                    break;

                case "-h":
                    help = true;
                    break;

                default:
                    continue;
            }
        }

        if(help)
        {
            Console.WriteLine("-s : IP of the database");
            Console.WriteLine("-port : the network port of the server. By default 3306");
            Console.WriteLine("-u : user of the database");
            Console.WriteLine("-p : password of the user");
            Console.WriteLine("-d : the database name");
            Console.WriteLine("-key : set the primary key. By default, the field how had the name \"id\" will be the primary key");
            Console.WriteLine("-f : set the file path");
            Console.WriteLine("-ft : set the field terminator. By default, '\\n'");
            Console.WriteLine("-fs : field separator. By default, ','");
            Console.WriteLine("-n : set the name of the table. By default, the name of the input file");
            Console.WriteLine("-h display this page");
            Environment.Exit(0);
        }

        if(serverIP == "") // check server IP
        {
            Console.WriteLine("No server defined.");
            Environment.Exit(-1);
        }
        else if(database == "")
        {
            Console.WriteLine("No database defined.");
            Environment.Exit(-1);
        }
        else if(user == "") // check the user
        {
            Console.WriteLine("No user defined");
            Environment.Exit(-1);
        }
        else if(password == "") // check the password
        {
            Console.WriteLine("No password defined");
            Environment.Exit(-1);
        }
        else if(filePath == "") // check the input file
        {
            Console.Write("No input file.");
            Environment.Exit(-1);
        }
        else if(tableName == "") // check the table name
        {
            tableName = filePath;
            string[] array;
            array = tableName.Split(".");
            array = array[array.Length-2].Split("/");
            array = array[array.Length-1].Split("\\");
            tableName = array[array.Length-1];
        }

        string[] lines = System.IO.File.ReadAllLines(filePath);
        List<string> fields = new List<string>();
        string field = "", mkTableCommand = "CREATE TABLE "+tableName+" (";

        for(int i=0; i < lines[0].Length; i++) // get the fields of the csv
        {
            char letter = lines[0][i];
            if(letter == ',')
            {
                fields.Add(field);
                field = "";
            }
            else
            {
                field += letter;
            }

            if(i == lines[0].Length - 1)
            {
                fields.Add(field);
                field = "";
            }
        }

        for(int i=0; i < fields.Count; i++)  // check the primary key
        {
            if(fields[i] == primaryKey) break;
            else if(i == fields.Count - 1)
            {
                Console.Write("Primary key not found.");
                Environment.Exit(-1);
            }
        }

        Console.WriteLine("Reading the file ...");
        List<List<string>> allArgs = new List<List<string>>(); // all data in board

        for(int i=0; i < lines.Length; i++)
        {
            List<string> temp = new List<string>();
            bool quote = false;
            string element = "";
            foreach(char letter in lines[i])
            {
                if(letter == '"' && !quote) quote = true;
                if(letter == '"' && quote) quote = false;

                if(letter == ',' && !quote)
                {
                    temp.Add(element);
                    element = "";
                }
                else element += letter;
            }
            temp.Add(element);
            allArgs.Add(temp);
        }

        
        // complete the query
        Console.WriteLine("Determinating the type of the columns ...");
        string[] fieldsName = new string[allArgs[0].Count];
        for(int i=0; i < allArgs[0].Count; i++) fieldsName[i] = allArgs[0][i];
        allArgs.RemoveAt(0);
        for(int i=0; i < fieldsName.Length; i++)
        {
            mkTableCommand += fieldsName[i] + " ";

            // check the type
            bool isInt = true, isDouble = true;
            for(int j=0; j < allArgs.Count; j++)
            {
                string element = allArgs[j][i];

                if(isInt) isInt = int.TryParse(element, out _);
                if(isDouble) isDouble = double.TryParse(element, out _);

                if(!isInt && !isDouble) break;
            }

            if(isInt) mkTableCommand += "INT ";
            else if(isDouble) mkTableCommand += "DOUBLE ";
            else mkTableCommand += "TEXT ";

            if(fieldsName[i] == primaryKey) mkTableCommand += "PRIMARY KEY NOT NULL";

            if(i != allArgs[0].Count - 1) mkTableCommand += ", ";
            else mkTableCommand += ");";
        }

        Console.WriteLine("Creating the table ...");
        string connString = "Server=" + serverIP +
        ";Database=" + database +
        ";port=" + port.ToString() +
        ";User Id=" + user +
        ";password=" + password;

        MySqlConnection conn = new MySqlConnection(connString);
        try
        {
            conn.Open();
        }
        catch
        {
            Console.WriteLine("Cannot connect to the database");
            Environment.Exit(-1);
        }

        MySqlCommand command = new MySqlCommand("DROP TABLE " + tableName + ";", conn);
        try {command.ExecuteNonQuery();}
        catch {}

        command = new MySqlCommand(mkTableCommand, conn);
        try {command.ExecuteNonQuery();}
        catch {}


        Console.WriteLine("Creating rows ...");

        string insertCommand = "INSERT INTO " + tableName + " VALUES ("; // make the INSERT INTO query

        for(int i=0; i < allArgs.Count; i++)
        {

            for(int j=0; j < allArgs[i].Count; j++)
            {
                if(allArgs[i][j] != "") insertCommand += allArgs[i][j];
                else insertCommand += "NULL";
                if(j != allArgs[i].Count - 1) insertCommand += ", ";
                else insertCommand += ")";
            }

            if(i != 0 && i % 500 == 0)
            {
                insertCommand += ";";
                command = new MySqlCommand(insertCommand, conn);
                try {command.ExecuteNonQuery();}
                catch(Exception err)
                {
                    Console.WriteLine(err);
                    Environment.Exit(-1);
                }
                insertCommand = "INSERT INTO " + tableName + " VALUES (";
            }
            else if(i != allArgs.Count - 1) insertCommand += ",(";
            else insertCommand += ";";
        }
        command = new MySqlCommand(insertCommand, conn);

        try {command.ExecuteNonQuery();}
        catch(Exception err) {Console.WriteLine(err);}
        Console.WriteLine("Finished");
    }
}