#include <iostream>
#include <windows.h>
#include <winable.h>
#include <tlhelp32.h>
#include <list>
#include <string>
#include <cstring>
#include <conio.h>
using namespace std;

list<void*> values_list;
list<void*> changed_values_list;
char Type_of_value;


void color(int color){
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
void gotoxy(int x, int y){
    COORD c;
    c.X = x;
    c.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),c);
}
void clear() {
    COORD topLeft  = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(
        console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    FillConsoleOutputAttribute(
        console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    SetConsoleCursorPosition(console, topLeft);
}


// Take address handle data type and new value then change value of address
void change_address_value(void* Address_to_change, HANDLE Phandle, char Datatype, double new_value){
    DWORD64 newvaluesize = sizeof(new_value);
    if(WriteProcessMemory(Phandle, Address_to_change, &new_value, newvaluesize, NULL)){
        cout << "Write success" << endl;
    }
    else{
        cout << "double write fail";
    }
}


// Same as above but for 4 byte vals ik its shit
void change_address_value_int(void* Address_to_change, HANDLE Phandle, char Datatype, int new_value){
    DWORD64 newvaluesize = sizeof(new_value);
    if(WriteProcessMemory(Phandle, Address_to_change, &new_value, newvaluesize, NULL)){
        cout << "Write success" << endl;
    }
    else{
        cout << "Int write fail";
    }
}


void change_address_value_float(void* Address_to_change, HANDLE Phandle, char Datatype, float new_value){
    DWORD64 newvaluesize = sizeof(new_value);
    if(WriteProcessMemory(Phandle, Address_to_change, &new_value, newvaluesize, NULL)){
        cout << "Write success" << endl;
    }
    else{
        cout << "float write fail";
    }
}


// Read all addresses that contained mathcing values the first scan and see if they now contain the new value
void read_addresses_for_change(HANDLE Phandle, int valuetolook, char Datatype){
    cout << "\nreading" << endl;
    changed_values_list.clear();
    for (list<void*>::iterator i=values_list.begin(); i!=values_list.end(); i++){

        double pointdouble;

        float pointfloat;

        unsigned int pointint;

        char pointchar[128*1024];

        SIZE_T ByteSize;

        if (Datatype == '4'){
            ReadProcessMemory(Phandle, *i, &pointint, sizeof(pointint), &ByteSize);
            unsigned int value = *reinterpret_cast<unsigned int *>(&pointint);
            if (value == valuetolook){
            cout << "Value changed at " << *i << " contained value " << value << endl;

            changed_values_list.push_back(*i);

        }
        }
        if (Datatype == 'd'){
            ReadProcessMemory(Phandle, *i, &pointdouble, sizeof(pointdouble), &ByteSize);
            double value = *reinterpret_cast<double *>(&pointdouble);
            if (value == valuetolook){
            cout << "Value changed at " << *i << " contained value " << value << endl;

            changed_values_list.push_back(*i);



        }
        }
        if (Datatype == 'f'){
            ReadProcessMemory(Phandle, *i, &pointfloat, sizeof(pointfloat), &ByteSize);
            float value = *reinterpret_cast<float *>(&pointfloat);
            if ((int)value == (int)valuetolook){
            cout << "Value changed at " << *i << " contained value " << value << endl;

            changed_values_list.push_back(*i);
        }
        }
        if (Datatype == 's'){
            ReadProcessMemory(Phandle, *i, &pointchar, sizeof(pointchar), &ByteSize);
            // TODO Compare words
        }
    }
}


// Takes address to block of memory (Memory basic information structure) and reads through its buffer to check for matches
void read_and_search(int find_value, MEMORY_BASIC_INFORMATION TESTmb, HANDLE Phandle, char Datatype, string string_to_srch){

    char* comc = new char[string_to_srch.length()];

    strcpy(comc, string_to_srch.c_str());

    int byte_size;

    char buffer[128*1024];

    unsigned int bytes_left;

    unsigned int total_read;

    SIZE_T bytes_to_read;

    SIZE_T bytes_read;

    bytes_left = TESTmb.RegionSize;

    total_read = 0;

    void* add_address;

    switch(Datatype){
    case 'd':
        byte_size = 8;
    case 'f':
        byte_size = 4;
    case '4':
        byte_size = 4;
    case 's':
        byte_size = 1;
    }

    while (bytes_left){

        bytes_to_read = (bytes_left > sizeof(buffer)) ? sizeof(buffer) : bytes_left;

        ReadProcessMemory(Phandle, TESTmb.BaseAddress + total_read, buffer, bytes_to_read, &bytes_read);

        unsigned int offset;

        for(offset = 0; offset < bytes_read; offset += byte_size){

            unsigned int tempval;

            if (Datatype == 'd'){
                tempval = *((double*)&buffer[offset]);
            }
            if (Datatype == 'f'){
                tempval = *((float*)&buffer[offset]);
            }
            if (Datatype == '4'){
                tempval = *((unsigned int*)&buffer[offset]);
            }
            if (Datatype == 's'){

                const char* tempchar = &buffer[offset];
                //TODO Get address if substring in string here currently only finds if string is alone
                if (strcmp(tempchar, comc)== 0){

                    cout << &buffer[offset] << " Located at memory location : ";

                    printf("0x%08x\n", TESTmb.BaseAddress + offset + total_read);

                    add_address = TESTmb.BaseAddress + offset + total_read;

                    values_list.push_back(add_address);

                }
            }

            if (Datatype != 's'){
                if (tempval == find_value){

                    printf ("Value of %i located at address 0x%08x\n", tempval, TESTmb.BaseAddress + offset + total_read);

                    cout << "Offset is " << offset << " Data type is " << Datatype << endl;

                    add_address = TESTmb.BaseAddress + offset + total_read;

                    values_list.push_back(add_address);

                }
            }

        }

        bytes_left -= bytes_read;

        total_read += bytes_read;

        if (bytes_read == 0){
            break;
        }
    }
}


// Function to use virtual query to scan program and copy contents to struct
void vqe_scan(HANDLE Processhandle, int find_value, char Datatype, string string_look){

    MEMORY_BASIC_INFORMATION membuffer;

    unsigned char *addy = 0;

    while(1){

        if (VirtualQueryEx(Processhandle, addy, &membuffer, sizeof(membuffer)) == 0){
            break;
        }

#define WRITEABLE (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)

        if ((membuffer.State & MEM_COMMIT) && (membuffer.Protect & WRITEABLE)){

            read_and_search(find_value, membuffer, Processhandle, Datatype, string_look);

        }

        addy = (unsigned char*)membuffer.BaseAddress + membuffer.RegionSize;
    }
}


// new scan takes input from user : data type, PID and value to look for then calls scans to add matches to list
HANDLE new_scan_ui(){

    int counter = 3;

    string str_pid;
    gotoxy(0,0);
    cout << "Please input the PID " << endl;
    getline(cin, str_pid);
    int App_PID = stoi(str_pid);
    cout << "You input " << App_PID << endl;

    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, App_PID);


    string Str_value;

    int Value_to_look;

    string Str_to_look;

    cout << "Please input data type " << endl;
    gotoxy(37,6);
    color(11);
    cout << "String  ";
    gotoxy(52,12);
    color(11);
    cout << "Double  ";
    gotoxy(68,18);
    color(11);
    cout << "Float  ";
    gotoxy(52,24);
    color(13);
    cout << "4 Byte <";
    char key;
    for (int i = 0;;){
        key = getch();
        if (key == '\r'){
            values_list.clear();
            gotoxy(37,6);
            cout << "          ";
            gotoxy(52,12);
            cout << "         ";
            gotoxy(68,18);
            cout << "                    ";
            gotoxy(52,24);
            cout << "        ";
            if (counter == 0){
                Type_of_value = 's';
                break;
            }
            if (counter == 1){
                Type_of_value = 'd';
                break;
            }
            if (counter == 2){
                Type_of_value = 'f';
                break;
            }
            if (counter == 3){
                Type_of_value = '4';
                break;
            }
        }
        gotoxy(37,6);
        color(11);
        cout << "String  ";
        gotoxy(52,12);
        color(11);
        cout << "Double  ";
        gotoxy(68,18);
        color(11);
        cout << "Float  ";
        gotoxy(52,24);
        color(11);
        cout << "4 Byte  ";
        if(key == 72){
            counter -= 1;
            if (counter < 0){
                counter = 0;
            }
        }
        if (key == 80){
            counter += 1;
            if (counter > 3){
                counter = 3;
            }
        }
        switch(counter){
        case 0:
            gotoxy(37,6);
            color(13);
            cout << "String <";
            break;
        case 1:
            gotoxy(52,12);
            color(13);
            cout << "Double <";
            break;
        case 2:
            gotoxy(68,18);
            color(13);
            cout << "Float <";
            break;
        case 3:
            gotoxy(52,24);
            color(13);
            cout << "4 Byte <";
        }



    }
    gotoxy(0,5);
    cout << "Looking for type " << Type_of_value << endl;
    string Str_vallook;
    if (Type_of_value == 's'){
        cout << "Please input string to search for " << endl;
        getline(cin, Str_to_look);
        cout << "Searching for " << Str_to_look << endl;
    }

    else{
        cout << "Please input value to search for " << endl;
        getline(cin, Str_vallook);
        Value_to_look = stoi(Str_vallook);
        cout << "Searching for " << Value_to_look << endl;
    }



    vqe_scan(processHandle, Value_to_look, Type_of_value, Str_to_look);
    return processHandle;
}


// compare scan takes user input : new value to compare to, read addresses for change with type of value adds matches to new list
void compare_scan_ui(HANDLE processHandle){
    string Str_cv;
    gotoxy(0,0);
    cout << "Input new value to compare ";
    getline(cin, Str_cv);
    int Compare_value = stoi(Str_cv);
    read_addresses_for_change(processHandle, Compare_value, Type_of_value);
    values_list.clear();
    for (list<void*>::iterator i=changed_values_list.begin(); i!=changed_values_list.end(); i++){
            //copy contents to values list in case user wants to scan comparison again
            //clear this changed value list before scanning again
        cout << *i << endl;
    }

}


// takes user input of value to change to and the previously declared data type
void change_val_ui(HANDLE processHandle){
    gotoxy(0,0);
    for (list<void*>::iterator i=changed_values_list.begin(); i!=changed_values_list.end(); i++){
        cout << *i << endl;
    }
    cout << "Above are all addresses in the match list, if none then find matches using compare" << endl;
    int choice;
    string str_choice;
    cout << "Change value of all addresses, manual add address or back" << endl;
    //getline(cin, str_choice);
    //choice = stoi(str_choice);
    gotoxy(88,1);
    cout << "All addresses  ";
    gotoxy(88,2);
    cout << "Manual add  ";
    gotoxy(88,3);
    cout << "Back <";
    int counter = 2;
    for (int i =0;;){
        char key = getch();
        if(key == '\r'){
            clear();
            break;
        }
        gotoxy(88,1);
        cout << "All addresses  ";
        gotoxy(88,2);
        cout << "Manual add  ";
        gotoxy(88,3);
        cout << "Back  ";
        if(key == 72){
            counter -= 1;
            if (counter < 0){
                counter = 0;
            }
        }
        if (key == 80){
            counter += 1;
            if (counter > 2){
                counter = 2;
            }
        }

        if (counter == 0){
            gotoxy(88,1);
            cout << "All addresses <";

        }
        if (counter == 1){
            gotoxy(88,2);
            cout << "Manual add <";

        }
        if (counter == 2){
            gotoxy(88,3);
            cout << "Back <";

        }

    }
    if(counter == 0){
        // Add according vals for data type
        string str_testval;
        cout << "Input value to change to " << endl;
        getline(cin, str_testval);
        if (Type_of_value == 'd'){
            double test_val;
            test_val = stoi(str_testval);
            for (list<void*>::iterator i=changed_values_list.begin(); i!=changed_values_list.end(); i++){
            //copy contents to values list in case user wants to scan comparison again

            change_address_value(*i, processHandle, 'd', test_val);
            }
        }
        if (Type_of_value == 'f'){
            float test_val;
            test_val = stoi(str_testval);
            for (list<void*>::iterator i=changed_values_list.begin(); i!=changed_values_list.end(); i++){
            //copy contents to values list in case user wants to scan comparison again
            change_address_value_float(*i, processHandle, 'd', test_val);
            }
        }
        if (Type_of_value == '4'){
            int test_val;
            test_val = stoi(str_testval);
            cout << "changing 4";
            for (list<void*>::iterator i=changed_values_list.begin(); i!=changed_values_list.end(); i++){
            //copy contents to values list in case user wants to scan comparison again
            cout << "iterating";
            change_address_value_int(*i, processHandle, 'd', test_val);
            }
        }
        if (Type_of_value == 's'){
            double test_val;
            test_val = stoi(str_testval);
        }

    }
    if(counter == 2){
        return;
    }
    if (counter == 1){
            // Use diff data types ccording to data type
        gotoxy(0,0);
        for (list<void*>::iterator i=changed_values_list.begin(); i!=changed_values_list.end(); i++){
            cout << *i << endl;
        }
        string address_to_use;
        cout << "Input address to change (double) ";
        getline(cin, address_to_use);
        int address_val = stoi(address_to_use,0,16);
        string str_testval;
        cout << "Input value to change to " << endl;
        getline(cin, str_testval);
        double test_val = stoi(str_testval);

        DWORD64 newvaluesize = sizeof(test_val);
        cout << address_val << endl;
        if(WriteProcessMemory(processHandle, (LPVOID)address_val, &test_val, newvaluesize, NULL)){
        cout << "Write success" << endl;
        }
        else{
        cout << "double write fail";
        }

    }
}


int main()
{
    // Still need to fix new scan empty list and start over
    gotoxy(37,6);
    color(11);
    cout << "New scan  ";
    gotoxy(52,12);
    color(11);
    cout << "Compare  ";
    gotoxy(68,18);
    color(13);
    cout << "Change all matches <";
    gotoxy(0,20);
    color(11);
    cout << R"(     .-') _                          .-. .-')           ('-.       .-') _                         .-') _   ('-.
    ( OO ) )                         \  ( OO )        _(  OO)     ( OO ) )                       ( OO ) )_(  OO)
,--./ ,--,'  .-'),-----.  .-'),-----. ;-----.\       (,------.,--./ ,--,'  ,----.     ,-.-') ,--./ ,--,'(,------.
|   \ |  |\ ( OO'  .-.  '( OO'  .-.  '| .-.  |        |  .---'|   \ |  |\ '  .-./-')  |  |OO)|   \ |  |\ |  .---'
|    \|  | )/   |  | |  |/   |  | |  || '-' /_)       |  |    |    \|  | )|  |_( O- ) |  |  \|    \|  | )|  |
|  .     |/ \_) |  |\|  |\_) |  |\|  || .-. `.       (|  '--. |  .     |/ |  | .--, \ |  |(_/|  .     |/(|  '--.
|  |\    |    \ |  | |  |  \ |  | |  || |  \  |       |  .--' |  |\    | (|  | '. (_/,|  |_.'|  |\    |  |  .--'
|  | \   |     `'  '-'  '   `'  '-'  '| '--'  /       |  `---.|  | \   |  |  '--'  |(_|  |   |  | \   |  |  `---.
`--'  `--'       `-----'      `-----' `------'        `------'`--'  `--'   `------'   `--'   `--'  `--'  `------' )" << "\nPersonal project for fun it won't cheat very well but it does it's best... (Use compare after new scan to generate list of addresses to change)";;
    int counter = 2;
    char key;
    HANDLE processHandle = NULL;
    for (int i =0;;){
        key = getch();
        clear();
        if (key == '\r'){
            gotoxy(37,6);
            cout << "          ";
            gotoxy(52,12);
            cout << "         ";
            gotoxy(68,18);
            cout << "                    ";
            if (counter == 0){
                processHandle = new_scan_ui();
                key = getch();
                clear();

            }
            if (counter == 1){
                if (processHandle == NULL){
                    cout << "No process";
                }
                else{
                    compare_scan_ui(processHandle);
                    key = getch();
                    clear();
                }
            }
            if (counter == 2){
                if (processHandle == NULL){
                    cout << "No process";

                }
                else{
                    change_val_ui(processHandle);
                    key = getch();
                    clear();
                }
            }
        }
        gotoxy(37,6);
        color(11);
        cout << "New scan  ";
        gotoxy(52,12);
        color(11);
        cout << "Compare  ";
        gotoxy(68,18);
        color(11);
        cout << "Change all matches  ";
        gotoxy(0,20);
    color(11);
    cout << R"(     .-') _                          .-. .-')           ('-.       .-') _                         .-') _   ('-.
    ( OO ) )                         \  ( OO )        _(  OO)     ( OO ) )                       ( OO ) )_(  OO)
,--./ ,--,'  .-'),-----.  .-'),-----. ;-----.\       (,------.,--./ ,--,'  ,----.     ,-.-') ,--./ ,--,'(,------.
|   \ |  |\ ( OO'  .-.  '( OO'  .-.  '| .-.  |        |  .---'|   \ |  |\ '  .-./-')  |  |OO)|   \ |  |\ |  .---'
|    \|  | )/   |  | |  |/   |  | |  || '-' /_)       |  |    |    \|  | )|  |_( O- ) |  |  \|    \|  | )|  |
|  .     |/ \_) |  |\|  |\_) |  |\|  || .-. `.       (|  '--. |  .     |/ |  | .--, \ |  |(_/|  .     |/(|  '--.
|  |\    |    \ |  | |  |  \ |  | |  || |  \  |       |  .--' |  |\    | (|  | '. (_/,|  |_.'|  |\    |  |  .--'
|  | \   |     `'  '-'  '   `'  '-'  '| '--'  /       |  `---.|  | \   |  |  '--'  |(_|  |   |  | \   |  |  `---.
`--'  `--'       `-----'      `-----' `------'        `------'`--'  `--'   `------'   `--'   `--'  `--'  `------' )" << "\nPersonal project for fun it won't cheat very well but it does it's best... (Use compare after new scan to generate list of addresses to change)";
        if(key == 72){
            counter -= 1;
            if (counter < 0){
                counter = 0;
            }
        }
        if (key == 80){
            counter += 1;
            if (counter > 2){
                counter = 2;
            }
        }

        switch(counter){
        case 0:
            gotoxy(37,6);
            color(13);
            cout << "New Scan <";
            break;
        case 1:
            gotoxy(52,12);
            color(13);
            cout << "Compare <";
            break;
        case 2:
            gotoxy(68,18);
            color(13);
            cout << "Change all matches <";
        }
    }

    return 0;
}
