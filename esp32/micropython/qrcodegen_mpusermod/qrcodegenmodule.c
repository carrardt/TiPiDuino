// Include MicroPython API.
#include "py/runtime.h"
#include "py/mphal.h"
#include "qrcodegen.h"

// This structure represents Timer instance objects.
#define MP_QRCODEGEN_MAX_SIZE 64
typedef struct _qrcodegen_QRCode_obj_t {
    // All objects start with the base.
    mp_obj_base_t base;
    
    // QRCode size (must not exceed 64)
    mp_uint_t m_size;

    // encoding properties
    enum qrcodegen_Ecc m_errCorLvl;
    
    // data storage
    byte m_data[ (MP_QRCODEGEN_MAX_SIZE * MP_QRCODEGEN_MAX_SIZE) / 8 ] ;
    
} qrcodegen_QRCode_obj_t;

// This is the QRCode.size() method. After creating a QRCode object, this
// can be called to get the square size.
static mp_obj_t qrcodegen_QRCode_size(mp_obj_t self_in)
{
    qrcodegen_QRCode_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int_from_uint( self->m_size );
}
static MP_DEFINE_CONST_FUN_OBJ_1(qrcodegen_QRCode_size_obj, qrcodegen_QRCode_size);


// This is the QRCode.module(x,y) method. After creating a QRCode object, this
// can be called to get the module (boolean value) at a particular location in qrcode bitmap.
static mp_obj_t qrcodegen_QRCode_resize(mp_obj_t self_in, mp_obj_t sz_obj)
{
    qrcodegen_QRCode_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t sz = mp_obj_get_int( sz_obj );
    if( sz<0 || sz>MP_QRCODEGEN_MAX_SIZE )
    {
      mp_raise_ValueError(MP_ERROR_TEXT("invalid QRCode size, must be in range [0;MP_QRCODEGEN_MAX_SIZE]"));
    }
    if( sz != self->m_size )
    {
      self->m_size = sz;
      memset(self->m_data, 0, (sz+7)/8 );
    }
    return mp_obj_new_int_from_uint( self->m_size );
}
static MP_DEFINE_CONST_FUN_OBJ_2(qrcodegen_QRCode_resize_obj, qrcodegen_QRCode_resize);

// This is the QRCode.module(x,y) method. After creating a QRCode object, this
// can be called to get the module (boolean value) at a particular location in qrcode bitmap.
static mp_obj_t qrcodegen_QRCode_module(mp_obj_t self_in, mp_obj_t x_obj, mp_obj_t y_obj)
{
    qrcodegen_QRCode_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t x = mp_obj_get_int( x_obj );
    mp_int_t y = mp_obj_get_int( y_obj );
    if( x<0 || x>=self->m_size || y<0 || y>=self->m_size ) return mp_const_false;
    int i = ( y * self->m_size + x );
    int b = i % 8;
    i /= 8;
    return ( ( self->m_data[i] >> b ) & 0x01 ) != 0 ? mp_const_true : mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_3(qrcodegen_QRCode_module_obj, qrcodegen_QRCode_module);

// initializes QRCode from a text string
static mp_obj_t qrcodegen_QRCode_fromText(mp_obj_t self_in, mp_obj_t text_obj)
{  
  qrcodegen_QRCode_obj_t *self = MP_OBJ_TO_PTR(self_in);
  const char *text = mp_obj_str_get_str(text_obj);                // User-supplied text
  
  // Make and print the QR Code symbol
  uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
  bool ok = false;
  {
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
    ok = qrcodegen_encodeText(text, tempBuffer, qrcode, self->m_errCorLvl,
            qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
  }
  if( ! ok )
  {
    mp_raise_ValueError(MP_ERROR_TEXT("QRCode generation failed"));   
    self->m_size = 0;
    return mp_const_none;
  }
  
  int size = qrcodegen_getSize(qrcode);
  if( size > MP_QRCODEGEN_MAX_SIZE )
  {
    mp_raise_ValueError(MP_ERROR_TEXT("QRCode invalid size"));   
    self->m_size = 0;
    return mp_const_none;
  }
  self->m_size = size;
  for (int y = 0; y < size ; y++)
  {
    for (int x = 0; x < size ; x++)
    {
      int i = ( y * self->m_size + x );
      int b = i % 8;
      i /= 8;
      self->m_data[i] &= ~(1u<<b);
      if( qrcodegen_getModule(qrcode, x, y) ) self->m_data[i] |= (1u<<b);
      else self->m_data[i] &= ~(1u<<b);
    }
  }
  return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(qrcodegen_QRCode_fromText_obj, qrcodegen_QRCode_fromText);


// This collects all methods and other static class attributes of the QRCode.
// The table structure is similar to the module table, as detailed below.
static const mp_rom_map_elem_t qrcodegen_QRCode_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_size), MP_ROM_PTR(&qrcodegen_QRCode_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_resize), MP_ROM_PTR(&qrcodegen_QRCode_resize_obj) },
    { MP_ROM_QSTR(MP_QSTR_module), MP_ROM_PTR(&qrcodegen_QRCode_module_obj) },
    { MP_ROM_QSTR(MP_QSTR_fromText), MP_ROM_PTR(&qrcodegen_QRCode_fromText_obj) },
};
static MP_DEFINE_CONST_DICT(qrcodegen_QRCode_locals_dict, qrcodegen_QRCode_locals_dict_table);


// Handles AdvancedTimer.__repr__, AdvancedTimer.__str__.
static void qrcodegen_QRCode_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
  qrcodegen_QRCode_obj_t *self = MP_OBJ_TO_PTR(self_in);
  if (kind == PRINT_STR)
  {
    for(int y=0;y<self->m_size;y++)
    {
      for(int x=0;x<self->m_size;x++)
      {
        int i = ( y * self->m_size + x );
        int b = i % 8;
        i /= 8;
        int m = ( self->m_data[i] & (1u<<b) ) != 0 ;
        mp_printf(print, "%s", m ? "##" : "  " );
      }
      mp_printf(print, "\n");
    }
  }

  if (kind != PRINT_STR)
  {
    mp_printf(print, "<%q>", MP_QSTR_QRCode);
  }
}

// This represents Timer.__new__ and Timer.__init__, which is called when
// the user instantiates a Timer object.
static mp_obj_t qrcodegen_QRCode_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
    // Allocates the new object and sets the type.
    qrcodegen_QRCode_obj_t *self = mp_obj_malloc(qrcodegen_QRCode_obj_t, type);

    // Initializes the qrcode to empty bitmap.
    self->m_size = 0;
    self->m_errCorLvl = qrcodegen_Ecc_LOW;

    // The make_new function always returns self.
    return MP_OBJ_FROM_PTR(self);
}

// This defines the type(QRCode) object.
MP_DEFINE_CONST_OBJ_TYPE(
    qrcodegen_type_QRCode,
    MP_QSTR_QRCode,
    MP_TYPE_FLAG_NONE,
    print, qrcodegen_QRCode_print,
    make_new, qrcodegen_QRCode_make_new,
    locals_dict, &qrcodegen_QRCode_locals_dict
    );

// Define all attributes of the module.
// Table entries are key/value pairs of the attribute name (a string)
// and the MicroPython object reference.
// All identifiers and strings are written as MP_QSTR_xxx and will be
// optimized to word-sized integers by the build system (interned strings).
static const mp_rom_map_elem_t qrcodegen_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_qrcodegen) },
    { MP_ROM_QSTR(MP_QSTR_QRCode),    MP_ROM_PTR(&qrcodegen_type_QRCode) },
};
static MP_DEFINE_CONST_DICT(qrcodegen_module_globals, qrcodegen_module_globals_table);

// Define module object.
const mp_obj_module_t qrcodegen_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&qrcodegen_module_globals,
};

// Register the module to make it available in Python.
MP_REGISTER_MODULE(MP_QSTR_qrcodegen, qrcodegen_user_cmodule);
