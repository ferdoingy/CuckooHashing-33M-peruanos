import argparse # para indicar cuantos ciudadanos se generan sin modificar el código
import csv # para guardar los datos en un archivo csv
import math # para las operaciones
import random # para seleccionar aleatoriamentede las listas
from pathlib import Path # para crear carpetas donde se guarda
from faker import Faker

# Para generar DNIs unicos 
TOTAL_DNI = 100_000_000 # para los 100 millones de combinaciones posibles para 8 dig del DNI

while True:
    MULTIPLICADOR = random.randint(1, 99_999_999) # para generar un número aleatorio entre 1 y 99 millones

    if math.gcd(MULTIPLICADOR, TOTAL_DNI) == 1: # si el máximo común divisor es 1, entonces son coprimos
        break # garantiza la unicidad de los DNIs

# Datos de Perú
NOMBRES_PERUANOS = [
    "José", "Luis", "Juan", "Carlos", "Miguel",
    "Fernando", "Diego", "Alejandro", "Jorge",
    "Pedro", "Ricardo", "Andrés", "Sebastián",
    "María", "Rosa", "Ana", "Lucía", "Valeria",
    "Camila", "Daniela", "Milagros", "Carmen",
    "Andrea", "Fiorella", "Gabriela", "Paola"
]

APELLIDOS_PERUANOS = [
    "Quispe", "Flores", "Sánchez", "García",
    "Rodríguez", "Huamán", "Mamani", "Condori",
    "Chávez", "Rojas", "Torres", "Vargas",
    "Mendoza", "Salazar", "Castillo", "Paredes",
    "Gutiérrez", "Ramírez", "Espinoza", "Díaz",
    "Fernández", "Medina", "Cáceres", "Vásquez",
    "Huanca", "Apaza", "Choque", "Yupanqui"
]

NACIONALIDADES = [
    "Peruana"
]

ESTADOS_CIVILES = [
    "Soltero/a",
    "Casado/a",
    "Divorciado/a",
    "Viudo/a"
]

# Dirección
UBICACIONES = [
    ("Lima", "Lima", "Lima", "Miraflores"),
    ("Lima", "Lima", "Lima", "San Isidro"),
    ("Lima", "Lima", "Lima", "Santiago de Surco"),
    ("Lima", "Lima", "Lima", "San Miguel"),
    ("Arequipa", "Arequipa", "Arequipa", "Cayma"),
    ("Arequipa", "Arequipa", "Arequipa", "Cerro Colorado"),
    ("Cusco", "Cusco", "Cusco", "Wanchaq"),
    ("Cusco", "Cusco", "Cusco", "San Sebastián"),
    ("Piura", "Piura", "Piura", "Castilla"),
    ("Piura", "Piura", "Piura", "Veintiséis de Octubre"),
    ("La Libertad", "Trujillo", "Trujillo", "Trujillo"),
    ("Lambayeque", "Chiclayo", "Chiclayo", "La Victoria"),
    ("Junín", "Huancayo", "Huancayo", "El Tambo"),
    ("Puno", "Puno", "Puno", "Puno"),
    ("Loreto", "Maynas", "Iquitos", "Iquitos")
]

TIPOS_VIA = [
    "Av.", "Jr.", "Calle", "Pasaje"
]

