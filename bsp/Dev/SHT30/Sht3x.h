//
// Created by 衣陈 on 2023/6/28.
//

#ifndef F103TEST_SHT3X_H
#define F103TEST_SHT3X_H

#include "IIC.h"
class Sht3x : public  IIC7bitDev{
public:
    using IIC7bitDev::IIC7bitDev;

    void start_period();
    void fetch_data(double& T, double& RH);
};


#endif //F103TEST_SHT3X_H
