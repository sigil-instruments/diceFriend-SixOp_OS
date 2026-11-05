#include <stdint.h>
// USB device name, required for every diceFriend OS: "diceFriend v.01".
// Project-local strong overrides of the Teensy core strings (kept in C on purpose:
// in C++ the declaration would clash with the core header). Do not rename.
struct sixop_usb_string { uint8_t length, type; uint16_t text[16]; };
struct sixop_usb_string usb_string_product_name = {
    32, 3, {'d','i','c','e','F','r','i','e','n','d',' ','v','.','0','1'}
};
struct sixop_usb_string usb_string_manufacturer_name = {
    34, 3, {'S','i','g','i','l','I','n','s','t','r','u','m','e','n','t','s'}
};
