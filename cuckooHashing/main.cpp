#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept> // exception, runtime_error
#include "Cache.h"
#include "CuckooHash.h"
#include "Catalogos.h"
#include <chrono>  // para medir los tiempos (el cronometro)
#include <fstream> // para guardar los resultados del benchmark en un csv y despues graficar en excel

using namespace std;

// rutas para los archivos binario y catalogo
const string RUTA_BIN = "../setDatosBIN/setDatos/dataset.bin";
const string RUTA_CATALOGOS = "../setDatosBIN/setDatos/catalogos.txt";

// en el benchmark cada operacion se repite 10000 veces y se saca el promedio
// porque si medimos una sola vez el tiempo sale muy variado
const uint32_t CANTIDAD_PRUEBAS = 10000;

// muestra el mensaje y devuelve la linea escrita
string leerLinea(const string& mensaje) {
    cout << mensaje;
    string s; // la entrada se guarda en s
    if (!getline(cin, s)) throw runtime_error("Entrada terminada"); // si esta vacia
    return s; // retorna el texto
}

// pide hasta recibir un numero entre minimo y maximo
uint32_t leerNumero(const string& mensaje, uint32_t minimo, uint32_t maximo) { // el min y max lo define el caracter o sea dni tel tipo via
    while (true) {
        string texto = leerLinea(mensaje);
        bool valido = texto.size() >= 1 && texto.size() <= 9; // 9 digitos caben en uint32_t
        for (char ch : texto) {
            if (ch < '0' || ch > '9') valido = false; // si no estan del 0 - 9 falso
        }
        if (valido) {
            uint32_t n = stoul(texto); // si es que si es valido se convierte ese string en numero con stoul
            if (n >= minimo && n <= maximo) return n; //sirve para todo numero de DNI telefono y tipo via
        }
        cout << "Valor invalido\n"; // avisa y vuelve al while
    }
}

// pide un texto del catalogo; si no existe ofrece agregarlo
uint32_t leerDeCatalogo(const string& mensaje, vector<string>& lista, size_t maximo, Catalogos& c) { // recibe la lista para modificar el catalogo y c el catalogo completo
    while (true) { // c el catalogo completo que necsita para guardar y reescribir el catalogo
        string texto = leerLinea(mensaje); // lee el texto
        int i = buscarEnCatalogo(lista, texto); // lo busca en el catalogo y guarda la posicion en i
        if (i >= 0) return i; // si es mayor igual a 0 es decir que lo encontro entonces devuelve ese elemento del catalogo

        string respuesta = leerLinea("No existe en el catalogo. Agregarlo? (s/n): "); // si no pregunta si quiere agregarlo
        if (respuesta == "s" && lista.size() < maximo) // si es si y el tamaño de la lista < que el limite del tipo de dato
        {
            lista.push_back(texto);
            guardarCatalogos(c, RUTA_CATALOGOS); // guarda el catalogo con el nuevo valor
            return lista.size() - 1;
        }
        if (lista.size() >= maximo) cout << "El catalogo esta lleno\n";
    }
}

// muestra la lista numerada y devuelve el indice elegido (0 agrega uno nuevo)
uint32_t elegirDeLista(const string& titulo, vector<string>& lista, bool permiteAgregar, size_t maximo, Catalogos& c, bool esUbicacion) {
    cout << titulo << "\n"; // el titulo es el encabezado
    for (size_t i = 0; i < lista.size(); i++) { // recorre y muestra
        string valor;
        for (char ch : lista[i]) {
            if (ch == '\t') valor += " / "; // TAB se muestra como " / " -> Lima/Lima/Lima/Miraflores
            else valor += ch;
        }
        cout << "  " << i + 1 << ") " << valor << "\n"; // muestra desde el 1
    }
    if (permiteAgregar && lista.size() < maximo) cout << "  0) Agregar nuevo\n"; // si la lista no esta llena entonces se permite agregar nuevo

    uint32_t opcion = leerNumero("Opcion: ", (permiteAgregar && lista.size() < maximo) ? 0 : 1, lista.size()); // 0 solo si se puede agregar
    if (opcion > 0) return opcion - 1;

    string nuevo; // guarda lo que lee en departamento, provincia, ciudad, distrito
    if (esUbicacion) { // departamento\tprovincia\tciudad\tdistrito los separa con TAB para tener el mismo formato que en el catalogo
        nuevo = leerLinea("Departamento: ");
        nuevo += '\t' + leerLinea("Provincia: ");
        nuevo += '\t' + leerLinea("Ciudad: ");
        nuevo += '\t' + leerLinea("Distrito: ");
    } else {
        nuevo = leerLinea("Nuevo valor: "); // si no pide una calle nueva por ejemplo
    }
    lista.push_back(nuevo);
    guardarCatalogos(c, RUTA_CATALOGOS); // guarda el catalogo con el nuevo valor
    return lista.size() - 1; // devuelve el ultimo indice
}

// pide los datos de una persona nueva, la guarda en el .bin y en el cuckoo
void registrar(Cache& cache, CuckooHash& tabla, Catalogos& c) {
    uint32_t dni = leerNumero("DNI: ", 1, 99999999);
    if (tabla.buscar(dni) != VACIO) {
        cout << "Ya existe una persona con ese DNI\n";
        return;
    }

    Persona p;
    p.dni = dni;
    p.nombre1   = leerDeCatalogo("Primer nombre: ", c.nombres, 65535, c);
    p.nombre2   = leerDeCatalogo("Segundo nombre: ", c.nombres, 65535, c);
    p.apellido1 = leerDeCatalogo("Apellido paterno: ", c.apellidos, 65535, c);
    p.apellido2 = leerDeCatalogo("Apellido materno: ", c.apellidos, 65535, c);
    p.nacionalidad = 0; // Peruana

    p.lugarNacimiento = elegirDeLista("Lugar de nacimiento (nuevo: Ciudad, Departamento):", c.lugaresNacimiento, true, 255, c, false);
    p.ubicacion = elegirDeLista("Ubicacion:", c.ubicaciones, true, 255, c, true);
    p.tipoVia   = elegirDeLista("Tipo de via:", c.tiposVia, false, 255, c, false);
    p.nombreVia = elegirDeLista("Nombre de via:", c.nombresVia, true, 255, c, false);

    p.numeroVia = leerNumero("Numero de via (1-9999): ", 1, 9999);
    p.telefono  = leerNumero("Telefono (9 digitos, empieza en 9): ", 900000000, 999999999);
    p.estadoCivil = elegirDeLista("Estado civil:", c.estadosCiviles, false, 255, c, false);
    p.dominioCorreo = 0; // example.com

    // correo: nombres sin tildes ni espacios + dni de 8 digitos
    string d = to_string(dni);
    d = string(8 - d.size(), '0') + d; // 8 digitos
    string nombres = sinTildes(c.nombres[p.nombre1] + c.nombres[p.nombre2]);
    string sinEspacios;
    for (char ch : nombres) {
        if (ch != ' ') sinEspacios += ch;
    }
    p.usuarioCorreo = sinEspacios + d;

    // el cronometro empieza recien aca, cuando ya se escribieron todos los datos
    // si lo ponemos arriba estariamos midiendo lo que se demora el usuario en tipear
    auto inicioRegistrar = chrono::high_resolution_clock::now();
    uint32_t idx = cache.agregar(p); // escribe al final del .bin
    tabla.insertar(p.dni, idx);      // {dni, idx} en el cuckoo
    auto finRegistrar = chrono::high_resolution_clock::now();

    cout << "Persona registrada\n";
    // fin - inicio es lo que demoro, lo pasamos a microsegundos (us) para que se lea mejor
    cout << "Tiempo de registro: " << chrono::duration_cast<chrono::microseconds>(finRegistrar - inicioRegistrar).count() << " us\n";
    mostrarPersona(p, c); // esto va despues de medir porque imprimir en consola es lento
}

// borra a la persona del .bin y del cuckoo
void eliminar(Cache& cache, CuckooHash& tabla) {
    uint32_t dni = leerNumero("DNI a eliminar: ", 1, 99999999); // lee el numero

    // igual que en registrar, se empieza a medir despues de que el usuario escribio el dni
    // se cuenta la busqueda + el borrado en el disco + el borrado en el cuckoo
    auto inicioEliminar = chrono::high_resolution_clock::now();
    uint32_t idx = tabla.buscar(dni); // lo busca
    if (idx == VACIO) { // si no hay
        cout << "No existe registro con ese DNI\n";
        return; // retorna
    }
    cache.marcarBorrado(idx); // primero en el disco
    tabla.eliminar(dni);      // despues en el cuckoo
    auto finEliminar = chrono::high_resolution_clock::now();

    cout << "Persona eliminada\n";
    cout << "Tiempo de eliminacion: " << chrono::duration_cast<chrono::microseconds>(finEliminar - inicioEliminar).count() << " us\n";
}

// funcion de ayuda para no repetir lo mismo varias veces:
// agarra el tiempo total de un monton de operaciones y lo divide entre cuantas fueron
// asi sale cuanto demoro cada una en promedio (en nanosegundos, porque son muy rapidas)
double promedioNs(chrono::high_resolution_clock::time_point inicio, chrono::high_resolution_clock::time_point fin, uint32_t cantidad) {
    return chrono::duration_cast<chrono::nanoseconds>(fin - inicio).count() / (double)cantidad;
}

// hace el benchmark para un tamaño n (por ej 1 millon de personas)
// la idea es armar un cuckoo aparte solo para la prueba, asi no malogramos
// la tabla principal ni escribimos nada en el .bin
void medirConTamano(const vector<uint32_t>& dnis, uint32_t n, ofstream& archivo) {
    // le damos espacio de mas (n + 10000) porque despues vamos a insertar 10000 personas de prueba
    CuckooHash prueba(n + CANTIDAD_PRUEBAS);

    // 1) construccion: metemos las primeras n personas a la tabla
    // esto viene a ser la insercion de todo el lote de golpe
    auto inicio = chrono::high_resolution_clock::now();
    for (uint32_t i = 0; i < n; i++) {
        if (dnis[i] != DNI_BORRADO) prueba.insertar(dnis[i], i); // las borradas no van
    }
    auto fin = chrono::high_resolution_clock::now();
    long long construccionMs = chrono::duration_cast<chrono::milliseconds>(fin - inicio).count();

    // ojo: desde aca hasta antes del paso 2 NO se mide nada, solo preparamos los dnis de prueba

    // dnis que si existen: no agarramos los primeros 10000 nomas, sino que saltamos de "paso" en "paso"
    // para que esten repartidos en toda la tabla (ej: con 1 millon el paso es 100 -> dni 0, 100, 200...)
    vector<uint32_t> existentes;
    uint32_t paso = n / CANTIDAD_PRUEBAS;
    if (paso == 0) paso = 1; // por si n es menor que 10000 y la division da 0
    for (uint32_t i = 0; i < n && existentes.size() < CANTIDAD_PRUEBAS; i += paso) {
        if (dnis[i] != DNI_BORRADO) existentes.push_back(dnis[i]);
    }

    // dnis que no existen: empezamos desde el dni mas alto (99999999) y vamos bajando
    // solo guardamos los que el cuckoo dice que no estan
    // estos mismos se usan despues para probar insertar y eliminar
    vector<uint32_t> noExistentes;
    for (uint32_t d = 99999999; noExistentes.size() < CANTIDAD_PRUEBAS; d--) {
        if (prueba.buscar(d) == VACIO) noExistentes.push_back(d);
    }

    // 2) busqueda cuando el dni SI esta
    // contamos los encontrados para comprobar que la busqueda funciona (deberian ser todos)
    uint32_t encontrados = 0;
    inicio = chrono::high_resolution_clock::now();
    for (uint32_t i = 0; i < existentes.size(); i++) {
        if (prueba.buscar(existentes[i]) != VACIO) encontrados++;
    }
    fin = chrono::high_resolution_clock::now();
    double busquedaSi = promedioNs(inicio, fin, existentes.size());

    // 3) busqueda cuando el dni NO esta
    // se mide aparte porque el cuckoo tiene que revisar las dos tablas antes de decir que no hay
    uint32_t noEncontrados = 0;
    inicio = chrono::high_resolution_clock::now();
    for (uint32_t i = 0; i < noExistentes.size(); i++) {
        if (prueba.buscar(noExistentes[i]) == VACIO) noEncontrados++;
    }
    fin = chrono::high_resolution_clock::now();
    double busquedaNo = promedioNs(inicio, fin, noExistentes.size());

    // 4) insercion de una en una de las 10000 personas nuevas
    // como posicion le ponemos n + i, total es una prueba y la tabla se borra al final
    inicio = chrono::high_resolution_clock::now();
    for (uint32_t i = 0; i < noExistentes.size(); i++) {
        prueba.insertar(noExistentes[i], n + i);
    }
    fin = chrono::high_resolution_clock::now();
    double insercion = promedioNs(inicio, fin, noExistentes.size());

    // 5) eliminacion de esas mismas 10000 que acabamos de meter
    inicio = chrono::high_resolution_clock::now();
    for (uint32_t i = 0; i < noExistentes.size(); i++) {
        prueba.eliminar(noExistentes[i]);
    }
    fin = chrono::high_resolution_clock::now();
    double eliminacion = promedioNs(inicio, fin, noExistentes.size());

    // mostramos todo en pantalla
    cout << "\n--- " << n << " personas ---\n";
    cout << "Construccion (insertar las " << n << "): " << construccionMs << " ms\n";
    cout << "Busqueda (existe):    " << busquedaSi  << " ns promedio (" << encontrados << "/" << existentes.size() << " encontrados)\n";
    cout << "Busqueda (no existe): " << busquedaNo  << " ns promedio (" << noEncontrados << "/" << noExistentes.size() << " no encontrados)\n";
    cout << "Insercion:            " << insercion   << " ns promedio\n";
    cout << "Eliminacion:          " << eliminacion << " ns promedio\n";

    // y guardamos una fila en el csv, separada por comas, para las graficas
    archivo << n << "," << construccionMs << "," << busquedaSi << "," << busquedaNo << "," << insercion << "," << eliminacion << "\n";

    // cuando se acaba la funcion la tabla "prueba" se destruye sola y libera la memoria
}

// corre el benchmark con varios tamaños
// asi en la grafica se ve como cambia (o no cambia) el tiempo cuando hay mas personas
// si la linea sale casi plana es porque la busqueda es O(1)
void benchmark(const vector<uint32_t>& dnis) {
    const uint32_t tamanos[] = {1000000, 5000000, 10000000, 20000000, 33000000};

    // se crea el csv con la primera fila que son los titulos de las columnas
    ofstream archivo("benchmark.csv");
    archivo << "personas,construccion_ms,busqueda_existe_ns,busqueda_no_existe_ns,insercion_ns,eliminacion_ns\n";

    for (uint32_t n : tamanos) {
        if (n > dnis.size()) n = dnis.size(); // si el dataset tiene menos personas usamos las que haya
        medirConTamano(dnis, n, archivo);
        if (n == dnis.size()) break; // ya llegamos al total, no tiene sentido seguir
    }
    cout << "\nResultados guardados en benchmark.csv\n";
}

int main() {
    try {
        Cache cache;

        // cuanto se demora en recuperar todo lo que esta guardado en el disco
        // (la guia pide que cada vez que se abre el programa se cargue lo de la ultima vez)
        auto inicioCarga = chrono::high_resolution_clock::now();
        vector<uint32_t> dnis = cache.cargar(RUTA_BIN); // dnis[i] = DNI de la persona i
        auto finCarga = chrono::high_resolution_clock::now();
        cout << "Personas en la cache: " << cache.size() << "\n";
        // aca usamos milisegundos (ms) porque cargar millones de personas si demora
        cout << "Tiempo de carga del disco: " << chrono::duration_cast<chrono::milliseconds>(finCarga - inicioCarga).count() << " ms\n";

        // cuanto se demora en meter a todas las personas al cuckoo
        auto inicioCuckoo = chrono::high_resolution_clock::now();
        CuckooHash tabla(cache.size());
        for (uint32_t i = 0; i < cache.size(); i++) {
            if (dnis[i] != DNI_BORRADO) tabla.insertar(dnis[i], i); // las marcadas no se insertan
        }
        auto finCuckoo = chrono::high_resolution_clock::now();
        cout << "Personas insertadas en el cuckoo\n";
        cout << "Tiempo de construccion del cuckoo: " << chrono::duration_cast<chrono::milliseconds>(finCuckoo - inicioCuckoo).count() << " ms\n";

        Catalogos catalogos = leerCatalogos(RUTA_CATALOGOS);

        while (true) {
            string opcion = leerLinea("\n1) Buscar\n2) Registrar\n3) Eliminar\n4) Benchmark\n0) Salir\nOpcion: ");
            if (opcion == "1") {
                uint32_t dni = leerNumero("DNI a buscar: ", 1, 99999999);

                // la busqueda tiene 2 partes y las medimos por separado:
                // primero el cuckoo nos dice en que posicion esta la persona (esto es en memoria, rapidisimo)
                auto inicioBuscar = chrono::high_resolution_clock::now();
                uint32_t idx = tabla.buscar(dni); // posicion en la cache o VACIO
                auto finBuscar = chrono::high_resolution_clock::now();

                if (idx == VACIO) {
                    cout << "No existe registro con ese DNI\n";
                } else {
                    // y despues vamos al .bin a traer los datos de esa persona (esto ya es disco, mas lento)
                    auto inicioLeer = chrono::high_resolution_clock::now();
                    Persona p = cache.leer(idx);
                    auto finLeer = chrono::high_resolution_clock::now();

                    mostrarPersona(p, catalogos); // se muestra despues de medir para no contar el cout
                    cout << "Tiempo de lectura del disco: " << chrono::duration_cast<chrono::microseconds>(finLeer - inicioLeer).count() << " us\n";
                }
                // la del cuckoo va en nanosegundos (ns) porque en microsegundos saldria 0
                cout << "Tiempo de busqueda en cuckoo: " << chrono::duration_cast<chrono::nanoseconds>(finBuscar - inicioBuscar).count() << " ns\n";
            } else if (opcion == "2") {
                registrar(cache, tabla, catalogos);
            } else if (opcion == "3") {
                eliminar(cache, tabla);
            } else if (opcion == "4") {
                benchmark(dnis); // prueba automatica para el informe (puede demorar un rato)
            } else if (opcion == "0") {
                break;
            } else {
                cout << "Opcion invalida\n";
            }
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
