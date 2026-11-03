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
uint64_t convertirLittleEndien(
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

struct cabecera{
    string identificador;
    uint32_t version;
    uint64_t cantidad_personas;
};

struct Persona{
    uint32_t dni;
    uint16_t nombre1;
    uint16_t nombre1;
    uint16_t apellido1;
    uint16_t apellido2;

    uint8_t nacionalidad;
    uint8_t lugarNacimiento;
    uint8_t ubicacion;
    uint8_t tipoVia;
    uint8_t nombreVia;

    uint16_t numeroVia;
    uint32_t telefono;

    uint8_t especialCivil;
    uint8_t dominioCorreo;

    string usuarioCorreo;
};

