from scipy.stats import genextreme
import numpy as np

data = {
    103: 1,
    104: 2,
    105: 74,
    106: 619,
    107: 2385,
    108: 5261,
    109: 8191,
    110: 10057,
    111: 9809,
    112: 8346,
    113: 6615,
    114: 4782,
    115: 3288,
    116: 2190,
    117: 1454,
    118: 960,
    119: 554,
    120: 393,
    121: 229,
    122: 130,
    123: 77,
    124: 48,
    125: 29,
    126: 18,
    127: 10,
    128: 7,
    129: 3,
    130: 1,
    131: 3,
}

samples = []
for max_bucket, frequency in data.items():
    samples.extend([max_bucket] * frequency)

samples = np.array(samples)

params = genextreme.fit(samples)

print(f"GEV parameters: Shape={params[0]}, Location={params[1]}, Scale={params[2]}")

p = 1 - 2 ** -40
threshold = genextreme.ppf(p, *params)
print(f"Threshold for probability {p}: {threshold}")
