#include <iostream>
#include <fstream> //para abrir archivos
#include <string> //manipular cadenas de textos
#include <iomanip> //Dar un formato a lo que se imprime
#include <stdexcept> //Crear errores para explicar algun problema
#include <cstdint> //usar enteros con tamaños especificos
#include "Persona.h"

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

int main(int argc, char* argv[]) {
    // Se esperan dos argumentos:
    // argv[0]: nombre del ejecutable.
    // argv[1]: ruta del archivo binario.
    if (argc != 2) {
        cerr << "Uso: lector.exe \"ruta/al/dataset.bin\"\n";
        return 1; // Termina indicando un error.
    }

    // Permite capturar los errores lanzados durante la lectura.
    try {
        // Abre la ruta recibida como argumento.
        // ios::binary evita las transformaciones propias del modo texto.
        ifstream archivo(argv[1], ios::binary);

        // Comprueba que el archivo se haya abierto correctamente.
        if (!archivo.is_open()) {
            cerr << "No se pudo abrir el archivo.\n";
            return 1; // Termina porque no se puede continuar sin el archivo.
        }

        // Lee la cabecera y guarda el resultado.
        // const impide modificar esta variable después de inicializarla.
        const Cabecera cabecera = leerCabecera(archivo);

        // Muestra el identificador leído.
        cout << "Identificador: "
             << cabecera.identificador << '\n';

        // Muestra la versión del formato.
        cout << "Version: "
             << cabecera.version << '\n';

        // Muestra cuántas personas anuncia la cabecera.
        cout << "Cantidad de personas: "
             << cabecera.cantidad_personas << '\n';

        // El archivo está después de la cabecera.
        // Esta llamada lee solamente la primera persona.
        const Persona persona = leerPersona(archivo);

        cout << "\nPrimera persona:\n"; // Título antes de mostrar sus datos.

        // setw(8) fija un ancho mínimo de 8 caracteres para el DNI.
        // setfill('0') rellena con ceros a la izquierda si faltan dígitos.
        // Después se restaura el carácter de relleno a un espacio.
        cout << "DNI: "
             << setfill('0') << setw(8) << persona.dni
             << setfill(' ') << '\n';

        // Muestra el índice del primer nombre, no su texto.
        cout << "Indice del primer nombre: "
             << persona.nombre1 << '\n';

        // Muestra el índice del segundo nombre.
        cout << "Indice del segundo nombre: "
             << persona.nombre2 << '\n';

        // Muestra el índice del primer apellido.
        cout << "Indice del primer apellido: "
             << persona.apellido1 << '\n';

        // Muestra el índice del segundo apellido.
        cout << "Indice del segundo apellido: "
             << persona.apellido2 << '\n';

        // El teléfono ya es un valor numérico, no un índice del catálogo.
        cout << "Telefono: "
             << persona.telefono << '\n';

        // Muestra el usuario del correo leído después de los 26 bytes fijos.
        cout << "Usuario del correo: "
             << persona.usuarioCorreo << '\n';

    } catch (const exception& error) {
        // Captura una excepción lanzada dentro del bloque try.
        // Se recibe por referencia constante para evitar copiarla.
        cerr << "Error: " << error.what() << '\n';

        return 1; // Informa que el programa terminó con un error.
    }

    // El archivo se cierra automáticamente al destruirse su ifstream.
    return 0;
}


