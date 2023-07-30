/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <tusb.h>

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = USB_BCD,

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = USB_VID,
    .idProduct = USB_PID,
    .bcdDevice = DEVICE_BCD,

    .iManufacturer = STRING_DESC_MANUFACTURER,
    .iProduct = STRING_DESC_PRODUCT,
    .iSerialNumber = STRING_DESC_SERIAL,

    .bNumConfigurations = 0x01};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void) { return (uint8_t const *)&desc_device; }

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+
enum {
  ITF_NUM_GS_0 = 0,
  // ITF_NUM_DFU_RT,
  ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN \
  (TUD_CONFIG_DESC_LEN + TUD_VENDOR_DESC_LEN * CFG_TUD_VENDOR + TUD_DFU_RT_DESC_LEN * CFG_TUD_DFU_RUNTIME)
#define EPNUM_GS_EPIN 0x81
#define EPNUM_GS_EPOUT 0x02

uint8_t const desc_fs_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
    TUD_VENDOR_DESCRIPTOR(ITF_NUM_GS_0, STRING_DESC_GS_USB_INTERFACE, EPNUM_GS_EPOUT, EPNUM_GS_EPIN,
                          CFG_TUD_VENDOR_EPSIZE),
    //    TUD_DFU_RT_DESCRIPTOR(ITF_NUM_DFU_RT, 6, 0x0B, 1000, 4096),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
  (void)index;  // for multiple configurations

  return desc_fs_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const *string_desc_arr[] = {
    [0] = (const char[]){0x09, 0x04},                            // 0: is supported language is English (0x0409)
    [STRING_DESC_MANUFACTURER] = USB_DESC_MANUFACTURER,          // 1: Manufacturer
    [STRING_DESC_PRODUCT] = USB_DESC_PRODUCT,                    // 2: Product
    [STRING_DESC_SERIAL] = USB_DESC_SERIAL,                      // 3: Serials, should use chip ID
    [STRING_DESC_CDC_INTERFACE] = USB_DESC_CDC_INTERFACE,        // 4: CDC Interface
    [STRING_DESC_GS_USB_INTERFACE] = USB_DESC_GS_USB_INTERFACE,  // 5: GS_USB Interface
    [STRING_DESC_DFU_INTERFACE] = USB_DESC_DFU_INTERFACE,        // 6: DFU
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;

  uint8_t chr_count;
  switch(index){
  case 0: {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
    break;
  };
  case STRING_DESC_SERIAL:{
  
  }
  default: {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) return NULL;

    const char *str = string_desc_arr[index];

    // Cap at max char
    chr_count = (uint8_t)strlen(str);
    if (chr_count > 31) chr_count = 31;

    // Convert ASCII string into UTF-16
    for (uint8_t i = 0; i < chr_count; i++) {
      _desc_str[1 + i] = str[i];
    }
  }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

  return _desc_str;
}
