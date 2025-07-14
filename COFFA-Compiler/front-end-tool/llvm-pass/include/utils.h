#include <cmath>
#include <queue>
#include <iostream>
#include <fstream>
#include <variant>
#include <assert.h>

//varType can be:
//1. int (normal constant)
//2. double (when pattern calculation process has fractional)
//3. std::string (when fixed variable participate calculation, remain expression as std::string)
class varType{
    public:
        varType operator=(const int& var);
        varType operator=(const double& var);
        varType operator=(const std::string& var);
        varType operator=(const varType var);
        varType operator=(const char* var);
        // +
        friend varType operator+(const int& op, const varType &var);
        friend varType operator+(const varType &var, const int& op);
        friend varType operator+(const varType &var1, const varType &var2);
        // -
        friend varType operator-(const int& op, const varType &var);
        friend varType operator-(const varType &var, const int& op);
        friend varType operator-(const varType &var1, const varType &var2);
        // *
        friend varType operator*(const int& op, const varType &var);
        friend varType operator*(const varType &var, const int& op);
        friend varType operator*(const varType &var1, const varType &var2);
        friend varType operator*(const double& op, const varType &var);
        friend varType operator*(const varType &var, const double& op);
        // /
        friend varType operator/(const int& op, const varType &var);
        friend varType operator/(const varType &var, const int& op);
        friend varType operator/(const double& op, const varType &var);
        friend varType operator/(const varType &var, const double& op);
        friend varType operator/(const varType &var1, const varType &var2);
        // %
        friend varType operator%(const varType &var, const int& op);
        //<<
        // friend varType operator%(const varType &var, const int& op);
        // ==
        friend bool operator==(const varType &var, const char* op);
        friend bool operator==(const char* op, const varType &var);
        friend bool operator==(const varType &var, const int& op);
        friend bool operator==(const int& op, const varType &var);
        // >
        friend bool operator>(const int& op, const varType &var);

        friend varType operator+=(varType &var, const int& op);
        friend varType operator+=(varType &var1, const varType &var2);
        friend varType operator-=(varType &var, const int& op);
        friend varType operator-=(varType &var1, const varType &var2);
        friend varType operator*=(varType &var, const int& op);
        friend varType operator*=(varType &var1, const varType &var2);
        friend varType operator/=(varType &var, const int& op);
        friend varType operator/=(varType &var, const double &var2);
        friend varType operator/=(varType &var, const varType &var2);
        std::string to_string();
        int index();

        template<typename TYPE>
        TYPE getNum(){
            if(value.index() == 2){
                std::cout << "[WARNING] you are transforming a string to number\n";
                return (int)0;
            }else if(value.index() == 0){
                if(std::is_same<TYPE,double>::value){
                    std::cout << "[WARNING] you are transforming int to double\n";
                }
                return std::get<int>(value);
            }else{
                if(std::is_same<TYPE,int>::value){
                    std::cout << "[WARNING] you are transforming double to int\n";
                }
                return std::get<double>(value);
            }
        }

        varType();
        varType(int var);
        varType(double var);
        varType(std::string var);
        varType(const char* var);
        std::variant<int, double, std::string> value;
    private:
};

varType varPow(int a, varType var);