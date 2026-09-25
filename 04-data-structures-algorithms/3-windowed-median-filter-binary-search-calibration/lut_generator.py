#!/usr/bin/env python3
import scipy.constants as const
import math

def main():
    output = {}

    altitudes = [10, 6, 3, 0]

    # Temperature in Kelvin
    temp = 23 + 273.15 

    # Universal gas constant
    R = const.R 

    # Acceleration due to gravity
    g = const.g

    # Molar mas of Earth's air
    M = 0.02896

    # Standard atmosphere reference pressure at sea level
    P_sea_level = const.atm

    for altitude_val in altitudes:
        pressure = P_sea_level * math.exp(-(M*g*altitude_val)/(R*temp))
        output[pressure] = altitude_val

    print_content = ", ".join(f"{{{int(k*256)}, {v}}}" for k, v in output.items())

    print(print_content)

if __name__ == "__main__":
    main()