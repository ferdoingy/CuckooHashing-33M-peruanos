import argparse # para indicar cuantos ciudadanos se generan sin modificar el código
from genBIN import generar_dataset_bin # para generar el dataset en binario
 
if __name__ == "__main__":
 
    parser = argparse.ArgumentParser()
 
    parser.add_argument(
        "--cantidad",
        type=int,
        default=1000,
        help="Cantidad de ciudadanos a generar"
    )
 
    parser.add_argument(
        "--salida",
        type=str,
        default="setDatos/dataset.bin",
        help="Ruta del archivo binario de salida"
    )
 
    parser.add_argument(
        "--catalogos",
        type=str,
        default="setDatos/catalogos.txt",
        help="Ruta del archivo de catalogos"
    )
 
    parser.add_argument(
        "--semilla",
        type=int,
        default=42,
        help="Semilla para el generador de números aleatorios"
    )
 
    args = parser.parse_args()
 
    generar_dataset_bin(args.cantidad, args.salida, args.catalogos, args.semilla)