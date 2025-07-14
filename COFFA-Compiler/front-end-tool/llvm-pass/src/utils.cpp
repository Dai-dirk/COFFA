#include "utils.h"

varType::varType(){
    value = 0;
}
varType::varType(int var){
    value = var;
}
varType::varType(double var){
    value = var;
}
varType::varType(std::string var){
    value = var;
}
varType::varType(const char* var){
    value = var;
}

std::string varType::to_string(){
    if (value.index() == 0)
    { // int
        return std::to_string(std::get<int>(value));
    }
    else if (value.index() == 1) // double
    {
        return std::to_string(std::get<double>(value));
    }
    else if (value.index() == 2) // string
    {
        return std::get<std::string>(value);
    }
    else
    {
        assert(false && "wrong variant type");
    }
}

int varType::index(){
    return (int)value.index();
}



varType varType::operator=(const varType var){
    value = var.value;
    return *this;
}
varType varType::operator=(const int& var){
    value = var;
    return *this;
}
varType varType::operator=(const double& var){
    value = var;
    return *this;
}
varType varType::operator=(const std::string& var){
    value = var;
    return *this;
}
varType varType::operator=(const char* var){
    value = var;
    return *this;
}
// +
varType operator+(const varType &var, const int& op){
    varType result = 0;
    if(var.value.index() == 2){
        if(op != 0)
            result = std::get<std::string>(var.value) + "+" + std::to_string(op);
        else
            result = std::get<std::string>(var.value);
    }else if(var.value.index() == 0){
        result = std::get<int>(var.value) + op;
    }else if(var.value.index() == 1){
        result = std::get<double>(var.value) + op;
    }
    return result;
}
varType operator+(const int& op, const varType &var){
    varType result =  var + op;
    return result;
}
varType operator+(const varType &var1, const varType &var2){
    varType result = 0;
    if(var1.value.index() == 2 && var2.value.index() == 2){//both string
        result = ((varType)var1).to_string() + "+" + ((varType)var2).to_string();
    }else{
        if(var1.value.index() == 0){
            int tem1 = std::get<int>(var1.value);
            result = tem1 + var2;
        }else if(var1.value.index() == 1){
            double tem1 = std::get<double>(var1.value);
            result = tem1 + var2;
        }else{
            std::string tem1 = std::get<std::string>(var1.value);
            if(var2.value.index() == 0){
                int tem2 = std::get<int>(var2.value);
                if(tem2 == 0)
                    result = tem1;
                else
                    result = tem1 + tem2;
            }else if(var1.value.index() == 1){
                double tem2 = std::get<double>(var2.value);
                if(tem2 == 0)
                    result = tem1;
                else
                    result = tem1 + tem2;
            }else{
                assert(false);
            }
        }
    }
    return result;
}
// -
varType operator-(const int& op, const varType &var){
    varType result = 0;
    if(var.value.index() == 2){
        if(op != 0)
            result = std::to_string(op) + "-" + std::get<std::string>(var.value);
        else
            result = "-" + std::get<std::string>(var.value);
    }else if(var.value.index() == 0){
        result = op - std::get<int>(var.value);
    }else if(var.value.index() == 1){
        result = op - std::get<double>(var.value);
    }
    return result;
}
varType operator-(const varType &var, const int& op){
    varType result = 0;
    if(var.value.index() == 2){
        if(op != 0)
            result = std::get<std::string>(var.value) + "-" + std::to_string(op);
        else
            result = std::get<std::string>(var.value);
    }else if(var.value.index() == 0){
        result = std::get<int>(var.value) - op;
    }else if(var.value.index() == 1){
        result = std::get<double>(var.value) - op;
    }
    return result;
}
varType operator-(const varType &var1, const varType &var2){
    varType result = 0;
    if(var1.value.index() == 2 && var2.value.index() == 2){
        result = ((varType)var1).to_string() + "-" + ((varType)var2).to_string();//both string
    }else{
        if(var1.value.index() == 0){
            int tem1 = std::get<int>(var1.value);
            result = tem1 - var2;
        }else if(var1.value.index() == 1){
            double tem1 = std::get<double>(var1.value);
            result = tem1 - var2;
        }else{
            std::string tem1 = std::get<std::string>(var1.value);
            if(var2.value.index() == 0){
                int tem2 = std::get<int>(var2.value);
                if(tem2 == 0)
                    result = tem1;
                else
                    result = tem1 - tem2;
            }else if(var2.value.index() == 1){
                double tem2 = std::get<double>(var2.value);
                if(tem2 == 0)
                    result = tem1;
                else
                    result = tem1 - tem2;
            }else{
                assert(false);
            }
        }
    }
    return result;
}
// *
varType operator*(const varType &var, const int& op){
    varType result = 0;
    if(var.value.index() == 2){
        if(op == 1){
            result = var;
        }else if(op == 0){
            result = 0;
        }else{
            result = "(" + std::get<std::string>(var.value) + ")*" + std::to_string(op);
        }
    }else if(var.value.index() == 0){
        result = std::get<int>(var.value) * op;
    }else if(var.value.index() == 1){
        result = std::get<double>(var.value) * op;
    }
    return result;
}
varType operator*(const int& op, const varType &var){
    varType result = var * op;
    return result;
}
varType operator*(const varType &var, const double& op){
    varType result = 0;
    if(var.value.index() == 2){
        if(op == 1){
            result = var;
        }else{
            result = "(" + std::get<std::string>(var.value) + ")*" + std::to_string(op);
        }
    }else if(var.value.index() == 0){
        result = std::get<int>(var.value) * op;
    }else if(var.value.index() == 1){
        result = std::get<double>(var.value) * op;
    }
    return result;
}
varType operator*(const double& op, const varType &var){
    varType result = var * op;
    return result;
}
varType operator*(const varType &var1, const varType &var2){
    varType result = 0;
    int idx1 = var1.value.index(), idx2 = var2.value.index();
    if(idx1 == 0){
        result = std::get<int>(var1.value) * var2;
    }else if(idx1 == 1){
        result = std::get<double>(var1.value) * var2;
    }else if(idx2 == 0){
        result = var1 * std::get<int>(var2.value);
    }else if(idx2 == 1){
        result = var1 * std::get<double>(var2.value);
    }else{
        result = "(" + ((varType)var1).to_string() + ") * (" + ((varType)var2).to_string() + ")";
    }
    return result;
}
// /
varType operator/(const int& op, const varType &var){
    varType result = 0;
    if(var.value.index() == 2){
        result = std::to_string(op) + "/(" + std::get<std::string>(var.value) + ")";
    }else if(var.value.index() == 0){
        result = op / std::get<int>(var.value);
    }else if(var.value.index() == 1){
        result = op / std::get<double>(var.value);
    }
    return result;
}
varType operator/(const varType &var, const int& op){
    varType result = 0;
    if(var.value.index() == 2){
        if(op == 1){
            result = var;
        }else{
            result = "(" + std::get<std::string>(var.value) + ")/" + std::to_string(op);
        }
    }else if(var.value.index() == 0){
        result = std::get<int>(var.value) / op;
    }else if(var.value.index() == 1){
        result = std::get<double>(var.value) / op;
    }
    return result;
}
varType operator/(const double& op, const varType &var){
    varType result = 0;
    if(var.value.index() == 2){
        result = std::to_string(op) + "/(" + std::get<std::string>(var.value) + ")";
    }else if(var.value.index() == 0){
        result = op / std::get<int>(var.value);
    }else if(var.value.index() == 1){
        result = op / std::get<double>(var.value);
    }
    return result;
}
varType operator/(const varType &var, const double& op){
    varType result = 0;
    if(var.value.index() == 2){
        if(op == 1){
            result = var;
        }else{
            result = "(" + std::get<std::string>(var.value) + ")/" + std::to_string(op);
        }
    }else if(var.value.index() == 0){
        result = std::get<int>(var.value) / op;
    }else if(var.value.index() == 1){
        result = std::get<double>(var.value) / op;
    }
    return result;
}
varType operator/(const varType &var1, const varType &var2){
    varType result = 0;
    int idx1 = var1.value.index(), idx2 = var2.value.index();
    if(idx1 == 0){
        result = std::get<int>(var1.value) / var2;
    }else if(idx1 == 1){
        result = std::get<double>(var1.value) / var2;
    }else if(idx2 == 0){
        result = var1 / std::get<int>(var2.value);
    }else if(idx2 == 1){
        result = var1 / std::get<double>(var2.value);
    }else{
        result = ((varType)var1).to_string() + "/" + ((varType)var2).to_string();
    }
    return result;
}
//%
varType operator%(const varType &var, const int& op){
    varType result = 0;
    if(var.value.index() == 2){
        result = "(" + std::get<std::string>(var.value) + ")%" + std::to_string(op);
    }else if(var.value.index() == 0){
        result = std::get<int>(var.value) % op;
    }else if(var.value.index() == 1){
        result = (int)(std::get<double>(var.value)) % op;
    }
    return result;
}
// ==
bool operator==(const varType &var, const char* op){
    if(var.value.index() == 2){
        std::string varstring = std::get<std::string>(var.value);
        std::string charstring = op;
        if(varstring == charstring){
            return true;
        }
    }
    return false;
}
bool operator==(const char* op, const varType &var){
    if(var == op){
        return true;
    }else{
        return false;
    }
}
bool operator==(const varType &var, const int& op){
    if(var.value.index() == 0){
        int varint = std::get<int>(var.value);
        if(varint == op){
            return true;
        }
    }
    else if(var.value.index() == 1){
        int varint = std::get<double>(var.value);
        if(varint == op){
            return true;
        }
    }else{
        std::cout << "[WARNING] can not compare int with string\n";
    }
    return false;
}
bool operator==(const int& op, const varType &var){
    if(var == op){
        return true;
    }else{
        return false;
    }
}
//>
bool operator>(const int& op, const varType &var){
    if(var.value.index() == 0){
        int varint = std::get<int>(var.value);
        if(op > varint){
            return true;
        }
    }else if(var.value.index() == 1){
        int varint = std::get<double>(var.value);
        if(op > varint){
            return true;
        }
    }else{
        std::cout << "[WARNING] can not compare int with string\n";
    }
    return false;
}
// accumulation
varType operator+=(varType &var, const int& op){
    varType result = var + op;
    var = result;
    return result;
}
varType operator+=(varType &var1, const varType &var2){
    varType result = var1 + var2;
    var1 = result;
    return result;
}
varType operator-=(varType &var, const int& op){
    varType result = var - op;
    var = result;
    return result;
}
varType operator-=(varType &var1, const varType &var2){
    varType result = var1 - var2;
    var1 = result;
    return result;
}
varType operator*=(varType &var, const int& op){
    varType result = var * op;
    var = result;
    return result;
}
varType operator*=(varType &var1, const varType &var2){
    varType result = var1 * var2;
    var1 = result;
    return result;
}
varType operator/=(varType &var, const int& op){
    varType result = var / op;
    var = result;
    return result;
}

varType operator/=(varType &var, const double& op){
    varType result = var / op;
    var = result;
    return result;
}

varType operator/=(varType &var, const varType &var2){
    varType result = var / var2;
    var = result;
    return result;
}

varType varPow(int a, varType var){
    varType result = 0;
    if(var.value.index() == 2){
        result = std::to_string(a) + " ^ (" + std::get<std::string>(var.value) + ")";
    }else if(var.value.index() == 0){
        result = pow(a, std::get<int>(var.value));
    }else if(var.value.index() == 1){
        result = pow(a, std::get<double>(var.value));
    }
    return result;
}