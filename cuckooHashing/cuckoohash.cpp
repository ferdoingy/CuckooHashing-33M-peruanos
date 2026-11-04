#include <iostream>
#include <vector>
#include <cstdint>   // enteros de tamaño fijo (uint32_t, uint64_t)
#include <algorithm> // swap, max
#include <random>    // generador para las semillas del rehash
#include <chrono>    // medir tiempos
#include <cmath>     // ceil
#include <cstdlib>   // strtoul: leer la cantidad desde la línea de comandos

using namespace std;

// Cada casilla guarda una pareja: el DNI (la clave) y la posición de la
// persona dentro del vector<Persona> (idx). Así la tabla solo ocupa 8 bytes
// por casilla y los datos completos de la persona se quedan en el vector.
struct Entrada {
    uint32_t dni;
    uint32_t idx;
};

// Marca de casilla vacía. No se usa el DNI para marcar vacío porque el
// DNI 0 (00000000) puede existir; en cambio, ningún idx válido llega a
// UINT32_MAX (habría que tener más de 4 mil millones de personas).
const uint32_t VACIO = UINT32_MAX;

class cuckooHashTable {
    static const int MAX_DESPLAZAMIENTOS = 500; // vueltas del bucle antes de rendirse
    static const int MAX_INTENTOS_REHASH = 5;   // rehashes seguidos antes de duplicar
    static constexpr double CARGA_MAXIMA = 0.45; // carga máxima entre las dos tablas

    size_t capacidad;         // casillas de CADA tabla (las dos tienen el mismo tamaño)
    vector<Entrada> table1;
    vector<Entrada> table2;

    uint32_t semilla1;        // semilla de hash1
    uint32_t semilla2;        // semilla de hash2 (distinta a la de hash1)

    size_t cantidad = 0;      // parejas guardadas
    size_t rehashes = 0;      // rehashes por ciclo (nuevas semillas, misma capacidad)
    size_t crecimientos = 0;  // veces que se duplicó la capacidad

    mt19937 generador{12345}; // semilla fija: las pruebas se pueden repetir

    // Mezcla final de MurmurHash3 (fmix32). Se trabaja con uint32_t SIN signo:
    // así los desplazamientos >> meten ceros y la multiplicación da la vuelta
    // (módulo 2^32) sin comportamiento indefinido.
    static uint32_t mezclar(uint32_t x) {
        x ^= x >> 16;
        x *= 0x85ebca6bu;
        x ^= x >> 13;
        x *= 0xc2b2ae35u;
        x ^= x >> 16;
        return x;
    }

    // Las dos funciones usan la misma mezcla, pero con semillas distintas.
    // dni ^ semilla cambia los bits de entrada, así h1 y h2 son independientes.
    size_t hash1(uint32_t dni) const {
        return mezclar(dni ^ semilla1) % capacidad;
    }

    size_t hash2(uint32_t dni) const {
        return mezclar(dni ^ semilla2) % capacidad;
    }

    // Elige semillas nuevas (y distintas entre sí) para el rehash.
    void nuevasSemillas() {
        semilla1 = generador();
        do {
            semilla2 = generador();
        } while (semilla2 == semilla1);
    }

    // Es el mismo bucle de desplazamientos de clase, pero moviendo la pareja
    // completa. Devuelve true si la pareja quedó ubicada.
    // Si devuelve false, "entrada" contiene la pareja que quedó FUERA de las
    // tablas (puede ser otra distinta a la que llegó), para no perderla.
    bool colocar(Entrada& entrada) {
        size_t pos1 = hash1(entrada.dni);
        if (table1[pos1].idx == VACIO) {
            table1[pos1] = entrada;
            return true;
        }
        // si no está vacía se desplaza la pareja existente
        swap(entrada, table1[pos1]); // prevalece la pareja que llega

        for (int i = 0; i < MAX_DESPLAZAMIENTOS; i++) {
            // la pareja desplazada intenta ir a su casilla de la tabla 2
            size_t pos2 = hash2(entrada.dni);
            if (table2[pos2].idx == VACIO) {
                table2[pos2] = entrada;
                return true;
            }
            // si no está vacía, se intercambian y la que sale vuelve a la tabla 1
            swap(entrada, table2[pos2]);

            pos1 = hash1(entrada.dni);
            if (table1[pos1].idx == VACIO) {
                table1[pos1] = entrada;
                return true;
            }
            swap(entrada, table1[pos1]);
        }
        // Se superó el límite: probablemente hay un ciclo. "entrada" queda pendiente.
        return false;
    }

    // Vuelve a construir las tablas con "nuevaCapacidad" y semillas nuevas,
    // reinsertando todas las parejas guardadas y, si hay, la pendiente.
    // Si falla MAX_INTENTOS_REHASH veces seguidas, duplica la capacidad.
    void reconstruir(size_t nuevaCapacidad, const Entrada* pendiente) {
        // 1. Se copian todas las parejas a una lista temporal.
        vector<Entrada> todas;
        todas.reserve(cantidad + 1);
        for (const Entrada& e : table1) if (e.idx != VACIO) todas.push_back(e);
        for (const Entrada& e : table2) if (e.idx != VACIO) todas.push_back(e);
        if (pendiente != nullptr) todas.push_back(*pendiente);

        int fallosSeguidos = 0;
        while (true) {
            // 2. Tablas vacías con la capacidad pedida y funciones hash nuevas.
            capacidad = nuevaCapacidad;
            table1.assign(capacidad, Entrada{0, VACIO});
            table2.assign(capacidad, Entrada{0, VACIO});
            nuevasSemillas();

            // 3. Se reinsertan todas las parejas.
            bool exito = true;
            for (const Entrada& e : todas) {
                Entrada copia = e;
                if (!colocar(copia)) {
                    exito = false;
                    break;
                }
            }
            if (exito) {
                return; // todas (incluida la pendiente) quedaron ubicadas
            }

            // 4. Falló: se cuenta un rehash más y se vuelve a intentar.
            //    Si falla varias veces seguidas, la tabla está muy apretada y se duplica.
            rehashes++;
            fallosSeguidos++;
            if (fallosSeguidos >= MAX_INTENTOS_REHASH) {
                nuevaCapacidad *= 2;
                crecimientos++;
                fallosSeguidos = 0;
            }
        }
    }

public:
    // Recibe cuántas personas se van a guardar (n) y reserva desde el inicio
    // n / 0.9 casillas por tabla: entre las dos tablas hay n / 0.45 casillas,
    // así con n personas la carga queda en 45% y no hace falta crecer.
    explicit cuckooHashTable(size_t n) {
        capacidad = max<size_t>(1, static_cast<size_t>(ceil(n / 0.9)));
        // Se llenan las tablas como vacías (como el fill con -1 de clase).
        table1.assign(capacidad, Entrada{0, VACIO});
        table2.assign(capacidad, Entrada{0, VACIO});
        nuevasSemillas();
    }

    // Muestra las dos tablas. Solo para tablas pequeñas.
    void display() const {
        if (capacidad > 50) {
            cout << "display(): la tabla tiene " << capacidad
                 << " casillas por tabla; solo se muestra con 50 o menos.\n";
            return;
        }
        cout << "TABLA 1" << endl;
        for (const Entrada& e : table1) {
            if (e.idx == VACIO) cout << "- ";
            else cout << e.dni << ":" << e.idx << " ";
        }
        cout << endl;

        cout << "TABLA 2" << endl;
        for (const Entrada& e : table2) {
            if (e.idx == VACIO) cout << "- ";
            else cout << e.dni << ":" << e.idx << " ";
        }
        cout << endl;
    }

    // Devuelve el idx de la persona o VACIO (UINT32_MAX) si no existe.
    // Revisa como máximo UNA casilla en cada tabla: búsqueda O(1) en el peor caso.
    uint32_t search(uint32_t dni) const {
        const Entrada& e1 = table1[hash1(dni)];
        if (e1.idx != VACIO && e1.dni == dni) {
            return e1.idx;
        }
        const Entrada& e2 = table2[hash2(dni)];
        if (e2.idx != VACIO && e2.dni == dni) {
            return e2.idx;
        }
        return VACIO;
    }

    // Inserta la pareja (dni, idx). Devuelve false si el DNI ya existía.
    bool insert(uint32_t dni, uint32_t idx) {
        // 1. No se permiten DNIs repetidos.
        if (search(dni) != VACIO) {
            return false;
        }

        // 2. Si con esta pareja la carga pasaría del 45%, se duplica la capacidad.
        if (cantidad + 1 > CARGA_MAXIMA * 2.0 * capacidad) {
            crecimientos++;
            reconstruir(capacidad * 2, nullptr);
        }

        // 3. Bucle de desplazamientos. Si falla, "pendiente" es la pareja que
        //    quedó fuera y se reinserta junto con todas en el rehash.
        Entrada pendiente{dni, idx};
        if (!colocar(pendiente)) {
            rehashes++;
            reconstruir(capacidad, &pendiente); // nuevas semillas, misma capacidad
        }

        cantidad++;
        return true;
    }

    size_t size() const { return cantidad; }
    size_t getCapacidad() const { return capacidad; }
    double factorCarga() const { return static_cast<double>(cantidad) / (2.0 * capacidad); }
    size_t getRehashes() const { return rehashes; }
    size_t getCrecimientos() const { return crecimientos; }
};

// Segundos transcurridos desde "inicio".
static double segundosDesde(chrono::steady_clock::time_point inicio) {
    return chrono::duration<double>(chrono::steady_clock::now() - inicio).count();
}

// Prueba con N DNIs únicos generados con (a*i + b) % 10^8.
// Como a es coprimo con 10^8, la fórmula no repite DNIs para
// i = 0 .. 10^8 - 1. Los i a partir de N dan DNIs que NO están en la tabla.
// Uso: cuckoo.exe            -> 33 000 000 DNIs
//      cuckoo.exe 1000000    -> la cantidad indicada (máximo 99 000 000)
int main(int argc, char* argv[]) {
    const uint64_t TOTAL_DNI = 100000000ULL;
    const uint64_t A = 48271;    // impar y no termina en 0 ni 5: coprimo con 10^8
    const uint64_t B = 12345678;
    const uint32_t NO_EXISTEN = 1000000;

    uint32_t N = 33000000;
    if (argc == 2) {
        const unsigned long valor = strtoul(argv[1], nullptr, 10);
        // Deben quedar 1 millón de DNIs libres para probar los inexistentes.
        if (valor == 0 || valor > TOTAL_DNI - NO_EXISTEN) {
            cerr << "Cantidad invalida. Debe estar entre 1 y "
                 << TOTAL_DNI - NO_EXISTEN << ".\n";
            return 1;
        }
        N = static_cast<uint32_t>(valor);
    }

    {
        cout << "=== Cuckoo hashing con " << N << " DNIs ===\n";
        auto generarDni = [&](uint64_t i) {
            return static_cast<uint32_t>((A * i + B) % TOTAL_DNI);
        };

        auto inicio = chrono::steady_clock::now();
        cuckooHashTable table(N);
        cout << "Capacidad por tabla: " << table.getCapacidad()
             << " (creacion: " << segundosDesde(inicio) << " s)\n";

        // Inserción: idx = i, como si fuera la posición en el vector<Persona>.
        inicio = chrono::steady_clock::now();
        size_t rechazados = 0;
        for (uint32_t i = 0; i < N; i++) {
            if (!table.insert(generarDni(i), i)) rechazados++;
        }
        const double tInsercion = segundosDesde(inicio);
        cout << "Insercion de " << N << " DNIs: " << tInsercion << " s"
             << " (rechazados: " << rechazados << ")\n";

        // Búsqueda de todos los que existen: cada uno debe devolver su idx.
        inicio = chrono::steady_clock::now();
        size_t correctos = 0;
        for (uint32_t i = 0; i < N; i++) {
            if (table.search(generarDni(i)) == i) correctos++;
        }
        const double tExisten = segundosDesde(inicio);
        cout << "Busqueda de " << N << " existentes: " << tExisten << " s"
             << " | correctos: " << correctos << "/" << N << "\n";

        // Búsqueda de 1 millón que NO existen: ninguno debe encontrarse.
        inicio = chrono::steady_clock::now();
        size_t falsosPositivos = 0;
        for (uint64_t i = N; i < static_cast<uint64_t>(N) + NO_EXISTEN; i++) {
            if (table.search(generarDni(i)) != VACIO) falsosPositivos++;
        }
        const double tNoExisten = segundosDesde(inicio);
        cout << "Busqueda de " << NO_EXISTEN << " inexistentes: " << tNoExisten << " s"
             << " | encontrados por error: " << falsosPositivos << "\n";

        cout << "Carga final: " << table.factorCarga() * 100 << "%"
             << " | crecimientos: " << table.getCrecimientos()
             << " | rehashes por ciclo: " << table.getRehashes() << "\n";
    }

    return 0;
}
