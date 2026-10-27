import argparse # para indicar cuantos ciudadanos se generan sin modificar el código
from gencsv import generar_dataset # para generar el dataset

if __name__ == "__main__":

    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--cantidad",
        type=int,
        default=33000000,
        help="Cantidad de ciudadanos a generar"
    )

    parser.add_argument(
        "--salida",
        type=str,
        default="setDatos/dataset.csv",
        help="Ruta del archivo CSV de salida"
    )

    parser.add_argument(
        "--semilla",
        type=int,
        default=42,
        help="Semilla para el generador de números aleatorios"
    )

    args = parser.parse_args()

    generar_dataset(args.cantidad, args.salida, args.semilla)