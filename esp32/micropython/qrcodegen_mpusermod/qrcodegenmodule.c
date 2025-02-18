// Include MicroPython API.
#include "py/runtime.h"
#include "py/mphal.h"
#include "qrcodegen.h"

static mp_obj_t qrcodegen_from_text(mp_obj_t text_obj)
{  
  const char *text = mp_obj_str_get_str(text_obj);                // User-supplied text
  
  enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level

  // Make and print the QR Code symbol
  uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
  bool ok = false;
  {
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
    ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
            qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
  }
  if( ! ok )
  {
    return mp_obj_new_str_from_cstr("<QR-Code error>");
  }
  
  int size = qrcodegen_getSize(qrcode);
  int border = 4;
  int totsize = size+border*2;
  if( totsize*totsize >= 1024 )
  {
    return mp_obj_new_str_from_cstr("<QR-Code too large>");
  }
  char str[1024];
  int c = 0;

  for (int y = -border; y < size + border; y++)
  {
    for (int x = -border; x < size + border; x++)
    {
      str[c++] = qrcodegen_getModule(qrcode, x, y) ? '#' : ' ';
    }
    str[c++] = '\n';
  }
  return mp_obj_new_str(str,c);
}

// Define a Python reference to the function above.
static MP_DEFINE_CONST_FUN_OBJ_1(qrcodegen_from_text_obj, qrcodegen_from_text);

// Define all attributes of the module.
// Table entries are key/value pairs of the attribute name (a string)
// and the MicroPython object reference.
// All identifiers and strings are written as MP_QSTR_xxx and will be
// optimized to word-sized integers by the build system (interned strings).
static const mp_rom_map_elem_t qrcodegen_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_qrcodegen) },
    { MP_ROM_QSTR(MP_QSTR_from_text), MP_ROM_PTR(&qrcodegen_from_text_obj) },
};
static MP_DEFINE_CONST_DICT(qrcodegen_module_globals, qrcodegen_module_globals_table);

// Define module object.
const mp_obj_module_t qrcodegen_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&qrcodegen_module_globals,
};

// Register the module to make it available in Python.
MP_REGISTER_MODULE(MP_QSTR_qrcodegen, qrcodegen_user_cmodule);
