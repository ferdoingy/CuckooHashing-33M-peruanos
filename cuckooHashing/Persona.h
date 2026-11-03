#include <cstdint> //usar enteros con tamaños especificos
#include <string> 

struct Persona{ //estructura que almacena la información de la persona 
    uint32_t dni;
    uint16_t nombre1;
    uint16_t nombre2;
    uint16_t apellido1;
    uint16_t apellido2;

    uint8_t nacionalidad;
    uint8_t lugarNacimiento;
    uint8_t ubicacion;
    uint8_t tipoVia;
    uint8_t nombreVia;

    uint16_t numeroVia;
    uint32_t telefono;

    uint8_t estadoCivil;
    uint8_t dominioCorreo;

    string usuarioCorreo;
};