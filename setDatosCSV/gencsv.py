from configG import *
from catalogog import *
import csv # para guardar los datos en un archivo csv

def generar_dataset(cantidad, ruta, semilla): # numero de ciudadanos que generaremos, ubicacion del csv y valor que permite repoducir los mismos datos aleatorios
    if not 1 <= cantidad <= TOTAL_DNI:
        raise ValueError(
            "La cantidad debe ser  entre 1 y 100 millones"
        )

    if math.gcd(MULTIPLICADOR, TOTAL_DNI) != 1:
        raise ValueError(
            "El multiplicador no garantiza DNI unicos"
        )

    rng = random.Random(semilla) # se genera un generador de numeros aleatorios con la semilla que se le pasa como parametro

    nombres, apellidos = crear_catalogos(semilla) #se generan los catalogos de nombres y apellidos con la funcion crear_catalogos

    print(f"Nombres disponibles: {len(nombres)}")
    print(f"Apellidos disponibles: {len(apellidos)}")

    ruta = Path(ruta) # se convierte la ruta en un objeto Path
    ruta.parent.mkdir(parents=True, exist_ok=True) # se crea la carpeta donde se guardara el csv

    columnas = [
        "dni",
        "nombre",
        "apellidos",
        "nacionalidad",
        "lugar_nacimiento",
        "departamento",
        "provincia",
        "ciudad",
        "distrito",
        "ubicacion",
        "telefono_referencia",
        "correo",
        "estado_civil"
    ]

    with ruta.open("w", newline= "", encoding="utf-8") as archivo:
        escritor = csv.writer(archivo)
        escritor.writerow(columnas)

        for i in range(cantidad):
            ciudadano = generar_ciudadano(i, rng, nombres, apellidos) #se genera un ciudadano con la funcion generar_ciudadano
            escritor.writerow(ciudadano)
            if (i + 1) % 100_000 == 0:
                print(f"Generados: {i + 1:,} / {cantidad:,}")

    print("Dataset generado")
    print(f"Archivo: {ruta}")
