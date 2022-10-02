#define Log(x) std::cout << x << std::endl;
#define Lognl(x) std::cout << std::endl << x << std::endl;


unsigned char randChar(int lower, int higher)
{
    return (unsigned char)(rand()%(higher - lower + 1) + lower);
}

void splitIntoArray3(std::string message, char delimiter, std::string (&array)[3])
{
    std::string t1;
    for (int i = 0; i < 2; i++)
    {
        t1 = message.substr(0, message.find(delimiter));
        array[i] = t1;
        for (int i = 0; i < t1.length()+1; i++) {
            message.erase(0,1);
        }
    }
    array[2] = message;
}

void splitIntoArray4(std::string message, char delimiter, std::string (&array)[4])
{
    std::string t1;
    for (int i = 0; i < 3; i++)
    {
        t1 = message.substr(0, message.find(delimiter));
        array[i] = t1;
        for (int i = 0; i < t1.length()+1; i++) {
            message.erase(0,1);
        }
    }
    array[3] = message;
}