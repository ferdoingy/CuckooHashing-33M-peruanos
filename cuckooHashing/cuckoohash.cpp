#include <iostream>
#include <fstream> //para abrir archivos
#include <string> //manipular cadenas de textos
#include <cstdint> //usar enteros con tamaños especificos
#include <iomanip> //Dar un formato a lo que se imprime
#include <stdexcept> //Crear errores para explicar algun problema

using namespace std;

//funcion para validar que el archivo este completo

void leerBytes(istream& archivo, char* destino, size_t cantidad){
    if(cantidad == 0){
        return;
    }
    if(!archivo.read(destino, static_cast<streamsize>(cantidad))){
        throw runtime_error(
            "No se puede leer completo los bytes. "
            "El archivo puede que este incompleto "
        );
    }
}
//funcion para convertir los enteros generados en binario y guardados en little endian con python
uint64_t convertirLittleEndian(
    const unsigned char* bytes, //puntero al primer byte de un arreglo de bytes que no modificara sus datos
    size_t cantidad //cantidad de bytes que hay que leer del arreglo
){
    uint64_t numero = 0; //variable acumuladora,
    //Se recorren los bytes uno por uno desde el indice 0 hasta cantidad-1
    for(size_t i=0;i<cantidad;i++){
        numero |= static_cast<uint64_t>(bytes[i]) << (8*i); 
        //static_cast<uint64_t>(bytes[i]) :se convierte el byte a 64 bits para no perder bits
        //<< (8*i) :se desplaza el byte a su posición correcta (byte 0 -> desplaza 0 bits; byte 1-> desplaza 8 bits...)
        //numero |= :combina el resultado con numero(acumulador) a nivel de bits, 
    }

    return numero; //Retorna el numero ya interpretado
}

struct Cabecera{
    string identificador;
    uint32_t version; //tipo de version del archivo
    uint64_t cantidad_personas; //saber la cantidad de personas que se colocaron en el archivo
};

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

//funcion que recibe un archivo y devuelve la cabecera con los datos ya leidos
Cabecera leerCabecera(istream& archivo){
    unsigned char bytes[16]; //Reserva espacio para la cabecera
    leerBytes(
        archivo,
        reinterpret_cast<char*>(bytes), //compilador cambia el tipo de dato de bytes al del parametro "destino"
        sizeof(bytes) //tamañode bytes
    );

    Cabecera cabecera;

    //Construye un string con los bytes 0 a 3
    cabecera.identificador = string(
        reinterpret_cast<const char*>(bytes), 4
    );

    //Se leen 4 bytes (posiciones 4 a 7) y se convierten a número. convertirLittleEndian devuelve uint64_t. 
    //se realiza un cast para hacer más pequeño el tamaño
    cabecera.version = static_cast<uint32_t>(
        convertirLittleEndian(bytes + 4, 4)
    );
    //Se hace lo mismo que el anterior pero con las posicion 5 a la 8
    cabecera.cantidad_personas =
        convertirLittleEndian(bytes + 8, 8);

    //runtimes error en caso
    if (cabecera.identificador != "DNIP") {
        //verifica si la firma es DNIP
        throw runtime_error("El archivo no tiene el formato DNIP.");
    }
        //verifica si la version es compatible
    if (cabecera.version != 1) {
        throw runtime_error("Version del archivo no compatible.");
    }
    //verifica si la cantidad de personas es valida
    if (cabecera.cantidad_personas == 0 ||
        cabecera.cantidad_personas > 100'000'000ULL) {
        throw runtime_error("Cantidad de personas invalida.");
    }

    return cabecera; //Retorna la cabecera ya le ida
}

// Lee una persona desde la posición actual del archivo.
// Debe llamarse estando al inicio de un registro.
Persona leerPersona(istream& archivo) {
    unsigned char bytes[26]; // Reserva espacio para la parte fija del registro.

    // Lee los 26 bytes fijos; el correo se leerá después.
    leerBytes(
        archivo,                        // Archivo abierto.
        reinterpret_cast<char*>(bytes), // Dirección donde guardar los bytes.
        sizeof(bytes)                   // Cantidad que se debe leer: 26 bytes.
    );

    Persona persona; // Variable que recibirá todos los campos interpretados.

    // Los desplazamientos siguientes comienzan a contar desde cero.
    // Corresponden al formato Python: <IHHHHBBBBBHIBBB.

    // DNI: bytes 0, 1, 2 y 3.
    persona.dni = static_cast<uint32_t>(
        convertirLittleEndian(bytes, 4)
    );

    // Primer nombre: bytes 4 y 5.
    persona.nombre1 = static_cast<uint16_t>(
        convertirLittleEndian(bytes + 4, 2)
    );

    // Segundo nombre: bytes 6 y 7.
    persona.nombre2 = static_cast<uint16_t>(
        convertirLittleEndian(bytes + 6, 2)
    );

    // Primer apellido: bytes 8 y 9.
    persona.apellido1 = static_cast<uint16_t>(
        convertirLittleEndian(bytes + 8, 2)
    );

    // Segundo apellido: bytes 10 y 11.
    persona.apellido2 = static_cast<uint16_t>(
        convertirLittleEndian(bytes + 10, 2)
    );

    // Los campos de un solo byte no necesitan conversión de orden.
    persona.nacionalidad = bytes[12];     // Byte 12.
    persona.lugarNacimiento = bytes[13]; // Byte 13.
    persona.ubicacion = bytes[14];        // Byte 14.
    persona.tipoVia = bytes[15];          // Byte 15.
    persona.nombreVia = bytes[16];        // Byte 16.

    // Número de vía: bytes 17 y 18.
    persona.numeroVia = static_cast<uint16_t>(
        convertirLittleEndian(bytes + 17, 2)
    );

    // Teléfono: bytes 19, 20, 21 y 22.
    persona.telefono = static_cast<uint32_t>(
        convertirLittleEndian(bytes + 19, 4)
    );

    persona.estadoCivil = bytes[23];   // Byte 23: índice del estado civil.
    persona.dominioCorreo = bytes[24]; // Byte 24: índice del dominio del correo.

    // Byte 25: cantidad de bytes del usuario del correo.
    const uint8_t longitudCorreo = bytes[25];

    // Ajusta la longitud del string para poder escribir los caracteres dentro.
    persona.usuarioCorreo.resize(longitudCorreo);

    // Lee los bytes que siguen a la parte fija del registro.
    leerBytes(
        archivo,                      // Continúa desde la posición actual.
        persona.usuarioCorreo.data(), // Memoria del string.
        longitudCorreo                // Número de caracteres que se deben leer.
    );

    // Devuelve la persona completa.
    // El archivo queda posicionado al inicio del siguiente registro.
    return persona;
}



