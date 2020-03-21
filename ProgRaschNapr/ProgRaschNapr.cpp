#define NOMINMAX
#include <conio.h>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <Windows.h>

using namespace std;

#pragma region Structs
struct  FieldParams
{
    float timePercentage = 0;//p
    float frequency = 0; //частота(f)
    float antennaHeight = 0; //h
    float distance = 0;//d
    float fieldStrength = 0;//E
};

struct FieldGraph
{
    vector<FieldParams> fields;
    int distanceStep = 0;
};
#pragma endregion

#pragma region Consts
const float heights[9] = { 1200, 600, 300, 150, 75, 37.5, 20, 10 };

const float C0 = 2.515517f;
const float C1 = 0.802853f;
const float C2 = 0.010328f;
const float D1 = 1.432788f;
const float D2 = 0.189269f;
const float D3 = 0.001308f;

const string VALIDATION_ERROR_MEASSAGE = "ERROR VALUE";
#pragma endregion

#pragma region Functions Prototypes
FieldParams ReadFromKeyboard();

void AddInTable(int inputMethod);
void LoadingAnimation();

float CalculateFieldStrValue(FieldParams field);
float CalculateFieldStrValue(float timePercentage, float frequency, float antennaHeight, float distance);

float DAnnex();
float HAnnex(float d, float h, float t, float f);

float Interpolation(float eInf, float eSup, float x, string dataType);

float ReadFieldStrengthValue(float timePercentage, float frequency, float antennaHeight, float distance);

float Validation(string s);
void ValidationError();

float SearchInf(float data, string dataType);
float SearchSup(float data, string dataType);

float Q(float x);
float T(float x);
float Xi(float x);
float J(float h, float f);

void CreateExcel(vector<FieldParams> fields, ios::iostate mode);

float sqr(float a);
float round(float value, int numberOfSymbols);
#pragma endregion

int main()
{
    setlocale(LC_ALL, "");

    FieldGraph graph;
    char flag;

    while (true)
    {
        printf("<1> Построить новую таблицу\n");
        printf("<2> Добавить график в таблицу\n");
        printf("<0> Выход\n\n");

        flag = _getch();

        if (flag == '1')
        {
            system("cls");
            AddInTable(1);
            system("cls");
        }
        else if (flag == '2')
        {
            system("cls");
            AddInTable(2);
            system("cls");
        }
        else if (flag == '0')
            break;
        else
        {
            system("cls");
            continue;
        }
    }
}

void LoadingAnimation()
{
    srand(time(NULL));
    float temp = 0;
    float progress = 0.0;
    int barWidth = 70;
    int pos = 0;
    while (progress <= 1.0) {

        std::cout << "[";
        pos = barWidth * progress;
        for (int i = 0; i < barWidth; i++) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        if(progress <= 1.0)
            std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();

        temp = rand() % 13 + 16;
        temp /= 100;
        progress += temp;
        Sleep(rand() % 400 + 100);
    }

    std::cout << "[";
    pos = barWidth * 0.99;
    for (int i = 0; i < barWidth; i++) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << "Done!";
    std::cout.flush();

    std::cout  << std::endl;
    system("pause");
}

void AddInTable(int inputMethod)
{
    FieldGraph graph;
    FieldParams field;
    graph.fields.push_back(ReadFromKeyboard());
    graph.distanceStep = Validation("distance step(km): ");
    

    for (int j = 0; j <= 1000; j += graph.distanceStep)
    {
        graph.fields[graph.fields.size() - 1].distance = j;
        graph.fields[graph.fields.size() - 1].fieldStrength = CalculateFieldStrValue(graph.fields[graph.fields.size() - 1]);

        field.frequency = graph.fields[0].frequency;
        field.timePercentage = graph.fields[0].timePercentage;
        field.antennaHeight = graph.fields[0].antennaHeight;

        if (j + graph.distanceStep <= 1000) graph.fields.push_back(field);
    }

    if(inputMethod == 1)
        CreateExcel(graph.fields, ios_base::out);
    else
        CreateExcel(graph.fields, ios_base::app);
}

FieldParams ReadFromKeyboard()
{
    FieldParams field;

    field.frequency = Validation("frequency(MHz): ");
    field.timePercentage = Validation("time percentage(%): ");
    field.antennaHeight = Validation("height of antenna(m): ");

    return field;
}

float CalculateFieldStrValue(FieldParams field)
{
    return CalculateFieldStrValue(field.timePercentage, field.frequency, field.antennaHeight, field.distance);
}

float CalculateFieldStrValue(float timePercentage, float frequency, float antennaHeight, float distance)
{
    float E = 0;
    float Einf_t = 0;
    float Esup_t = 0;
    float Einf_f = 0;
    float Esup_f = 0;
    float Einf_h = 0;
    float Esup_h = 0;

    bool flag = false;

    if (distance < 1)
    {
        //TODO: DAnnex
        return -1;
    }

    float timePercentageTemp = SearchInf(timePercentage, "timePercentage");
    float frequencyTemp = SearchInf(frequency, "frequency");
    float antennaHeightTemp = SearchInf(antennaHeight, "antennaHeight");

    while (!flag) {
        if (distance == SearchInf(distance, "distance"))
        {
            E = ReadFieldStrengthValue(timePercentageTemp,
                frequencyTemp,
                antennaHeightTemp,
                distance);
        }
        else
        {
            E = Interpolation(
                ReadFieldStrengthValue(timePercentageTemp,
                    frequencyTemp,
                    antennaHeightTemp,
                    SearchInf(distance, "distance")),
                ReadFieldStrengthValue(timePercentageTemp,
                    frequencyTemp,
                    antennaHeightTemp,
                    SearchSup(distance, "distance")),
                distance, "distance");
        }
        if (antennaHeight != SearchInf(antennaHeight, "antennaHeight"))
        {
            if (antennaHeight < 10)
            {
                E = HAnnex(distance, antennaHeight, timePercentageTemp, frequencyTemp);
            }
            else
            {
                if (antennaHeightTemp == SearchSup(antennaHeight, "antennaHeight"))
                {
                    Esup_h = E;
                    antennaHeightTemp = SearchInf(antennaHeight, "antennaHeight");
                    E = Interpolation(Einf_h, Esup_h, antennaHeight, "antennaHeight");
                }
                else
                {
                    Einf_h = E;
                    antennaHeightTemp = SearchSup(antennaHeight, "antennaHeight");
                    continue;
                }
            }
        }
        if (frequency != SearchInf(frequency, "frequency"))
        {
            if (frequencyTemp == SearchSup(frequency, "frequency"))
            {
                Esup_f = E;
                frequencyTemp = SearchInf(frequency, "frequency");
                E = Interpolation(Einf_f, Esup_f, frequency, "frequency");
            }
            else
            {
                Einf_f = E;
                frequencyTemp = SearchSup(frequency, "frequency");
                continue;
            }
        }
        if (timePercentage == SearchInf(timePercentage, "timePercentage"))
        {
            flag = true;
            continue;
        }
        else
        {
            if (timePercentageTemp == SearchSup(timePercentage, "timePercentage"))
            {
                Esup_t = E;
                E = Interpolation(Einf_t, Esup_t, timePercentage, "timePercentage");
                flag = true;
                continue;
            }
            else
            {
                Einf_t = E;
                timePercentageTemp = SearchSup(timePercentage, "timePercentage");
                continue;
            }
        }
    }
    return E;
}

float SearchInf(float data, string dataType)
{
    if (dataType == "frequency")
    {
        if (data < 600)
            return 100;
        else return 600;
    }
    else if (dataType == "timePercentage") 
    {
        if (data < 10)
            return 1;
        else return 10;
    }
    else if (dataType == "antennaHeight")
    {
        if (data > 1200)
        {
            return 600;
        }
        else if (data < 10)
        {
            return 10;
        }
        else
        {
            for (int i = 0; i < 9; i++)
            {
                if (heights[i] <= data)
                {
                    data = heights[i];
                    break;
                }
            }
        }
    }
    else if(dataType == "distance")
    {
        if (data <= 20)
            return data;
        else if (data > 20 && data < 25)
            return 20;
        else if (data >= 25 && data < 100)
        {
            data = (int)data / 5;
            data *= 5;
        }
        else if (data >= 100 && data < 200)
        {
            data = (int)data / 10;
            data *= 10;
        }
        else if (data >= 200 && data < 1000)
        {
            data = (int)data/25;
            data *= 25;
        }
        else if (data >= 1000)
        {
            data = 1000;
        }
    }

    return data;
}

float SearchSup(float data, string dataType)
{
    if (dataType == "frequency")
    {
        if (data < 600)
            return 600;
        else return 2000;
    }
    else if (dataType == "timePercentage")
    {
        if (data < 10)
            return 10;
        else return 50;
    }
    else if (dataType == "antennaHeight")
    {
        if (data > 1200)
        {
            return 1200;
        }
        else if (data < 10)
        {
            return 20;
        }
        else
        {
            for (int i = 8; i >= 0; i--)
            {
                if (heights[i] >= data)
                {
                    data = heights[i];
                    break;
                }
            }
        }
        
    }
    else if (dataType == "distance")
    {
        if (data <= 20)
            return data;
        else if (data > 20 && data <= 100)
        {
            data -= 20;
            data = ceil(data/5) * 5;
            data += 20;
        }
        else if (data > 100 && data <= 200)
        {
            data -= 100;
            data = ceil(data / 10) * 10;
            data += 100;
        }
        else if (data > 200 && data <= 1000)
        {
            data -= 200;
            data = ceil(data / 25) * 25;
            data += 200;
        }
        else if (data > 1000)
        {
            data = 1000;
        }
    }

    return data;
}

float Interpolation(float eInf, float eSup, float x, string dataType)
{
    float xInf = SearchInf(x, dataType);
    float xSup = SearchSup(x, dataType);

    if (dataType == "timePercentage")
    {
        float Qt = Q(x / 100);
        float Qinf = Q(xInf / 100);
        float Qsup = Q(xSup / 100);

        return eSup * (Qinf - Qt) / (Qinf - Qsup) + eInf * (Qt - Qsup) / (Qinf - Qsup);
    }

    return eInf + ((eSup - eInf) * (log(x / xInf) / log(xSup / xInf)));
}

float DAnnex()
{
    return -1;
}

float HAnnex(float d, float h, float t, float f)
{
    float E10 = Interpolation(ReadFieldStrengthValue(t, f, SearchInf(h, "antennaHeight"), SearchSup(d, "distance")), 
        ReadFieldStrengthValue(t, f, SearchSup(10, "antennaHeight"), SearchSup(d, "distance")),
        10, "antennaHeight");
    float E20 = Interpolation(ReadFieldStrengthValue(t, f, SearchInf(h, "antennaHeight"), SearchSup(d, "distance")),
        ReadFieldStrengthValue(t, f, SearchSup(h, "antennaHeight"), SearchSup(d, "distance")),
        10, "antennaHeight");;

    float C1020 = E10 - E20;
    float Ch1neg10 = 6.03f - J(h, f);
    float E0 = E10 + 0.5f * (C1020 + Ch1neg10);
    float E = E0 + 0.1f * h * (E10 - E0);

    return E;
}

float J(float h, float f)
{
    float Kv = 0;
    if (f == 100)
        Kv = 1.35f;
    else if (f == 600)
        Kv = 3.31f;
    else if (f == 2000)
        Kv = 6.00f;
        
    float O = atan(-h / 9000);
    float v = Kv * O;
    float j = 6.9f + 20 * (float)log(sqrt(sqr(v - 0.1f) + 1) + v - 0.1f);
    return 0;
}

float Q(float x)
{
    if (x <= 0.5)
        return T(x) - Xi(x);
    else
        return -(T(1 - x) - Xi(1 - x));
}

float T(float x)
{
    return round(sqrt(-2 * log(x)), 3);
}

float Xi(float x)
{
    return round((((C2 * T(x) + C1) * T(x)) + C0) / (((D3 * T(x) + D2) * T(x) + D1) * T(x) + 10), 3);
}

float ReadFieldStrengthValue(float timePercentage, float frequency, float antennaHeight, float distance)
{
    ifstream dataCurveFile;
    string fileName = to_string((int)frequency) + " MHz " + to_string((int)timePercentage) + " time";
    float result = 0;
    int d = (int)distance;
    int h = 0;

   for (int i = 8; i >= 0; i--) {
        if (antennaHeight == heights[i])
        {
            h = i + 1;
            break;
        }
    }

    if (d > 20 && d <= 100)
    {
        d -= 20;
        d /= 5;
        d += 20;
    }
    else if (d > 100 && d <= 200)
    {
        d -= 100;
        d /= 10;
        d += 36;
    }
    else if (d > 200 && d < 1000)
    {
        d -= 200;
        d /= 25;
        d += 46;
    }
    else if (d >= 1000)
    {
        d = 76;
    }

            
    dataCurveFile.open("Data/" + fileName + ".txt");
    if (!dataCurveFile.is_open())
        return -1;

    for (int i = 0; i < d + 1; i++) 
    {
        dataCurveFile.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    for (int i = 0; i <= h + 1; i++) 
    {
        dataCurveFile >> result;
    }

    dataCurveFile.close();
    return round((float)result, 3);
}

float Validation(string s)
{
    bool flag = false;
    float result;


    while (true)
    {
        cout << s;
        cin >> result;

        if (cin.fail()) //input fail check
        {
            ValidationError();
            continue;
        }
        else
        {
            if (s == "frequency(MHz): ")
            {
                if (result < 100 || result > 2000) ValidationError();
                else break;
            }
            else if (s == "height of antenna(m): ")
            {
                if (result <= 0) ValidationError();
                else break;
            }
            else if (s == "distance step(km): ")
            {
                if (result < 0 || result > 1000) ValidationError();
                else break;
            }
            else if (s == "time percentage(%): ")
            {
                if (result < 1 || result > 50) ValidationError();
                else break;
            }
            else ValidationError();
        }
    }
    return result;
}

void ValidationError()
{
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << VALIDATION_ERROR_MEASSAGE << endl;
    system("pause");
    system("cls");
}

void CreateExcel(vector<FieldParams> fields, ios::iostate mode)
{
    ofstream outData;

    outData.open("outfile.csv", mode);

    if (!outData.is_open())
    {
        cout << "Error. Can't open excel file" << endl;
        system("pause");
        return;
    }

    if (mode == ios::out)
        outData << "Field Strength(DBm)" << ';'
        << "Distance(km)" << ';'
        << "Frequency(MHz)" << ';'
        << "Time(%)" << ';'
        << "Height(m)" << endl;
    else 
        outData << endl << endl
        << "Field Strength(DBm)" << ';'
        << "Distance(km)" << ';'
        << "Frequency(MHz)" << ';'
        << "Time(%)" << ';'
        << "Height(m)" << endl;

    for (int i = 0; i < fields.size(); i++)
    {
        outData << fields[i].fieldStrength << ';'
            << fields[i].distance << ';';
        if (i == 0)
        {
            outData << fields[i].frequency << ';'
                << fields[i].timePercentage << ';'
                << fields[i].antennaHeight;
        }

        outData << endl;
    }

    outData.close();
    LoadingAnimation();
}

float sqr(float a)
{
    return a * a;
}

float round(float value, int numberOfSymbols)
{
    return round(value * pow(10, numberOfSymbols)) / pow(10, numberOfSymbols);
}