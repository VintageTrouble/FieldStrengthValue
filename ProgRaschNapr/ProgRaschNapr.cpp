#include <string>
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

#pragma region Structs
struct  FieldParams
{
    double timePercentage = 0;//p
    double frequency = 0; //частота(f)
    double antennaHeight = 0; //h
    double distance = 0;//d
};
#pragma endregion

#pragma region Consts
const double heights[9] = { 1200, 600, 300, 150, 75, 437.5, 20, 10 };

const double C0 = 2.515517;
const double C1 = 0.802853;
const double C2 = 0.010328;
const double D1 = 1.432788;
const double D2 = 0.189269;
const double D3 = 0.001308;

const string VALIDATION_ERROR_MEASSAGE = "ERROR VALUE";
#pragma endregion

#pragma region Functions Prototypes
FieldParams ReadFromKeyboard();

double CalculateFieldStrValue(FieldParams field);
double CalculateFieldStrValue(double timePercentage, double frequency, double antennaHeight, double distance);

double DAnnex();
double HAnnex(double d, double h, double t, double f);

double Interpolation(double eInf, double eSup, double x, string dataType);

double ReadFieldStrengthValue(double timePercentage, double frequency, double antennaHeight, double distance);

double Validation(string s);
void ValidationError();

double SearchInf(double data, string dataType);
double SearchSup(double data, string dataType);

double Q(double x);
double T(double x);
double Xi(double x);
double J(double h, double f);

double sqr(double a);
double round(double value, int numberOfSymbols);
#pragma endregion

int main()
{
    FieldParams field = ReadFromKeyboard();
    system("cls");

    cout << "frequency(MHz): " << field.frequency << endl;
    cout << "time percentage(%): " << field.timePercentage << endl;
    cout << "height of transmitting/base antenna(m): " << field.antennaHeight << endl;
    cout << "distance(km): " << field.distance << endl << endl;

    cout << "Field-strength value: " << round(CalculateFieldStrValue(field), 3) << " DBm" << endl << endl;

    system("pause");
}

FieldParams ReadFromKeyboard()
{
    FieldParams field;

    field.frequency = Validation("frequency(MHz): ");
    field.timePercentage = Validation("time percentage(%): ");
    field.antennaHeight = Validation("height of transmitting/base antenna(m): ");
    field.distance = Validation("distance(km): ");

    return field;
}

double CalculateFieldStrValue(FieldParams field)
{
    return CalculateFieldStrValue(field.timePercentage, field.frequency, field.antennaHeight, field.distance);
}

double CalculateFieldStrValue(double timePercentage, double frequency, double antennaHeight, double distance)
{
    double E = 0;
    double Einf_t = 0;
    double Esup_t = 0;
    double Einf_f = 0;
    double Esup_f = 0;
    double Einf_h = 0;
    double Esup_h = 0;

    bool flag = false;

    if (distance < 1)
    {
        //TODO: DAnnex
        return -1;
    }

    double timePercentageTemp = SearchInf(timePercentage, "timePercentage");
    double frequencyTemp = SearchInf(frequency, "frequency");
    double antennaHeightTemp = SearchInf(antennaHeight, "antennaHeight");

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

double SearchInf(double data, string dataType) 
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
            data -= 20;
            data = fmod(data, 5) * 5;
            data += 20;
        }
        else if (data >= 100 && data < 200)
        {
            data -= 100;
            data = fmod(data, 10) * 10;
            data += 100;
        }
        else if (data >= 200 && data < 1000)
        {
            data -= 200;
            data = fmod(data, 25) * 25;
            data += 200;
        }
        else if (data >= 1000)
        {
            data = 1000;
        }
    }

    return data;
}

double SearchSup(double data, string dataType)
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

double Interpolation(double eInf, double eSup, double x, string dataType)
{
    double xInf = SearchInf(x, dataType);
    double xSup = SearchSup(x, dataType);

    if (dataType == "timePercentage")
    {
        double Qt = Q(x / 100);
        double Qinf = Q(xInf / 100);
        double Qsup = Q(xSup / 100);

        return eSup * (Qinf - Qt) / (Qinf - Qsup) + eInf * (Qt - Qsup) / (Qinf - Qsup);
    }

    return eInf + ((eSup - eInf) * (log(x / xInf) / log(xSup / xInf)));
}

double DAnnex()
{
    return -1;
}

double HAnnex(double d, double h, double t, double f)
{
    double E10 = Interpolation(ReadFieldStrengthValue(t, f, SearchInf(h, "antennaHeight"), SearchSup(d, "distance")), 
        ReadFieldStrengthValue(t, f, SearchSup(10, "antennaHeight"), SearchSup(d, "distance")),
        10, "antennaHeight");
    double E20 = Interpolation(ReadFieldStrengthValue(t, f, SearchInf(h, "antennaHeight"), SearchSup(d, "distance")),
        ReadFieldStrengthValue(t, f, SearchSup(h, "antennaHeight"), SearchSup(d, "distance")),
        10, "antennaHeight");;

    double C1020 = E10 - E20;
    double Ch1neg10 = 6.03 - J(h, f);
    double E0 = E10 + 0.5 * (C1020 + Ch1neg10);
    double E = E0 + 0.1 * h * (E10 - E0);

    return E;
}

double J(double h, double f)
{
    double Kv = 0;
    if (f == 100)
        Kv = 1.35;
    else if (f == 600)
        Kv = 3.31;
    else if (f == 2000)
        Kv = 6.00;
        
    double O = atan(-h / 9000);
    double v = Kv * O;
    double j = 6.9 + 20 * log(sqrt(sqr(v - 0.1) + 1) + v - 0.1);
    return 0;
}

double Q(double x)
{
    if (x <= 0.5)
        return T(x) - Xi(x);
    else
        return -(T(1 - x) - Xi(1 - x));
}

double T(double x)
{
    return round(sqrt(-2 * log(x)), 3);
}

double Xi(double x)
{
    return round((((C2 * T(x) + C1) * T(x)) + C0) / (((D3 * T(x) + D2) * T(x) + D1) * T(x) + 10), 3);
}

double ReadFieldStrengthValue(double timePercentage, double frequency, double antennaHeight, double distance)
{
    ifstream dataCurveFile;
    string fileName = to_string((int)frequency) + " MHz " + to_string((int)timePercentage) + " time";
    double result = 0;
    int d = distance;
    int h = antennaHeight;

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
    return round(result, 3);
}

double Validation(string s)
{
    bool flag = false;
    double result;


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
                if (result < 100 || result > 50) ValidationError();
                else break;
            }
            else if (s == "height of transmitting/base antenna(m): ")
            {
                if (result <= 0) ValidationError();
                else break;
            }
            else if (s == "distance(km): ")
            {
                if (result < 0 || result > 1000) ValidationError();
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

double sqr(double a)
{
    return a * a;
}

double round(double value, int numberOfSymbols)
{
    double res = round(value * pow(10, numberOfSymbols)) / pow(10, numberOfSymbols);
    return res;
}