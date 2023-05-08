#include "LOG.hpp"
template<>
void print<char &>(char &ch) {
    HAL_UART_Transmit(LOGGER,&ch,1,300);
}
template<>
void print<char &&>(char &&ch) {
    HAL_UART_Transmit(LOGGER,&ch,1,300);
}

template<>
void print<const bool &>(const bool &ch) {
    std::string_view str=ch?"true":"false";
    HAL_UART_Transmit(LOGGER,str.data(),str.size(),300);
}
template<>
void print<const bool &&>(const bool &&ch) {
    std::string_view str=ch?"true":"false";
    HAL_UART_Transmit(LOGGER,str.data(),str.size(),300);
}