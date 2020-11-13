#pragma once
#include <Arduino.h>

class StringFormatHelper
{
public:
  static String ToString(int number, int minDigits)
  {
    String result(number);
    while(result.length() < minDigits)
    {
      result = "0" + result;
    }
    return result;
  }
};
