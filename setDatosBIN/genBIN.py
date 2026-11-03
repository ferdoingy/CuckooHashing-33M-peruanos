from catalogog import *

TAM_BUFFER = 8 * 1024 * 1024 # se escribe al disco cada 8 mb aprox

def generar_dataset_bin(cantidad, ruta_bin, ruta_catalogos, semilla):
    if not 1 <= cantidad <= TOTAL_DNI:
        raise ValueError(f"cantidad debe estar entre 1 y {TOTAL_DNI}")

    validar_catalogos()

    rng = random.Random(semilla) # misma semilla mismo datos
    multiplicador = calcular_multiplicador(semilla)

    ruta_bin, ruta_catalogos = Path(ruta_bin), Path(ruta_catalogos)
    